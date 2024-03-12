/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
 *
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

DCCEXProtocol::DCCEXProtocol(int maxCmdBuffer) {
  // Init streams
  _stream = &_nullStream;
  _console = &_nullStream;

  // Allocate memory for command buffer
  _cmdBuffer = new char[maxCmdBuffer];
  _maxCmdBuffer = maxCmdBuffer;

  // Setup command parser
  DCCEXInbound::setup(MAX_COMMAND_PARAMS);
  _cmdBuffer[0] = 0;
  _bufflen = 0;
}

// Set the delegate instance for callbacks
void DCCEXProtocol::setDelegate(DCCEXProtocolDelegate *delegate) { this->_delegate = delegate; }

// Set the Stream used for logging
void DCCEXProtocol::setLogStream(Stream *console) { this->_console = console; }

void DCCEXProtocol::connect(Stream *stream) {
  _init();
  this->_stream = stream;
}

void DCCEXProtocol::disconnect() {
  sprintf(_outboundCommand, "%s", "<U DISCONNECT>");
  _sendCommand();
  this->_stream = nullptr;
}

void DCCEXProtocol::check() {
  if (_stream) {
    while (_stream->available()) {
      // Read from our stream
      int r = _stream->read();
      if (_bufflen < _maxCmdBuffer - 1) {
        _cmdBuffer[_bufflen] = r;
        _bufflen++;
        _cmdBuffer[_bufflen] = 0;
      }

      if (r == '>') {
        if (DCCEXInbound::parse(_cmdBuffer)) {
          // Process stuff here
          _console->print("<== ");
          _console->println(_cmdBuffer);
          _processCommand();
        }
        // Clear buffer after use
        _cmdBuffer[0] = 0;
        _bufflen = 0;
      }
    }
  }
}

