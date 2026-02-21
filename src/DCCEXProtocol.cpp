/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
 *
 * Copyright © 2024 Peter Cole
 * Copyright © 2023 Peter Akers
 * Copyright © 2023 Peter Cole
 *
 * This work is licensed under the Creative Commons Attribution-ShareAlike
 * 4.0 International License. To view a copy of this license, visit
 * http://creativecommons.org/licenses/by-sa/4.0/ or send a letter to
 * Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
 *
 * Attribution — You must give appropriate credit, provide a link to the
 * license, and indicate if changes were made. You may do so in any
 * reasonable manner, but not in any way that suggests the licensor
 * endorses you or your use.
 *
 * ShareAlike — If you remix, transform, or build upon the material, you
 * must distribute your contributions under the same license as the
 * original.
 *
 * All other rights reserved.
 *
 */

/*
Conventions

Function/method prefixes
- received... = notificaion to the client app (_delegate)
- send... = sends a command to the CS
- process... - process a response from the CS
- init... - initialise an object
- set... = Sets internal variables/values. May subsequently call a 'send'
- get... - Gets internal variables/values. May subsequently call a 'send'
*/

#include "DCCEXProtocol.h"

static const int MIN_SPEED = 0;
static const int MAX_SPEED = 126;

// DCCEXProtocol class
// Public methods
// Protocol and server methods

DCCEXProtocol::DCCEXProtocol(int maxCmdBuffer, int maxCommandParams, unsigned long userChangeDelay) {
  // Init streams
  _stream = &_nullStream;
  _console = &_nullStream;

  // Allocate memory for command buffer
  _cmdBuffer = new char[maxCmdBuffer];
  _maxCmdBuffer = maxCmdBuffer;

  // Setup command parser
  DCCEXInbound::setup(maxCommandParams);
  _cmdBuffer[0] = 0;
  _bufflen = 0;

  // Set user change delay
  _userChangeDelay = userChangeDelay;
  _lastUserChange = 0;

  // Set heartbeat defaults
  _enableHeartbeat = 0;
  _heartbeatDelay = 0;
  _lastHeartbeat = 0;
}

DCCEXProtocol::~DCCEXProtocol() {
  // Clean up all lists
  clearAllLists();

  // Free memory for command buffer
  delete[] (_cmdBuffer);

  // Cleanup command parser
  DCCEXInbound::cleanup();
}

// Set the delegate instance for callbacks
void DCCEXProtocol::setDelegate(DCCEXProtocolDelegate *delegate) { this->_delegate = delegate; }

// Set the Stream used for logging
void DCCEXProtocol::setLogStream(Stream *console) { this->_console = console; }

void DCCEXProtocol::enableHeartbeat(unsigned long heartbeatDelay) {
  _enableHeartbeat = true;
  _heartbeatDelay = heartbeatDelay;
}

void DCCEXProtocol::connect(Stream *stream) {
  _init();
  this->_stream = stream;
}

void DCCEXProtocol::disconnect() { return; }

void DCCEXProtocol::check() {
  if (_stream) {
    while (_stream->available()) {
      // Read from our stream
      int r = _stream->read();
      if (_bufflen < _maxCmdBuffer - 1) {
        _cmdBuffer[_bufflen] = r;
        _bufflen++;
        _cmdBuffer[_bufflen] = 0;
      } else {
        // Clear buffer if full
        _cmdBuffer[0] = 0;
        _bufflen = 0;
      }

      if (r == '>') {
        if (DCCEXInbound::parse(_cmdBuffer)) {
          // Process stuff here
          if (_debug) {
            _console->print("<== ");
            _console->println(_cmdBuffer);
          }
          _processCommand();
        }
        // Clear buffer after use
        _cmdBuffer[0] = 0;
        _bufflen = 0;
      }
    }
    if (_enableHeartbeat) {
      _sendHeartbeat();
    }

    _processPendingUserChanges();
  }
}

void DCCEXProtocol::sendCommand(const char *cmd) {
  _cmdStart();
  _cmdAppend(cmd);
  _cmdSend();
}

// Gated method to get the required lists to avoid overloading the buffer
void DCCEXProtocol::getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired,
                             bool turntableListRequired) {
  // Serial.println(F("getLists()"));
  if (_receivedLists)
    return;

  // Start with roster if it's required and get it, do not continue
  if (rosterRequired && !_rosterRequested) {
    _getRoster();
    return;
  }

  // If we're still waiting for the roster, do not continue
  if (_rosterRequested && !_receivedRoster) {
    return;
  }

  // If we get here, get turnouts if required
  if (turnoutListRequired && !_turnoutListRequested) {
    _getTurnouts();
    return;
  }

  // If we're still waiting for turnouts, do not continue
  if (_turnoutListRequested && !_receivedTurnoutList) {
    return;
  }

  // If we get here, get routes if required
  if (routeListRequired && !_routeListRequested) {
    _getRoutes();
    return;
  }

  // If we're still waiting for routes, do not continue
  if (_routeListRequested && !_receivedRouteList) {
    return;
  }

  // If we get here, get turntables if required
  if (turntableListRequired && !_turntableListRequested) {
    _getTurntables();
    return;
  }

  // If we're still waiting for turntables, do not continue
  if (_turntableListRequested && !_receivedTurntableList) {
    return;
  }

  // If we get here, all lists received
  _receivedLists = true;
}

bool DCCEXProtocol::receivedLists() { return _receivedLists; }

void DCCEXProtocol::requestServerVersion() { _sendOpcode('s'); }

bool DCCEXProtocol::receivedVersion() { return _receivedVersion; }

int DCCEXProtocol::getMajorVersion() { return _version[0]; }

int DCCEXProtocol::getMinorVersion() { return _version[1]; }

int DCCEXProtocol::getPatchVersion() { return _version[2]; }

const char *DCCEXProtocol::getLibraryVersion() { return DCCEX_PROTOCOL_VERSION; }

unsigned long DCCEXProtocol::getLastServerResponseTime() { return _lastServerResponseTime; }

void DCCEXProtocol::clearAllLists() {
  clearRoster();
  clearLocalLocos();
  clearTurnoutList();
  clearTurntableList();
  clearRouteList();
}

void DCCEXProtocol::refreshAllLists() {
  refreshRoster();
  refreshTurnoutList();
  refreshTurntableList();
  refreshRouteList();
}

void DCCEXProtocol::setDebug(bool debug) { _debug = debug; }

// Consist/loco methods

void DCCEXProtocol::setThrottle(Loco *loco, int speed, Direction direction) {
  loco->setUserSpeed(speed);
  loco->setUserDirection(direction);
}

void DCCEXProtocol::setThrottle(Consist *consist, int speed, Direction direction) {
  for (ConsistLoco *cl = consist->getFirst(); cl; cl = cl->getNext()) {
    Direction effectiveDir =
        (cl->getFacing() == FacingReversed) ? (direction == Forward ? Reverse : Forward) : direction;
    cl->getLoco()->setUserSpeed(speed);
    cl->getLoco()->setUserDirection(effectiveDir);
  }
}

