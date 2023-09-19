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

DCCEXProtocol::DCCEXProtocol(bool server) {
	// store server/client
    this->server = server;
		
	// init streams
    stream = &nullStream;
	console = &nullStream;
}

// ******************************************************************************************************
// Connection related

//private
// init the DCCEXProtocol instance after connection to the server
void DCCEXProtocol::init() {
    console->println("init()");
    
	// allocate input buffer and init position variable
	memset(inputbuffer, 0, sizeof(inputbuffer));
	nextChar = 0;
	
    //last Response time
    lastServerResponseTime = millis() /1000;

    console->println("init(): end");
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
}

void DCCEXProtocol::disconnect() {
    sendCommand("<U DISCONNECT>");
    this->stream = NULL;
}

bool DCCEXProtocol::check() {
    // console->println("check()");
    bool changed = false;

    if (stream) {
        while(stream->available()) {
            char b = stream->read();
            // console->print("check(): char: ~"); console->print(b); console->println("~");
            if (b == NEWLINE || b==CR) {
                // server sends TWO newlines after each command, we trigger on the
                // first, and this skips the second one
                if (nextChar != 0) {
                    inputbuffer[nextChar] = 0;
                    changed |= processCommand(inputbuffer, nextChar);
                }
                nextChar = 0;
            } else {
                inputbuffer[nextChar] = b;
                nextChar += 1;
                if (nextChar == (sizeof(inputbuffer)-1) ) {
                    inputbuffer[sizeof(inputbuffer)-1] = 0;
                    console->print("ERROR LINE TOO LONG: >");
                    console->print(sizeof(inputbuffer));
                    console->print(": ");
                    console->println(inputbuffer);
                    nextChar = 0;
                }
            }
            // console->println("check(): end-loop");
        }
        // console->println("check(): end-stream");
        return changed;
    }
    else {
        // console->println("check(): end");
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
    Direction dir = Forward;
    if (speedByte >= 128) {
        dir = Reverse;
    }
    return dir;
}

int DCCEXProtocol::getSpeedFromSpeedByte(int speedByte) {
    int speed = speedByte;
    if (speed >= 128) {
        speed = speed - 128;
    }
    if (speed>1) {
        speed = speed - 1; // get round the idiotic design of the speed command
    } else {
        speed=0;
    }
    return speed;
}

Functions DCCEXProtocol::getFunctionStatesFromFunctionMap( int * states, int functionMap) {
    
    // ??????????????????????????????????
    Functions fns;
    return fns;
}

// ******************************************************************************************************
// sending and receiving commands from the CS

//private
void DCCEXProtocol::sendCommand(String cmd) {
    if (stream) {
        // TODO: what happens when the write fails?
        stream->println(cmd);
        if (server) {
            stream->println("");
        }
        console->print("==> "); console->println(cmd);
    }
}

//private
bool DCCEXProtocol::processCommand(char *c, int len) {
    console->println("processCommand()");
    bool changed = false;

    lastServerResponseTime = millis()/1000;

    console->print("<== "); console->println(c);

    String s = String(c);
    LinkedList<String> args;
    bool rslt = splitCommand(args, s.substring(1, s.length() - 1),' ');
    char char0 = args.get(0).charAt(0);
    char char1 = args.get(0).charAt(1);
    int noOfParameters = args.size();

    console->print("processing: "); console->println(s);
    for (int i=0; i<args.size();i++) {
        console->print("arg"); console->print(i); console->print(": ~"); console->print(args.get(i)); console->println("~");
    }

    if (delegate) {
        if (len > 3 && char0 == 'i' && args.get(0) == "iDCC-EX") {  //<iDCCEX version / microprocessorType / MotorControllerType / buildNumber>
            processServerDescription(args);

        } else if (len > 1 && char0 == 'p') { //<p onOff>
            processTrackPower(args);

        } else if (len > 3 && char0 == 'l') { //<l cab reg speedByte functMap>
            processLocoAction(args);

        } else if (len > 3 && char0 == 'j' && char1 == 'R' && (noOfParameters > 1)) { 
            if ( (noOfParameters<3) || (args.get(2).charAt(0) != '"') ) {  // loco list
                processRosterList(args); //<jR [id1 id2 id3 ...]>
            } else { // individual
                processRosterEntry(args); //<jR id ""|"desc" ""|"funct1/funct2/funct3/...">
            }

        } else if (len > 3 && char0=='j' && char1=='T' && (noOfParameters > 1)) { 
            if ( (noOfParameters == 4) && (args.get(3).charAt(0) == '"') 
            || (noOfParameters == 3) && (args.get(2).charAt(0) == 'X')) {
                processTurnoutEntry(args); //<jT id state |"[desc]">   or    <jT id X">
            } else {
                processTurnoutList(args); //<jT [id1 id2 id3 ...]>
            }

        } else if (len > 3 && char0 == 'H') { //<H id state>
            if (delegate) {
                // TurnoutState state = (int) args.get(2).toInt();
                if (args.size()==3) {
                    processTurnoutAction(args);
                    delegate->receivedTurnoutAction(args.get(1).toInt(), args.get(2).toInt());
                }
            }

        } else if (len > 3 && char0=='j' && char1=='O' && (noOfParameters>1)) { 
            if ( (noOfParameters == 6) && (args.get(5).charAt(0) == '"') ) { //<jO id type position position_count "[desc]">
                processTurntableEntry(args); 
            } else {
                processTurntableList(args); //<jO [id1 id2 id3 ...]>
            } 

        } else if (len > 3 && char0=='i' && args.get(0)!="iDCC-EX") { //<i id position>   or   <i id position moving>
            if (delegate) {
                TurntableState state = TurntableStationary;
                if (args.size()<3) { 
                    state = TurntableMoving;
                }
                processTurntableAction(args);
                delegate->receivedTurntableAction(args.get(1).toInt(), args.get(2).toInt(), state);
            }

        } else if (len > 3 && char0 == 'j' && char1 == 'T' && (noOfParameters > 1)) { 
            if ( (noOfParameters == 4) && (args.get(3).charAt(0) == '"') 
            || (noOfParameters == 3) && (args.get(2).charAt(0) == 'X')) {
                processRouteEntry(args); //<jA id type |"desc">   or  <jA id X>
            } else {
                processRouteList(args); //<jA [id0 id1 id2 ..]>
            }
        } else {
            processUnknownCommand(s);
        }
    }

    console->println("processCommand() end");
    return true;
}

//private
void DCCEXProtocol::processUnknownCommand(String unknownCommand) {
    console->println("processUnknownCommand()");
    if (delegate) {
        console->printf("unknown command '%s'\n", unknownCommand);
    }
    console->println("processUnknownCommand() end");
}

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// Process responses from the CS

//private
void DCCEXProtocol::processServerDescription(LinkedList<String> &args) { //<iDCCEX version / microprocessorType / MotorControllerType / buildNumber>
    console->println("processServerDescription()");
    if (delegate) {
        serverVersion = args.get(1);
        serverMicroprocessorType = args.get(3);
        serverMotorcontrollerType = args.get(5);
        // serverBuildNumber = args.get(6);

        delegate->receivedServerDescription(serverMicroprocessorType, serverVersion);
    }
    console->println("processServerDescription(): end");
}

//private
void DCCEXProtocol::processTrackPower(LinkedList<String> &args) {
    console->println("processTrackPower()");
    if (delegate) {
        TrackPower state = PowerUnknown;
        if (args.get(0).charAt(0)=='0') {
            state = PowerOff;
        } else if (args.get(0).charAt(0)=='1') {
            state = PowerOn;
        }

        delegate->receivedTrackPower(state);
    }
     console->println("processTrackPower(): end");
}

// ****************
// roster

//private
void DCCEXProtocol::processRosterList(LinkedList<String> &args) {
    console->println("processRosterList()");
    if (delegate) {
        if (roster.size()>0) { // already have a roster so this is an update
            roster.clear();
        } 
        for (int i=1; i<args.size(); i++) {
            int address = args.get(i).toInt();
             console->print("processRosterList() address: ");  console->println(address);
            Loco loco;
            console->println("processRosterList() 0");
            bool rslt = loco.initLoco(address, "", LocoSourceRoster);
            console->println("processRosterList() 1");
            roster.add(loco);
            console->println("processRosterList() 2");
            sendRosterEntryRequest(address);
            console->println("processRosterList() 3");
        }
    }
    console->println("processRosterList(): end");
}

//private
void DCCEXProtocol::sendRosterEntryRequest(int address) {
    console->println("sendRosterEntryRequest()");
    if (delegate) {
        sendCommand("<JR " + String(address) + ">");
    }
    console->println("sendRosterEntryRequest() end");
}

//private
void DCCEXProtocol::processRosterEntry(LinkedList<String> &args) { //<jR id ""|"desc" ""|"funct1/funct2/funct3/...">
    console->println("processRosterEntry()");
    if (delegate) {
        //find the roster entry to update
        if (roster.size()>0) { 
            for (int i=0; i<roster.size(); i++) {
                int address = args.get(1).toInt();
                Loco rosterLoco = roster.get(i);
                if (rosterLoco.getLocoAddress() == address) {
                    int address = args.get(1).toInt();
                    Loco loco;
                    bool rslt = loco.initLoco(address, args.get(2), LocoSourceRoster);
// ????????????????
//                    String functions[] = args.get(2).split("/", 999);
                    // for (int j=0; (j<functions.length() && j<MAX_FUNCTIONS); j++ ) {
                    //     FunctionLatching latching = FunctionLatchingTrue;
                    //     if (functions[j].charAt(0)=='*') {
                    //         latching = FunctionLatchingFalse;
                    //     }
                    //     loco.locoFunctions[j].initFunction(j, functions[j], latching, FunctionStateOff); 
                    // }
// ????????????????
                    roster.set(i, loco);
                    sendLocoUpdateRequest(address);
                }
            }
        } 
    }
    console->println("processRosterEntry()");
}

// ****************
// Turnouts/Points

//private
void DCCEXProtocol::processTurnoutList(LinkedList<String> &args) {
    console->println("processTurnoutList()");
    if (delegate) {
        if (turnouts.size()>0) { // already have a turnouts list so this is an update
            turnouts.clear();
        } 
        for (int i=1; i<args.size(); i++) {
            int id = args.get(i).toInt();
            Turnout turnout;
            bool rslt = turnout.initTurnout(id, "", 0);
            turnouts.add(turnout);
            sendTurntableEntryRequest(id);
        }
    }
    console->println("processTurnoutList(): end");
}

//private
void DCCEXProtocol::sendTurnoutEntryRequest(int id) {
    console->println("sendTurnoutEntryRequest()");
    if (delegate) {
        sendCommand("<JT " + String(id) + ">");
    }
    console->println("sendTurnoutEntryRequest() end");
}

//private
void DCCEXProtocol::processTurnoutEntry(LinkedList<String> &args) {
    console->println("processTurnoutEntry()");
    if (delegate) {
        //find the turnout entry to update
        if (turnouts.size()>0) { 
            for (int i=0; i<turnouts.size(); i++) {
                Turnout turnoutsTurnout = turnouts.get(i);
                int id = args.get(1).toInt();
                if (turnoutsTurnout.getTurnoutId()==id) {
                    int id = args.get(1).toInt();
                    Turnout turnout = turnoutsTurnout;
                    bool rslt = turnout.initTurnout(id, args.get(2), TurnoutClosed);
                    turnouts.set(i, turnout);
                }
            }
        } 
    }
    console->println("processTurnoutEntry() end");
}

//private
bool DCCEXProtocol::sendTurnoutAction(int turnoutId, TurnoutAction action) {
    sendCommand("<T " + String(turnoutId) + " " + String(action) + ">");
    return true;
}

//private
void DCCEXProtocol::processTurnoutAction(LinkedList<String> &args) { //<H id state>
    console->println("processTurnoutAction(): ");
    if (delegate) {
        //find the Turnout entry to update
        if (turnouts.size()>0) { 
            for (int i=0; i<turnouts.size(); i++) {
                int id = args.get(1).toInt();
                Turnout turnoutsTurnout = turnouts.get(i);
                if (turnoutsTurnout.getTurnoutId()==id) {
                    TurnoutState state = args.get(2).toInt();
                    Turnout turnout = turnoutsTurnout;
                    if (args.size() >= 3) {
                        bool rslt = turnout.setTurnoutState(state);
                        delegate->receivedTurnoutAction(id, state);
                    }
                }
            }
        } 
    }
    console->println("processTurnoutAction(): end");
}

// ****************
// Routes

//private
void DCCEXProtocol::processRouteList(LinkedList<String> &args) {
    console->println("processRouteList()");
    if (delegate) {
        if (routes.size()>0) { // already have a routes list so this is an update
            routes.clear();
        } 
        for (int i=1; i<args.size(); i++) {
            int id = args.get(i).toInt();
            Route route;
            bool rslt = route.initRoute(id, "");
            routes.add(route);
            sendRouteEntryRequest(id);
        }
    }
    console->println("processRouteList(): end");
}

//private
void DCCEXProtocol::sendRouteEntryRequest(int address) {
    console->println("sendRouteEntryRequest()");
    if (delegate) {
        sendCommand("<JA " + String(address) + ">");
    }
    console->println("sendRouteEntryRequest() end");
}

//private
void DCCEXProtocol::processRouteEntry(LinkedList<String> &args) {
    console->println("processRouteEntry()");
    if (delegate) {
        //find the Route entry to update
        if (routes.size()>0) { 
            for (int i=0; i<routes.size(); i++) {
                int id = args.get(1).toInt();
                Route routesRoute = routes.get(i);
                if (routesRoute.getRouteId()==id) {
                    int id = args.get(1).toInt();
                    Route route = routesRoute;
                    bool rslt = route.initRoute(id, args.get(2));
                    routes.set(i, route);
                }
            }
        } 
    }
    console->println("processRouteEntry() end");
}

// void DCCEXProtocol::processRouteAction(LinkedList<String> &args) {
//     console->println("processRouteAction(): ");
//     if (delegate) {

//     }
//     console->println("processRouteAction(): end");
// }

// ****************
// Turntables

void DCCEXProtocol::processTurntableList(LinkedList<String> &args) {  // <jO [id1 id2 id3 ...]>
    console->println("processTurntableList(): ");
    if (delegate) {
        if (turntables.size()>0) { // already have a turntables list so this is an update
            turntables.clear();
        } 
        for (int i=1; i<args.size(); i++) {
            int id = args.get(i).toInt();
            Turntable turntable;
            bool rslt = turntable.initTurntable(id, "", TurntableTypeUnknown, 0);
            turntables.add(turntable);
            sendTurnoutEntryRequest(id);
            sendTurntableIndexEntryRequest(id);
        }
    }
    console->println("processTurntableList(): end");
}

//private
void DCCEXProtocol::sendTurntableEntryRequest(int id) {
    console->println("sendTurntableEntryRequest()");
    if (delegate) {
        sendCommand("<JO " + String(id) + ">");
    }
    console->println("sendTurntableEntryRequest() end");
}

//private
void DCCEXProtocol::sendTurntableIndexEntryRequest(int id) {
    console->println("sendTurntableIndexEntryRequest()");
    if (delegate) {
        sendCommand("<JP " + String(id) + ">");
    }
    console->println("sendTurntableIndexEntryRequest() end");
}

//private
void DCCEXProtocol::processTurntableEntry(LinkedList<String> &args) {  // <jO id type position position_count "[desc]">
    console->println("processTurntableEntry(): ");
    if (delegate) {
        //find the Turntable entry to update
        if (turntables.size()>0) { 
            for (int i=0; i<turntables.size(); i++) {
                int id = args.get(1).toInt();
                Turntable turntablesTurntable = turntables.get(i);
                if (turntablesTurntable.getTurntableId()==id) {
                    int id = args.get(1).toInt();
                    Turntable turntable = turntablesTurntable;
                    if (args.size() <= 3) {  // server did not find the id
                        bool rslt = turntable.initTurntable(id, "Unknown", TurntableTypeUnknown, 0);
                    } else {
                        bool rslt = turntable.initTurntable(id, args.get(5), args.get(2).toInt(), args.get(3).toInt());
                    }
                    turntables.set(i, turntable);
                }
            }
        } 
    }
    console->println("processTurntableEntry(): end");
}

//private
void DCCEXProtocol::processTurntableIndexEntry(LinkedList<String> &args) { // <jP id index angle "[desc]">
    console->println("processTurntableIndexEntry(): ");
    if (delegate) {
        if (args.size() <= 3) {  // server did not find the index
            //find the Turntable entry to update
            if (turntables.size()>0) { 
                for (int i=0; i<turntables.size(); i++) {
                    int id = args.get(1).toInt();
                    Turntable turntablesTurntable = turntables.get(i);
                    if (turntablesTurntable.getTurntableId()==id) {
                        int id = args.get(1).toInt();
                        Turntable turntable = turntablesTurntable;

                        //this assumes we are always starting from scratch, not updating indexes
                        int index = args.get(2).toInt();
                        int angle = args.get(3).toInt();
                        String name = args.get(4);

                        bool rslt = turntablesTurntable.addTurntableIndex (index, name, angle);
                        turntables.set(i, turntable);
                    }
                }
            }
        }
    }
    console->println("processTurntableIndexEntry(): end");
}

//private
void DCCEXProtocol::processTurntableAction(LinkedList<String> &args) { // <i id position moving>
    console->println("processTurntableAction(): ");
    if (delegate) {
        int id = args.get(1).toInt();
        int newPos = args.get(2).toInt();
        TurntableState state = args.get(3).toInt();
        int pos = findTurntableListPositionFromId(id);
        if (pos!=newPos) {
            turntables.get(pos).actionTurntableExternalChange(newPos, state);
        }
        delegate->receivedTurntableAction(id, newPos, state);
    }
    console->println("processTurntableAction(): end");
}

// ****************
// Locos

//private
bool DCCEXProtocol::processLocoAction(LinkedList<String> &args) { //<l cab reg speedByte functMap>
    console->println("processLocoAction()");
    if (delegate) {
        int address = args.get(1).toInt();
        int speedByte = args.get(3).toInt();
        int functMap = args.get(4).toInt();
        int throttleNo = findThrottleWithLoco(address);
        if (throttleNo>=0) {
            int rslt = throttleConsists[throttleNo].consistGetLocoPosition(address);
            if (rslt==0) {  // ignore everything that is not the lead loco
                int speed = getSpeedFromSpeedByte(speedByte);
                Direction dir = getDirectionFromSpeedByte(speedByte);
                int functionStates[MAX_FUNCTIONS];
                Functions fns = getFunctionStatesFromFunctionMap(functionStates, functMap);
                bool rslt = throttleConsists[throttleNo].actionConsistExternalChange(speed, dir, fns);

                delegate->receivedSpeed(throttleNo, speed);
                delegate->receivedDirection(throttleNo, dir);
            }
        } else {
            console->print("processLocoAction(): unknown loco");
        }
    }
    console->println("processLocoAction() end");
}

// ******************************************************************************************************
// server commands

bool DCCEXProtocol::sendServerDetailsRequest() {
    console->println("sendServerDetailsRequest(): ");
    if (delegate) {
       sendCommand("<s>");	
    }
    console->println("sendServerDetailsRequest(): end");
    return true; 
}

// ******************************************************************************************************
// power commands

bool DCCEXProtocol::sendTrackPower(TrackPower state) {
    console->println("sendTrackPower(): ");
    if (delegate) {
       sendCommand("<" + String(state) + ">");	
    }
    console->println("sendTrackPower(): end");
    return true;
}

bool DCCEXProtocol::sendTrackPower(TrackPower state, char track) {
    console->println("sendTrackPower(): ");
    if (delegate) {
        sendCommand("<" + String(state) + " " + String(track) + ">");	
    }
    console->println("sendTrackPower(): end");
    return true;
}

// ******************************************************************************************************

void DCCEXProtocol::sendEmergencyStop() {
    console->println("emergencyStop(): ");
    if (delegate) {
            sendCommand("<!>");
    }
    console->println("emergencyStop(): end");
}

// ******************************************************************************************************

bool DCCEXProtocol::sendFunction(int throttle, String address, int funcNum, bool pressed) {
    console->println("sendFunction(): ");
    if (delegate) {
        sendCommand("<F " + String(address) + " " + String(funcNum) + " " + String(pressed) + ">");
    }
    console->println("sendFunction(): end"); 
    return true;
}


// ******************************************************************************************************
// throttle

bool DCCEXProtocol::sendThrottleAction(int throttle, int speed, Direction direction) {
    console->println("sendThrottleAction(): ");
    if (delegate) {
        if (throttleConsists[throttle].consistGetNumberOfLocos()>0) {
            throttleConsists[throttle].consistSetSpeed(speed);
            throttleConsists[throttle].consistSetDirection(direction);
            for (int i=0; i<throttleConsists[throttle].consistGetNumberOfLocos(); i++) {
                ConsistLoco conLoco = throttleConsists[throttle].consistGetLocoAtPosition(i);
                int address = conLoco.getLocoAddress();
                Direction dir = direction;
                if (conLoco.getConsistLocoFacing()==Reverse) {
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
    console->println("sendThrottleAction(): end");
    return true;
}



// ******************************************************************************************************
// individual locos
// 

bool DCCEXProtocol::sendLocoUpdateRequest(int address) {
    console->println("sendLocoUpdateRequest()");
    if (delegate) {
        sendCommand("<t " + String(address) + ">");
    }
    console->println("sendLocoUpdateRequest() end");
    return true;
}

bool DCCEXProtocol::sendLocoAction(int address, int speed, Direction direction) {
    console->print("sendLocoAction(): "); console->println(address);
    if (delegate) {
        sendCommand("<t " + String(address) + " " + String(speed) + " " + String(direction) + ">");
    }
    console->println("sendLocoAction(): end");
    return true;
}

// ******************************************************************************************************

bool DCCEXProtocol::sendRouteAction(int routeId) {
    console->println("sendRouteAction()");
    if (delegate) {
        sendCommand("</START " + String(routeId) + ">");
    }
    console->println("sendRouteAction() end");
    return true;
}

bool DCCEXProtocol::sendPauseRoutes() {
    console->println("sendPauseRoutes()");
    if (delegate) {
        sendCommand("</PAUSE>");
    }
    console->println("sendPauseRoutes() end");
    return true;
}

bool DCCEXProtocol::sendResumeRoutes() {
    console->println("sendResumeRoutes()");
    if (delegate) {
        sendCommand("</RESUME>");
    }
    console->println("sendResumeRoutes() end");
    return true;
}

// ******************************************************************************************************


bool DCCEXProtocol::sendTurntableAction(int turntableId, int position, int activity) {
    console->println("sendTurntable()");
    if (delegate) {
        sendCommand("<I " + String(turntableId) + " " + String(position) + ">");
    }
    console->println("sendTurntable() end");
    return true;
}

bool DCCEXProtocol::sendAccessoryAction(int accessoryAddress, int activate) {
    console->println("sendAccessory()");
    if (delegate) {
        sendCommand("<a " + String(accessoryAddress) + " " + String(activate) + ">");
    }
    console->println("sendAccessory() end");
    return true;
}

bool DCCEXProtocol::sendAccessoryAction(int accessoryAddress, int accessorySubAddr, int activate) {
    console->println("sendAccessory()");
    if (delegate) {
        sendCommand("<a " + String(accessoryAddress) + " " + String(accessorySubAddr) + " " + String(activate) + ">");
    }
    console->println("sendAccessory() end");
    return true;
}

// ******************************************************************************************************

bool DCCEXProtocol::getRoster() {
    console->println("getRoster()");
    if (delegate) {
        sendCommand("<JR>");
    }
    console->println("getRoster() end");
    return true;
}

bool DCCEXProtocol::getTurnouts() {
    console->println("getTurnouts()");
    if (delegate) {
        sendCommand("<T>");
    }
    console->println("getTurnouts() end");
    return true;
}

bool DCCEXProtocol::getRoutes() {
    console->println("getRoutes()");
    if (delegate) {
        sendCommand("</ROUTES>");
    }
    console->println("getRoutes() end");
    return true;
}

bool DCCEXProtocol::getTurntables() {
    console->println("getTurntable()");
    if (delegate) {
        sendCommand("<T>");
    }
    console->println("getTurntable() end");
    return true;
}

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// helper functions

// private
// find which, if any, throttle has this loco selected
int DCCEXProtocol::findThrottleWithLoco(int address) {
    for (int i=0; i<MAX_THROTTLES; i++) {
        if (throttleConsists[i].consistGetNumberOfLocos()>0) {
            int pos = throttleConsists[i].consistGetLocoPosition(address);
            if (pos>0) {
                return i;
            }
        }
    }
    return -1;  //not found
}

int DCCEXProtocol::findTurnoutListPositionFromId(int id) {
    if (turnouts.size()>0) {
        for (int i=0; i<turnouts.size(); i++) {
            if (turnouts.get(i).getTurnoutId()==id) {
                return i;
            }
        }
    }
    return -1;
}

int DCCEXProtocol::findRouteListPositionFromId(int id) {
    if (routes.size()>0) {
        for (int i=0; i<routes.size(); i++) {
            if (routes.get(i).getRouteId()==id) {
                return i;
            }
        }
    }
    return -1;
}

int DCCEXProtocol::findTurntableListPositionFromId(int id) {
    if (turntables.size()>0) {
        for (int i=0; i<turntables.size(); i++) {
            if (turntables.get(i).getTurntableId()==id) {
                return i;
            }
        }
    }
    return -1;
}

// LinkedList<String> DCCEXProtocol::splitCommand(String text, char splitChar) {
bool DCCEXProtocol::splitCommand(LinkedList<String> &args, String text, char splitChar) {
    console->println("splitCommand()");
    String s = text;
    if (text.charAt(0)!=splitChar)  s = String(splitChar) + text;  // add a leading space if not already there
    if (text.charAt(text.length()-1)!=splitChar)  s = s + String(splitChar) ;  // add a trailing space if not already there
    int splitCount = countSplitCharacters(s, splitChar);
    console->print("splitCommand(): number of splits found: "); console->println(splitCount);
    // LinkedList<String> returnValue = LinkedList<String>();
    int index = -1;
    int index2;

    for(int i = 0; i < splitCount; i++) {
        index = s.indexOf(splitChar, index + 1);
        index2 = s.indexOf(splitChar, index + 1);
        if (s.charAt(index)==splitChar) index=index+1; // clear leading spaces

        if(index2 < 0) index2 = s.length() - 1;
        // returnValue.add(s.substring(index, index2));
        args.add(s.substring(index, index2));
        // console->print(index); console->print("-"); console->print(index);
        // console->print(": "); console->println(s.substring(index, index2));
    }
    console->println("splitCommand(): end");
    // return returnValue;
    return true;
}

int DCCEXProtocol::countSplitCharacters(String text, char splitChar) {
    console->println("countSplitCharacters()");
    int returnValue = 0;
    int index = 0;

    while (index > -1) {
        index = text.indexOf(splitChar, index + 1);
        if(index > -1) returnValue+=1;
    }
    console->println("countSplitCharacters() end");
    return returnValue;
} 

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// subsidary classes

// class Functions

    bool Functions::initFunction(int functionNumber, String label, FunctionLatching latching, FunctionState state) {
        functionName[functionNumber] = label;
        functionLatching[functionNumber] = latching;
        functionState[functionNumber] = state;
        return true;
    }
    bool Functions::setFunctionState(int functionNumber, FunctionState state) {

        // ??????????????????????????????????

        return true;
    }

    bool Functions::actionFunctionStateExternalChange(int functionNumber, FunctionState state) {

        // ??????????????????????????????????

    }

    String Functions::getFunctionName(int functionNumber) {
        return functionName[functionNumber];
    }
    FunctionState Functions::getFunctionState(int functionNumber) {
        return functionState[functionNumber];
    }
    FunctionLatching Functions::getFunctionLatching(int functionNumber) {
        return functionLatching[functionNumber];
    }

// class Loco

    bool Loco::initLoco(int address, String name, LocoSource source) {
        locoAddress = address;
        locoName = name;
        locoSource = source;
        locoDirection = Forward;
        locoSpeed = 0;
    }
    bool Loco::setLocoSpeed(int speed) {
        if (locoSpeed!=speed) {
            locoSpeed = speed;
            // sendLocoAction(locoAddress, speed, locoDirection);
        }
        locoSpeed = speed;
        return true;
    }
    bool Loco::setLocoDirection(Direction direction) {
        if (locoDirection!=direction) {
            locoDirection = direction;
        }
        locoDirection = direction;
        return true;
    }

    int Loco::getLocoAddress() {
        return locoAddress;
    }
    String Loco::getLocoName() {
        return locoName;
    }
    LocoSource Loco::getLocoSource() {
        return locoSource;
    }
    int  Loco::getLocoSpeed() {
        return locoSpeed;
    }
    Direction Loco::getLocoDirection() {
        return locoDirection;
    }

// class ConsistLoco : public Loco

    bool ConsistLoco::setConsistLocoFacing(Facing facing) {
        consistLocoFacing = facing;
    }
    Facing ConsistLoco::getConsistLocoFacing() {
        return consistLocoFacing;
    }

// class Consist

    bool Consist::initConsist(String name) {
        consistName = name;
    }
    bool Consist::consistAddLoco(Loco loco, Facing facing) {
        int address = loco.getLocoAddress();
        int rslt = consistGetLocoPosition(address);
        if (rslt<0) { // not already in the list, so add it
            ConsistLoco conLoco;
            conLoco.initConsistLoco(loco, facing);
            consistLocos.add(conLoco);
            return true;
        }
        return false;
    }
    bool Consist::consistReleaseAllLocos()  {
        if (consistLocos.size()>0) {
            consistLocos.clear();
        }
    }
    bool Consist::consistReleaseLoco(int locoAddress)  {
        int rslt = consistGetLocoPosition(locoAddress);
        if (rslt>=0) {
            consistLocos.remove(rslt);
            return true;
        }
        return false;
    }
    int Consist::consistGetNumberOfLocos() {
        return consistLocos.size();
        return 0;
    }
    ConsistLoco Consist::consistGetLocoAtPosition(int position) {
        if (position<consistLocos.size()) {
            return consistLocos.get(position);
        }
        return {};
    }
    int Consist::consistGetLocoPosition(int locoAddress) {
        for (int i; i<consistLocos.size(); i++) {
            if (consistLocos.get(i).getLocoAddress() == locoAddress) {
                return i;
            }
        }
        return -1;
    }
    bool Consist::consistSetSpeed(int speed) {
        if (consistLocos.size()>0) {
            if (consistSpeed!=speed) {
                for (int i=0; i<consistLocos.size(); i++) {
                    bool rslt = consistLocos.get(i).setLocoSpeed(speed);
                }
            }
        }
        consistSpeed = speed;
        return true;
    }
    int Consist::consistGetSpeed() {
        return consistSpeed;
    }
    bool Consist::consistSetDirection(Direction direction) {
        if (consistLocos.size()>0) {
            if (consistDirection!=direction) {
                for (int i=0; i<consistLocos.size(); i++) {
                    Direction locoDir = direction;
                    if (consistLocos.get(i).getConsistLocoFacing()!=FacingForward) { // lead loco 'facing' is always assumed to be forward
                        if (direction == Forward) {
                            locoDir = Reverse;
                        } else {
                            locoDir = Forward;
                        }
                    }
                    bool rslt = consistLocos.get(i).setLocoSpeed(consistSpeed);
                    rslt = consistLocos.get(i).setLocoDirection(locoDir);
                }
            }
        }
        consistDirection = direction;
        return true;
    }
    bool Consist::actionConsistExternalChange(int speed, Direction direction, Functions functions) {
        if (consistLocos.size()>0) {
            if ( (consistDirection != direction) || (consistSpeed != speed) ) {
                for (int i=0; i<consistLocos.size(); i++) {
                    Direction locoDir = direction;
                    if (consistLocos.get(i).getConsistLocoFacing()!=FacingForward) { // lead loco 'facing' is always assumed to be forward
                        if (direction == Forward) {
                            locoDir = Reverse;
                        } else {
                            locoDir = Forward;
                        }
                    }
                    bool rslt = consistLocos.get(i).setLocoSpeed(consistSpeed);
                    rslt = consistLocos.get(i).setLocoDirection(locoDir);
                }
            }
        }

        // ??????????????????????????????????

        // delegate->receivedFunction(throttle, function, state);
    }

    Direction Consist::consistGetDirection() {
        return consistDirection;
    }
    bool Consist::consistSetFunction(int functionNo, FunctionState state) {
        // ??????????????????????????????????
    }
    bool Consist::consistSetFunction(int address, int functionNo, FunctionState state) {
        // ??????????????????????????????????
    }

    String Consist::getConsistName() {
        return consistName;
    }

// class Turnout

    bool Turnout::initTurnout(int id, String name, TurnoutState state) {
        turnoutId = id;
        turnoutName = name;
        turnoutState = state;
    }
    bool Turnout::setTurnoutState(TurnoutAction action) {
        TurnoutState newState = action;
        if (action == TurnoutToggle) {
            if (turnoutState == TurnoutClosed ) {
                newState = TurnoutThrown;
            } else { // Thrown or Inconsistant
                newState = TurnoutClosed;
            }
        }
        if (newState<=TurnoutThrow) { // ignore TurnoutExamine
            turnoutState = newState;
            // sendTurnoutAction(turnoutId, newState)
            return true;
        }
        return false;
    }
    // bool Turnout::sendTurnoutState(TurnoutAction action) {
    //     TurnoutState newState = action;
    //     if (action == TurnoutToggle) {
    //         if (turnoutState == TurnoutClosed ) {
    //             newState = TurnoutThrown;
    //         } else { // Thrown or Inconsistant
    //             newState = TurnoutClosed;
    //         }
    //     }
    //     if (newState<=TurnoutThrow) { // ignore TurnoutExamine
    //         turnoutState = newState;
    //         sendTurnoutAction(turnoutId, newState)
    //         return true;
    //     }
    //     return false;
    // }
    // bool Turnout::actionTurnoutExternalChange(TurnoutState state) {
    //     turnoutState = state;
    //     return true;
    // }
    int Turnout::getTurnoutId() {
        return turnoutId;
    }
    String Turnout::getTurnoutName() {
        return turnoutName;
    }
    TurnoutState Turnout::getTurnoutState() {
        return turnoutState;
    }

// class Route

    bool Route::initRoute(int id, String name) {
        routeId = id;
        routeName = name;
    }
    int Route::getRouteId() {
        return routeId;
    }

// class TurntableIndex

    bool TurntableIndex::initTurntableIndex(int index, String name, int angle) {
        turntableIndexIndex = index;
        turntableIndexName = name;
        turntableIndexAngle = angle;
    }

// class Turntable

    
    bool Turntable::initTurntable(int id, String name, TurntableType type, int position) {
        turntableId = id;
        turntableName = name;
        turntableType = type;
        turntableCurrentPosition = position;
    }

    int Turntable:: getTurntableId() {
        return turntableId;
    }
    String Turntable::getTurntableName() {
        return turntableName;
    }
    bool Turntable::addTurntableIndex(int index, String indexName, int indexAngle) {
        TurntableIndex turntableIndex;
        turntableIndex.turntableIndexIndex = index;
        turntableIndex.turntableIndexName = indexName;
        turntableIndex.turntableIndexAngle = indexAngle;
        turntableIndexes.add(turntableIndex);
    }
    bool Turntable::setTurntableCurrentPosition(int index) {
        if (turntableCurrentPosition != index) {
            turntableCurrentPosition = index;
            turntableIsMoving = TurntableMoving;
            return true;
        }
        return false;
    }    
    int Turntable::getTurntableCurrentPosition() {
        return turntableCurrentPosition;
    }
    int Turntable::getTurntableNumberOfIndexes() {
        return turntableIndexes.size();
    }
    TurntableIndex Turntable::getTurntableIndexAt(int positionInLinkedList) {
        return turntableIndexes.get(positionInLinkedList);
    }
    TurntableIndex Turntable::getTurntableIndex(int indexId) {
        for (int i=0; i<turntableIndexes.size(); i++) {
            if (turntableIndexes.get(i).turntableIndexId==indexId) {
                return turntableIndexes.get(i);
            }
        }
        return {};
    }
    // bool Turntable::sendTurntableRotateTo(int index) {
    //     if (turntableCurrentPosition != index) {
    //         sendCommand("<I " + String(turntableId) + " " + String(index) + ">");
    //         turntableCurrentPosition = index;
    //         turntableIsMoving = TurntableMoving;
    //         return true;
    //     }
    //     return false;
    // }
    bool Turntable::actionTurntableExternalChange(int index, TurntableState state) {
        turntableCurrentPosition = index;
        turntableIsMoving = state;
        return true;
    }
   TurntableState Turntable::getTurntableState() {
        TurntableState rslt = TurntableStationary;
        if (turntableIsMoving) {
            rslt = TurntableMoving;
        }
        return rslt;
    }
