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

//#include <ArduinoTime.h>
//#include <TimeLib.h>

#include "DCCEXProtocol.h"

static const int MIN_SPEED = 0;
static const int MAX_SPEED = 126;


DCCEXProtocol::DCCEXProtocol(bool server) {
	// store server/client
    this->server = server;
		
	// init streams
    stream = &nullStream;
	console = &nullStream;
}

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
    String command = "<U DISCONNECT>";
    sendCommand(command);
    this->stream = NULL;
}

bool DCCEXProtocol::check() {
    bool changed = false;

    if (stream) {
        while(stream->available()) {
            char b = stream->read();
            if (b == NEWLINE || b==CR) {
                // server sends TWO newlines after each command, we trigger on the
                // first, and this skips the second one
                if (nextChar != 0) {
                    inputbuffer[nextChar] = 0;
                    changed |= processCommand(inputbuffer, nextChar);
                }
                nextChar = 0;
            }
            else {
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
        }

        return changed;

    }
    else {
        return false;
    }
}


long DCCEXProtocol::getLastServerResponseTime() {
  return lastServerResponseTime;   
}


// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************

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

bool DCCEXProtocol::processCommand(char *c, int len) {
    console->println("processCommand()");
    bool changed = false;

    lastServerResponseTime = millis()/1000;

    console->print("<== "); console->println(c);

    String s(c);
    if (!(s.charAt(0) == '<')) {
        if (s.contains("<")) { // see if we can clean it up
            s = responseStr.substring(s.indexOf("<"));
        }
    }
    // remove any escaped double quotes
    String s = responseStr.replace("\\\"", "'");
    //split on spaces, with stuff between doublequotes treated as one item
    String[] args = s.substring(1, responseStr.length() - 1).split(" (?=(?:[^\"]*\"[^\"]*\")*[^\"]*$)", 999);
    int cleanLen = s.length();
    char *cleanC = s.c_str(); 

    if (delegate) {
        if (len > 3 && cleanC[1]=='i' && args[0]=="iDCCEX") {  //<iDCCEX version / microprocessorType / MotorControllerType / buildNumber>
            processServerDescription(args, cleanC, cleanLen);

        } else if (len > 1 && cleanC[1]=='p') { //<p onOff>
            processTrackPower(cleanC, cleanLen);

        } else if (len > 3 && cleanC[1]=='l') { //<l cab reg speedByte functMap>
            processLocoAction(args, cleanC, cleanLen);

        } else if (len > 3 && cleanC[1]=='j' && cleanC[2]=='R' && (args.length>1)) { 
            if ( (args.length<3) || (args[2].charAt(0) != '"') ) {  // loco list
                processRosterList(args, cleanC, cleanLen); //<jR [id1 id2 id3 ...]>
            } else { // individual
                processRosterEntry(args, cleanC, cleanLen); //<jR id ""|"desc" ""|"funct1/funct2/funct3/...">
            }

        } else if (len > 3 && cleanC[1]=='j' && cleanC[2]=='T' && (args.length>1)) { 
            if ( (args.length == 4) && (args[3].charAt(0) == '"') 
            || (args.length == 3) && (args[2].charAt(0) == 'X')) {
                processTurnoutEntry(args, cleanC, cleanLen); //<jT id state |"[desc]">   or    <jT id X">
            } else {
                processTurnoutList(args cleanC, cleanLen); //<jT [id1 id2 id3 ...]>
            }

        } else if (len > 3 && cleanC[1]=='H') { //<H id state>
            if (delegate) {
                delegate->receivedTurnoutAction(arg[1].toInt(), args[2].toInt());
            }

        } else if (len > 3 && cleanC[1]=='j' && cleanC[2]=='O' && (args.length>1)) { 
            if ( (args.length == 6) && (args[5].charAt(0) == '"') ) { //<jO id type position position_count "[desc]">
                processTuntableEntry(args, cleanC, cleanLen); 
            } else {
                processTurtableList(args, cleanC, cleanLen); //<jO [id1 id2 id3 ...]>
            } 

        } else if (len > 3 && cleanC[1]=='i' && args[0]!="iDCCEX") { //<i id position>   or   <i id position moving>
            if (delegate) {
                TurntableState state = TurntableStationary;
                if (args.length==3) {
                    state = args[2].toInt();
                }
                delegate->receivedTurnoutAction(arg[1].toInt(), state);
            }

        } else if (len > 3 && cleanC[1]=='j' && cleanC[2]=='T' && (args.length>1)) { 
            if ( (args.length == 4) && (args[3].charAt(0) == '"') 
            || (args.length == 3) && (args[2].charAt(0) == 'X')) {
                processRoutesEntry(args, cleanC, cleanLen); //<jA id type |"desc">   or  <jA id X>
            } else {
                processRoutesList(args, cleanC, cleanLen); //<jA [id0 id1 id2 ..]>
            }
        } else {
            processUnknownCommand(cleanC, cleanLen);
        }
    }

    console->println("processCommand() end");
    return false;
}

void DCCEXProtocol::processUnknownCommand(const String& unknownCommand) {
    console->println("processUnknownCommand()");
    if (delegate) {
        console->printf("unknown command '%s'\n", c);
    }
    console->println("processUnknownCommand() end");
}

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************

bool DCCEXProtocol::processLocoAction(String args[], char *c, int len) {
    console->println("processLocoAction()");
    if (delegate) {

        // delegate->receivedSpeed(throttle, speed);
        // delegate->receivedDirection(throttle, direction);
        // delegate->receivedFunction(throttle, function, state);
    }
    console->println("processLocoAction() end");
}

void DCCEXProtocol::processServerDescription(String args[], char *c, int len) { //<iDCCEX version / microprocessorType / MotorControllerType / buildNumber>
    if (delegate) {
        serverVersion = args[1];
        serverMicroprocessorType = args[2];
        serverMotorcontrollerType = args [3];
        serverBuildNumber = args[4];

        delegate->receivedServerDescription(serverDescription, serverVersion);
    }
}

void DCCEXProtocol::processTrackPower(char *c, int len) {
    console->println("processTrackPower()");

    if (delegate) {
        if (len > 0) {
            TrackPower state = PowerUnknown;
            if (c[1]=='0') {
                state = PowerOff;
            }
            else if (c[1]=='1') {
                state = PowerOn;
            }

            delegate->receivedTrackPower(state);
        }
    }
}

bool DCCEXProtocol::setTrackPower(TrackPower state) {
    sendCommand("<" + state + ">");	
    return true;
}

bool DCCEXProtocol::setTrackPower(TrackPower state, char track) {
    sendCommand("<" + state + " " + track + ">");	
    return true;
}

// ****************

void DCCEXProtocol::processRosterList(String args[], String args[], char *c, int len) {
    console->println("processRosterList()");
    if (delegate) {
        if (roster.size()>0) { // already have a roster so this is an update
            roster.clear();
        } 
        for (i=1; i<args.length(); i++) {
            int address = arg[1].toInt();
            Loco loco = new Loco();
            bool rslt = loco.setLoco(address, "", LocoSourceRoster)
            roster.add(loco);
            requestRosterEntry(address);
        }
    }
    console->println("processRosterList(): end");
}

void requestRosterEntry(int address) {
    console->println("requestRosterEntry()");
    if (delegate) {
        sendCommand("<JR "+address">");
    }
    console->println("requestRosterEntry() end");
}

void processRosterEntry(String args[], char *c, int len) { //<jR id ""|"desc" ""|"funct1/funct2/funct3/...">
    console->println("processRosterEntry()");
    if (delegate) {
        //find the roster entry to update
        if (roster.size()>0) { 
            for (i=0; i<roster.size(); i++) {
                int address = args[1].toInt();
                Loco rosterLoco = roster.get(i);
                if (rosterLoco.getLocoAddress()==address) {
                    int address = arg[1].toInt();
                    Loco loco = new Loco();
                    bool rslt = loco.setLoco(address, args[2], LocoSourceRoster)

                    String functions[] = args[2].split("/", 999);
                    for (j=0; (j<functions.length() && j<MAX_FUNCTIONS) ) {
                        FunctionLatching latching = FunctionLatchingTrue;
                        if (functions[j].charAt(0)=='*') {
                            latching = FunctionLatchingFalse;
                        }
                        loco.locoFunctions[j].setFunction(j, functions[j], latching, FunctionStateOff); 
                    }
                    roster.set(loco);
                }
            }
        } 
    }
    console->println("processRosterEntry()");
}

// ****************

void DCCEXProtocol::processTurnoutList(String args[], char *c, int len) {
    console->println("processTurnoutList()");
    if (delegate) {
        if (turnouts.size()>0) { // already have a turnouts list so this is an update
            turnouts.clear();
        } 
        for (i=1; i<args.length(); i++) {
            int id = arg[1].toInt();
            Turnout turnout = new Turnout();
            bool rslt = turnout.setTurnout(id, "", TurnoutClose);
            turnouts.add(turnout);
            requestTurnoutEntry(id);
        }
    }
    console->println("processTurnoutList(): end");
}

void requestTurnoutEntry(int address) {
    console->println("requestTurnoutEntry()");
    if (delegate) {
        sendCommand("<JT "+address">");
    }
    console->println("requestTurnoutEntry() end");
}

void DCCEXProtocol::processTurnoutEntry(String args[], char *c, int len) {
    console->println("processTurnoutEntry()");
    if (delegate) {
        //find the turnout entry to update
        if (turnouts.size()>0) { 
            for (i=0; i<turnouts.size(); i++) {
                Turnout turnoutsTurnout = turnouts.get(i);
                int id = args[1].toInt();
                if (turnoutsTurnout.getTurnoutId()==id) {
                    int id = arg[1].toInt();
                    Turnout turnout = new Turnout();
                    bool rslt = turnout.setTurnout(id, args[2], TurnoutClosed);
                    turnouts.set(turnout);
                }
            }
        } 
    }
    console->println("processTurnoutEntry() end");
}

bool DCCEXProtocol::setTurnout(int turnoutId, TurnoutAction action) {
    sendCommand("<T " + turnoutId + " " + action + ">");
    return true;
}

void DCCEXProtocol::processTurnoutAction(String args[], char *c, int len) {
    console->println("processTurnoutAction(): ");
    if (delegate) {

    }
    console->println("processTurnoutAction(): end");
}

// ****************

void DCCEXProtocol::processRouteList(String args[], char *c, int len) {
    console->println("processRouteList()");
    if (delegate) {
        if (routes.size()>0) { // already have a routes list so this is an update
            routes.clear();
        } 
        for (i=1; i<args.length(); i++) {
            int id = arg[1].toInt();
            Route route = new Route();
            bool rslt = routes.setRoute(id, "");
            routes.add(route);
            requestTurnoutEntry(id);
        }
    }
    console->println("processRouteList(): end");
}

void DCCEXProtocol::processRouteEntry(String args[], char *c, int len) {
    console->println("processRouteEntry()");
    if (delegate) {
        //find the Route entry to update
        if (routes.size()>0) { 
            for (i=0; i<routes.size(); i++) {
                int id = arg[1].toInt();
                Route routesRoute = routes.get(i);
                if (routesRoute.getId()==id) {
                    int id = arg[1].toInt();
                    Route route = new Route();
                    bool rslt = route.setRoute(id, args[2]);
                    routes.set(route);
                }
            }
        } 
    }
    console->println("processTurnoutEntry() end");
}
bool DCCEXProtocol::setRoute(int routeId) {
    sendCommand("</START " + routeId + ">");
    return true;
}

void DCCEXProtocol::processRouteAction(String args[], char *c, int len) {
    console->println("processRouteAction(): ");
    if (delegate) {

    }
    console->println("processRouteAction(): end");
}

bool DCCEXProtocol::pauseRoutes() {
    console->println("pauseRoutes()");
    if (delegate) {
    }
    console->println("pauseRoutes() end");
    return true;
}

bool DCCEXProtocol::resumeRoutes() {
    console->println("resumeRoutes()");
    if (delegate) {
    }
    console->println("resumeRoutes() end");
    return true;
}

// ****************

void DCCEXProtocol::processTurntableAction(String args[], char *c, int len) {
    console->println("processTurntableAction(): ");
    if (delegate) {

    }
    console->println("processTurntableAction(): end");
}

// ******************************************************************************************************
// throttle methods

bool DCCEXProtocol::addLocomotive(int throttle, int address) {
    console->print("addLocomotive(): "); console->print(throttle); console->print(" : "); console->println(address);
    bool ok = false;
    return ok;
}

bool DCCEXProtocol::releaseLocomotive(int throttle, String address) {
    console->print("releaseLocomotive(): "); console->print(throttle); console->print(" : "); console->println(address);
    return true;
}

int DCCEXProtocol::getLocomotiveAtPosition(int throttle, int position) {
    console->print("getLocomotiveAtPosition(): "); console->print(throttle); console->print(" : "); console->println(position);
    return {};
}

int DCCEXProtocol::getNumberOfLocos(int throttle) {
    console->print("getNumberOfLocos(): "); console->println(throttle);
    int size = 0;
    console->print("getNumberOfLocomotives(): end "); console->println(size);
    return size;
}

bool DCCEXProtocol::setSpeed(int throttle, int speed) {
    console->print("setSpeed(): "); console->print(throttle); console->print(" : "); console->println(speed);
    console->println("setSpeed(): end"); 
    return true;
}


void DCCEXProtocol::emergencyStop() {
    console->print("emergencyStop(): ");
    sendCommand("<!>");
    console->print("emergencyStop(): end");
}

void DCCEXProtocol::setFunction(int throttle, String address, int funcNum, bool pressed) {
    console->print("setFunction(): "); console->print(multiThrottle); console->print(" : "); console->println(funcNum);

    sendCommand(cmd);

    console->println("setFunction(): end"); 
}

// ******************************************************************************************************
// individual locos

void DCCEXProtocol::requestLocoUpdate(int address) {

}

int DCCEXProtocol::sendLoco(int address, int speed, Direction direction) {
    console->print("setLoco(): "); console->println(address);
    console->println("setLoco(): end");
}


// ******************************************************************************************************

// ******************************************************************************************************


bool DCCEXProtocol::setTurntable(int TurntableId, int position, int activity) {

    return true;
}

bool DCCEXProtocol::setAccessory(int accessoryAddress, int activate) {

    return true;
}

bool DCCEXProtocol::setAccessory(int accessoryAddress, int accessorySubAddr, int activate) {

    return true;
}



void DCCEXProtocol::processRouteEntry(char *c, int len) {

}

void DCCEXProtocol::processTurntableList(char *c, int len) {

}

void DCCEXProtocol::processTurntableEntry(char *c, int len) {

}

void DCCEXProtocol::processTurntableIndexEntry(char *c, int len) {

}

void DCCEXProtocol::processTurntableAction(char *c, int len) {

}

// *****************************************************************

class Functions {
    String functionName[MAX_FUNCTIONS];
    int functionState[MAX_FUNCTIONS];
    int functionLatching[MAX_FUNCTIONS];
    int functionState[MAX_FUNCTIONS];

    bool setFunction(int functionNumber, String label, FunctionLatching latching, FunctionState state) {}
    bool setFunctionState(int functionNumber, FunctionState state) {}
    String getFunctionName(int functionNumber) {
        return FunctionName[functionNumber];
    }
    FunctionState getFunctionState(int functionNumber) {
        return FunctionState[functionNumber];
    }
    FunctionLatching getFunctionLatching(int functionNumber) {
        return FunctionLatching[functionNumber];
    }

}

class Loco {
    int locoAddress;
    String locoName;
    int locoSpeed;
    Direction locoDirection;
    Functions locoFunctions;
    LocoSource locoSource;

    bool setLoco(int address, String name, LocoSource source) {
        locoAddress = address;
        locoName = name;
        locoSource = source;
    }
    bool setLocoSpeed(int speed) {
        locoSpeed  =speed
    }
    bool setLocoDirection(Direction direction) {
        locoDirection = direction;
    }

    int getLocoAddress() {
        return locoAddress;
    }
    String getLocoName() {
        return locoName;
    }
    LocoSource getLocoSource() {
        return locoSource;
    }
    int  getLocoSpeed() {
        return locoSpeed;
    }
    Direction getLocoDirection() {
        return locoDirection
    }
}

class ConsistLoco : public Loco {
    Facing consistLocoFacing;
    bool setConsistLocoFacing(Facing facing) {
        consistLocoFacing = facing;
    }
    Facing getConsistLocoFacing() {
        return consistLocoFacing;
    }
}

class Consist {
    LinkedList<ConsistLoco> consistLocos = LinkedList<ConsistLoco>();
    int consistSpeed;
    Direction consistDirection;
    String consistName;

    bool consistAddLoco(Loco loco, Facing facing) {}
    bool consistReleaseLoco() {}
    
    bool consistReleaseLoco(int locoAddress) {}
    bool consistGetNumberOfLocos() {}
    bool consistGetLocoAtPosition(int position) {}
    bool consistGetLocoPosition(int locoAddress) {}

    bool consistSetSpeed(int speed) {}
    int consistGetSpeed() {}
    bool consistSetDirection(Direction direction) {}
    Direction consistGetDirection() {}
    
    bool consistSetFunction(int functionNo, FunctionState state) {}
    bool consistSetFunction(int address, int functionNo, FunctionState state) {}

    String getConsistName() {
        return consistName;
    }

}

class Turnout {
    int turnoutId;
    String turnoutName;
    TurnoutState turnoutState;

    bool setTurnout(int id, String name, TurnoutState state) {}
    bool setTurnoutState(TurnoutState state) {}
    int getTurnoutId() {
        return turnoutId;
    }
    String getTurnoutName() {
        return turnoutName;
    }
    TurnoutState getTurnoutState() {
        return turnoutState;
    }
}

class Route {
    int routeId;
    String routeName;

    bool setRoute(int id, String name) {
        routeId = id;
        routeName = name;
    }
    int getRouteId() {
        return routeId;
    }
}

class TurntableIndex {
    int turntableIndexId;
    String turntableIndexName;
    int turntableAngle;
}

class Turntable {
    int turntableId;
    String turntableName;
    int turntableCurrentPosition;
    LinkedList<TurntableIndex> turntableIndexes = LinkedList<TurntableIndex>;
    bool turntableIsMoving;
    
    int getTurntableId() {
        return turntableId;
    }
    String getTurntableName() {
        return turntableName;
    }
    bool addTurntableIndex(int turntableIndexId, String turntableIndexName, int turntableAngle) {
        TurntableIndex index = new TurntableIndex();
        index.turntableIndexId = turntableIndexId;
        index.turntableIndexName = turntableIndexName;
        index.turntableAngle = turntableAngle;
    }
    int getTurntableCurrentPosition() {
        return turntableCurrentPosition;
    }
    int getTurntableNumberOfIndexes() {
        return turntableIndexes.size();
    }
    TurntableIndex getTurntableIndexAt(int positionInLinkedList) {
        return turntableIndexes.get(positionInLinkedList);
    }
    TurntableIndex getTurntableIndex(int indexId) {
        for (int i=0; i<turntableIndexes.size(); i++) {
            if (turntableIndexes.get(i).turntableId==indexId) {
                return turntableIndexes.get(i);
            }
        }
        return {};
    }
    bool rotateTurntableTo(int index) {}
    TurntableState getTurntableState() {
        TurntableState rslt = TurntableStationary;
        if (turntableIsMoving) {
            rslt = TurntableMoving;
        }
        return rslt;
    }
}