void DCCEXProtocol::setThrottle(CSConsist *csConsist, int speed, Direction direction) {
  // Can only proceed if consist provided and is valid
  if (!csConsist || !csConsist->isValid())
    return;

  int leadLoco = csConsist->getFirstMember()->address;
  // Attempt to get an existing Loco for lead address
  Loco *loco = Loco::getByAddress(leadLoco);

  if (loco == nullptr)
    loco = new Loco(leadLoco, LocoSource::LocoSourceEntry);

  setThrottle(loco, speed, direction);
}

void DCCEXProtocol::functionOn(Loco *loco, int function) {
  int address = loco->getAddress();
  if (address >= 0) {
    _sendThreeParams('F', address, function, 1);
  }
}

void DCCEXProtocol::functionOn(Consist *consist, int function) {
  for (ConsistLoco *cl = consist->getFirst(); cl; cl = cl->getNext()) {
    functionOn(cl->getLoco(), function);
  }
}

void DCCEXProtocol::functionOn(CSConsist *csConsist, int function) {
  if (!csConsist || !csConsist->isValid())
    return;

  CSConsistMember *first = csConsist->getFirstMember();
  Loco *loco = Loco::getByAddress(first->address);

  if (loco == nullptr)
    loco = new Loco(first->address, LocoSource::LocoSourceEntry);

  _sendThreeParams('F', first->address, function, true);

  if (csConsist->getReplicateFunctions())
    _setCSConsistMemberFunction(first->next, function, true);
}

void DCCEXProtocol::functionOff(Loco *loco, int function) {
  int address = loco->getAddress();
  if (address >= 0) {
    _sendThreeParams('F', address, function, 0);
  }
}

void DCCEXProtocol::functionOff(Consist *consist, int function) {
  for (ConsistLoco *cl = consist->getFirst(); cl; cl = cl->getNext()) {
    functionOff(cl->getLoco(), function);
  }
}

void DCCEXProtocol::functionOff(CSConsist *csConsist, int function) {
  if (!csConsist || !csConsist->isValid())
    return;

  CSConsistMember *first = csConsist->getFirstMember();
  Loco *loco = Loco::getByAddress(first->address);

  if (loco == nullptr)
    loco = new Loco(first->address, LocoSource::LocoSourceEntry);

  _sendThreeParams('F', first->address, function, false);

  if (csConsist->getReplicateFunctions())
    _setCSConsistMemberFunction(first->next, function, false);
}

bool DCCEXProtocol::isFunctionOn(Loco *loco, int function) { return loco->isFunctionOn(function); }

bool DCCEXProtocol::isFunctionOn(Consist *consist, int function) {
  ConsistLoco *firstCL = consist->getFirst();
  return firstCL->getLoco()->isFunctionOn(function);
}

bool DCCEXProtocol::isFunctionOn(CSConsist *csConsist, int function) {
  if (!csConsist || !csConsist->isValid())
    return false;

  CSConsistMember *first = csConsist->getFirstMember();
  Loco *loco = Loco::getByAddress(first->address);

  if (loco == nullptr)
    return false;

  return loco->isFunctionOn(function);
}

void DCCEXProtocol::requestLocoUpdate(int address) { _sendOneParam('t', address); }

void DCCEXProtocol::readLoco() { _sendOpcode('R'); }

void DCCEXProtocol::emergencyStop() { _sendOpcode('!'); }

// Roster methods

int DCCEXProtocol::getRosterCount() { return _rosterCount; }

bool DCCEXProtocol::receivedRoster() { return _receivedRoster; }

Loco *DCCEXProtocol::findLocoInRoster(int address) {
  for (Loco *r = roster->getFirst(); r; r = r->getNext()) {
    if (r->getAddress() == address) {
      return r;
    }
  }
  return nullptr;
}

void DCCEXProtocol::clearRoster() {
  Loco::clearRoster();
  roster = nullptr;
  _rosterCount = 0;
}

void DCCEXProtocol::clearLocalLocos() { Loco::clearLocalLocos(); }

void DCCEXProtocol::refreshRoster() {
  clearRoster();
  _receivedLists = false;
  _receivedRoster = false;
  _rosterRequested = false;
}

// CSConsist methods

void DCCEXProtocol::requestCSConsists() { _sendOpcode('^'); }

CSConsist *DCCEXProtocol::createCSConsist(int leadLoco, bool reversed, bool replicateFunctions) {
  if (leadLoco < 1 || leadLoco > 10239)
    return nullptr;

  // First check if one already exists
  CSConsist *csConsist = CSConsist::getLeadLocoCSConsist(leadLoco);
  if (csConsist != nullptr)
    return csConsist;

  // Ensure the lead loco isn't in any other consists, if it is then fail
  if (CSConsist::getMemberCSConsist(leadLoco))
    return nullptr;

  csConsist = new CSConsist(replicateFunctions);
  csConsist->addMember(leadLoco, reversed);

  return csConsist;
}

bool DCCEXProtocol::addCSConsistMember(CSConsist *csConsist, int address, bool reversed) {
  // Validate provided parameters
  if (csConsist == nullptr || address < 1 || address > 10239)
    return false;

  // If address is in any other consist, fail
  if (CSConsist::getMemberCSConsist(address) != nullptr)
    return false;

  // Add the new member
  csConsist->addMember(address, reversed);
  if (csConsist->isValid()) {
    // If it's valid, build the command
    _sendCreateCSConsist(csConsist);
    return true;
  } else {
    // Otherwise fail
    return false;
  }
}

bool DCCEXProtocol::removeCSConsistMember(CSConsist *csConsist, int address) {
  // Validate parameters
  if (csConsist == nullptr || address < 1 || address > 10239)
    return false;

  // If the consist has no members, delete it
  if (csConsist->getMemberCount() == 0) {
    delete csConsist;
    return false;
  }

  // Don't remove if it's not in there to start with
  if (!csConsist->isInConsist(address))
    return false;

  // Remove the member
  csConsist->removeMember(address);

  if (csConsist->isValid()) {
    // If valid, send the updated consist
    _sendCreateCSConsist(csConsist);
    return true;
  } else {
    // Otherwise delete the CSConsist as it is no longer required
    _sendDeleteCSConsist(csConsist);
    delete csConsist;
    return true;
  }
}

void DCCEXProtocol::deleteCSConsist(int leadLoco) {
  CSConsist *csConsist = CSConsist::getLeadLocoCSConsist(leadLoco);

  if (csConsist == nullptr)
    return;

  delete csConsist;
}

void DCCEXProtocol::deleteCSConsist(CSConsist *csConsist) {
  if (!csConsist)
    return;

  delete csConsist;
}