// sequentially request and get the required lists. To avoid overloading the buffer
void DCCEXProtocol::getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired,
                             bool turntableListRequired) {
  // console->println(F("getLists()"));
  if (!_receivedLists) {
    if (rosterRequired && !_rosterRequested) {
      _getRoster();
    } else {
      if (!rosterRequired || _receivedRoster) {
        if (turnoutListRequired && !_turnoutListRequested) {
          _getTurnouts();
        } else {
          if (!turnoutListRequired || _receivedTurnoutList) {
            if (routeListRequired && !_routeListRequested) {
              _getRoutes();
            } else {
              if (!routeListRequired || _receivedRouteList) {
                if (turntableListRequired && !_turntableListRequested) {
                  _getTurntables();
                } else {
                  if (!turntableListRequired || _receivedTurntableList) {
                    _receivedLists = true;
                    _console->println(F("Lists Fully Received"));
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  // console->println(F("getLists(): end"));
}

bool DCCEXProtocol::receivedLists() { return _receivedLists; }

void DCCEXProtocol::requestServerVersion() {
  // console->println(F("requestServerVersion(): "));
  if (_delegate) {
    sprintf(_outboundCommand, "<s>");
    _sendCommand();
  }
  // console->println(F("requestServerVersion(): end"));
}

bool DCCEXProtocol::receivedVersion() { return _receivedVersion; }

int DCCEXProtocol::getMajorVersion() { return _majorVersion; }

int DCCEXProtocol::getMinorVersion() { return _minorVersion; }

int DCCEXProtocol::getPatchVersion() { return _patchVersion; }

unsigned long DCCEXProtocol::getLastServerResponseTime() { return _lastServerResponseTime; }

// Consist/loco methods

void DCCEXProtocol::setThrottle(Loco *loco, int speed, Direction direction) {
  if (_delegate) {
    int address = loco->getAddress();
    _setLoco(address, speed, direction);
  }
}

void DCCEXProtocol::setThrottle(Consist *consist, int speed, Direction direction) {
  if (_delegate) {
    for (ConsistLoco *cl = consist->getFirst(); cl; cl = cl->getNext()) {
      int address = cl->getLoco()->getAddress();
      if (cl->getFacing() == FacingReversed) {
        if (direction == Forward) {
          direction = Reverse;
        } else {
          direction = Forward;
        }
      }
      _setLoco(address, speed, direction);
    }
  }
}

void DCCEXProtocol::functionOn(Loco *loco, int function) {
  // console->println(F("sendFunction(): "));
  if (_delegate) {
    int address = loco->getAddress();
    if (address >= 0) {
      sprintf(_outboundCommand, "<F %d %d 1>", address, function);
      _sendCommand();
    }
  }
  // console->println(F("sendFunction(): end"));
}

void DCCEXProtocol::functionOff(Loco *loco, int function) {
  // console->println(F("sendFunction(): "));
  if (_delegate) {
    int address = loco->getAddress();
    if (address >= 0) {
      sprintf(_outboundCommand, "<F %d %d 0>", address, function);
      _sendCommand();
    }
  }
  // console->println(F("sendFunction(): end"));
}

bool DCCEXProtocol::isFunctionOn(Loco *loco, int function) {
  if (_delegate) {
    return loco->isFunctionOn(function);
  }
  return false;
}

void DCCEXProtocol::functionOn(Consist *consist, int function) {
  // console->println(F("sendFunction(): "));
  if (_delegate) {
    for (ConsistLoco *cl = consist->getFirst(); cl; cl = cl->getNext()) {
      functionOn(cl->getLoco(), function);
    }
  }
  // console->println(F("sendFunction(): end"));
}

void DCCEXProtocol::functionOff(Consist *consist, int function) {
  // console->println(F("sendFunction(): "));
  if (_delegate) {
    for (ConsistLoco *cl = consist->getFirst(); cl; cl = cl->getNext()) {
      functionOff(cl->getLoco(), function);
    }
  }
  // console->println(F("sendFunction(): end"));
}

bool DCCEXProtocol::isFunctionOn(Consist *consist, int function) {
  if (_delegate) {
    ConsistLoco *firstCL = consist->getFirst();
    return firstCL->getLoco()->isFunctionOn(function);
  }
  return false;
}

void DCCEXProtocol::requestLocoUpdate(int address) {
  // console->println(F("sendLocoUpdateRequest()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<t %d>", address);
    _sendCommand();
  }
  // console->println(F("sendLocoUpdateRequest() end"));
}

void DCCEXProtocol::readLoco() {
  if (_delegate) {
    sprintf(_outboundCommand, "<R>");
    _sendCommand();
  }
}

void DCCEXProtocol::emergencyStop() {
  // console->println(F("emergencyStop(): "));
  if (_delegate) {
    sprintf(_outboundCommand, "<!>");
    _sendCommand();
  }
  // console->println(F("emergencyStop(): end"));
}

// Roster methods

int DCCEXProtocol::getRosterCount() { return _rosterCount; }

bool DCCEXProtocol::receivedRoster() {
  // console->println(F("isRosterFullyReceived()"));
  return _receivedRoster;
}

Loco *DCCEXProtocol::findLocoInRoster(int address) {
  for (Loco *r = roster->getFirst(); r; r = r->getNext()) {
    if (r->getAddress() == address) {
      return r;
    }
  }
  return nullptr;
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

void DCCEXProtocol::closeTurnout(int turnoutId) {
  if (_delegate) {
    sprintf(_outboundCommand, "<T %d 0>", turnoutId);
    _sendCommand();
  }
}

void DCCEXProtocol::throwTurnout(int turnoutId) {
  if (_delegate) {
    sprintf(_outboundCommand, "<T %d 1>", turnoutId);
    _sendCommand();
  }
}

void DCCEXProtocol::toggleTurnout(int turnoutId) {
  for (Turnout *t = turnouts->getFirst(); t; t = t->getNext()) {
    if (t->getId() == turnoutId) {
      // console->println(t->getThrown());
      bool thrown = t->getThrown() ? 0 : 1;
      sprintf(_outboundCommand, "<T %d %d>", turnoutId, thrown);
      _sendCommand();
    }
  }
}

// Route methods

int DCCEXProtocol::getRouteCount() { return _routeCount; }

bool DCCEXProtocol::receivedRouteList() { return _receivedRouteList; }

void DCCEXProtocol::startRoute(int routeId) {
  // console->println(F("sendRouteAction()"));
  if (_delegate) {
    sprintf(_outboundCommand, "</START  %d >", routeId);
    _sendCommand();
  }
  // console->println(F("sendRouteAction() end"));
}

void DCCEXProtocol::pauseRoutes() {
  // console->println(F("sendPauseRoutes()"));
  if (_delegate) {
    sprintf(_outboundCommand, "</PAUSE>");
    _sendCommand();
  }
  // console->println(F("sendPauseRoutes() end"));
}

void DCCEXProtocol::resumeRoutes() {
  // console->println(F("sendResumeRoutes()"));
  if (_delegate) {
    sprintf(_outboundCommand, "</RESUME>");
    _sendCommand();
  }
  // console->println(F("sendResumeRoutes() end"));
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
  // console->println(F("sendTurntable()"));
  if (_delegate) {
    Turntable *tt = turntables->getById(turntableId);
    if (tt) {
      if (tt->getType() == TurntableTypeEXTT) {
        if (position == 0) {
          activity = 2;
        }
        sprintf(_outboundCommand, "<I %d %d %d>", turntableId, position, activity);
      } else {
        sprintf(_outboundCommand, "<I %d %d>", turntableId, position);
      }
    }
    _sendCommand();
  }
  // console->println(F("sendTurntable() end"));
}

// Track management methods

void DCCEXProtocol::powerOn() {
  if (_delegate) {
    sprintf(_outboundCommand, "<1>");
    _sendCommand();
  }
}

void DCCEXProtocol::powerOff() {
  if (_delegate) {
    sprintf(_outboundCommand, "<0>");
    _sendCommand();
  }
}

void DCCEXProtocol::powerTrackOn(char track) {
  if (_delegate) {
    sprintf(_outboundCommand, "<1 %c>", track);
    _sendCommand();
  }
}

void DCCEXProtocol::powerTrackOff(char track) {
  if (_delegate) {
    sprintf(_outboundCommand, "<0 %c>", track);
    _sendCommand();
  }
}

void DCCEXProtocol::setTrackType(char track, TrackManagerMode type, int address) {
  if (_delegate) {
    switch (type) {
    case MAIN:
      sprintf(_outboundCommand, "<= %c MAIN>", track);
      break;
    case PROG:
      sprintf(_outboundCommand, "<= %c PROG>", track);
      break;
    case DC:
      sprintf(_outboundCommand, "<= %c DC %d>", track, address);
      break;
    case DCX:
      sprintf(_outboundCommand, "<= %c DCX %d>", track, address);
      break;
    case NONE:
      sprintf(_outboundCommand, "<= %c NONE>", track);
      break;
    default:
      return;
    }
    _sendCommand();
  }
}

// DCC accessory methods

void DCCEXProtocol::activateAccessory(int accessoryAddress, int accessorySubAddr) {
  // console->println(F("sendAccessory()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<a %d %d 1>", accessoryAddress, accessorySubAddr);
    _sendCommand();
  }
  // console->println(F("sendAccessory() end"));
}

void DCCEXProtocol::deactivateAccessory(int accessoryAddress, int accessorySubAddr) {
  // console->println(F("sendAccessory()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<a %d %d 0>", accessoryAddress, accessorySubAddr);
    _sendCommand();
  }
  // console->println(F("sendAccessory() end"));
}

void DCCEXProtocol::activateLinearAccessory(int linearAddress) {
  // console->println(F("sendAccessory()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<a %d 1>", linearAddress);
    _sendCommand();
  }
  // console->println(F("sendAccessory() end"));
}

void DCCEXProtocol::deactivateLinearAccessory(int linearAddress) {
  // console->println(F("sendAccessory()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<a %d 0>", linearAddress);
    _sendCommand();
  }
  // console->println(F("sendAccessory() end"));
}

// Private methods
// Protocol and server methods

// init the DCCEXProtocol instance after connection to the server
void DCCEXProtocol::_init() {
  // console->println(F("init()"));
  // allocate input buffer and init position variable
  memset(_inputBuffer, 0, sizeof(_inputBuffer));
  _nextChar = 0;
  // last Response time
  _lastServerResponseTime = millis();
  // console->println(F("init(): end"));
}

void DCCEXProtocol::_sendCommand() {
  if (_stream) {
    _stream->println(_outboundCommand);
    _console->print("==> ");
    _console->println(_outboundCommand);
    *_outboundCommand = 0; // clear it once it has been sent
  }
}

void DCCEXProtocol::_processCommand() {
  if (_delegate) {
    switch (DCCEXInbound::getOpcode()) {
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

    case 'j': // Throttle list response jA|O|P|R|T
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
            DCCEXInbound::isTextParameter(4)) { // Turntable position index enry
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
      _processReadResponse();
      break;

    default:
      break;
    }
  }
}

void DCCEXProtocol::_processServerDescription() { //<iDCCEX version / microprocessorType / MotorControllerType /
                                                  //buildNumber>
  // console->println(F("processServerDescription()"));
  if (_delegate) {
    char *description;
    description = (char *)malloc(strlen(DCCEXInbound::getSafeText(0)) + 1);
    sprintf(description, "%s", DCCEXInbound::getText(0));
    int versionStartAt = 7; // e.g. "DCC-EX V-"
    char *temp = _nextServerDescriptionParam(description, versionStartAt, true);
    _majorVersion = atoi(temp);
    versionStartAt = versionStartAt + strlen(temp) + 1;
    temp = _nextServerDescriptionParam(description, versionStartAt, true);
    _minorVersion = atoi(temp);
    versionStartAt = versionStartAt + strlen(temp) + 1;
    temp = _nextServerDescriptionParam(description, versionStartAt, true);
    _patchVersion = atoi(temp);
    _receivedVersion = true;
    _delegate->receivedServerVersion(_majorVersion, _minorVersion, _patchVersion);
  }
  // console->println(F("processServerDescription(): end"));
}

void DCCEXProtocol::_processMessage() { //<m "message">
  _delegate->receivedMessage(DCCEXInbound::getSafeText(0));
}

char *DCCEXProtocol::_nextServerDescriptionParam(char *description, int startAt, bool lookingAtVersionNumber) {
  char _tempString[MAX_SERVER_DESCRIPTION_PARAM_LENGTH];
  int i = 0;
  size_t j;
  bool started = false;
  for (j = startAt; j < strlen(description) && i < (MAX_SERVER_DESCRIPTION_PARAM_LENGTH - 1); j++) {
    if (started) {
      if (description[j] == ' ' || description[j] == '\0')
        break;
      if (lookingAtVersionNumber && (description[j] == '-' || description[j] == '.'))
        break;
      _tempString[i] = description[j];
      i++;
    } else {
      if (description[j] == ' ')
        started = true;
      if (lookingAtVersionNumber && (description[j] == '-' || description[j] == '.'))
        started = true;
    }
  }
  _tempString[i] = '\0';
  char *_result;
  _result = (char *)malloc(strlen(_tempString));
  sprintf(_result, "%s", _tempString);
  // console->println(_result);
  return _result;
}

// Consist/loco methods

void DCCEXProtocol::_processLocoBroadcast() { //<l cab reg speedByte functMap>
  int address = DCCEXInbound::getNumber(0);
  int speedByte = DCCEXInbound::getNumber(2);
  int functMap = _getValidFunctionMap(DCCEXInbound::getNumber(3));
  // Loco* loco=Loco::getByAddress(address);
  // if (!loco) return;
  // int speed=_getSpeedFromSpeedByte(speedByte);
  // Direction dir=_getDirectionFromSpeedByte(speedByte);
  // loco->setSpeed(speed);
  // loco->setDirection(dir);
  // loco->setFunctionStates(functMap);
  // _delegate->receivedLocoUpdate(loco);

  for (Loco *l = Loco::getFirst(); l; l = l->getNext()) {
    if (l->getAddress() == address) {
      int speed = _getSpeedFromSpeedByte(speedByte);
      Direction dir = _getDirectionFromSpeedByte(speedByte);
      l->setSpeed(speed);
      l->setDirection(dir);
      l->setFunctionStates(functMap);
      _delegate->receivedLocoUpdate(l);
    }
  }
}

int DCCEXProtocol::_getValidFunctionMap(int functionMap) {
  // Mask off anything above 28 bits/28 functions
  if (functionMap > 0xFFFFFFF) {
    functionMap &= 0xFFFFFFF;
  }
  return functionMap;
}

int DCCEXProtocol::_getSpeedFromSpeedByte(int speedByte) {
  int speed = speedByte;
  if (speed >= 128) {
    speed = speed - 128;
  }
  if (speed > 1) {
    speed = speed - 1; // get around the idiotic design of the speed command
  } else {
    speed = 0;
  }
  return speed;
}

Direction DCCEXProtocol::_getDirectionFromSpeedByte(int speedByte) { return (speedByte >= 128) ? Forward : Reverse; }

void DCCEXProtocol::_setLoco(int address, int speed, Direction direction) {
  // console->print(F("sendLocoAction(): ")); console->println(address);
  if (_delegate) {
    sprintf(_outboundCommand, "<t %d %d %d>", address, speed, direction);
    _sendCommand();
  }
  // console->println(F("sendLocoAction(): end"));
}

void DCCEXProtocol::_processReadResponse() { // <r id> - -1 = error
  int address = DCCEXInbound::getNumber(0);
  _delegate->receivedReadLoco(address);
}

// Roster methods

void DCCEXProtocol::_getRoster() {
  // console->println(F("getRoster()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<JR>");
    _sendCommand();
    _rosterRequested = true;
  }
  // console->println(F("getRoster() end"));
}

bool DCCEXProtocol::_requestedRoster() { return _rosterRequested; }

void DCCEXProtocol::_processRosterList() {
  // console->println(F("processRosterList()"));
  if (roster != nullptr) { // already have a roster so this is an update
    // console->println(F("processRosterList(): roster list already received. Ignoring this!"));
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
  // console->println(F("processRosterList(): end"));
}

void DCCEXProtocol::_requestRosterEntry(int address) {
  // console->println(F("sendRosterEntryRequest()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<JR %d>", address);
    _sendCommand();
  }
  // console->println(F("sendRosterEntryRequest(): end"));
}

void DCCEXProtocol::_processRosterEntry() { //<jR id ""|"desc" ""|"funct1/funct2/funct3/...">
  // console->println(F("processRosterEntry()"));
  // find the roster entry to update
  int address = DCCEXInbound::getNumber(1);
  char *name = DCCEXInbound::getSafeText(2);
  char *funcs = DCCEXInbound::getSafeText(3);
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
    // console->print(F("processRosterEntry(): received all: "));
    // console->println(getRosterCount());
    _delegate->receivedRosterList();
  }
  // console->println(F("processRosterEntry(): end"));
}

// Turnout methods

void DCCEXProtocol::_getTurnouts() {
  // console->println(F("getTurnouts()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<JT>");
    _sendCommand();
    _turnoutListRequested = true;
  }
  // console->println(F("getTurnouts() end"));
}

bool DCCEXProtocol::_requestedTurnouts() { return _turnoutListRequested; }

void DCCEXProtocol::_processTurnoutList() {
  // <jT id1 id2 id3 ...>
  // console->println(F("processTurnoutList()"));
  if (turnouts != nullptr) {
    // console->println(F("processTurnoutList(): Turnout/Points list already received. Ignoring this!"));
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
  // console->println(F("processTurnoutList(): end"));
}

void DCCEXProtocol::_requestTurnoutEntry(int id) {
  // console->println(F("sendTurnoutEntryRequest()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<JT %d>", id);
    _sendCommand();
  }
  // console->println(F("sendTurnoutEntryRequest() end"));
}

void DCCEXProtocol::_processTurnoutEntry() {
  if (DCCEXInbound::getParameterCount() != 4)
    return;
  // console->println(F("processTurnoutEntry()"));
  // find the turnout entry to update
  int id = DCCEXInbound::getNumber(1);
  bool thrown = (DCCEXInbound::getNumber(2) == 'T');
  char *name = DCCEXInbound::getSafeText(3);
  bool missingTurnouts = false;

  Turnout *t = turnouts->getById(id);
  if (t) {
    t->setName(name);
    t->setThrown(thrown);
    if (t->getNext() && t->getNext()->getName() == nullptr) {
      missingTurnouts = true;
      _requestTurnoutEntry(t->getNext()->getId());
    }
  }

  if (!missingTurnouts) {
    _receivedTurnoutList = true;
    // console->println(F("processTurnoutsEntry(): received all"));
    _delegate->receivedTurnoutList();
  }
  // console->println(F("processTurnoutEntry() end"));
}

void DCCEXProtocol::_processTurnoutBroadcast() { //<H id state>
  // console->println(F("processTurnoutAction(): "));
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
  // console->println(F("processTurnoutAction(): end"));
}

// Route methods

void DCCEXProtocol::_getRoutes() {
  // console->println(F("getRoutes()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<JA>");
    _sendCommand();
    _routeListRequested = true;
  }
  // console->println(F("getRoutes() end"));
}

bool DCCEXProtocol::_requestedRoutes() { return _routeListRequested; }

void DCCEXProtocol::_processRouteList() {
  // console->println(F("processRouteList()"));
  if (routes != nullptr) {
    // console->println(F("processRouteList(): Routes/Automation list already received. Ignoring this!"));
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
  // console->println(F("processRouteList(): end"));
}

void DCCEXProtocol::_requestRouteEntry(int id) {
  // console->println(F("sendRouteEntryRequest()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<JA %d>", id);
    _sendCommand();
  }
  // console->println(F("sendRouteEntryRequest() end"));
}

void DCCEXProtocol::_processRouteEntry() {
  // console->println(F("processRouteEntry()"));
  // find the Route entry to update
  int id = DCCEXInbound::getNumber(1);
  RouteType type = (RouteType)DCCEXInbound::getNumber(2);
  char *name = DCCEXInbound::getSafeText(3);
  bool missingRoutes = false;

  Route *r = routes->getById(id);
  if (r) {
    r->setType(type);
    r->setName(name);
    if (r->getNext() && r->getNext()->getName() == nullptr) {
      missingRoutes = true;
      _requestRouteEntry(r->getNext()->getId());
    }
  }

  if (!missingRoutes) {
    _receivedRouteList = true;
    // console->println(F("processRoutesEntry(): received all"));
    _delegate->receivedRouteList();
  }
  // console->println(F("processRouteEntry() end"));
}

// Turntable methods

void DCCEXProtocol::_getTurntables() {
  // console->println(F("getTurntables()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<JO>");
    _sendCommand();
    _turntableListRequested = true;
  }
  // console->println(F("getTurntables() end"));
}

