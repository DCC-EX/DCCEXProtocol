/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCC-EX protocol connection,
 * allow a device to communicate with a DCC-EX EX CommnadStation
 *
 * Copyright © 2023 - Peter Akers
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
- received... = notificaion to the client app (delegate)
- send... = sends a command to the CS
- process... - process a response from the CS
- init... - initialise an object
- set... = Sets internal variables/values. May subsequently call a 'send' 
- get... - Gets internal variables/values. May subsequently call a 'send'
*/

#include "DCCEXProtocol.h"

static const int MIN_SPEED = 0;
static const int MAX_SPEED = 126;


// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************

DCCEXProtocol::DCCEXProtocol(int maxThrottles, unsigned long retrievalDelay, bool server) {
	// store server/client
    this->server = server;
    _maxThrottles=maxThrottles;
    _retrievalDelay=retrievalDelay;
    throttle=new Consist[_maxThrottles];
		
	// init streams
    stream = &nullStream;
	console = &nullStream;

    DCCEXInbound::setup(MAX_COMMAND_PARAMS);
    cmdBuffer[0] = 0;
    bufflen = 0;
}

// ******************************************************************************************************
// Connection related

//private
// init the DCCEXProtocol instance after connection to the server
void DCCEXProtocol::init() {
    console->println(F("init()"));
    
	// allocate input buffer and init position variable
	memset(inputbuffer, 0, sizeof(inputbuffer));
	nextChar = 0;
	
    //last Response time
    lastServerResponseTime = millis() /1000;

    // console->println(F("init(): end"));
}

// Set the delegate instance for callbasks
void DCCEXProtocol::setDelegate(DCCEXProtocolDelegate *delegate) {
    this->delegate = delegate;
}

// Set the Stream used for logging
void DCCEXProtocol::setLogStream(Stream *console) {
    this->console = console;
}

void DCCEXProtocol::connect(Stream *stream) {
    init();
    this->stream = stream;
    _delayUntil=millis()+_retrievalDelay;
}

void DCCEXProtocol::disconnect() {
    sprintf(outboundCommand,"%s","<U DISCONNECT>");
    sendCommand();
    this->stream = NULL;
}

bool DCCEXProtocol::check() {
    // console->println(F("check()"));
    bool changed = false;

    if (stream) {
        while(stream->available()) {
            // Read from our stream
            int r=stream->read();
            if (bufflen<MAX_COMMAND_BUFFER-1) {
                cmdBuffer[bufflen]=r;
                bufflen++;
                cmdBuffer[bufflen]=0;
            }

            if (r=='>') {
                if (DCCEXInbound::parse(cmdBuffer)) {
                    // Process stuff here
                    processCommand();
                } else {
                    // Parsing failed here
                }
                
                // Clear buffer after use
                cmdBuffer[0]=0;
                bufflen=0;
            }
            // console->println(F("check(): end-loop"));
        }
        // console->println(F("check(): end-stream"));
        return changed;
    }
    else {
        // console->println(F("check(): end"));
        return false;
    }
}

long DCCEXProtocol::getLastServerResponseTime() {
  return lastServerResponseTime;   
}

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************

// ******************************************************************************************************
//helper functions

Direction DCCEXProtocol::getDirectionFromSpeedByte(int speedByte) {
    return (speedByte>=128) ? Forward : Reverse; 
}