void DCCEXProtocol::clearCSConsists() { CSConsist::clearCSConsists(); }

// Momentum methods

void DCCEXProtocol::setMomentumAlgorithm(MomentumAlgorithm algorithm) {
  // Algorithm lookup table
  static const char *const ALGORITHMS[] = {"LINEAR", "POWER"};

  // Check out of bounds before sending command
  if (algorithm >= 0 && algorithm < (sizeof(ALGORITHMS) / sizeof(ALGORITHMS[0]))) {
    _sendOneParam('m', ALGORITHMS[algorithm]);
  }
}

void DCCEXProtocol::setDefaultMomentum(int momentum) { _sendTwoParams('m', 0, momentum); }

void DCCEXProtocol::setDefaultMomentum(int accelerating, int braking) {
  _sendThreeParams('m', 0, accelerating, braking);
}

void DCCEXProtocol::setMomentum(int address, int momentum) {
  if (address < 1 || address > 10239)
    return;

  _sendTwoParams('m', address, momentum);
}

void DCCEXProtocol::setMomentum(Loco *loco, int momentum) {
  if (!loco)
    return;

  _sendTwoParams('m', loco->getAddress(), momentum);
}

void DCCEXProtocol::setMomentum(int address, int accelerating, int braking) {
  if (address < 1 || address > 10239)
    return;

  _sendThreeParams('m', address, accelerating, braking);
}

void DCCEXProtocol::setMomentum(Loco *loco, int accelerating, int braking) {
  if (!loco)
    return;

  _sendThreeParams('m', loco->getAddress(), accelerating, braking);
}

// Turnout methods

int DCCEXProtocol::getTurnoutCount() { return _turnoutCount; }

bool DCCEXProtocol::receivedTurnoutList() { return _receivedTurnoutList; }

// find the turnout/point in the turnout list by id. return a pointer or null is not found
Turnout *DCCEXProtocol::getTurnoutById(int turnoutId) {
  for (Turnout *turnout = turnouts->getFirst(); turnout; turnout = turnout->getNext()) {
    if (turnout->getId() == turnoutId) {
      return turnout;
    }
  }
  return nullptr; // not found
}

void DCCEXProtocol::closeTurnout(int turnoutId) { _sendTwoParams('T', turnoutId, 0); }

void DCCEXProtocol::throwTurnout(int turnoutId) { _sendTwoParams('T', turnoutId, 1); }

void DCCEXProtocol::toggleTurnout(int turnoutId) {
  for (Turnout *t = turnouts->getFirst(); t; t = t->getNext()) {
    if (t->getId() == turnoutId) {
      bool thrown = t->getThrown() ? 0 : 1;
      _sendTwoParams('T', turnoutId, thrown);
    }
  }
}

void DCCEXProtocol::clearTurnoutList() {
  Turnout::clearTurnoutList();
  turnouts = nullptr;
  _turnoutCount = 0;
}

void DCCEXProtocol::refreshTurnoutList() {
  clearTurnoutList();
  _receivedLists = false;
  _receivedTurnoutList = false;
  _turnoutListRequested = false;
}

// Route methods

int DCCEXProtocol::getRouteCount() { return _routeCount; }

bool DCCEXProtocol::receivedRouteList() { return _receivedRouteList; }

void DCCEXProtocol::startRoute(int routeId) { _sendTwoParams('/', "START", routeId); }

void DCCEXProtocol::handOffLoco(int locoAddress, int automationId) {
  Route *automation = routes->getById(automationId);
  if (!automation || automation->getType() != RouteType::RouteTypeAutomation)
    return;
  _sendThreeParams('/', "START", locoAddress, automationId);
}

void DCCEXProtocol::pauseRoutes() { _sendOneParam('/', "PAUSE"); }

void DCCEXProtocol::resumeRoutes() { _sendOneParam('/', "RESUME"); }

void DCCEXProtocol::clearRouteList() {
  Route::clearRouteList();
  routes = nullptr;
  _routeCount = 0;
}

void DCCEXProtocol::refreshRouteList() {
  clearRouteList();
  _receivedLists = false;
  _receivedRouteList = false;
  _routeListRequested = false;
}

// Turntable methods

int DCCEXProtocol::getTurntableCount() { return _turntableCount; }

bool DCCEXProtocol::receivedTurntableList() { return _receivedTurntableList; }

Turntable *DCCEXProtocol::getTurntableById(int turntableId) {
  for (Turntable *tt = turntables->getFirst(); tt; tt = tt->getNext()) {
    if (tt->getId() == turntableId) {
      return tt;
    }
  }
  return nullptr;
}

void DCCEXProtocol::rotateTurntable(int turntableId, int position, int activity) {
  Turntable *tt = turntables->getById(turntableId);
  if (tt) {
    if (tt->getType() == TurntableTypeEXTT) {
      if (position == 0) {
        activity = 2;
      }
      _sendThreeParams('I', turntableId, position, activity);
    } else {
      _sendTwoParams('I', turntableId, position);
    }
  }
  _sendCommand();
}

void DCCEXProtocol::clearTurntableList() {
  Turntable::clearTurntableList();
  turntables = nullptr;
  _turntableCount = 0;
}

void DCCEXProtocol::refreshTurntableList() {
  clearTurntableList();
  _receivedLists = false;
  _receivedTurntableList = false;
  _turntableListRequested = false;
}

// Track management methods

void DCCEXProtocol::powerOn() { _sendOpcode('1'); }

void DCCEXProtocol::powerOff() { _sendOpcode('0'); }

void DCCEXProtocol::powerMainOn() { _sendOneParam('1', "MAIN"); }

void DCCEXProtocol::powerMainOff() { _sendOneParam('0', "MAIN"); }

void DCCEXProtocol::powerProgOn() { _sendOneParam('1', "PROG"); }

void DCCEXProtocol::powerProgOff() { _sendOneParam('0', "PROG"); }

void DCCEXProtocol::joinProg() { _sendOneParam('1', "JOIN"); }

void DCCEXProtocol::powerTrackOn(char track) { _sendOneParam('1', track); }

void DCCEXProtocol::powerTrackOff(char track) { _sendOneParam('0', track); }

void DCCEXProtocol::setTrackType(char track, TrackManagerMode type, int address) {
  switch (type) {
  case MAIN:
    _sendTwoParams('=', track, "MAIN");
    break;
  case PROG:
    _sendTwoParams('=', track, "PROG");
    break;
  case DC:
    _sendThreeParams('=', track, "DC", address);
    break;
  case DCX:
    _sendThreeParams('=', track, "DCX", address);
    break;
  case NONE:
    _sendTwoParams('=', track, "NONE");
    break;
  default:
    return;
  }
}

void DCCEXProtocol::requestTrackCurrentGauges() { _sendOneParam('J', 'G'); }

