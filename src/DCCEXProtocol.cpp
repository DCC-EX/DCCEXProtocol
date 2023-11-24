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

DCCEXProtocol::DCCEXProtocol(int maxThrottles) {
  _maxThrottles=maxThrottles;
  // create array containing all Consist/throttle objects
  throttle=new Consist[_maxThrottles];
		
	// init streams
  _stream = &_nullStream;
	_console = &_nullStream;

  // setup command parser
  DCCEXInbound::setup(MAX_COMMAND_PARAMS);
  _cmdBuffer[0] = 0;
  _bufflen = 0;
}

// Set the delegate instance for callbasks
void DCCEXProtocol::setDelegate(DCCEXProtocolDelegate *delegate) {
  this->_delegate = delegate;
}

// Set the Stream used for logging
void DCCEXProtocol::setLogStream(Stream *console) {
  this->_console = console;
}

void DCCEXProtocol::connect(Stream *stream) {
  _init();
  this->_stream = stream;
}

void DCCEXProtocol::disconnect() {
  sprintf(_outboundCommand,"%s","<U DISCONNECT>");
  _sendCommand();
  this->_stream = nullptr;
}

void DCCEXProtocol::check() {
  if (_stream) {
    while(_stream->available()) {
      // Read from our stream
      int r=_stream->read();
      if (_bufflen<MAX_COMMAND_BUFFER-1) {
        _cmdBuffer[_bufflen]=r;
        _bufflen++;
        _cmdBuffer[_bufflen]=0;
      }

      if (r=='>') {
        if (DCCEXInbound::parse(_cmdBuffer)) {
          // Process stuff here
          processCommand();
        }
        // Clear buffer after use
        _cmdBuffer[0]=0;
        _bufflen=0;
      }
    }
  }
}

