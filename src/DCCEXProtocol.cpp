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

    String s(c);
    // if (!(s.charAt(0) == '<')) {
    //     if (s.indexOf("<")>0) { // see if we can clean it up
    //         s = s.substring(s.indexOf("<"));
    //     }
    // }
    // // remove any escaped double quotes
    // s = s.replace("\\\"", "'");

    //split on spaces, with stuff between doublequotes treated as one item
    // String args[] = s.substring(1, s.length() - 1).split(" (?=(?:[^\"]*\"[^\"]*\")*[^\"]*$)", 999);

    LinkedList<String> args = LinkedList<String>();
    args = splitCommand(s.substring(1, s.length() - 1),' ');
    char char0 = args.get(0).charAt(0);
    char char1 = args.get(0).charAt(1);
    int noOfParameters = args.size();

    if (delegate) {
        if (len > 3 && char0 == 'i' && args.get(0) == "iDCCEX") {  //<iDCCEX version / microprocessorType / MotorControllerType / buildNumber>
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
                delegate->receivedTurnoutAction(args.get(1).toInt(), args.get(2).toInt());
            }

        } else if (len > 3 && char0=='j' && char1=='O' && (noOfParameters>1)) { 
            if ( (noOfParameters == 6) && (args.get(5).charAt(0) == '"') ) { //<jO id type position position_count "[desc]">
                processTurntableEntry(args); 
            } else {
                processTurntableList(args); //<jO [id1 id2 id3 ...]>
            } 

        } else if (len > 3 && char0=='i' && args.get(0)!="iDCCEX") { //<i id position>   or   <i id position moving>
            if (delegate) {
                TurntableState state = TurntableStationary;
                if (getSize(args)==3) {
                    state = args.get(2).toInt();
                }
                processTurntableAction(args);
            }

        } else if (len > 3 && char0 == 'j' && char1 == 'T' && (noOfParameters > 1)) { 
            if ( (noOfParameters == 4) && (args.get(3).charAt(0) == '"') 
            || (noOfParameters == 3) && (args.get(2).charAt(0) == 'X')) {
                processRouteEntry(args); //<jA id type |"desc">   or  <jA id X>
            } else {
                processRouteList(args); //<jA [id0 id1 id2 ..]>
            }
        } else {
            processUnknownCommand(c, len);
        }
    }

    console->println("processCommand() end");
    return false;
}

//private
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
// Process responses from the CS

//private
void DCCEXProtocol::processServerDescription(LinkedList<String> args) { //<iDCCEX version / microprocessorType / MotorControllerType / buildNumber>
    if (delegate) {
        serverVersion = args.get(1);
        serverMicroprocessorType = args.get(2);
        serverMotorcontrollerType = args [3];
        serverBuildNumber = args.get(4);

        delegate->receivedServerDescription(serverDescription, serverVersion);
    }
}

//private
void DCCEXProtocol::processTrackPower(LinkedList<String> args) {
    console->println("processTrackPower()");
    if (delegate) {
        TrackPower state = PowerUnknown;
        if (args.get(0).charAt(0)=='0') {
            state = PowerOff;
        }
        else if (args.get(0).charAt(0)=='1') {
            state = PowerOn;
        }

        delegate->receivedTrackPower(state);
    }
}

// ****************
// roster

//private
void DCCEXProtocol::processRosterList(LinkedList<String> args) {
    console->println("processRosterList()");
    if (delegate) {
        if (roster.size()>0) { // already have a roster so this is an update
            roster.clear();
        } 
        for (i=1; i<getSize(args); i++) {
            int address = args.get(i).toInt();
            Loco loco = new Loco();
            bool rslt = loco.initLoco(address, "", LocoSourceRoster)
            roster.add(loco);
            requestRosterEntry(address);
        }
    }
    console->println("processRosterList(): end");
}

//private
void DCCEXProtocol::sendRosterEntryRequest(int address) {
    console->println("requestRosterEntry()");
    if (delegate) {
        sendCommand("<JR "+address">");
    }
    console->println("requestRosterEntry() end");
}