void DCCEXProtocol::requestTrackCurrents() { _sendOneParam('J', 'I'); }

// DCC accessory methods

void DCCEXProtocol::activateAccessory(int accessoryAddress, int accessorySubAddr) {
  _sendThreeParams('a', accessoryAddress, accessorySubAddr, 1);
}

void DCCEXProtocol::deactivateAccessory(int accessoryAddress, int accessorySubAddr) {
  _sendThreeParams('a', accessoryAddress, accessorySubAddr, 0);
}

void DCCEXProtocol::activateLinearAccessory(int linearAddress) { _sendTwoParams('a', linearAddress, 1); }

void DCCEXProtocol::deactivateLinearAccessory(int linearAddress) { _sendTwoParams('a', linearAddress, 0); }

void DCCEXProtocol::getNumberSupportedLocos() { _sendOpcode('#'); }

// CV programming methods

void DCCEXProtocol::readCV(int cv) { _sendOneParam('R', cv); }

void DCCEXProtocol::validateCV(int cv, int value) { _sendTwoParams('V', cv, value); }

void DCCEXProtocol::validateCVBit(int cv, int bit, int value) { _sendThreeParams('V', cv, bit, value); }

void DCCEXProtocol::writeLocoAddress(int address) { _sendOneParam('W', address); }

void DCCEXProtocol::writeCV(int cv, int value) { _sendTwoParams('W', cv, value); }

void DCCEXProtocol::writeCVBit(int cv, int bit, int value) { _sendThreeParams('B', cv, bit, value); }

void DCCEXProtocol::writeCVOnMain(int address, int cv, int value) { _sendThreeParams('w', address, cv, value); }

void DCCEXProtocol::writeCVBitOnMain(int address, int cv, int bit, int value) {
  _sendFourParams('b', address, cv, bit, value);
}

// Fast clock methods

void DCCEXProtocol::setFastClock(int minutes, int speedFactor) {
  if (minutes < 0 || minutes > 1440 || speedFactor < 1)
    return;

  _sendThreeParams('J', 'C', minutes, speedFactor);
}

void DCCEXProtocol::requestFastClockTime() { _sendOneParam('J', 'C'); }

// Private methods
// Protocol and server methods

// init the DCCEXProtocol instance after connection to the server
void DCCEXProtocol::_init() {
  // allocate input buffer and init position variable
  memset(_inputBuffer, 0, sizeof(_inputBuffer));
  _nextChar = 0;
  // last Response time
  _lastServerResponseTime = millis();
}

void DCCEXProtocol::_sendCommand() {
  if (_stream) {
    _stream->print(_outboundCommand);
    if (_debug) {
      _console->print("==> ");
      _console->println(_outboundCommand);
    }
    *_outboundCommand = 0;     // clear it once it has been sent
    _lastHeartbeat = millis(); // If we sent a command, a heartbeat isn't necessary
  }
}

void DCCEXProtocol::_processCommand() {
  // last Response time
  _lastServerResponseTime = millis();

  switch (DCCEXInbound::getOpcode()) {
  case '@': // Screen update
    if (DCCEXInbound::isTextParameter(2) && DCCEXInbound::getParameterCount() == 3) {
      _processScreenUpdate();
    }
    break;

  case 'i': // iDCC-EX server info
    if (DCCEXInbound::isTextParameter(0)) {
      _processServerDescription();
    }
    break;

  case 'm': // Broadcast message
    if (DCCEXInbound::isTextParameter(0)) {
      _processMessage();
    }
    break;

  case 'I': // Turntable broadcast
    if (DCCEXInbound::getParameterCount() == 3) {
      _processTurntableBroadcast();
    }
    break;

  case 'p': // Power broadcast
    if (DCCEXInbound::isTextParameter(0) || DCCEXInbound::getParameterCount() > 2)
      break;
    _processTrackPower();
    break;

  case '=': // Track type broadcast
    if (DCCEXInbound::getParameterCount() < 2)
      break;
    _processTrackType();
    break;

  case 'l': // Loco/cab broadcast
    if (DCCEXInbound::isTextParameter(0) || DCCEXInbound::getParameterCount() != 4)
      break;
    _processLocoBroadcast();
    break;

  case 'j': // Throttle list response jA|O|P|R|T|G|I
    if (DCCEXInbound::isTextParameter(0))
      break;
    if (DCCEXInbound::getNumber(0) == 'A') {        // Receive route/automation info
      if (DCCEXInbound::getParameterCount() == 0) { // Empty list, no routes/automations
        _receivedRouteList = true;
      } else if (DCCEXInbound::getParameterCount() == 4 && DCCEXInbound::isTextParameter(3)) { // Receive route entry
        _processRouteEntry();
      } else { // Receive route/automation list
        _processRouteList();
      }
    } else if (DCCEXInbound::getNumber(0) == 'O') { // Receive turntable info
      if (DCCEXInbound::getParameterCount() == 0) { // Empty turntable list
        _receivedTurntableList = true;
      } else if (DCCEXInbound::getParameterCount() == 6 && DCCEXInbound::isTextParameter(5)) { // Turntable entry
        _processTurntableEntry();
      } else { // Turntable list
        _processTurntableList();
      }
    } else if (DCCEXInbound::getNumber(0) == 'P') { // Receive turntable position info
      if (DCCEXInbound::getParameterCount() == 5 &&
          DCCEXInbound::isTextParameter(4)) { // Turntable position index entry
        _processTurntableIndexEntry();
      }
    } else if (DCCEXInbound::getNumber(0) == 'R') { // Receive roster info
      if (DCCEXInbound::getParameterCount() == 1) { // Empty list, no roster
        _receivedRoster = true;
      } else if (DCCEXInbound::getParameterCount() == 4 && DCCEXInbound::isTextParameter(2) &&
                 DCCEXInbound::isTextParameter(3)) { // Roster entry
        // <jR id "desc" "func1/func2/func3/...">
        _processRosterEntry();
      } else { // Roster list
        // <jR id1 id2 id3 ...>
        _processRosterList();
      }
    } else if (DCCEXInbound::getNumber(0) == 'T') { // Receive turnout info
      if (DCCEXInbound::getParameterCount() == 1) { // Empty list, no turnouts defined
        _receivedTurnoutList = true;
      } else if (DCCEXInbound::getParameterCount() == 4 && DCCEXInbound::isTextParameter(3)) { // Turnout entry
        // <jT id state "desc">
        _processTurnoutEntry();
      } else { // Turnout list
        // <jT id1 id2 id3 ...>
        _processTurnoutList();
      }
    } else if (DCCEXInbound::getNumber(0) == 'G') { // Receive track current gauges <jG a b ...>
      _processTrackCurrentGauges();
    } else if (DCCEXInbound::getNumber(0) == 'I') { // Receive track currents <jI a b ...>
      _processTrackCurrents();
    } else if (DCCEXInbound::getNumber(0) == 'C') { // Receive fast clock info
      if (DCCEXInbound::getParameterCount() == 2) {
        _processFastClockTime();
      } else if (DCCEXInbound::getParameterCount() == 3) {
        _processSetFastClock();
      }
    }
    break;

  case 'H': // Turnout broadcast
    if (DCCEXInbound::isTextParameter(0))
      break;
    _processTurnoutBroadcast();
    break;

  case 'r': // Read loco response
    if (DCCEXInbound::isTextParameter(0))
      break;
    if (DCCEXInbound::getParameterCount() == 1) {
      _processReadResponse();
    } else if (DCCEXInbound::getParameterCount() == 2) {
      _processWriteCVResponse();
    }
    break;

  case 'w': // Write loco response
    if (DCCEXInbound::isTextParameter(0))
      break;
    _processWriteLocoResponse();
    break;

  case 'v': // Validate CV response
    if (DCCEXInbound::isTextParameter(0))
      break;
    if (DCCEXInbound::getParameterCount() == 2) {
      _processValidateCVResponse();
    } else if (DCCEXInbound::getParameterCount() == 3) {
      _processValidateCVBitResponse();
    }
    break;

  case '^': // Receive CSConsist
    _processCSConsist();
    break;

  default:
    break;
  }
}