int DCCEXProtocol::getSpeedFromSpeedByte(int speedByte) {
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

int DCCEXProtocol::getValidFunctionMap(int functionMap) {
    // Mask off anything above 28 bits/28 functions
    if (functionMap > 0xFFFFFFF) {
        functionMap &= 0xFFFFFFF;
    }
    return functionMap;
    // This needs to set the current loco function map now
}

// ******************************************************************************************************
// sending and receiving commands from the CS

//private
void DCCEXProtocol::sendCommand() {
    if (stream) {
        // TODO: what happens when the write fails?
        stream->println(outboundCommand);
        if (server) {
            stream->println("");
        }
        console->print("==> "); console->println(outboundCommand);

        *outboundCommand = 0; // clear it once it has been sent
    }
}

//private
void DCCEXProtocol::processCommand() {
    if (delegate) {
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
                        routeListFullyReceived = true;
                    } else if (DCCEXInbound::getParameterCount()==4 && DCCEXInbound::isTextParameter(3)) {  // Receive route entry
                        processRouteEntry();
                    } else {    // Receive route/automation list
                        processRouteList();
                    }
                } else if (DCCEXInbound::getNumber(0)=='O') {   // Receive turntable info
                    if (DCCEXInbound::getParameterCount()==0) { // Empty turntable list
                        turntableListFullyReceived = true;
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
                        rosterFullyReceived = true;
                    } else if (DCCEXInbound::getParameterCount()==4 && DCCEXInbound::isTextParameter(2) && DCCEXInbound::isTextParameter(3)) {  // Roster entry
                        // <jR id "desc" "func1/func2/func3/...">
                        processRosterEntry();
                    } else {    // Roster list
                        // <jR id1 id2 id3 ...>
                        processRosterList();
                    }
                } else if (DCCEXInbound::getNumber(0)=='T') {   // Receive turnout info
                    if (DCCEXInbound::getParameterCount()==1) { // Empty list, no turnouts defined
                        turnoutListFullyReceived = true;
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
    if (delegate) {
        // console->print(F("Process server description with "));
        // console->print(DCCEXInbound::getParameterCount());
        // console->println(F(" params"));
        
        char *_serverDescription;
        _serverDescription = (char *) malloc(strlen(DCCEXInbound::getText(0))+1);
        sprintf(_serverDescription,"%s",DCCEXInbound::getText(0));
        serverDescription = _serverDescription;
        console->println(serverDescription);

        int startAt = 6 ;
        serverVersion = nextServerDescriptionParam(startAt, false);
        console->println(serverVersion);
        startAt = startAt + strlen(serverVersion)+2; // get past the " / "


        int versionStartAt = 7; // e.g. "DCC-EX V-"
        serverVersionMajor = nextServerDescriptionParam(versionStartAt, true);
        versionStartAt = versionStartAt + strlen(serverVersionMajor)+1;
        serverVersionMinor = nextServerDescriptionParam(versionStartAt, true);
        versionStartAt = versionStartAt + strlen(serverVersionMinor)+1;
        serverVersionPatch = nextServerDescriptionParam(versionStartAt, true);


        serverMicroprocessorType = nextServerDescriptionParam(startAt, false);
        startAt = startAt + strlen(serverMicroprocessorType)+3; // get past the " / "

        serverMotorcontrollerType = nextServerDescriptionParam(startAt, false);
        startAt = startAt + strlen(serverMotorcontrollerType)+2; // get past the " / "

        serverBuildNumber = nextServerDescriptionParam(startAt, false);

        console->println(serverVersion);
        console->println(serverVersionMajor);
        console->println(serverVersionMinor);
        console->println(serverVersionPatch);
        console->println(serverMicroprocessorType);
        console->println(serverMotorcontrollerType);
        console->println(serverBuildNumber);

        haveReceivedServerDetails = true;
        delegate->receivedServerDescription(serverVersion);
    }
    // console->println(F("processServerDescription(): end"));
}

bool DCCEXProtocol::isServerDetailsReceived() {
    return haveReceivedServerDetails;
}

//private
void DCCEXProtocol::processTrackPower() {
    // console->println(F("processTrackPower()"));
    if (delegate) {
        TrackPower state = PowerUnknown;
        if (DCCEXInbound::getNumber(0)==PowerOff) {
            state = PowerOff;
        } else if (DCCEXInbound::getNumber(0)==PowerOn) {
            state = PowerOn;
        }

        delegate->receivedTrackPower(state);
    }
    // console->println(F("processTrackPower(): end"));
}

// ****************
// roster

//private
void DCCEXProtocol::processRosterList() {
    // console->println(F("processRosterList()"));
    if (roster!=nullptr) {  // already have a roster so this is an update
        // roster.clear();
        console->println(F("processRosterList(): roster list already received. Ignoring this!"));
        return;
    } 
    for (int i=1; i<DCCEXInbound::getParameterCount(); i++) {
        int address = DCCEXInbound::getNumber(i);
        new Loco(address, LocoSourceRoster);
        // sendRosterEntryRequest(address);
    }
    sendRosterEntryRequest(Loco::getFirst()->getAddress());
    _rosterCount = DCCEXInbound::getParameterCount()-1;
    // console->println(F("processRosterList(): end"));
}

//private
void DCCEXProtocol::sendRosterEntryRequest(int address) {
    // console->println(F("sendRosterEntryRequest()"));
    if (delegate) {
        sprintf(outboundCommand, "<JR %d>", address);
        sendCommand();
    }
    // console->println(F("sendRosterEntryRequest(): end"));
}

//private
void DCCEXProtocol::processRosterEntry() { //<jR id ""|"desc" ""|"funct1/funct2/funct3/...">
    console->println(F("processRosterEntry()"));
    //find the roster entry to update
    int address=DCCEXInbound::getNumber(1);
    char *name=DCCEXInbound::getSafeText(2);
    char *funcs=DCCEXInbound::getSafeText(3);
    bool missingRosters=false;
    
    // for (Loco* r=roster->getFirst(); r; r=r->getNext()) {
    //     if (r->getAddress() == address) {
    //         // console->print("processRosterEntry(): found: "); console->println(address);
    //         r->setName(name);
    //         r->setupFunctions(funcs);
    //         // If r->getNext()==nullptr at end of list, we're done
    //         // If r->getNext()->getName()==nullptr means we're not done
    //     } else {
    //         if (r->getName()==nullptr) {
    //             // console->print(F("processRosterEntry(): not received yet: ~"));
    //             // console->println(r->getAddress());
    //             missingRosters=true;
    //             break;
    //         }
    //     }
    // }

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
        rosterFullyReceived = true;
        // console->print(F("processRosterEntry(): received all: "));
        // console->println(getRosterCount());
        delegate->receivedRosterList(getRosterCount());
    }
    console->println(F("processRosterEntry(): end"));
}

// ****************
// Turnouts/Points

//private
void DCCEXProtocol::processTurnoutList() {
    // <jT id1 id2 id3 ...>
    // console->println(F("processTurnoutList()"));
    if (turnouts!=nullptr) {
        // turnouts.clear();
        console->println(F("processTurnoutList(): Turnout/Points list already received. Ignoring this!"));
        return;
    } 

    for (int i=1; i<DCCEXInbound::getParameterCount(); i++) {
        auto id = DCCEXInbound::getNumber(i);
        new Turnout(id, false);
        // sendTurnoutEntryRequest(id);
    }
    sendTurnoutEntryRequest(Turnout::getFirst()->getId());
    _turnoutsCount = DCCEXInbound::getParameterCount()-1;
    // console->println(F("processTurnoutList(): end"));
}

//private
void DCCEXProtocol::sendTurnoutEntryRequest(int id) {
    // console->println(F("sendTurnoutEntryRequest()"));
    if (delegate) {
        sprintf(outboundCommand, "<JT %d>", id);

        sendCommand();
    }
    // console->println(F("sendTurnoutEntryRequest() end"));
}

//private
void DCCEXProtocol::processTurnoutEntry() {
    if (DCCEXInbound::getParameterCount()!=4) return;
    console->println(F("processTurnoutEntry()"));
    //find the turnout entry to update
    int id=DCCEXInbound::getNumber(1);
    bool thrown=(DCCEXInbound::getNumber(2)=='T');
    char* name=DCCEXInbound::getSafeText(3);
    bool missingTurnouts=false;

    // for (Turnout* t=turnouts->getFirst(); t; t=t->getNext()) {
    //     if (t->getId()==id) {
    //         t->setName(name);
    //         t->setThrown(thrown);
    //     } else {
    //         if (t->getName()==nullptr) {
    //             // console->print(F("processTurnoutsEntry(): not received yet: ~"));
    //             // console->println(t->getId());
    //             missingTurnouts = true;
    //             break;
    //         }
    //     }
    // }

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
        turnoutListFullyReceived=true;
        console->println(F("processTurnoutsEntry(): received all"));
        delegate->receivedTurnoutList(getTurnoutsCount());
    }
    console->println(F("processTurnoutEntry() end"));
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
    if (delegate) {
        sprintf(outboundCommand, "<T %d 0>", turnoutId);
        sendCommand();
    }
}