bool DCCEXProtocol::_requestedTurntables() { return _turntableListRequested; }

void DCCEXProtocol::_processTurntableList() { // <jO [id1 id2 id3 ...]>
  // console->println(F("processTurntableList(): "));
  if (turntables != nullptr) { // already have a turntables list so this is an update
    // console->println(F("processTurntableList(): Turntable list already received. Ignoring this!"));
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
  // console->print("processTurntableList(): end: size:"); console->println(turntables.size());
}

void DCCEXProtocol::_requestTurntableEntry(int id) {
  // console->println(F("sendTurntableEntryRequest()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<JO %d>", id);
    _sendCommand();
  }
  // console->println(F("sendTurntableEntryRequest(): end"));
}

void DCCEXProtocol::_processTurntableEntry() { // <jO id type position position_count "[desc]">
  // console->println(F("processTurntableEntry(): "));
  // find the Turntable entry to update
  int id = DCCEXInbound::getNumber(1);
  TurntableType ttType = (TurntableType)DCCEXInbound::getNumber(2);
  int index = DCCEXInbound::getNumber(3);
  int indexCount = DCCEXInbound::getNumber(4);
  char *name = DCCEXInbound::getSafeText(5);

  Turntable *tt = turntables->getById(id);
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
  // console->println(F("processTurntableEntry(): end"));
}