void DCCEXProtocol::_processServerDescription() { //<iDCCEX version / microprocessorType / MotorControllerType /
                                                  // buildNumber>
  char *description{DCCEXInbound::getTextParameter(0) + 7};
  int *version = _version;

  while (description < _cmdBuffer + _maxCmdBuffer) {
    // Delimiter
    char const delim = *description++;
    if (delim != '-' && delim != '.')
      continue;

    // Int
    char const first_digit = *description;
    if (!isdigit(first_digit))
      continue;

    // string to int
    int const v = atoi(description);
    if (v < 0)
      return; // Error
    else if (v < 10)
      description += 1;
    else if (v < 100)
      description += 2;
    else if (v < 1000)
      description += 3;
    else
      return; // Error

    // Done after 3 numbers
    *version++ = v;
    if (version - _version >= 3)
      break;
  }

  _receivedVersion = true;

  if (_delegate)
    _delegate->receivedServerVersion(_version[0], _version[1], _version[2]);
}

void DCCEXProtocol::_processMessage() { //<m "message">
  if (!_delegate)
    return;

  _delegate->receivedMessage(DCCEXInbound::getTextParameter(0));
}

void DCCEXProtocol::_processScreenUpdate() { //<@ screen row "message">
  if (!_delegate)
    return;

  _delegate->receivedScreenUpdate(DCCEXInbound::getNumber(0), DCCEXInbound::getNumber(1),
                                  DCCEXInbound::getTextParameter(2));
}

void DCCEXProtocol::_sendHeartbeat() {
  if (millis() - _lastHeartbeat > _heartbeatDelay) {
    _lastHeartbeat = millis();
    _sendOpcode('#');
  }
}

// Consist/loco methods

void DCCEXProtocol::_processLocoBroadcast() { //<l cab reg speedByte functMap>
  int address = DCCEXInbound::getNumber(0);
  int speedByte = DCCEXInbound::getNumber(2);
  int functionMap = _getValidFunctionMap(DCCEXInbound::getNumber(3));
  int speed = _getSpeedFromSpeedByte(speedByte);
  Direction direction = _getDirectionFromSpeedByte(speedByte);

  // Loco address 0 is invalid and should never do anything
  if (address == 0)
    return;

  // Iterate through locos to update the appropriate one, send speedByte to cater for EStop
  _updateLocos(Loco::getFirst(), address, speedByte, direction, functionMap);
  _updateLocos(Loco::getFirstLocalLoco(), address, speedByte, direction, functionMap);

  // Send a broadcast as well in case it's a local Loco not in the roster
  if (_delegate)
    _delegate->receivedLocoBroadcast(address, speed, direction, functionMap);
}

int DCCEXProtocol::_getValidFunctionMap(int functionMap) {
  // Mask off anything above 28 bits/28 functions
  if (functionMap > 0xFFFFFFF) {
    functionMap &= 0xFFFFFFF;
  }
  return functionMap;
}

int DCCEXProtocol::_getSpeedFromSpeedByte(int speedByte) {
  int speed = 127 & speedByte;
  if (speed > 1) {
    speed = speed - 1; // get around the idiotic design of the speed command
  } else {
    speed = 0;
  }
  return speed;
}

Direction DCCEXProtocol::_getDirectionFromSpeedByte(int speedByte) { return (speedByte >= 128) ? Forward : Reverse; }

void DCCEXProtocol::_setLocos(Loco *firstLoco) {
  for (Loco *loco = firstLoco; loco; loco = loco->getNext()) {
    if (!loco->getUserChangePending())
      continue;

    loco->resetUserChangePending();
    _sendThreeParams('t', loco->getAddress(), loco->getUserSpeed(), loco->getUserDirection());
  }
}

void DCCEXProtocol::_updateLocos(Loco *firstLoco, int address, int speedByte, Direction direction, int functionMap) {
  bool eStop = (speedByte == 1 || speedByte == 129) ? true : false;
  int speed = _getSpeedFromSpeedByte(speedByte);
  for (Loco *loco = firstLoco; loco; loco = loco->getNext()) {
    if (loco->getAddress() == address) {
      loco->setSpeed(speed);
      loco->setDirection(direction);
      loco->setFunctionStates(functionMap);
      if (loco->getUserChangePending()) {
        if (eStop) {
          loco->resetUserChangePending();
          loco->setUserSpeed(speed);
        } else if (speed == loco->getUserSpeed() && direction == loco->getUserDirection()) {
          loco->resetUserChangePending();
        }
      }
      if (_delegate)
        _delegate->receivedLocoUpdate(loco);
    }
  }
}

void DCCEXProtocol::_processReadResponse() { // <r id> - -1 = error
  if (!_delegate)
    return;

  int address = DCCEXInbound::getNumber(0);
  _delegate->receivedReadLoco(address);
}

void DCCEXProtocol::_processPendingUserChanges() {
  if (millis() - _lastUserChange > _userChangeDelay) {
    _lastUserChange = millis();
    _setLocos(Loco::getFirst());
    _setLocos(Loco::getFirstLocalLoco());
  }
}

void DCCEXProtocol::_processCSConsist() { // <^ leadLoco [-]address [-]address>
  if (DCCEXInbound::isTextParameter(0))
    return;

  int locoCount = DCCEXInbound::getParameterCount();
  unsigned int leadLoco = abs(DCCEXInbound::getNumber(0));

  // Should never receive less than 2 locos but just in case
  if (locoCount < 2)
    return;

  // Check if there is already a consist with this lead loco
  CSConsist *csConsist = CSConsist::getLeadLocoCSConsist(leadLoco);

  // If there is, clean it up and build with the CS list instead
  if (csConsist != nullptr) {
    csConsist->removeAllMembers();
  } else {
    csConsist = new CSConsist();
  }
  _buildCSConsist(csConsist, locoCount);
  if (_delegate)
    _delegate->receivedCSConsist(leadLoco, csConsist);
}