void DCCEXProtocol::throwTurnout(int turnoutId) {
    if (delegate) {
        sprintf(outboundCommand, "<T %d 1>", turnoutId);
        sendCommand();
    }
}

void DCCEXProtocol::toggleTurnout(int turnoutId) {
    for (Turnout* t=turnouts->getFirst(); t; t=t->getNext()) {
        if (t->getId()==turnoutId) {
            console->println(t->getThrown());
            if (t->getThrown()) {
                console->println(F("Thrown"));
            } else {
                console->println(F("Closed"));
            }
            bool thrown=t->getThrown() ? 0 : 1;
            sprintf(outboundCommand, "<T %d %d>", turnoutId, thrown);
            sendCommand();
        }
    }
}

Turntable* DCCEXProtocol::getTurntableById(int turntableId) {
    // for (int i = 0; i < turntables.size(); i++) {
    //     Turntable* tt = turntables.get(i);
    for (Turntable* tt=turntables->getFirst(); tt; tt=tt->getNext()) {
        if (tt->getId() == turntableId) {
            return tt;
        }
    }
    return nullptr;
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
            delegate->receivedTurnoutAction(id, thrown);
        }
    }
    // console->println(F("processTurnoutAction(): end"));
}

// ****************
// Routes

//private
void DCCEXProtocol::processRouteList() {
    // console->println(F("processRouteList()"));
    if (delegate) {
        // if (routes.size()>0) { // already have a routes list so this is an update
        if (routes!=nullptr) {
            // routes.clear();
            console->println(F("processRouteList(): Routes/Automation list already received. Ignoring this!"));
            return;
        } 

        for (int i=1; i<DCCEXInbound::getParameterCount(); i++) {
            int id = DCCEXInbound::getNumber(i);
            // routes.add(new Route(id));
            new Route(id);
            sendRouteEntryRequest(id);
        }
        _routesCount = DCCEXInbound::getParameterCount()-1;

    }
    // console->println(F("processRouteList(): end"));
}