//private
void DCCEXProtocol::processRosterEntry(LinkedList<String> args) { //<jR id ""|"desc" ""|"funct1/funct2/funct3/...">
    console->println("processRosterEntry()");
    if (delegate) {
        //find the roster entry to update
        if (roster.size()>0) { 
            for (i=0; i<roster.size(); i++) {
                int address = args.get(1).toInt();
                Loco rosterLoco = roster.get(i);
                if (rosterLoco.getLocoAddress()==address) {
                    int address = args.get(1).toInt();
                    Loco loco = new Loco();
                    bool rslt = loco.setLoco(address, args.get(2), LocoSourceRoster)

                    String functions[] = args.get(2).split("/", 999);
                    for (j=0; (j<functions.length() && j<MAX_FUNCTIONS) ) {
                        FunctionLatching latching = FunctionLatchingTrue;
                        if (functions[j].charAt(0)=='*') {
                            latching = FunctionLatchingFalse;
                        }
                        loco.locoFunctions[j].initFunction(j, functions[j], latching, FunctionStateOff); 
                    }
                    roster.set(loco);
                }
            }
        } 
    }
    console->println("processRosterEntry()");
}

// ****************
// Turnouts/Points

//private
void DCCEXProtocol::processTurnoutList(LinkedList<String> args) {
    console->println("processTurnoutList()");
    if (delegate) {
        if (turnouts.size()>0) { // already have a turnouts list so this is an update
            turnouts.clear();
        } 
        for (i=1; i<getSize(args); i++) {
            int id = args.get(i).toInt();
            Turnout turnout = new Turnout();
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
        sendCommand("<JT " + id + ">");
    }
    console->println("sendTurnoutEntryRequest() end");
}

//private
void DCCEXProtocol::processTurnoutEntry(LinkedList<String> args) {
    console->println("processTurnoutEntry()");
    if (delegate) {
        //find the turnout entry to update
        if (turnouts.size()>0) { 
            for (i=0; i<turnouts.size(); i++) {
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
    sendCommand("<T " + turnoutId + " " + action + ">");
    return true;
}

//private
void DCCEXProtocol::processTurnoutAction(LinkedList<String> args) {
    console->println("processTurnoutAction(): ");
    if (delegate) {

    // ??????????????????????????????????

    }
    console->println("processTurnoutAction(): end");
}

// ****************
// Routes

//private
void DCCEXProtocol::processRouteList(LinkedList<String> args) {
    console->println("processRouteList()");
    if (delegate) {
        if (routes.size()>0) { // already have a routes list so this is an update
            routes.clear();
        } 
        for (i=1; i<args(size)); i++) {
            int id = args.get(i).toInt();
            Route route = new Route();
            bool rslt = routes.setRoute(id, "");
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
        sendCommand("<JA " + address + ">");
    }
    console->println("sendRouteEntryRequest() end");
}

//private
void DCCEXProtocol::processRouteEntry(LinkedList<String> args) {
    console->println("processRouteEntry()");
    if (delegate) {
        //find the Route entry to update
        if (routes.size()>0) { 
            for (i=0; i<routes.size(); i++) {
                int id = args.get(1).toInt();
                Route routesRoute = routes.get(i);
                if (routesRoute.getId()==id) {
                    int id = args.get(1).toInt();
                    Route route = routesRoute;
                    bool rslt = route.setRoute(id, args.get(2));
                    routes.set(i, route);
                }
            }
        } 
    }
    console->println("processRouteEntry() end");
}

// void DCCEXProtocol::processRouteAction(LinkedList<String> args) {
//     console->println("processRouteAction(): ");
//     if (delegate) {

//     }
//     console->println("processRouteAction(): end");
// }

// ****************
// Turntables

void DCCEXProtocol::processTurntableList(LinkedList<String> args) {  // <jO [id1 id2 id3 ...]>
    console->println("processTurntableList(): ");
    if (delegate) {
        if (turntables.size()>0) { // already have a turntables list so this is an update
            turntables.clear();
        } 
        for (i=1; i<getSize(args); i++) {
            int id = args.get(i).toInt();
            Turntable turntable = new Turntable();
            bool rslt = turntable.initTurntable(id, "", TurntableTypeUnknown, 0, 0);
            turnouts.add(turnout);
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
        sendCommand("<JO " + id + ">");
    }
    console->println("sendTurntableEntryRequest() end");
}

//private
void DCCEXProtocol::sendTurntableIndexEntryRequest(int id) {
    console->println("sendTurntableIndexEntryRequest()");
    if (delegate) {
        sendCommand("<JP " + id + ">");
    }
    console->println("sendTurntableIndexEntryRequest() end");
}

//private
void DCCEXProtocol::processTurntableEntry(LinkedList<String> args) {  // <jO id type position position_count "[desc]">
    console->println("processTurntableEntry(): ");
    if (delegate) {
        //find the Turntable entry to update
        if (turntables.size()>0) { 
            for (i=0; i<turntables.size(); i++) {
                int id = args.get(1).toInt();
                Turntable turntablesTurntable = turntables.get(i);
                if (turntablesTurntable.getId()==id) {
                    int id = args.get(1).toInt();
                    Turntable turntable = turntablesTurntable;
                    if (getSize(args) <= 3) {  // server did not find the id
                        bool rslt = turntable.initTurntable(id, "Unknown", TurntableTypeUnknown, 0, 0);
                    } else {
                        bool rslt = turntable.initTurntable(id, args.get(5), args.get(2), args.get(3), args.get(4));
                    }
                    turntable.set(i, turntable);
                }
            }
        } 
    }
    console->println("processTurntableEntry(): end");
}

//private
void DCCEXProtocol::processTurntableIndexEntry(LinkedList<String> args) { // <jP id index angle "[desc]">
    console->println("processTurntableIndexEntry(): ");
    if (delegate) {
        if (getSize(args) <= 3) {  // server did not find the index
            //find the Turntable entry to update
            if (turntables.size()>0) { 
                for (i=0; i<turntables.size(); i++) {
                    int id = args.get(1).toInt();
                    Turntable turntablesTurntable = turntables.get(i);
                    if (turntablesTurntable.getId()==id) {
                        int id = args.get(1).toInt();
                        Turntable turntable = turntablesTurntable;

                        //this assumes we are always starting from scratch, not updating indexes
                        TurntableIndex index = new TurntableIndex();
                        bool rslt = index.initTurntableIndex (args.get(2).toInt(), args.get(4), args.get(3).toInt() );
                        turntable.turntableIndexes.add(index);
                        turntables.set(i, turntable);
                    }
                }
            }
        }
    }
    console->println("processTurntableIndexEntry(): end");
}

//private
void DCCEXProtocol::processTurntableAction(LinkedList<String> args) { // <i id position moving>
    console->println("processTurntableAction(): ");
    if (delegate) {
        int pos = findTurntableListPositionFromId(id);
        if (I>0) {
            turntables.get(pos).actionTurntableExternalChange(args.get(1).toInt(), args.get(2).toInt())
        }
        delegate->receivedTurnoutAction(args.get(1).toInt(), state);
    }
    console->println("processTurntableAction(): end");
}

// ****************
// Locos

//private
bool DCCEXProtocol::processLocoAction(LinkedList<String> args) { //<l cab reg speedByte functMap>
    console->println("processLocoAction()");
    if (delegate) {
        int address = args.get(1).toInt();
        int throttleNo = findThrottleWithLoco(address);
        if (throttleNo>=0) {
            bool rslt = throttleConsists[throttleNo].consistGetLocoPosition();
            if (rslt==0) {  /// ignore everythign that is not the lead loco
                bool actionConsistExternalChange(getSpeedFromSpeedByte(args.get(3).toInt()), getDirectionFromSpeedbyte(args.get(3).toInt()), getFunctionsFromFunctionMap(args.get(3).toInt());
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
    console->print("sendServerDetailsRequest(): ");
    if (delegate) {
       sendCommand("<s>");	
    }
    console->print("sendServerDetailsRequest(): end");
    return true; 
}

// ******************************************************************************************************
// power commands

bool DCCEXProtocol::sendTrackPower(TrackPower state) {
    console->print("sendTrackPower(): ");
    if (delegate) {
       sendCommand("<" + state + ">");	
    }
    console->print("sendTrackPower(): end");
    return true;
}

bool DCCEXProtocol::sendTrackPower(TrackPower state, char track) {
    console->print("sendTrackPower(): ");
    if (delegate) {
        sendCommand("<" + state + " " + track + ">");	
    }
    console->print("sendTrackPower(): end");
    return true;
}

// ******************************************************************************************************

void DCCEXProtocol::sendEmergencyStop() {
    console->print("emergencyStop(): ");
    if (delegate) {
            sendCommand("<!>");
    }
    console->print("emergencyStop(): end");
}

// ******************************************************************************************************

void DCCEXProtocol::sendFunction(int throttle, String address, int funcNum, bool pressed) {
    console->print("sendFunction(): "); console->print(multiThrottle); console->print(" : "); console->println(funcNum);
    if (delegate) {
        sendCommand(cmd);
    }
    console->println("sendFunction(): end"); 
}

// ******************************************************************************************************
// individual locos
// 

//private
bool DCCEXProtocol::sendLocoUpdateRequest(int address) {
    console->println("sendLocoUpdateRequest()");
    if (delegate) {
        sendCommand("<t " + address + ">");
    }
    console->println("sendLocoUpdateRequest() end");
    return true;
}

//private
bool DCCEXProtocol::sendLocoAction(int address, int speed, Direction direction) {
    console->print("sendLocoAction(): "); console->println(address);
    if (delegate) {
        sendCommand("<t " + address + " " + speed + " " + direction + ">");
    }
    console->println("sendLocoAction(): end");
    return true;
}


// ******************************************************************************************************

bool DCCEXProtocol::sendRouteAction(int routeId) {
    console->println("sendRouteAction()");
    if (delegate) {
        sendCommand("</START " + routeId + ">");
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
        sendCommand("<I " + turntableId + " " + position + ">");
    }
    console->println("sendTurntable()" end);
    return true;
}

bool DCCEXProtocol::sendAccessoryAction(int accessoryAddress, int activate) {
    console->println("sendAccessory()");
    if (delegate) {
        sendCommand("<a " + accessoryAddress + " " + activate + ">");
    }
    console->println("sendAccessory()" end);
    return true;
}

bool DCCEXProtocol::sendAccessoryAction(int accessoryAddress, int accessorySubAddr, int activate) {
    console->println("sendAccessory()");
    if (delegate) {
        sendCommand("<a " + accessoryAddress + " " + accessorySubAddr + " " + activate + ">");
    }
    console->println("sendAccessory()" end);
    return true;
}

// ******************************************************************************************************

bool DCCEXProtocol::getRoster() {
    console->println("getRoster()");
    if (delegate) {
        sendCommand("<JR>");
    }
    console->println("getRoster()" end);
    return true;
}

bool DCCEXProtocol::getTurnouts() {
    console->println("getTurnouts()");
    if (delegate) {
        sendCommand("<T>");
    }
    console->println("getTurnouts()" end);
    return true;
}

bool DCCEXProtocol::getRoutes() {
    console->println("getRoutes()");
    if (delegate) {
        sendCommand("</ROUTES>");
    }
    console->println("getRoutes()" end);
    return true;
}

bool DCCEXProtocol::getTurntables() {
    console->println("getTurntable()");
    if (delegate) {
        sendCommand("<T>");
    }
    console->println("getTurntable()" end);
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
        if (throttleConsists[i] != NULL) {
            int pos = throttleConsists[i].consistGetLocoPosition(address);
            if (pos>0) {
                return i;
            }
        }
    }
    return -1;  //not found
}


int DCCEXProtocol::findTurnoutListPositionFromId(int id) {
    if (turnouts != NULL) {
        for (int i=0; i<turnouts.size(); i++) {
            if (turnouts.get(i).getTurnoutId()==id) {
                return i;
            }
        }
    }
    return -1;
}

int DCCEXProtocol::findRouteListPositionFromId(int id) {
    if (routes != NULL) {
        for (int i=0; i<routes.size(); i++) {
            if (routes.get(i).getRouteId()==id) {
                return i;
            }
        }
    }
    return -1;
}

int DCCEXProtocol::findTurntableListPositionFromId(int id) {
    if (turntables != NULL) {
        for (int i=0; i<turntables.size(); i++) {
            if (turntables.get(i).getTurntableId()==id) {
                return i;
            }
        }
    }
    return -1;
}

int DCCEXProtocol::getSpeedFromSpeedByte(int speedByte) {
    int speed = speedByte;
    if (speed >= 128) {
        speed = speed - 128;
        dir = 1;
    }
    if (speed>1) {
        speed = speed - 1; // get round and idiotic design of the speed command
    } else {
        speed=0;
    }
    return speed;
}

int DCCEXProtocol::getDirectionFromSpeedByte(int speedByte) {
    Direction dir = Forward;
    int speed = speedByte;
    if (speed >= 128) {
        speed = speed - 128;
        dir = 1;
    }
    if (speed>1) {
        speed = speed - 1; // get round and idiotic design of the speed command
    } else {
        speed=0;
    }
    return dir;
}

int[] DCCEXProtocol::getFunctionStatesFromFunctionMap(int functionMap) {
    int states[MAX_FUNCTIONS];

    // ??????????????????????????????????

    return states;
}

LinkedList<String> DCCEXProtocol::splitCommand(String text, char splitChar) {
    int splitCount = countSplitCharacters(text, splitChar);
    LinkedList<String> returnValue = New LinkedList<String>();
    int index = -1;
    int index2;

    for(int i = 0; i < splitCount - 1; i++) {
        index = text.indexOf(splitChar, index + 1);
        index2 = text.indexOf(splitChar, index + 1);

        if(index2 < 0) index2 = text.length() - 1;
        returnValue.add(text.substring(index, index2));
    }

    return returnValue;
}

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// subsidary classes

class Functions {
    String functionName[MAX_FUNCTIONS];
    int functionLatching[MAX_FUNCTIONS];
    int functionState[MAX_FUNCTIONS];

    bool initFunction(int functionNumber, String label, FunctionLatching latching, FunctionState state) {
        functionName[functionNumber] = label;
        functionLatching[functionNumber] = latching;
        functionState[functionNumber] = state;
        return true;
    }
    bool setFunctionState(int functionNumber, FunctionState state) {

        // ??????????????????????????????????

        return true;
    }

    bool actionFunctionStateExternalChange(int functionNumber, FunctionState state) {

        // ??????????????????????????????????

    }

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

    bool initLoco(int address, String name, LocoSource source) {
        locoAddress = address;
        locoName = name;
        locoSource = source;
        locoDirection = Forward;
        locoSpeed = 0;

        sendLocoUpdateRequest(address);
    }
    bool setLocoSpeed(int speed) {
        if (locoSpeed!=speed) {
            sendLocoAction(locoAddress, speed, locoDirection);
        }
        locoSpeed = speed;
        return true;
    }
    bool setLocoDirection(Direction direction) {
        if (locoDirection!=direction) {
            sendLocoAction(locoAddress, speed, locoDirection);
        }
        locoDirection = direction;
        return true;
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

    bool initConsist(String name) {
        consistName = name;
    }
    bool consistAddLoco(Loco loco, Facing facing) {
        int address = loco.getLocoAddress();
        int rslt = consistGetLocoPosition(address);
        if (rslt<0) { // not already in the list, so add it
            consistLocos.add(loco);
            return true;
        }
        return false;
    }
    bool consistReleaseAllLocos()  {
        if (consistLocos != NULL) {
            consistLocos.clear();
        }
    }
    bool consistReleaseLoco(int locoAddress)  {
        int rslt = consistGetLocoPosition(locoAddress);
        if (rslt>=0) {
            consistLocos.remove(rslt);
            return true;
        }
        return false;
    }
    int consistGetNumberOfLocos() {
        if (consistLocos != NULL) {
            return consistLocos.size();
        }
        return 0;
    }
    Loco consistGetLocoAtPosition(int position) {
        if (cosistLocos != NULL) {
            if (position<consistLocos.size()) {
                return consistLocos.get(position);
            }
        }
        return false;
    }
    int consistGetLocoPosition(int locoAddress) {
        for (int i; i<consistLocos.size(); i++) {
            if (consistLocos.get(i).getLocoAddress() == locoAddress) {
                return i;
            }
        }
        return -1;
    }
    bool consistSetSpeed(int speed) {
        if (consistLocos != NULL) {
            if (consistSpeed!=speed) {
                for (int i=0; i<consistLocos.size(); i++) {
                    sendLocoAction(consistLocos.get(i).getLocoAddress(), speed, consistLocos.get(i).getLocoDirection());
                    bool rslt = consistLocos.get(i).setLocoSpeed(speed);
                }
            }
        }
        consistSpeed = speed;
        return true;
    }
    int consistGetSpeed() {
        return consistSpeed;
    }
    bool consistSetDirection(Direction direction) {
        if (consistLocos != NULL) {
            if (consistDirection!=direction) {
                for (int i=0; i<consistLocos.size(); i++) {
                    Direction locoDir = direction;
                    if (consistLocos.get(i).getConsistLocoFacing()!=FacingForward) { // lead loco 'facing' is always assumed to be forward
                        if (direction == Forward) {
                            locoDir = Reverse;
                        } else {
                            locoDir = Forward;
                        }
                        bool rslt = consistLocos.get(i).setLocoSpeed(speed);
                        rslt = consistLocos.get(i).setLocoDirection(locoDir);
                    }
                    sendLocoAction(consistLocos.get(i).getLocoAddress(), speed, locoDir);
                }
            }
        }
        consistDirection = direction;
        return true;
    }
    bool actionConsistExternalChange(int speed, Direction, direction, Functions functions) {
        if (consistLocos != NULL) {
            if ( (consistDirection != direction) || (consistSpeed != speed) ) {

                delegate->receivedSpeed(throttle, speed);
                delegate->receivedDirection(throttle, direction);

                for (int i=0; i<consistLocos.size(); i++) {
                    Direction locoDir = direction;
                    if (consistLocos.get(i).getConsistLocoFacing()!=FacingForward) { // lead loco 'facing' is always assumed to be forward
                        if (direction == Forward) {
                            locoDir = Reverse;
                        } else {
                            locoDir = Forward;
                        }
                        bool rslt = consistLocos.get(i).setLocoDirection(locoDir);
                    }
                    sendLocoAction(consistLocos.get(i).getLocoAddress(), speed, locoDir);
                }
            }
        }

        // ??????????????????????????????????

        // delegate->receivedFunction(throttle, function, state);
    }

    Direction consistGetDirection() {
        return consistDirection;
    }
    bool consistSetFunction(int functionNo, FunctionState state) {
        // ??????????????????????????????????
    }
    bool consistSetFunction(int address, int functionNo, FunctionState state) {
        // ??????????????????????????????????
    }

    String getConsistName() {
        return consistName;
    }
}

class Turnout {
    int turnoutId;
    String turnoutName;
    TurnoutState turnoutState;

    bool initTurnout(int id, String name, TurnoutState state) {
        turnoutId = id;
        turnoutName = name;
        turnoutState = state;
    }
    bool sendTurnoutState(TurnoutAction action) {
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
            sendTurnoutAction(turnoutId, newState)
            return true;
        }
        return false;
    }
    bool actionTurnoutExternalChange(TurnoutState state) {
        turnoutState = state;
        delegate->receivedTurnoutAction(turnoutId, state);
        return true;
    }
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
    int turntableIndexAngle;

    bool initTurntableIndex(int id, String name, TurntableType type, int angle) {
        turntableIndexId = id;
        turntableIndexName = name;
        turntableIndexAngle = type;
    }
}

class Turntable {
    int turntableId;
    String turntableName;
    int turntableCurrentPosition;
    LinkedList<TurntableIndex> turntableIndexes = LinkedList<TurntableIndex>;
    bool turntableIsMoving;
    
    bool initTurntable(int id, String name, TurntableType type, int position) {
        turntableId = id;
        turntableName = name;
        turntableCurrentPosition = type;
    }

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
    bool sendTurntableRotateTo(int index) {
        if (turntableCurrentPosition != index) {
            sendCommand("<I " + turntableId + " " + index + ">");
            turntableCurrentPosition = index;
            turntableIsMoving = TurntableMoving;
            return true;
        }
        return false;
    }
    bool actionTurntableExternalChange(int index, TurntableState state) {
        turntableCurrentPosition = index;
        turntableIsMoving = state;
        delegate->receivedTurntableAction(turntableId, position, state);
        return true;
    }
   TurntableState getTurntableState() {
        TurntableState rslt = TurntableStationary;
        if (turntableIsMoving) {
            rslt = TurntableMoving;
        }
        return rslt;
    }
}