void DCCEXProtocol::_buildCSConsist(CSConsist *csConsist, int memberCount) {
  for (int i = 0; i < memberCount; i++) {
    int member = DCCEXInbound::getNumber(i);
    unsigned int address = abs(member);
    bool reversed = (member < 0);
    // Ensure members aren't in any other CSConsist objects
    while (CSConsist *checkCSConsist = CSConsist::getMemberCSConsist(address)) {
      checkCSConsist->removeMember(address);
    }
    csConsist->addMember(address, reversed);
  }
}

void DCCEXProtocol::_sendCreateCSConsist(CSConsist *csConsist) {
  _cmdStart('^');
  _cmdAppend(' ');
  for (CSConsistMember *member = csConsist->getFirstMember(); member; member = member->next) {
    // Leading - says reversed
    if (member->reversed)
      _cmdAppend('-');
    _cmdAppend(member->address);
    // If not the last in the least, add a space separator
    if (member->next != nullptr)
      _cmdAppend(' ');
  }
  // Send it
  _cmdSend();
}

void DCCEXProtocol::_sendDeleteCSConsist(CSConsist *csConsist) {
  // Can't delete it if it doesn't have a lead loco
  if (csConsist->getMemberCount() == 0)
    return;

  _sendOneParam('^', csConsist->getFirstMember()->address);
}

void DCCEXProtocol::_setCSConsistMemberFunction(CSConsistMember *member, int function, bool state) {
  for (member = member; member; member = member->next) {
    _sendThreeParams('F', member->address, function, state);
  }
}

// Roster methods

void DCCEXProtocol::_getRoster() {
  _sendOneParam('J', 'R');
  _rosterRequested = true;
}

bool DCCEXProtocol::_requestedRoster() { return _rosterRequested; }

void DCCEXProtocol::_processRosterList() {
  if (roster != nullptr) { // already have a roster so this is an update
    return;
  }
  if (DCCEXInbound::getParameterCount() == 1) { // roster empty
    _receivedRoster = true;
    return;
  }
  for (int i = 1; i < DCCEXInbound::getParameterCount(); i++) {
    int address = DCCEXInbound::getNumber(i);
    new Loco(address, LocoSourceRoster);
  }
  _requestRosterEntry(Loco::getFirst()->getAddress());
  _rosterCount = DCCEXInbound::getParameterCount() - 1;
}

void DCCEXProtocol::_requestRosterEntry(int address) { _sendTwoParams('J', 'R', address); }

void DCCEXProtocol::_processRosterEntry() { //<jR id ""|"desc" ""|"funct1/funct2/funct3/...">
  // find the roster entry to update
  int address = DCCEXInbound::getNumber(1);
  char *name = DCCEXInbound::copyTextParameter(2);
  char *funcs = DCCEXInbound::copyTextParameter(3);
  bool missingRosters = false;

  Loco *loco = roster->getByAddress(address);
  if (loco) {
    loco->setName(name);
    loco->setupFunctions(funcs);
    if (loco->getNext() && loco->getNext()->getName() == nullptr) {
      missingRosters = true;
      _requestRosterEntry(loco->getNext()->getAddress());
    }
  }

  if (!missingRosters) {
    _receivedRoster = true;
    if (_delegate)
      _delegate->receivedRosterList();
  }

  free(name);
  free(funcs);
}

// Turnout methods

void DCCEXProtocol::_getTurnouts() {
  _sendOneParam('J', 'T');
  _turnoutListRequested = true;
}

bool DCCEXProtocol::_requestedTurnouts() { return _turnoutListRequested; }

void DCCEXProtocol::_processTurnoutList() {
  // <jT id1 id2 id3 ...>
  if (turnouts != nullptr) {
    return;
  }
  if (DCCEXInbound::getParameterCount() == 1) { // turnout list is empty
    _receivedTurnoutList = true;
    return;
  }
  for (int i = 1; i < DCCEXInbound::getParameterCount(); i++) {
    auto id = DCCEXInbound::getNumber(i);
    new Turnout(id, false);
  }
  _requestTurnoutEntry(Turnout::getFirst()->getId());
  _turnoutCount = DCCEXInbound::getParameterCount() - 1;
}

void DCCEXProtocol::_requestTurnoutEntry(int id) { _sendTwoParams('J', 'T', id); }

void DCCEXProtocol::_processTurnoutEntry() {
  if (DCCEXInbound::getParameterCount() != 4)
    return;
  // find the turnout entry to update
  int id = DCCEXInbound::getNumber(1);
  bool thrown = (DCCEXInbound::getNumber(2) == 'T');
  char *name = DCCEXInbound::copyTextParameter(3);
  bool missingTurnouts = false;

  Turnout *t = Turnout::getById(id);
  if (t) {
    t->setName(name);
    t->setThrown(thrown);
    if (t->getNext() && t->getNext()->getName() == nullptr) {
      missingTurnouts = true;
      _requestTurnoutEntry(t->getNext()->getId());
    }
  }

  free(name);

  if (!missingTurnouts) {
    _receivedTurnoutList = true;
    if (_delegate)
      _delegate->receivedTurnoutList();
  }
}

void DCCEXProtocol::_processTurnoutBroadcast() { //<H id state>
  if (!_delegate)
    return;

  if (DCCEXInbound::getParameterCount() != 2)
    return;
  // find the Turnout entry to update
  int id = DCCEXInbound::getNumber(0);
  bool thrown = DCCEXInbound::getNumber(1);
  for (auto t = Turnout::getFirst(); t; t = t->getNext()) {
    if (t->getId() == id) {
      t->setThrown(thrown);
      _delegate->receivedTurnoutAction(id, thrown);
    }
  }
}

// Route methods

void DCCEXProtocol::_getRoutes() {
  _sendOneParam('J', 'A');
  _routeListRequested = true;
}

bool DCCEXProtocol::_requestedRoutes() { return _routeListRequested; }

void DCCEXProtocol::_processRouteList() {
  if (routes != nullptr) {
    return;
  }
  if (DCCEXInbound::getParameterCount() == 1) { // route list is empty
    _receivedRouteList = true;
    return;
  }
  for (int i = 1; i < DCCEXInbound::getParameterCount(); i++) {
    int id = DCCEXInbound::getNumber(i);
    new Route(id);
  }
  _requestRouteEntry(Route::getFirst()->getId());
  _routeCount = DCCEXInbound::getParameterCount() - 1;
}

void DCCEXProtocol::_requestRouteEntry(int id) { _sendTwoParams('J', 'A', id); }