//private
void DCCEXProtocol::sendRouteEntryRequest(int id) {
    // console->println(F("sendRouteEntryRequest()"));
    if (delegate) {
        sprintf(outboundCommand, "<JA %d>", id);

        sendCommand();
    }
    // console->println(F("sendRouteEntryRequest() end"));
}

//private
void DCCEXProtocol::processRouteEntry() {
    // console->println(F("processRouteEntry()"));
    //find the Route entry to update
    // if (routes.size()>0) { 
    // for (int i=0; i<routes.size(); i++) {
    int id=DCCEXInbound::getNumber(1);
    RouteType type=(RouteType)DCCEXInbound::getNumber(2);
    char* name=DCCEXInbound::getSafeText(3);
    bool missingRoutes = false;

    for (Route* r=routes->getFirst(); r; r=r->getNext()) {
        // auto r = routes.get(i);
        if (r->getId()==id) {
            // r->setRouteType((RouteType)DCCEXInbound::getNumber(2));
            // r->setRouteName(DCCEXInbound::getSafeText(3));
            // r->setHasReceivedDetails();
            r->setType(type);
            r->setName(name);
        } else {
            if (r->getName()==nullptr) {
                missingRoutes=true;
                break;
            }
        }
    }

    if (!missingRoutes) {
        routeListFullyReceived=true;
        console->println(F("processRoutesEntry(): received all"));
        delegate->receivedRouteList(getRoutesCount());
    }
    // console->println(F("processRouteEntry() end"));
}

// ****************
// Turntables

void DCCEXProtocol::processTurntableList() {  // <jO [id1 id2 id3 ...]>
    // console->println(F("processTurntableList(): "));
    if (turntables!=nullptr) {  // already have a turntables list so this is an update
        // turntables.clear();
        console->println(F("processTurntableList(): Turntable list already received. Ignoring this!"));
        return;
    } 
    for (int i=1; i<DCCEXInbound::getParameterCount(); i++) {
        int id = DCCEXInbound::getNumber(i);
        new Turntable(id);
        sendTurntableEntryRequest(id);
        sendTurntableIndexEntryRequest(id);
    }
    _turntablesCount = DCCEXInbound::getParameterCount()-1;
    // console->print("processTurntableList(): end: size:"); console->println(turntables.size());
}