// sequentially request and get the required lists. To avoid overloading the buffer
void DCCEXProtocol::getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired) {
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

bool DCCEXProtocol::receivedLists() {
  return _receivedLists;
}

void DCCEXProtocol::requestServerVersion() {
  // console->println(F("requestServerVersion(): "));
  if (_delegate) {
    sprintf(_outboundCommand, "<s>");
    _sendCommand();        
  }
  // console->println(F("requestServerVersion(): end"));
}

bool DCCEXProtocol::receivedVersion() {
    return _receivedVersion;
}

unsigned long DCCEXProtocol::getLastServerResponseTime() {
  return _lastServerResponseTime;   
}

// Consist/loco methods

void DCCEXProtocol::setThrottle(int throttleNo, int speed, Direction direction) {
  // console->println(F("sendThrottleAction(): "));
  if (_delegate) {
    if (throttle[throttleNo].getLocoCount()>0) {
      throttle[throttleNo].setSpeed(speed);
      throttle[throttleNo].setDirection(direction);
      for (ConsistLoco* conLoco=throttle[throttleNo].getFirst(); conLoco; conLoco=conLoco->getNext()) {
        int address = conLoco->getAddress();
        Direction dir = direction;
        if (conLoco->getFacing()==FacingReversed) {
          if (direction==Forward) {
            dir = Reverse;
          } else {
            dir = Forward;
          }                    
        }
        _setLoco(address, speed, dir);
      }
    }
  }
  // console->println(F("sendThrottleAction(): end"));
}

void DCCEXProtocol::setFunction(int throttleNo, int functionNumber, bool pressed) {
  // console->println(F("sendFunction(): "));
  if (_delegate) {
    for (ConsistLoco* conLoco=throttle[throttleNo].getFirst(); conLoco; conLoco=conLoco->getNext()) {
      int address = conLoco->getAddress();
      if (address>=0) {
        sprintf(_outboundCommand, "<F %d %d %d>", address, functionNumber, pressed);
        _sendCommand();
      }
    }
  }
  // console->println(F("sendFunction(): end"));
}

bool DCCEXProtocol::functionOn(int throttleNo, int functionNumber) {
  if (_delegate) {
    // ConsistLoco* conLoco = throttle[throttleNo].getLocoAtPosition(0);
    ConsistLoco* conLoco = throttle[throttleNo].getFirst();
    int address = conLoco->getAddress();
    if (address>=0) {
      // console->print(" '");
      // console->print(conLoco->isFunctionOn(functionNumber));
      // console->print("' ");
      conLoco->functionOn(functionNumber);
    }
    return conLoco->functionOn(functionNumber);
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

Consist DCCEXProtocol::getConsist(int throttleNo) {
  // console->println(F("getThrottleConsist(): "));
  if (_delegate) {
    return throttle[throttleNo];
  }
  // console->println(F("getThrottleConsist(): end"));
  return {};
}

// Roster methods

int DCCEXProtocol::getRosterCount() {
  return _rosterCount;
}

Loco* DCCEXProtocol::getRosterEntryNo(int entryNo) {
  int i=0;
  for (Loco* loco=roster->getFirst(); loco; loco=loco->getNext()) {
    if (i==entryNo) return loco;
    i++;
  }
  return {};
}

bool DCCEXProtocol::receivedRoster() {
  // console->println(F("isRosterFullyReceived()"));
  return _receivedRoster;
}

Loco* DCCEXProtocol::findLocoInRoster(int address) {
  for (Loco* r=roster->getFirst(); r; r=r->getNext()) {
    if (r->getAddress()==address) {
      return r;
    }
  }
  return nullptr;
}

// Turnout methods

int DCCEXProtocol::getTurnoutCount() {
  return _turnoutCount;
}

Turnout* DCCEXProtocol::getTurnoutEntryNo(int entryNo) {
  int i=0;
  for (Turnout* turnout=turnouts->getFirst(); turnout; turnout=turnout->getNext()) {
    if (i==entryNo) return turnout;
    i++;
  }
  return {};
}

bool DCCEXProtocol::receivedTurnoutList() {
  return _receivedTurnoutList;
}

// find the turnout/point in the turnout list by id. return a pointer or null is not found
Turnout* DCCEXProtocol::getTurnoutById(int turnoutId) {
  for (Turnout* turnout=turnouts->getFirst(); turnout; turnout=turnout->getNext()) {
    if (turnout->getId() == turnoutId) {
      return turnout;
    }
  }
  return nullptr;  // not found
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
  for (Turnout* t=turnouts->getFirst(); t; t=t->getNext()) {
    if (t->getId()==turnoutId) {
      // console->println(t->getThrown());
      bool thrown=t->getThrown() ? 0 : 1;
      sprintf(_outboundCommand, "<T %d %d>", turnoutId, thrown);
      _sendCommand();
    }
  }
}

// Route methods

int DCCEXProtocol::getRouteCount() {
  return _routeCount;
}

Route* DCCEXProtocol::getRouteEntryNo(int entryNo) {
  int i=0;
  for (Route* route=routes->getFirst(); route; route=route->getNext()) {
    if (i==entryNo) return route;
    i++;
  }
  return {};
}

bool DCCEXProtocol::receivedRouteList() {
  return _receivedRouteList;
}

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

int DCCEXProtocol::getTurntableCount() {
  return _turntableCount;
}

bool DCCEXProtocol::receivedTurntableList() {
  return _receivedTurntableList;
}

Turntable* DCCEXProtocol::getTurntableById(int turntableId) {
  for (Turntable* tt=turntables->getFirst(); tt; tt=tt->getNext()) {
    if (tt->getId() == turntableId) {
      return tt;
    }
  }
  return nullptr;
}

void DCCEXProtocol::rotateTurntable(int turntableId, int position, int activity) {
  // console->println(F("sendTurntable()"));
  if (_delegate) {
    Turntable* tt=turntables->getById(turntableId);
    if (tt) {
      if (tt->getType()==TurntableTypeEXTT) {
        if (position==0) {
            activity=2;
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
// Server and protocol methods
// init the DCCEXProtocol instance after connection to the server
void DCCEXProtocol::_init() {
  // console->println(F("init()"));
	// allocate input buffer and init position variable
	memset(_inputBuffer, 0, sizeof(_inputBuffer));
	_nextChar = 0;
  //last Response time
  _lastServerResponseTime = millis();
  // console->println(F("init(): end"));
}







// ******************************************************************************************************
//helper functions

Direction DCCEXProtocol::_getDirectionFromSpeedByte(int speedByte) {
  return (speedByte>=128) ? Forward : Reverse; 
}

int DCCEXProtocol::_getSpeedFromSpeedByte(int speedByte) {
  int speed = speedByte;
  if (speed >= 128) {
    speed = speed - 128;
  }
  if (speed>1) {
    speed = speed - 1; // get around the idiotic design of the speed command
  } else {
    speed=0;
  }
  return speed;
}

int DCCEXProtocol::_getValidFunctionMap(int functionMap) {
  // Mask off anything above 28 bits/28 functions
  if (functionMap > 0xFFFFFFF) {
    functionMap &= 0xFFFFFFF;
  }
  return functionMap;
  // This needs to set the current loco function map now
}



//private
void DCCEXProtocol::_sendCommand() {
  if (_stream) {
    _stream->println(_outboundCommand);
    _console->print("==> ");
    _console->println(_outboundCommand);
    *_outboundCommand = 0; // clear it once it has been sent
  }
}

//private
void DCCEXProtocol::processCommand() {
  if (_delegate) {
    switch (DCCEXInbound::getOpcode()) {
      case 'i':   // iDCC-EX server info
        if (DCCEXInbound::isTextParameter(0)) {
          processServerDescription();
        }
        break;
      
      case 'I':   // Turntable broadcast
        if (DCCEXInbound::getParameterCount()==3) {
          processTurntableAction();
        }
        break;

      case 'p':   // Power broadcast
        if (DCCEXInbound::isTextParameter(0) || DCCEXInbound::getParameterCount()!=1) break;
        processTrackPower();
        break;

      case 'l':   // Loco/cab broadcast
        if (DCCEXInbound::isTextParameter(0) || DCCEXInbound::getParameterCount()!=4) break;
        processLocoAction();
        break;

      case 'j':   // Throttle list response jA|O|P|R|T
        if (DCCEXInbound::isTextParameter(0)) break;
        if (DCCEXInbound::getNumber(0)=='A') {          // Receive route/automation info
          if (DCCEXInbound::getParameterCount()==0) { // Empty list, no routes/automations
            _receivedRouteList = true;
          } else if (DCCEXInbound::getParameterCount()==4 && DCCEXInbound::isTextParameter(3)) {  // Receive route entry
            processRouteEntry();
          } else {    // Receive route/automation list
            processRouteList();
          }
        } else if (DCCEXInbound::getNumber(0)=='O') {   // Receive turntable info
          if (DCCEXInbound::getParameterCount()==0) { // Empty turntable list
            _receivedTurntableList = true;
          } else if (DCCEXInbound::getParameterCount()==6 && DCCEXInbound::isTextParameter(5)) {  // Turntable entry
            processTurntableEntry();
          } else {    // Turntable list
            processTurntableList();
          }
        } else if (DCCEXInbound::getNumber(0)=='P') {   // Receive turntable position info
          if (DCCEXInbound::getParameterCount()==5 && DCCEXInbound::isTextParameter(4)) { // Turntable position index enry
            processTurntableIndexEntry();
          }
        } else if (DCCEXInbound::getNumber(0)=='R') {   // Receive roster info
          if (DCCEXInbound::getParameterCount()==1) { // Empty list, no roster
            _receivedRoster = true;
          } else if (DCCEXInbound::getParameterCount()==4 && DCCEXInbound::isTextParameter(2) && DCCEXInbound::isTextParameter(3)) {  // Roster entry
            // <jR id "desc" "func1/func2/func3/...">
            processRosterEntry();
          } else {    // Roster list
            // <jR id1 id2 id3 ...>
            processRosterList();
          }
        } else if (DCCEXInbound::getNumber(0)=='T') {   // Receive turnout info
          if (DCCEXInbound::getParameterCount()==1) { // Empty list, no turnouts defined
            _receivedTurnoutList = true;
          } else if (DCCEXInbound::getParameterCount()==4 && DCCEXInbound::isTextParameter(3)) {  // Turnout entry
            // <jT id state "desc">
            processTurnoutEntry();
          } else {    // Turnout list
            // <jT id1 id2 id3 ...>
            processTurnoutList();
          }
        }
        break;

      case 'H':   // Turnout broadcast
        if (DCCEXInbound::isTextParameter(0)) break;
        processTurnoutAction();
        break;

      case 'q':   // Sensor broadcast
        if (DCCEXInbound::isTextParameter(0)) break;
        processSensorEntry();
        break;

      case 'r':   // Read loco response
        if (DCCEXInbound::isTextParameter(0)) break;
        processReadLoco();
        break;

      default:
        break;
    }
  }
}

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// Process responses from the CS

//private
void DCCEXProtocol::processServerDescription() { //<iDCCEX version / microprocessorType / MotorControllerType / buildNumber>
  // console->println(F("processServerDescription()"));
  if (_delegate) {
    // console->print(F("Process server description with "));
    // console->print(DCCEXInbound::getParameterCount());
    // console->println(F(" params"));
    
    char *_serverDescription;
    _serverDescription = (char *) malloc(strlen(DCCEXInbound::getText(0))+1);
    sprintf(_serverDescription,"%s",DCCEXInbound::getText(0));

    int versionStartAt = 7; // e.g. "DCC-EX V-"
    char* temp=_nextServerDescriptionParam(versionStartAt, true);
    _majorVersion=atoi(temp);
    versionStartAt=versionStartAt + strlen(temp)+1;
    temp=_nextServerDescriptionParam(versionStartAt, true);
    _minorVersion=atoi(temp);
    versionStartAt=versionStartAt + strlen(temp)+1;
    temp=_nextServerDescriptionParam(versionStartAt, true);
    _patchVersion=atoi(temp);

    _receivedVersion = true;
    _delegate->receivedServerVersion(_majorVersion, _minorVersion, _patchVersion);
  }
  // console->println(F("processServerDescription(): end"));
}



//private
void DCCEXProtocol::processTrackPower() {
  // console->println(F("processTrackPower()"));
  if (_delegate) {
    TrackPower state = PowerUnknown;
    if (DCCEXInbound::getNumber(0)==PowerOff) {
      state = PowerOff;
    } else if (DCCEXInbound::getNumber(0)==PowerOn) {
      state = PowerOn;
    }
    _delegate->receivedTrackPower(state);
  }
  // console->println(F("processTrackPower(): end"));
}

// ****************
// roster

//private
void DCCEXProtocol::processRosterList() {
  // console->println(F("processRosterList()"));
  if (roster!=nullptr) {  // already have a roster so this is an update
    // console->println(F("processRosterList(): roster list already received. Ignoring this!"));
    return;
  }
  if (DCCEXInbound::getParameterCount()==1) { // roster empty
    _receivedRoster=true;
    return;
  }
  for (int i=1; i<DCCEXInbound::getParameterCount(); i++) {
    int address = DCCEXInbound::getNumber(i);
    new Loco(address, LocoSourceRoster);
  }
  sendRosterEntryRequest(Loco::getFirst()->getAddress());
  _rosterCount = DCCEXInbound::getParameterCount()-1;
  // console->println(F("processRosterList(): end"));
}

//private
void DCCEXProtocol::sendRosterEntryRequest(int address) {
  // console->println(F("sendRosterEntryRequest()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<JR %d>", address);
    _sendCommand();
  }
  // console->println(F("sendRosterEntryRequest(): end"));
}

//private
void DCCEXProtocol::processRosterEntry() { //<jR id ""|"desc" ""|"funct1/funct2/funct3/...">
  // console->println(F("processRosterEntry()"));
  //find the roster entry to update
  int address=DCCEXInbound::getNumber(1);
  char *name=DCCEXInbound::getSafeText(2);
  char *funcs=DCCEXInbound::getSafeText(3);
  bool missingRosters=false;
  
  Loco* loco=roster->getByAddress(address);
  if (loco) {
    loco->setName(name);
    loco->setupFunctions(funcs);
    if (loco->getNext() && loco->getNext()->getName()==nullptr) {
      missingRosters=true;
      sendRosterEntryRequest(loco->getNext()->getAddress());
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

void DCCEXProtocol::processReadLoco() { // <r id> - -1 = error
  int address=DCCEXInbound::getNumber(0);
  _delegate->receivedReadLoco(address);
}

// ****************
// Turnouts/Points

//private
void DCCEXProtocol::processTurnoutList() {
  // <jT id1 id2 id3 ...>
  // console->println(F("processTurnoutList()"));
  if (turnouts!=nullptr) {
    // console->println(F("processTurnoutList(): Turnout/Points list already received. Ignoring this!"));
    return;
  } 
  if (DCCEXInbound::getParameterCount()==1) { // turnout list is empty
    _receivedTurnoutList=true;
    return;
  }
  for (int i=1; i<DCCEXInbound::getParameterCount(); i++) {
    auto id = DCCEXInbound::getNumber(i);
    new Turnout(id, false);
  }
  sendTurnoutEntryRequest(Turnout::getFirst()->getId());
  _turnoutCount = DCCEXInbound::getParameterCount()-1;
  // console->println(F("processTurnoutList(): end"));
}

//private
void DCCEXProtocol::sendTurnoutEntryRequest(int id) {
  // console->println(F("sendTurnoutEntryRequest()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<JT %d>", id);
    _sendCommand();
  }
  // console->println(F("sendTurnoutEntryRequest() end"));
}

//private
void DCCEXProtocol::processTurnoutEntry() {
  if (DCCEXInbound::getParameterCount()!=4) return;
  // console->println(F("processTurnoutEntry()"));
  //find the turnout entry to update
  int id=DCCEXInbound::getNumber(1);
  bool thrown=(DCCEXInbound::getNumber(2)=='T');
  char* name=DCCEXInbound::getSafeText(3);
  bool missingTurnouts=false;

  Turnout* t=turnouts->getById(id);
  if (t) {
    t->setName(name);
    t->setThrown(thrown);
    if (t->getNext() && t->getNext()->getName()==nullptr) {
      missingTurnouts=true;
      sendTurnoutEntryRequest(t->getNext()->getId());
    }
  }

  if (!missingTurnouts) {
    _receivedTurnoutList=true;
    // console->println(F("processTurnoutsEntry(): received all"));
    _delegate->receivedTurnoutList();
  }
  // console->println(F("processTurnoutEntry() end"));
}







//private
void DCCEXProtocol::processTurnoutAction() { //<H id state>
  // console->println(F("processTurnoutAction(): "));
  if (DCCEXInbound::getParameterCount()!=2) return;
  //find the Turnout entry to update
  int id = DCCEXInbound::getNumber(0);
  bool thrown = DCCEXInbound::getNumber(1);
  for (auto t=Turnout::getFirst(); t ; t=t->getNext()) {
    if (t->getId()==id) {
      t->setThrown(thrown);
      _delegate->receivedTurnoutAction(id, thrown);
    }
  }
  // console->println(F("processTurnoutAction(): end"));
}

// ****************
// Routes

//private
void DCCEXProtocol::processRouteList() {
  // console->println(F("processRouteList()"));
  if (routes!=nullptr) {
    // console->println(F("processRouteList(): Routes/Automation list already received. Ignoring this!"));
    return;
  } 
  if (DCCEXInbound::getParameterCount()==1) { // route list is empty
    _receivedRouteList=true;
    return;
  }
  for (int i=1; i<DCCEXInbound::getParameterCount(); i++) {
    int id = DCCEXInbound::getNumber(i);
    new Route(id);
  }
  sendRouteEntryRequest(Route::getFirst()->getId());
  _routeCount = DCCEXInbound::getParameterCount()-1;
  // console->println(F("processRouteList(): end"));
}

//private
void DCCEXProtocol::sendRouteEntryRequest(int id) {
  // console->println(F("sendRouteEntryRequest()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<JA %d>", id);
    _sendCommand();
  }
  // console->println(F("sendRouteEntryRequest() end"));
}

//private
void DCCEXProtocol::processRouteEntry() {
  // console->println(F("processRouteEntry()"));
  //find the Route entry to update
  int id=DCCEXInbound::getNumber(1);
  RouteType type=(RouteType)DCCEXInbound::getNumber(2);
  char* name=DCCEXInbound::getSafeText(3);
  bool missingRoutes = false;

  Route* r=routes->getById(id);
  if (r) {
    r->setType(type);
    r->setName(name);
    if (r->getNext() && r->getNext()->getName()==nullptr) {
      missingRoutes=true;
      sendRouteEntryRequest(r->getNext()->getId());
    }
  }

  if (!missingRoutes) {
    _receivedRouteList=true;
    // console->println(F("processRoutesEntry(): received all"));
    _delegate->receivedRouteList();
  }
  // console->println(F("processRouteEntry() end"));
}

// ****************
// Turntables

void DCCEXProtocol::processTurntableList() {  // <jO [id1 id2 id3 ...]>
  // console->println(F("processTurntableList(): "));
  if (turntables!=nullptr) {  // already have a turntables list so this is an update
    // console->println(F("processTurntableList(): Turntable list already received. Ignoring this!"));
    return;
  }
  if (DCCEXInbound::getParameterCount()==1) { // list is empty so we have received it
    _receivedTurntableList=true;
    return;
  }
  for (int i=1; i<DCCEXInbound::getParameterCount(); i++) {
    int id = DCCEXInbound::getNumber(i);
    new Turntable(id);
  }
  sendTurntableEntryRequest(Turntable::getFirst()->getId());
  _turntableCount = DCCEXInbound::getParameterCount()-1;
  // console->print("processTurntableList(): end: size:"); console->println(turntables.size());
}

//private
void DCCEXProtocol::sendTurntableEntryRequest(int id) {
  // console->println(F("sendTurntableEntryRequest()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<JO %d>", id);
    _sendCommand();
  }
  // console->println(F("sendTurntableEntryRequest(): end"));
}

//private
void DCCEXProtocol::sendTurntableIndexEntryRequest(int id) {
  // console->println(F("sendTurntableIndexEntryRequest()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<JP %d>", id);
    _sendCommand();
  }
  // console->println(F("sendTurntableIndexEntryRequest() end"));
}

//private
void DCCEXProtocol::processTurntableEntry() {  // <jO id type position position_count "[desc]">
  // console->println(F("processTurntableEntry(): "));
  //find the Turntable entry to update
  int id=DCCEXInbound::getNumber(1);
  TurntableType ttType=(TurntableType)DCCEXInbound::getNumber(2);
  int index=DCCEXInbound::getNumber(3);
  int indexCount=DCCEXInbound::getNumber(4);
  char *name=DCCEXInbound::getSafeText(5);

  Turntable* tt=turntables->getById(id);
  if (tt) {
    tt->setType(ttType);
    tt->setIndex(index);
    tt->setNumberOfIndexes(indexCount);
    tt->setName(name);
    sendTurntableIndexEntryRequest(id);
    if (tt->getNext() && tt->getNext()->getName()==nullptr) {
        sendTurntableEntryRequest(tt->getNext()->getId());
    }
  }
  // console->println(F("processTurntableEntry(): end"));
}

//private
void DCCEXProtocol::processTurntableIndexEntry() { // <jP id index angle "[desc]">
  // console->println(F("processTurntableIndexEntry(): "));
  if (DCCEXInbound::getParameterCount()==5) {
    //find the Turntable entry to update
    int ttId=DCCEXInbound::getNumber(1);
    int index=DCCEXInbound::getNumber(2);
    int angle=DCCEXInbound::getNumber(3);
    char *name=DCCEXInbound::getSafeText(4);
    if (index==0) { // Index 0 is always home, and never has a label, so set one
      sprintf(name, "Home");
    }

    Turntable* tt=getTurntableById(ttId);
    if (!tt) return;
    
    int numIndexes=tt->getNumberOfIndexes();
    int idxCount=tt->getIndexCount();
    
    if (numIndexes!=idxCount) {
      TurntableIndex* newIndex=new TurntableIndex(ttId, index, angle, name);
      tt->addIndex(newIndex);
    }

    bool receivedAll=true;

    for (Turntable* tt=turntables->getFirst(); tt; tt=tt->getNext()) {
      int numIndexes=tt->getNumberOfIndexes();
      int indexCount=tt->getIndexCount();
      if (tt->getName()==nullptr || (numIndexes!=indexCount)) receivedAll=false;
    }

    if (receivedAll) {
      _receivedTurntableList = true;
      // console->println(F("processTurntableIndexEntry(): received all"));
      _delegate->receivedTurntableList();
    }      
  }
  // console->println(F("processTurntableIndexEntry(): end"));
}

//private
void DCCEXProtocol::processTurntableAction() { // <I id position moving>
    // console->println(F("processTurntableAction(): "));
    int id=DCCEXInbound::getNumber(0);
    int newIndex=DCCEXInbound::getNumber(1);
    bool moving=DCCEXInbound::getNumber(2);
    Turntable* tt=getTurntableById(id);
    if (tt && tt->getIndex()!=newIndex) {
        tt->setIndex(newIndex);
        tt->setMoving(moving);
    }
    _delegate->receivedTurntableAction(id, newIndex, moving);
    // console->println(F("processTurntableAction(): end"));
}

//private
void DCCEXProtocol::processSensorEntry() {  // <jO id type position position_count "[desc]">
    // console->println(F("processSensorEntry(): "));
    if (_delegate) {
        //????????????????? TODO
        // console->println(F("processSensorEntry(): Not doing anything with these yet"));
    }
    // console->println(F("processSensorEntry(): end"));
}

// ****************
// Locos

//private
bool DCCEXProtocol::processLocoAction() { //<l cab reg speedByte functMap>
  // console->println(F("processLocoAction()"));
  int address = DCCEXInbound::getNumber(0);
  int speedByte = DCCEXInbound::getNumber(2);
  int functMap = _getValidFunctionMap(DCCEXInbound::getNumber(3));
  int throttleNo = findThrottleWithLoco(address);
  if (throttleNo>=0) {
    ConsistLoco* loco=throttle[throttleNo].getFirst();
    if (loco->getAddress()==address) {    // ignore everything that is not the lead loco
      int speed = _getSpeedFromSpeedByte(speedByte);
      Direction dir = _getDirectionFromSpeedByte(speedByte);
      int currentFunc = loco->getFunctionStates();
      if (functMap != currentFunc) {
        int funcChanges=currentFunc^functMap;
        for (int f=0; f<MAX_FUNCTIONS; f++) {
          if (funcChanges & (1<<f)) {
                bool newState = functMap & (1<<f);
                _delegate->receivedFunction(throttleNo, f, newState);
            }
        }
        loco->setFunctionStates(functMap);
      }
      throttle[throttleNo].setSpeed(speed);
      throttle[throttleNo].setDirection(dir);

      _delegate->receivedSpeed(throttleNo, speed);
      _delegate->receivedDirection(throttleNo, dir);
    }
  } else {
    // console->println(F("processLocoAction(): unknown loco"));
    return false;
  }
  // console->println(F("processLocoAction() end"));
  return true;
}

// ******************************************************************************************************
// server commands



// ******************************************************************************************************
// power commands




















void DCCEXProtocol::_setLoco(int address, int speed, Direction direction) {
  // console->print(F("sendLocoAction(): ")); console->println(address);
  if (_delegate) {
    sprintf(_outboundCommand, "<t %d %d %d>", address, speed, direction);
    _sendCommand();
  }
  // console->println(F("sendLocoAction(): end"));
}




















void DCCEXProtocol::_getRoster() {
  // console->println(F("getRoster()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<JR>");
    _sendCommand();
    _rosterRequested = true;
  }
  // console->println(F("getRoster() end"));
}





bool DCCEXProtocol::isRosterRequested() {
  return _rosterRequested;
}


void DCCEXProtocol::_getTurnouts() {
  // console->println(F("getTurnouts()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<JT>");
    _sendCommand();
    _turnoutListRequested = true;
  }
  // console->println(F("getTurnouts() end"));
}





bool DCCEXProtocol::isTurnoutListRequested() {
  return _turnoutListRequested;
}


void DCCEXProtocol::_getRoutes() {
  // console->println(F("getRoutes()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<JA>");
    _sendCommand();
    _routeListRequested = true;
  }
  // console->println(F("getRoutes() end"));
}



bool DCCEXProtocol::isRouteListRequested() {
  return _routeListRequested;
}


void DCCEXProtocol::_getTurntables() {
  // console->println(F("getTurntables()"));
  if (_delegate) {
    sprintf(_outboundCommand, "<JO>");
    _sendCommand();
    _turntableListRequested = true;
  }
  // console->println(F("getTurntables() end"));
}



bool DCCEXProtocol::isTurntableListRequested() {
  return _turntableListRequested;
}







// private
// find which, if any, throttle has this loco selected
int DCCEXProtocol::findThrottleWithLoco(int address) {
    // console->println(F("findThrottleWithLoco()"));
    for (int i=0; i<_maxThrottles; i++) {
        // if (throttle[i].consistGetNumberOfLocos()>0) {
        //     int pos = throttle[i].consistGetLocoPosition(address);

        //     // console->print(F("checking consist: ")); console->print(i); console->print(" found: "); console->println(pos);
        //     // console->print(F("in consist: ")); console->println(throttle[i].consistGetNumberOfLocos()); 

        //     // for (int j=0; j<throttle[i].consistGetNumberOfLocos(); j++ ) {
        //     //      console->print(F("checking consist X: ")); console->print(j); console->print(" is: "); console->println(throttle[i].consistLocos.get(i)->getLocoAddress());
        //     // }    

        //     if (pos>=0) {
        //         // console->println(F("findThrottleWithLoco(): end. found"));
        //         return i;
        //     }
        // }
        if (throttle[i].getLocoCount()>0) {
            // int pos=throttle[i].getLocoPosition(address);
            for (ConsistLoco* cl=throttle[i].getFirst(); cl; cl=cl->getNext()) {
                if (cl->getAddress()==address) return i;
            }
        }
    }
    // console->println(F("findThrottleWithLoco(): end. not found"));
    return -1;  //not found
}

char* DCCEXProtocol::_nextServerDescriptionParam(int startAt, bool lookingAtVersionNumber) {
  char _tempString[MAX_SERVER_DESCRIPTION_PARAM_LENGTH];
  int i = 0; 
  size_t j;
  bool started = false;
  for (j=startAt; j<strlen(_serverDescription) && i<(MAX_SERVER_DESCRIPTION_PARAM_LENGTH-1); j++) {
    if (started) {
      if (_serverDescription[j]==' ' || _serverDescription[j]=='\0') break;
      if (lookingAtVersionNumber && (_serverDescription[j]=='-' || _serverDescription[j]=='.')) break;
      _tempString[i] = _serverDescription[j];
      i++;
    } else {
      if (_serverDescription[j]==' ') started=true;
      if (lookingAtVersionNumber && (_serverDescription[j]=='-' || _serverDescription[j]=='.')) started=true;
    }
  }
  _tempString[i] = '\0';
  
  char *_result;
  _result = (char *) malloc(strlen(_tempString));
  sprintf(_result, "%s", _tempString);
  // console->println(_result);
  return _result;
}