void DCCEXProtocol::_processRouteEntry() {
  // find the Route entry to update
  int id = DCCEXInbound::getNumber(1);
  RouteType type = (RouteType)DCCEXInbound::getNumber(2);
  char *name = DCCEXInbound::copyTextParameter(3);
  bool missingRoutes = false;

  Route *r = Route::getById(id);
  if (r) {
    r->setType(type);
    r->setName(name);
    if (r->getNext() && r->getNext()->getName() == nullptr) {
      missingRoutes = true;
      _requestRouteEntry(r->getNext()->getId());
    }
  }

  free(name);

  if (!missingRoutes) {
    _receivedRouteList = true;
    if (_delegate)
      _delegate->receivedRouteList();
  }
}

// Turntable methods

void DCCEXProtocol::_getTurntables() {
  _sendOneParam('J', 'O');
  _turntableListRequested = true;
}

bool DCCEXProtocol::_requestedTurntables() { return _turntableListRequested; }

void DCCEXProtocol::_processTurntableList() { // <jO [id1 id2 id3 ...]>
  if (turntables != nullptr) {                // already have a turntables list so this is an update
    return;
  }
  if (DCCEXInbound::getParameterCount() == 1) { // list is empty so we have received it
    _receivedTurntableList = true;
    return;
  }
  for (int i = 1; i < DCCEXInbound::getParameterCount(); i++) {
    int id = DCCEXInbound::getNumber(i);
    new Turntable(id);
  }
  _requestTurntableEntry(Turntable::getFirst()->getId());
  _turntableCount = DCCEXInbound::getParameterCount() - 1;
}

void DCCEXProtocol::_requestTurntableEntry(int id) { _sendTwoParams('J', 'O', id); }

void DCCEXProtocol::_processTurntableEntry() { // <jO id type position position_count "[desc]">
  // find the Turntable entry to update
  int id = DCCEXInbound::getNumber(1);
  TurntableType ttType = (TurntableType)DCCEXInbound::getNumber(2);
  int index = DCCEXInbound::getNumber(3);
  int indexCount = DCCEXInbound::getNumber(4);
  char *name = DCCEXInbound::copyTextParameter(5);

  Turntable *tt = Turntable::getById(id);
  if (tt) {
    tt->setType(ttType);
    tt->setIndex(index);
    tt->setNumberOfIndexes(indexCount);
    tt->setName(name);
    _requestTurntableIndexEntry(id);
    if (tt->getNext() && tt->getNext()->getName() == nullptr) {
      _requestTurntableEntry(tt->getNext()->getId());
    }
  }

  free(name);
}

void DCCEXProtocol::_requestTurntableIndexEntry(int id) { _sendTwoParams('J', 'P', id); }

void DCCEXProtocol::_processTurntableIndexEntry() { // <jP id index angle "[desc]">
  if (DCCEXInbound::getParameterCount() != 5)
    return;

  // find the Turntable entry to update
  int ttId = DCCEXInbound::getNumber(1);
  int index = DCCEXInbound::getNumber(2);
  int angle = DCCEXInbound::getNumber(3);
  char *parsedName = DCCEXInbound::copyTextParameter(4);
  const char *name = (index == 0) ? "Home" : parsedName;

  Turntable *tt = getTurntableById(ttId);
  if (tt) {
    if (tt->getNumberOfIndexes() != tt->getIndexCount()) {
      TurntableIndex *newIndex = new TurntableIndex(ttId, index, angle, name);
      tt->addIndex(newIndex);
    }

    bool receivedAll = true;

    for (Turntable *tt = turntables->getFirst(); tt; tt = tt->getNext()) {
      int numIndexes = tt->getNumberOfIndexes();
      int indexCount = tt->getIndexCount();
      if (tt->getName() == nullptr || (numIndexes != indexCount))
        receivedAll = false;
    }

    if (receivedAll) {
      _receivedTurntableList = true;
      if (_delegate)
        _delegate->receivedTurntableList();
    }
  }

  free(parsedName);
}

void DCCEXProtocol::_processTurntableBroadcast() { // <I id position moving>
  int id = DCCEXInbound::getNumber(0);
  int newIndex = DCCEXInbound::getNumber(1);
  bool moving = DCCEXInbound::getNumber(2);
  Turntable *tt = getTurntableById(id);
  if (tt) {
    tt->setIndex(newIndex);
    tt->setMoving(moving);
  }
  if (_delegate)
    _delegate->receivedTurntableAction(id, newIndex, moving);
}

// Track management methods

void DCCEXProtocol::_processTrackPower() {
  if (!_delegate)
    return;

  TrackPower state = PowerUnknown;
  if (DCCEXInbound::getNumber(0) == PowerOff) {
    state = PowerOff;
  } else if (DCCEXInbound::getNumber(0) == PowerOn) {
    state = PowerOn;
  }

  if (DCCEXInbound::getParameterCount() == 2) {
    int _track = DCCEXInbound::getNumber(1);
    _delegate->receivedIndividualTrackPower(state, _track);

    if (DCCEXInbound::getNumber(1) != 2698315) {
      return;
    } // not equal "MAIN"
  }
  _delegate->receivedTrackPower(state);
}

void DCCEXProtocol::_processTrackType() {
  if (!_delegate)
    return;
  char _track = DCCEXInbound::getNumber(0);
  int _type = DCCEXInbound::getNumber(1);
  TrackManagerMode _trackType;
  switch (_type) {
  case 2698315:
    _trackType = MAIN;
    break;
  case 2788330:
    _trackType = PROG;
    break;
  case 2183:
    _trackType = DC;
    break;
  case 71999:
    _trackType = DCX;
    break;
  case 2857034:
    _trackType = NONE;
    break;
  default:
    return;
  }
  int _address = 0;
  if (DCCEXInbound::getParameterCount() > 2)
    _address = DCCEXInbound::getNumber(2);

  _delegate->receivedTrackType(_track, _trackType, _address);
}

void DCCEXProtocol::_processTrackCurrentGauges() { // <jG a b ...>
  if (!_delegate)
    return;

  int trackCount = DCCEXInbound::getParameterCount();
  for (int track = 1; track < trackCount; track++) { // First param is G, rest are tracks
    _delegate->receivedTrackCurrentGauge('A' + track - 1, DCCEXInbound::getNumber(track));
  }
}

void DCCEXProtocol::_processTrackCurrents() { // <jI a b ...>
  if (!_delegate)
    return;

  int trackCount = DCCEXInbound::getParameterCount();
  for (int track = 1; track < trackCount; track++) { // First param is I, rest are tracks
    _delegate->receivedTrackCurrent('A' + track - 1, DCCEXInbound::getNumber(track));
  }
}

// CV programming methods

void DCCEXProtocol::_processValidateCVResponse() { // <v cv value>, value -1 = error
  if (!_delegate)
    return;

  int cv = DCCEXInbound::getNumber(0);
  int value = DCCEXInbound::getNumber(1);
  _delegate->receivedValidateCV(cv, value);
}