//private
void DCCEXProtocol::sendTurntableEntryRequest(int id) {
    // console->println(F("sendTurntableEntryRequest()"));
    if (delegate) {
        sprintf(outboundCommand, "<JO %d>", id);

        sendCommand();
    }
    // console->println(F("sendTurntableEntryRequest(): end"));
}

//private
void DCCEXProtocol::sendTurntableIndexEntryRequest(int id) {
    // console->println(F("sendTurntableIndexEntryRequest()"));
    if (delegate) {
        sprintf(outboundCommand, "<JP %d>", id);

        sendCommand();
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

    for (Turntable* tt=turntables->getFirst(); tt; tt=tt->getNext()) {
        if (tt->getId()==id) {
            tt->setType(ttType);
            tt->setIndex(index);
            tt->setNumberOfIndexes(indexCount);
            tt->setName(name);
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
            turntableListFullyReceived = true;
            // console->println(F("processTurntableIndexEntry(): received all"));
            delegate->receivedTurntableList(getTurntablesCount());
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
    delegate->receivedTurntableAction(id, newIndex, moving);
    // console->println(F("processTurntableAction(): end"));
}

//private
void DCCEXProtocol::processSensorEntry() {  // <jO id type position position_count "[desc]">
    // console->println(F("processSensorEntry(): "));
    if (delegate) {
        //????????????????? TODO
        console->println(F("processSensorEntry(): Not doing anything with these yet"));
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
    int functMap = getValidFunctionMap(DCCEXInbound::getNumber(3));
    int throttleNo = findThrottleWithLoco(address);
    if (throttleNo>=0) {
        // int rslt = throttle[throttleNo].getLocoPosition(address);
        ConsistLoco* loco=throttle[throttleNo].getFirst();
        if (loco->getAddress()==address) {    // ignore everything that is not the lead loco
        // if (rslt==0) {  // ignore everything that is not the lead loco
            int speed = getSpeedFromSpeedByte(speedByte);
            Direction dir = getDirectionFromSpeedByte(speedByte);
            // int currentFunc = throttle[throttleNo].getLocoAtPosition(0)->getFunctionStates();
            int currentFunc = loco->getFunctionStates();
            if (functMap != currentFunc) {
                int funcChanges=currentFunc^functMap;
                for (int f=0; f<MAX_FUNCTIONS; f++) {
                    if (funcChanges & (1<<f)) {
                        bool newState = functMap & (1<<f);
                        delegate->receivedFunction(throttleNo, f, newState);
                    }
                }
                // throttle[throttleNo].getLocoAtPosition(0)->setFunctionStates(functMap);
                loco->setFunctionStates(functMap);
            }
            // throttle[throttleNo].actionConsistExternalChange(speed, dir, functMap);
            throttle[throttleNo].setSpeed(speed);
            throttle[throttleNo].setDirection(dir);
            // throttle[throttleNo]

            delegate->receivedSpeed(throttleNo, speed);
            delegate->receivedDirection(throttleNo, dir);

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

void DCCEXProtocol::sendServerDetailsRequest() {
    // console->println(F("sendServerDetailsRequest(): "));
    if (delegate) {
        sprintf(outboundCommand, "<s>");
        sendCommand();        
    }
    // console->println(F("sendServerDetailsRequest(): end"));
}

// ******************************************************************************************************
// power commands

bool DCCEXProtocol::sendTrackPower(TrackPower state) {
    // console->println(F("sendTrackPower(): "));
    if (delegate) {
        sprintf(outboundCommand, "<%d>", state);

        sendCommand();
    }
    // console->println(F("sendTrackPower(): end"));
    return true;
}

bool DCCEXProtocol::sendTrackPower(TrackPower state, char track) {
    // console->println(F("sendTrackPower(): "));
    if (delegate) {
        sprintf(outboundCommand, "<%d %c>", state, track);

        sendCommand();
    }
    // console->println(F("sendTrackPower(): end"));
    return true;
}

// ******************************************************************************************************

void DCCEXProtocol::sendEmergencyStop() {
    // console->println(F("emergencyStop(): "));
    if (delegate) {
        sprintf(outboundCommand, "<!>");
        sendCommand();
    }
    // console->println(F("emergencyStop(): end"));
}

// ******************************************************************************************************

Consist DCCEXProtocol::getThrottleConsist(int throttleNo) {
    // console->println(F("getThrottleConsist(): "));
    if (delegate) {
        //????????????????? TODO
        // what if the throttle does not have a consist
        //
        return throttle[throttleNo];
    }
    // console->println(F("getThrottleConsist(): end"));
    return {};
}


// ******************************************************************************************************

// by default only send to the lead loco
bool DCCEXProtocol::sendFunction(int throttleNo, int functionNumber, bool pressed) {
    // console->println(F("sendFunction(): "));
    if (delegate) {
        // ConsistLoco* conLoco = throttle[throttleNo].getLocoAtPosition(0);
        ConsistLoco* conLoco = throttle[throttleNo].getFirst();
        int address = conLoco->getAddress();
        if (address>=0) {
            sendFunction(throttleNo, address, functionNumber, pressed);
        }
    }
    // console->println(F("sendFunction(): end")); 
    return true;
}

// send to a specific address on the throttle
bool DCCEXProtocol::sendFunction(int throttleNo, int address, int functionNumber, bool pressed) { // throttle is ignored
    // console->println(F("sendFunction(): "));
    if (delegate) {
        sprintf(outboundCommand, "<F %d %d %d>", address, functionNumber, pressed);
        sendCommand();
    }
    // console->println(F("sendFunction(): end")); 
    return true;
}

// by default only check the lead loco on the throttle
bool DCCEXProtocol::isFunctionOn(int throttleNo, int functionNumber) {
    if (delegate) {
        // ConsistLoco* conLoco = throttle[throttleNo].getLocoAtPosition(0);
        ConsistLoco* conLoco = throttle[throttleNo].getFirst();
        int address = conLoco->getAddress();
        if (address>=0) {
            console->print(" '");
            console->print(conLoco->isFunctionOn(functionNumber));
            console->print("' ");
            conLoco->isFunctionOn(functionNumber);
        }
        return conLoco->isFunctionOn(functionNumber);
    }
    return false;
}

// ******************************************************************************************************
// throttle

bool DCCEXProtocol::sendThrottleAction(int throttleNo, int speed, Direction direction) {
    // console->println(F("sendThrottleAction(): "));
    if (delegate) {
        if (throttle[throttleNo].getLocoCount()>0) {
            throttle[throttleNo].setSpeed(speed);
            throttle[throttleNo].setDirection(direction);
            // for (int i=0; i<throttle[throttleNo].getLocoCount(); i++) {
            //     ConsistLoco* conLoco = throttle[throttleNo].getLocoAtPosition(i);
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
                sendLocoAction(address, speed, dir);
            }
        }
    }
    // console->println(F("sendThrottleAction(): end"));
    return true;
}



// ******************************************************************************************************
// individual locos
// 

bool DCCEXProtocol::sendLocoUpdateRequest(int address) {
    // console->println(F("sendLocoUpdateRequest()"));
    if (delegate) {
        sprintf(outboundCommand, "<t %d>", address);
        sendCommand();
    }
    // console->println(F("sendLocoUpdateRequest() end"));
    return true;
}

bool DCCEXProtocol::sendLocoAction(int address, int speed, Direction direction) {
    // console->print(F("sendLocoAction(): ")); console->println(address);
    if (delegate) {
        sprintf(outboundCommand, "<t %d %d %d>", address, speed, direction);
        sendCommand();
    }
    // console->println(F("sendLocoAction(): end"));
    return true;
}

// ******************************************************************************************************

bool DCCEXProtocol::sendRouteAction(int routeId) {
    // console->println(F("sendRouteAction()"));
    if (delegate) {
        sprintf(outboundCommand, "</START  %d >", routeId);
        sendCommand();
    }
    // console->println(F("sendRouteAction() end"));
    return true;
}

bool DCCEXProtocol::sendPauseRoutes() {
    // console->println(F("sendPauseRoutes()"));
    if (delegate) {
        sprintf(outboundCommand, "</PAUSE>");
        sendCommand();
    }
    // console->println(F("sendPauseRoutes() end"));
    return true;
}

bool DCCEXProtocol::sendResumeRoutes() {
    // console->println(F("sendResumeRoutes()"));
    if (delegate) {
        sprintf(outboundCommand, "</RESUME>");
        sendCommand();
    }
    // console->println(F("sendResumeRoutes() end"));
    return true;
}

// ******************************************************************************************************


bool DCCEXProtocol::sendTurntableAction(int turntableId, int position, int activity) {
    // console->println(F("sendTurntable()"));
    if (delegate) {
        sprintf(outboundCommand, "<I %d %d %d>", turntableId, position, activity);
        sendCommand();
    }
    // console->println(F("sendTurntable() end"));
    return true;
}

bool DCCEXProtocol::sendAccessoryAction(int accessoryAddress, int activate) {
    // console->println(F("sendAccessory()"));
    if (delegate) {
        sprintf(outboundCommand, "<a %d %d>", accessoryAddress, activate);
        sendCommand();
    }
    // console->println(F("sendAccessory() end"));
    return true;
}

bool DCCEXProtocol::sendAccessoryAction(int accessoryAddress, int accessorySubAddr, int activate) {
    // console->println(F("sendAccessory()"));
    if (delegate) {
        sprintf(outboundCommand, "<a %d %d %d>", accessoryAddress, accessorySubAddr, activate);
        sendCommand();
    }
    // console->println(F("sendAccessory() end"));
    return true;
}

// ******************************************************************************************************

// sequentially request and get the required lists. To avoid overloading the buffer
void DCCEXProtocol::getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired) {
//    console->println(F("getLists()"));
    if (millis()<_delayUntil) return;
    if (!allRequiredListsReceived) {
        if (rosterRequired && !rosterRequested) {
            getRoster();
        } else { 
            if (!rosterRequired || rosterFullyReceived) {

                if (turnoutListRequired && !turnoutListRequested) {
                    getTurnouts();
                } else { 
                    if (!turnoutListRequired || turnoutListFullyReceived) {

                        if (routeListRequired && !routeListRequested) {
                            getRoutes();
                        } else { 
                            if (!routeListRequired || routeListFullyReceived) {

                                if (turntableListRequired && !turntableListRequested) {
                                    getTurntables();
                                } else { 
                                    if (!turntableListRequired || turntableListFullyReceived) {

                                        allRequiredListsReceived = true;
                                        console->println(F("Lists Fully Received"));
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

bool DCCEXProtocol::getRoster() {
    // console->println(F("getRoster()"));
    if (delegate) {
        sprintf(outboundCommand, "<JR>");
        sendCommand();
        rosterRequested = true;
    }
    // console->println(F("getRoster() end"));
    return true;
}

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

bool DCCEXProtocol::isRosterRequested() {
    return rosterRequested;
}
bool DCCEXProtocol::isRosterFullyReceived() {
    // console->println(F("isRosterFullyReceived()"));
    // if (rosterFullyReceived) console->println(F("true"));
    return rosterFullyReceived;
}

bool DCCEXProtocol::getTurnouts() {
    // console->println(F("getTurnouts()"));
    if (delegate) {
        sprintf(outboundCommand, "<JT>");
        sendCommand();
        turnoutListRequested = true;
    }
    // console->println(F("getTurnouts() end"));
    return true;
}

int DCCEXProtocol::getTurnoutsCount() {
    return _turnoutsCount;
}

Turnout* DCCEXProtocol::getTurnoutsEntryNo(int entryNo) {
    int i=0;
    for (Turnout* turnout=turnouts->getFirst(); turnout; turnout=turnout->getNext()) {
        if (i==entryNo) return turnout;
        i++;
    }
    return {};
}

bool DCCEXProtocol::isTurnoutListRequested() {
    return turnoutListRequested;
}
bool DCCEXProtocol::isTurnoutListFullyReceived() {
    return turnoutListFullyReceived;
}

bool DCCEXProtocol::getRoutes() {
    // console->println(F("getRoutes()"));
    if (delegate) {
        sprintf(outboundCommand, "<JA>");
        sendCommand();
        routeListRequested = true;
    }
    // console->println(F("getRoutes() end"));
    return true;
}

int DCCEXProtocol::getRoutesCount() {
    return _routesCount;
}

Route* DCCEXProtocol::getRoutesEntryNo(int entryNo) {
    int i=0;
    for (Route* route=routes->getFirst(); route; route=route->getNext()) {
        if (i==entryNo) return route;
        i++;
    }
    return {};
}

bool DCCEXProtocol::isRouteListRequested() {
    return routeListRequested;
}
bool DCCEXProtocol::isRouteListFullyReceived() {
    return routeListFullyReceived;
}

bool DCCEXProtocol::getTurntables() {
    // console->println(F("getTurntables()"));
    if (delegate) {
        sprintf(outboundCommand, "<JO>");
        sendCommand();
        turntableListRequested = true;
    }
    // console->println(F("getTurntables() end"));
    return true;
}

int DCCEXProtocol::getTurntablesCount() {
    return _turntablesCount;
}

bool DCCEXProtocol::isTurntableListRequested() {
    return turntableListRequested;
}
bool DCCEXProtocol::isTurntableListFullyReceived() {
    return turntableListFullyReceived;
}

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// helper functions

//private
Loco* DCCEXProtocol::findLocoInRoster(int address) {
    // if (roster.size()>0) {
    //     for (int i=0; i<roster.size(); i++) {
    //         if (roster.get(i)->getLocoAddress() == address) {
    //             return *roster.get(i);
    //         }
    //     }
    // }
    // return {};
    for (Loco* r=roster->getFirst(); r; r=r->getNext()) {
        if (r->getAddress()==address) {
            return r;
        }
    }
    return nullptr;
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

// int DCCEXProtocol::findTurnoutListPositionFromId(int id) {
//     if (turnouts.size()>0) {
//         for (int i=0; i<turnouts.size(); i++) {
//             if (turnouts.get(i)->getTurnoutId()==id) {
//                 return i;
//             }
//         }
//     }
//     return -1;
// }

// int DCCEXProtocol::findRouteListPositionFromId(int id) {
//     if (routes.size()>0) {
//         for (int i=0; i<routes.size(); i++) {
//             if (routes.get(i)->getRouteId()==id) {
//                 return i;
//             }
//         }
//     }
//     return -1;
// }

// int DCCEXProtocol::findTurntableListPositionFromId(int id) {
//     if (turntables.size()>0) {
//         for (int i=0; i<turntables.size(); i++) {
//             if (turntables.get(i)->getTurntableId()==id) {
//                 return i;
//             }
//         }
//     }
//     return -1;
// }

char* DCCEXProtocol::nextServerDescriptionParam(int startAt, bool lookingAtVersionNumber) {
    char _tempString[MAX_SERVER_DESCRIPTION_PARAM_LENGTH];
    int i = 0; 
    size_t j;
    bool started = false;
    for (j=startAt; j<strlen(serverDescription) && i<(MAX_SERVER_DESCRIPTION_PARAM_LENGTH-1); j++) {
        if (started) {
            if (serverDescription[j]==' ' || serverDescription[j]=='\0') break;
            if (lookingAtVersionNumber && (serverDescription[j]=='-' || serverDescription[j]=='.')) break;
            _tempString[i] = serverDescription[j];
            i++;
        } else {
            if (serverDescription[j]==' ') started=true;
            if (lookingAtVersionNumber && (serverDescription[j]=='-' || serverDescription[j]=='.')) started=true;
        }
    }
    _tempString[i] = '\0';
    
    char *_result;
    _result = (char *) malloc(strlen(_tempString));
    sprintf(_result, "%s", _tempString);
    // console->println(_result);
    return _result;
}