void DCCEXProtocol::_requestTurntableIndexEntry(int id) {
  // console->println(F("sendTurntableIndexEntryRequest()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<JP %d>", id);
    _sendCommand();
  }
  // console->println(F("sendTurntableIndexEntryRequest() end"));
}

void DCCEXProtocol::_processTurntableIndexEntry() { // <jP id index angle "[desc]">
  // console->println(F("processTurntableIndexEntry(): "));
  if (DCCEXInbound::getParameterCount() == 5) {
    // find the Turntable entry to update
    int ttId = DCCEXInbound::getNumber(1);
    int index = DCCEXInbound::getNumber(2);
    int angle = DCCEXInbound::getNumber(3);
    char *name = DCCEXInbound::getSafeText(4);
    if (index == 0) { // Index 0 is always home, and never has a label, so set one
      sprintf(name, "Home");
    }

    Turntable *tt = getTurntableById(ttId);
    if (!tt)
      return;

    int numIndexes = tt->getNumberOfIndexes();
    int idxCount = tt->getIndexCount();

    if (numIndexes != idxCount) {
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
      // console->println(F("processTurntableIndexEntry(): received all"));
      _delegate->receivedTurntableList();
    }
  }
  // console->println(F("processTurntableIndexEntry(): end"));
}

void DCCEXProtocol::_processTurntableBroadcast() { // <I id position moving>
  // console->println(F("processTurntableAction(): "));
  int id = DCCEXInbound::getNumber(0);
  int newIndex = DCCEXInbound::getNumber(1);
  bool moving = DCCEXInbound::getNumber(2);
  Turntable *tt = getTurntableById(id);
  if (tt && tt->getIndex() != newIndex) {
    tt->setIndex(newIndex);
    tt->setMoving(moving);
  }
  _delegate->receivedTurntableAction(id, newIndex, moving);
  // console->println(F("processTurntableAction(): end"));
}

// Track management methods

void DCCEXProtocol::_processTrackPower() {
  // _console->println(F("processTrackPower()"));
  if (_delegate) {
    TrackPower state = PowerUnknown;
    if (DCCEXInbound::getNumber(0) == PowerOff) {
      state = PowerOff;
    } else if (DCCEXInbound::getNumber(0) == PowerOn) {
      state = PowerOn;
    }
    // _console->print(F("processTrackPower(): state: "));
    // _console->println(state);

    if (DCCEXInbound::getParameterCount() == 2) {
      int _track = DCCEXInbound::getNumber(1);
      // _console->print("processTrackPower(): 2nd : ");
      // _console->println(_track);
      _delegate->receivedIndividualTrackPower(state, _track);

      if (DCCEXInbound::getNumber(1) != 2698315) {
        return;
      } // not equal "MAIN"
    }
    _delegate->receivedTrackPower(state);
  }
  // _console->print(F("processTrackPower(): end"));
}

void DCCEXProtocol::_processTrackType() {
  // _console->println(F("processTrackType()"));
  if (_delegate) {
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
  // _console->println(F("processTrackType(): end"));
}