void DCCEXProtocol::_processValidateCVBitResponse() { // <v cv bit value>, value -1 = error
  if (!_delegate)
    return;

  int cv = DCCEXInbound::getNumber(0);
  int bit = DCCEXInbound::getNumber(1);
  int value = DCCEXInbound::getNumber(2);
  _delegate->receivedValidateCVBit(cv, bit, value);
}

void DCCEXProtocol::_processWriteLocoResponse() { // <w id> - -1 = error
  if (!_delegate)
    return;

  int value = DCCEXInbound::getNumber(0);
  _delegate->receivedWriteLoco(value);
}

void DCCEXProtocol::_processWriteCVResponse() { // <r cv value>, value -1 = error
  if (!_delegate)
    return;

  int cv = DCCEXInbound::getNumber(0);
  int value = DCCEXInbound::getNumber(1);
  _delegate->receivedWriteCV(cv, value);
}

// Fast clock methods

void DCCEXProtocol::_processSetFastClock() { // <jC minutes speed>
  if (!_delegate)
    return;

  _delegate->receivedSetFastClock(DCCEXInbound::getNumber(1), DCCEXInbound::getNumber(2));
}

void DCCEXProtocol::_processFastClockTime() { // <jC minutes>
  if (!_delegate)
    return;

  _delegate->receivedFastClockTime(DCCEXInbound::getNumber(1));
}

// Helper methods to build the outbound command

void DCCEXProtocol::_cmdStart(char opcode) {
  _cmdIndex = 0;
  _outboundCommand[_cmdIndex++] = '<';
  if (opcode != '\0') {
    _outboundCommand[_cmdIndex++] = opcode;
  }
  _outboundCommand[_cmdIndex] = '\0';
}

void DCCEXProtocol::_cmdAppend(const char *s) {
  // Must leave room for '>' and null terminator
  while (*s && _cmdIndex < (MAX_OUTBOUND_COMMAND_LENGTH - 2)) {
    _outboundCommand[_cmdIndex++] = *s++;
  }
  _outboundCommand[_cmdIndex] = '\0';
}

void DCCEXProtocol::_cmdAppend(int n) {
  char buf[12]; // Enough for -2147483648
  itoa(n, buf, 10);
  _cmdAppend(buf);
}

void DCCEXProtocol::_cmdAppend(char c) {
  // Must leave room for '>' and null terminator
  if (_cmdIndex < (MAX_OUTBOUND_COMMAND_LENGTH - 2)) {
    _outboundCommand[_cmdIndex++] = c;
    _outboundCommand[_cmdIndex] = '\0';
  }
}

void DCCEXProtocol::_cmdSend() {
  _outboundCommand[_cmdIndex++] = '>';
  _outboundCommand[_cmdIndex] = '\0';
  _sendCommand();
}

void DCCEXProtocol::_sendOpcode(char opcode) {
  _cmdStart(opcode);
  _cmdSend();
}

void DCCEXProtocol::_sendOneParam(char opcode, char param) {
  _cmdStart(opcode);
  _cmdAppend(' ');
  _cmdAppend(param);
  _cmdSend();
}

void DCCEXProtocol::_sendOneParam(char opcode, const char *param) {
  _cmdStart(opcode);
  _cmdAppend(' ');
  _cmdAppend(param);
  _cmdSend();
}

void DCCEXProtocol::_sendOneParam(char opcode, int param) {
  _cmdStart(opcode);
  _cmdAppend(' ');
  _cmdAppend(param);
  _cmdSend();
}

void DCCEXProtocol::_sendTwoParams(char opcode, char param1, int param2) {
  _cmdStart(opcode);
  _cmdAppend(' ');
  _cmdAppend(param1);
  _cmdAppend(' ');
  _cmdAppend(param2);
  _cmdSend();
}

void DCCEXProtocol::_sendTwoParams(char opcode, int param1, int param2) {
  _cmdStart(opcode);
  _cmdAppend(' ');
  _cmdAppend(param1);
  _cmdAppend(' ');
  _cmdAppend(param2);
  _cmdSend();
}

void DCCEXProtocol::_sendTwoParams(char opcode, const char *param1, int param2) {
  _cmdStart(opcode);
  _cmdAppend(' ');
  _cmdAppend(param1);
  _cmdAppend(' ');
  _cmdAppend(param2);
  _cmdSend();
}

void DCCEXProtocol::_sendTwoParams(char opcode, int param1, char param2) {
  _cmdStart(opcode);
  _cmdAppend(' ');
  _cmdAppend(param1);
  _cmdAppend(' ');
  _cmdAppend(param2);
  _cmdSend();
}

void DCCEXProtocol::_sendTwoParams(char opcode, char param1, const char *param2) {
  _cmdStart(opcode);
  _cmdAppend(' ');
  _cmdAppend(param1);
  _cmdAppend(' ');
  _cmdAppend(param2);
  _cmdSend();
}

void DCCEXProtocol::_sendThreeParams(char opcode, const char *param1, int param2, int param3) {
  _cmdStart(opcode);
  _cmdAppend(' ');
  _cmdAppend(param1);
  _cmdAppend(' ');
  _cmdAppend(param2);
  _cmdAppend(' ');
  _cmdAppend(param3);
  _cmdSend();
}

void DCCEXProtocol::_sendThreeParams(char opcode, int param1, int param2, int param3) {
  _cmdStart(opcode);
  _cmdAppend(' ');
  _cmdAppend(param1);
  _cmdAppend(' ');
  _cmdAppend(param2);
  _cmdAppend(' ');
  _cmdAppend(param3);
  _cmdSend();
}

void DCCEXProtocol::_sendThreeParams(char opcode, char param1, const char *param2, int param3) {
  _cmdStart(opcode);
  _cmdAppend(' ');
  _cmdAppend(param1);
  _cmdAppend(' ');
  _cmdAppend(param2);
  _cmdAppend(' ');
  _cmdAppend(param3);
  _cmdSend();
}

void DCCEXProtocol::_sendThreeParams(char opcode, char param1, int param2, int param3) {
  _cmdStart(opcode);
  _cmdAppend(' ');
  _cmdAppend(param1);
  _cmdAppend(' ');
  _cmdAppend(param2);
  _cmdAppend(' ');
  _cmdAppend(param3);
  _cmdSend();
}

void DCCEXProtocol::_sendFourParams(char opcode, int param1, int param2, int param3, int param4) {
  _cmdStart(opcode);
  _cmdAppend(' ');
  _cmdAppend(param1);
  _cmdAppend(' ');
  _cmdAppend(param2);
  _cmdAppend(' ');
  _cmdAppend(param3);
  _cmdAppend(' ');
  _cmdAppend(param4);
  _cmdSend();
}
