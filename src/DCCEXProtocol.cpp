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
}

void DCCEXProtocol::disconnect() {
    sendCommand("<U DISCONNECT>");
    this->stream = NULL;
}

bool DCCEXProtocol::check() {
    // console->println(F("check()"));
    bool changed = false;

    if (stream) {
        while(stream->available()) {
            char b = stream->read();
            if (b == COMMAND_END) {
                if (nextChar != 0) {
                    inputbuffer[nextChar] = b;
                    nextChar++;
                    inputbuffer[nextChar] = 0;
                    changed |= processCommand(inputbuffer, nextChar);
                }
                nextChar = 0;
            } else if (b != NEWLINE && b != CR) {
            // } else {
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

void DCCEXProtocol::getFunctionStatesFromFunctionMap(FunctionState fnStates[], int functionMap) {
 
    for (uint i=0; i<MAX_FUNCTIONS; i++) {
        fnStates[i] = bitExtracted(functionMap, 1, i+1);
    }
}

int DCCEXProtocol::bitExtracted(int number, int k, int p)
{
    return (((1 << k) - 1) & (number >> (p - 1)));
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
    // console->println(F("processCommand()"));

    lastServerResponseTime = millis()/1000;

    console->print("<== "); console->println(c);

    String s = String(c);
    LinkedList<String> args;
    // splitCommand(args, s.substring(1, s.length() - 1),' ');
    splitValues(args, c);
    char char0 = args.get(0).charAt(0);
    char char1 = args.get(0).charAt(1);
    int noOfParameters = args.size();

    console->print("processing: "); console->println(s);
    // for (uint i=0; i<args.size();i++) {
    //     console->print("arg"); console->print(i); console->print(": ~"); console->print(args.get(i)); console->println("~");
    // }

    bool processed = false;
    if (delegate) {
        if (char0=='i') {
            if (args.get(0) == "iDCC-EX") {
                //<iDCCEX version / microprocessorType / MotorControllerType / buildNumber>
                processServerDescription(args);
                processed = true;
            } else {
                //<i id position>   or   <i id position moving>
                TurntableState state = TurntableStationary;
                if (args.size()<3) { 
                    state = TurntableMoving;
                    processed = true;
                }
                processTurntableAction(args);
                delegate->receivedTurntableAction(args.get(1).toInt(), args.get(2).toInt(), state);
                processed = true;
            }

        } else if (char0 == 'p') {
            //<p onOff>
            processTrackPower(args);
            processed = true;

        } else if (char0 == 'l') {
            //<l cab reg speedByte functMap>
            processLocoAction(args);
            processed = true;

        } else if (char0 == 'j') {
            if( char1 == 'R' ) {
                if (noOfParameters == 1) {  // empty roster
                    rosterFullyReceived = true;
                    processed = true;
                } else if (noOfParameters > 1) { 
                    if ( (noOfParameters<3) || (args.get(2).charAt(0) != '"') ) {  // loco list
                        //<jR [id1 id2 id3 ...]>
                        processRosterList(args); 
                        processed = true;
                    } else { // individual
                        //<jR id ""|"desc" ""|"funct1/funct2/funct3/...">
                        processRosterEntry(args); 
                        processed = true;
                    }
                }
            } else if (char1 == 'T') {
                if (noOfParameters == 1) { // empty list
                    turnoutListFullyReceived = true;
                    processed = true;
                } else if (noOfParameters > 1) { 
                    if ( ((noOfParameters == 4) && (args.get(3).charAt(0) == '"')) 
                    || ((noOfParameters == 3) && (args.get(2).charAt(0) == 'X')) ) {
                        //<jT id state |"[desc]">   or    <jT id X">
                        processTurnoutEntry(args); 
                        processed = true;
                    } else {
                        //<jT [id1 id2 id3 ...]>
                        processTurnoutList(args); 
                        processed = true;
                    }
                }

            } else if (char1=='O') {
                if (noOfParameters == 1) {  // empty list
                    turntableListFullyReceived = true;
                    processed = true;
                } else if (noOfParameters>1) { 
                    if ( (noOfParameters == 6) && (args.get(5).charAt(0) == '"') ) { 
                        //<jO id type position position_count "[desc]">
                        processTurntableEntry(args); 
                            processed = true;
                    } else {
                        //<jO [id1 id2 id3 ...]>
                        processTurntableList(args); 
                            processed = true;
                    } 
                }
            } else if  (char1=='P') {
                if (noOfParameters>1) { 
                    // <jP id index angle "[desc]">
                    processTurntableIndexEntry(args); 
                    processed = true;
                }

            } else if (char1 == 'A') {
                if (noOfParameters > 1) { 
                    if ( ((noOfParameters == 4) && (args.get(3).charAt(0) == '"'))
                    || ((noOfParameters == 3) && (args.get(2).charAt(0) == 'X')) ) {
                        //<jA id type |"desc">   or  <jA id X>
                        processRouteEntry(args); 
                        processed = true;
                    } else {
                        //<jA [id0 id1 id2 ..]>
                        processRouteList(args); 
                        processed = true;
                    }
                }
            }

        } else if (char0 == 'H') {
            // TurnoutState state = (int) args.get(2).toInt();
            if (args.size()==3) {
                processTurnoutAction(args);
                delegate->receivedTurnoutAction(args.get(1).toInt(), args.get(2).toInt());
                processed = true;
            }

        } else if (char0 == 'q') {
            // <q id>
            processSensorEntry(args);
            processed = true;

        } else if (char0 == 'X') {
            // error   Nothing we can do with it as we don't know what it is for
            processed = true;
        } 

        if (!processed) {
            processUnknownCommand(s);
        }

    }

    // if (delegate) {
    //     if (len > 3 && char0 == 'i' && args.get(0) == "iDCC-EX") {  //<iDCCEX version / microprocessorType / MotorControllerType / buildNumber>
    //         processServerDescription(args);

    //     } else if (len > 1 && char0 == 'p') { //<p onOff>
    //         processTrackPower(args);

    //     } else if (len > 3 && char0 == 'l') { //<l cab reg speedByte functMap>
    //         processLocoAction(args);

    //     } else if (len > 3 && char0 == 'j' && char1 == 'R' && (noOfParameters == 1)) {  // empty roster
    //         rosterFullyReceived = true;
    //     } else if (len > 3 && char0 == 'j' && char1 == 'R' && (noOfParameters > 1)) { 
    //         if ( (noOfParameters<3) || (args.get(2).charAt(0) != '"') ) {  // loco list
    //             processRosterList(args); //<jR [id1 id2 id3 ...]>
    //         } else { // individual
    //             processRosterEntry(args); //<jR id ""|"desc" ""|"funct1/funct2/funct3/...">
    //         }

    //     } else if (len > 3 && char0 == 'j' && char1 == 'T' && (noOfParameters == 1)) { // empty list
    //         turnoutListFullyReceived = true;
    //     } else if (len > 3 && char0 == 'j' && char1 == 'T' && (noOfParameters > 1)) { 
    //         if ( ((noOfParameters == 4) && (args.get(3).charAt(0) == '"')) 
    //         || ((noOfParameters == 3) && (args.get(2).charAt(0) == 'X')) ) {
    //             processTurnoutEntry(args); //<jT id state |"[desc]">   or    <jT id X">
    //         } else {
    //             processTurnoutList(args); //<jT [id1 id2 id3 ...]>
    //         }

    //     } else if (len > 3 && char0=='j' && char1=='A' && (noOfParameters == 1)) {  // empty list
    //         routeListFullyReceived = true;
    //     } else if (len > 3 && char0=='j' && char1=='A' && (noOfParameters > 1)) { 
    //         if ( ((noOfParameters == 4) && (args.get(3).charAt(0) == '"')) 
    //         || ((noOfParameters == 3) && (args.get(2).charAt(0) == 'X')) ) {
    //             processRouteEntry(args); //<jA id X|type |"desc">   or    <jA id X">
    //         } else {
    //             processRouteList(args); //<jA [id1 id2 id3 ...]>
    //         }

    //     } else if (len > 3 && char0 == 'H') { //<H id state>
    //         if (delegate) {
    //             // TurnoutState state = (int) args.get(2).toInt();
    //             if (args.size()==3) {
    //                 processTurnoutAction(args);
    //                 delegate->receivedTurnoutAction(args.get(1).toInt(), args.get(2).toInt());
    //             }
    //         }

    //     } else if (len > 3 && char0=='j' && char1=='O' && (noOfParameters == 1)) {  // empty list
    //         turntableListFullyReceived = true;
    //     } else if (len > 3 && char0=='j' && char1=='O' && (noOfParameters>1)) { 
    //         if ( (noOfParameters == 6) && (args.get(5).charAt(0) == '"') ) { //<jO id type position position_count "[desc]">
    //             processTurntableEntry(args); 
    //         } else {
    //             processTurntableList(args); //<jO [id1 id2 id3 ...]>
    //         } 

    //     } else if (len > 3 && char0=='j' && char1=='P' && (noOfParameters>1)) { // <jP id index angle "[desc]">
    //         processTurntableIndexEntry(args); 

    //     } else if (len > 3 && char0=='i' && args.get(0)!="iDCC-EX") { //<i id position>   or   <i id position moving>
    //         if (delegate) {
    //             TurntableState state = TurntableStationary;
    //             if (args.size()<3) { 
    //                 state = TurntableMoving;
    //             }
    //             processTurntableAction(args);
    //             delegate->receivedTurntableAction(args.get(1).toInt(), args.get(2).toInt(), state);
    //         }

    //     } else if (len > 3 && char0 == 'j' && char1 == 'A' && (noOfParameters > 1)) { 
    //         if ( ((noOfParameters == 4) && (args.get(3).charAt(0) == '"'))
    //         || ((noOfParameters == 3) && (args.get(2).charAt(0) == 'X')) ) {
    //             processRouteEntry(args); //<jA id type |"desc">   or  <jA id X>
    //         } else {
    //             processRouteList(args); //<jA [id0 id1 id2 ..]>
    //         }
    //     } else if (len > 3 && char0 == 'q') { // <q id>
    //         processSensorEntry(args);
    //     } else if (len >= 3 && char0 == 'X') { // <X>
    //         // error   Nothing we can do with it as we don't know what it is for
    //     } else {
    //         processUnknownCommand(s);
    //     }
    // }

    // console->println(F("processCommand() end"));
    return true;
}

//private
void DCCEXProtocol::processUnknownCommand(String unknownCommand) {
    console->println(F("processUnknownCommand()"));
    if (delegate) {
        console->print("unknown command '"); console->print("'"); console->println(unknownCommand);
    }
    console->println(F("processUnknownCommand() end"));
}

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// Process responses from the CS

//private
void DCCEXProtocol::processServerDescription(LinkedList<String> &args) { //<iDCCEX version / microprocessorType / MotorControllerType / buildNumber>
    // console->println(F("processServerDescription()"));
    if (delegate) {
        serverVersion = args.get(1);
        serverMicroprocessorType = args.get(3);
        serverMotorcontrollerType = args.get(5);
        // serverBuildNumber = args.get(6);

        haveReceivedServerDetails = true;
        delegate->receivedServerDescription(serverMicroprocessorType, serverVersion);
    }
    // console->println(F("processServerDescription(): end"));
}

bool DCCEXProtocol::isServerDetailsReceived() {
    return haveReceivedServerDetails;
}

//private
void DCCEXProtocol::processTrackPower(LinkedList<String> &args) {
    // console->println(F("processTrackPower()"));
    if (delegate) {
        TrackPower state = PowerUnknown;
        if (args.get(0).charAt(0)=='0') {
            state = PowerOff;
        } else if (args.get(0).charAt(0)=='1') {
            state = PowerOn;
        }

        delegate->receivedTrackPower(state);
    }
    //  console->println(F("processTrackPower(): end"));
}

// ****************
// roster

//private
void DCCEXProtocol::processRosterList(LinkedList<String> &args) {
    // console->println(F("processRosterList()"));
    if (delegate) {
        if (roster.size()>0) { // already have a roster so this is an update
            roster.clear();
        } 
        for (int i=1; i<args.size(); i++) {
            int address = args.get(i).toInt();
            roster.add(new Loco(address, "", LocoSourceRoster));
            sendRosterEntryRequest(address);
        }
    }
    // console->println(F("processRosterList(): end"));
}

//private
void DCCEXProtocol::sendRosterEntryRequest(int address) {
    // console->println(F("sendRosterEntryRequest()"));
    if (delegate) {
        sendCommand("<JR " + String(address) + ">");
    }
    // console->println(F("sendRosterEntryRequest(): end"));
}

//private
void DCCEXProtocol::processRosterEntry(LinkedList<String> &args) { //<jR id ""|"desc" ""|"funct1/funct2/funct3/...">
    // console->println(F("processRosterEntry()"));
    if (delegate) {
        //find the roster entry to update
        if (roster.size()>0) { 
            for (int i=0; i<roster.size(); i++) {
                int address = args.get(1).toInt();
                if (roster.get(i)->getLocoAddress() == address) {
                    // console->print("processRosterEntry(): found: "); console->println(address);
                    String name = args.get(2);
                    roster.get(i)->setLocoName(stripLeadAndTrailQuotes(name));
                    roster.get(i)->setLocoSource(LocoSourceRoster);
                    roster.get(i)->setIsFromRosterAndReceivedDetails();

                    // String functions = stripLeadAndTrailQuotes(args.get(3));
                    String functions = args.get(3);

                    LinkedList<String> functionArgs;
                    char* fns = new char[functions.length()+1];
                    strcpy(fns, functions.c_str());
                    // console->print("~~");console->print(fns);console->println("~~");
                    splitFunctions(functionArgs, fns);
                    int noOfParameters = functionArgs.size();

                    // for (uint i=0; i<functionArgs.size();i++) {
                    //     console->print("fn"); console->print(i); console->print(": ~"); console->print(functionArgs.get(i)); console->println("~");
                    // }

                    console->print("processing Functions: "); console->println(functions);

                    for (int j=0; (j<noOfParameters && j<MAX_FUNCTIONS); j++ ) {
                        // console->print("functionArgs"); console->print(j); console->print(": ~"); console->print(functionArgs.get(j)); console->println("~");
                        String functionName = functionArgs.get(j);
                        FunctionState state = FunctionStateOff;
                        FunctionLatching latching = FunctionLatchingTrue;
                        if (functionName.charAt(0)=='*') {
                            latching = FunctionLatchingFalse;
                        }
                        roster.get(i)->locoFunctions.initFunction(j, functionName, latching, state);
                    }
                }
            }
            bool rslt = true;
            for (int i=0; i<roster.size(); i++) {
                if (!roster.get(i)->getIsFromRosterAndReceivedDetails()) {
                    console->print(F("processRosterEntry(): not received yet: ~")); console->print(roster.get(i)->getLocoName()); console->print("~ ");console->println(roster.get(i)->getLocoAddress());
                    rslt = false;
                    break;
                }
            }
            if (rslt) {
                rosterFullyReceived = true;
                console->println(F("processRosterEntry(): received all"));
                delegate->receivedRosterList(roster.size());
            }
        } 
    }
    // console->println(F("processRosterEntry(): end"));
}

// ****************
// Turnouts/Points

//private
void DCCEXProtocol::processTurnoutList(LinkedList<String> &args) {
    // console->println(F("processTurnoutList()"));
    if (delegate) {
        if (turnouts.size()>0) { // already have a turnouts list so this is an update
            turnouts.clear();
        } 
        for (int i=1; i<args.size(); i++) {
            int id = args.get(i).toInt();
            turnouts.add(new Turnout(id, "", 0));
            sendTurnoutEntryRequest(id);
        }
    }
    // console->println(F("processTurnoutList(): end"));
}

//private
void DCCEXProtocol::sendTurnoutEntryRequest(int id) {
    // console->println(F("sendTurnoutEntryRequest()"));
    if (delegate) {
        sendCommand("<JT " + String(id) + ">");
    }
    // console->println(F("sendTurnoutEntryRequest() end"));
}

//private
void DCCEXProtocol::processTurnoutEntry(LinkedList<String> &args) {
    // console->println(F("processTurnoutEntry()"));
    if (delegate) {
        //find the turnout entry to update
        if (turnouts.size()>0) { 
            for (int i=0; i<turnouts.size(); i++) {
                int id = args.get(1).toInt();
                if (turnouts.get(i)->getTurnoutId()==id) {
                    TurnoutState state = TurnoutClosed;
                    String name = UNKNOWN;
                    if (args.get(2)!=UnknownIdResponse) {
                        name = args.get(3);
                        if (args.get(2)=TurnoutResponseClosed) {
                            state = TurnoutClosed;
                        } else {
                            state = TurnoutThrown;
                        }
                    }
                    turnouts.get(i)->setTurnoutId(id);
                    turnouts.get(i)->setTurnoutName(stripLeadAndTrailQuotes(name));
                    turnouts.get(i)->setHasReceivedDetails();
                    turnouts.get(i)->setTurnoutState(state);
                }
            }

            bool rslt = true;
            for (int i=0; i<turnouts.size(); i++) {
                if (!turnouts.get(i)->getHasReceivedDetails()) {
                    console->print(F("processTurnoutsEntry(): not received yet: ~")); console->print(turnouts.get(i)->getTurnoutName()); console->print(F("~ "));console->println(turnouts.get(i)->getTurnoutId());
                    rslt = false;
                    break;
                }
            }
            if (rslt) {
                turnoutListFullyReceived = true;
                console->println(F("processTurnoutsEntry(): received all"));
                delegate->receivedTurnoutList(turnouts.size());
            }            
        } 
    }
    // console->println(F("processTurnoutEntry() end"));
}

//private
bool DCCEXProtocol::sendTurnoutAction(int turnoutId, TurnoutAction action) {
    sendCommand("<T " + String(turnoutId) + " " + String(action) + ">");
    return true;
}

//private
void DCCEXProtocol::processTurnoutAction(LinkedList<String> &args) { //<H id state>
    // console->println(F("processTurnoutAction(): "));
    if (delegate) {
        //find the Turnout entry to update
        if (turnouts.size()>0) { 
            for (int i=0; i<turnouts.size(); i++) {
                int id = args.get(1).toInt();
                if (turnouts.get(i)->getTurnoutId()==id) {
                    TurnoutState state = args.get(2).toInt();
                    if (args.size() >= 3) {
                        turnouts.get(i)->setTurnoutState(state);
                        delegate->receivedTurnoutAction(id, state);
                    }
                }
            }
        } 
    }
    // console->println(F("processTurnoutAction(): end"));
}

// ****************
// Routes

//private
void DCCEXProtocol::processRouteList(LinkedList<String> &args) {
    // console->println(F("processRouteList()"));
    if (delegate) {
        if (routes.size()>0) { // already have a routes list so this is an update
            routes.clear();
        } 
        for (int i=1; i<args.size(); i++) {
            int id = args.get(i).toInt();
            routes.add(new Route(id, ""));
            sendRouteEntryRequest(id);
        }
    }
    // console->println(F("processRouteList(): end"));
}

//private
void DCCEXProtocol::sendRouteEntryRequest(int address) {
    // console->println(F("sendRouteEntryRequest()"));
    if (delegate) {
        sendCommand("<JA " + String(address) + ">");
    }
    // console->println(F("sendRouteEntryRequest() end"));
}

//private
void DCCEXProtocol::processRouteEntry(LinkedList<String> &args) {
    // console->println(F("processRouteEntry()"));
    if (delegate) {
        //find the Route entry to update
        if (routes.size()>0) { 
            for (int i=0; i<routes.size(); i++) {
                int id = args.get(1).toInt();
                if (routes.get(i)->getRouteId()==id) {
                    String type = RouteTypeRoute;
                    String name = UNKNOWN;
                    if (args.get(2)!="X") {
                        type = args.get(2);
                        name = args.get(3);
                    }
                    routes.get(i)->setRouteName(stripLeadAndTrailQuotes(name));
                    routes.get(i)->setRouteType(type);
                    routes.get(i)->setHasReceivedDetails();
                }
            }

            bool rslt = true;
            for (int i=0; i<routes.size(); i++) {
                if (!routes.get(i)->getHasReceivedDetails()) {
                    console->print(F("processRoutesEntry(): not received yet: ~")); console->print(routes.get(i)->getRouteName()); console->print(F("~ "));console->println(routes.get(i)->getRouteId());
                    rslt = false;
                    break;
                }
            }
            if (rslt) {
                routeListFullyReceived = true;
                console->println(F("processRoutesEntry(): received all"));
                delegate->receivedRouteList(routes.size());
            }            
        } 
    }
    // console->println(F("processRouteEntry() end"));
}

// void DCCEXProtocol::processRouteAction(LinkedList<String> &args) {
//     console->println(F("processRouteAction(): "));
//     if (delegate) {

//     }
//     console->println(F("processRouteAction(): end"));
// }

// ****************
// Turntables

void DCCEXProtocol::processTurntableList(LinkedList<String> &args) {  // <jO [id1 id2 id3 ...]>
    // console->println(F("processTurntableList(): "));
    if (delegate) {
        if (turntables.size()>0) { // already have a turntables list so this is an update
            turntables.clear();
        } 
        for (int i=1; i<args.size(); i++) {
            int id = args.get(i).toInt();
            turntables.add(new Turntable(id, "", TurntableTypeUnknown, 0, 0));
            sendTurntableEntryRequest(id);
            sendTurntableIndexEntryRequest(id);
        }
    }
    // console->print("processTurntableList(): end: size:"); console->println(turntables.size());
}

//private
void DCCEXProtocol::sendTurntableEntryRequest(int id) {
    // console->println(F("sendTurntableEntryRequest()"));
    if (delegate) {
        sendCommand("<JO " + String(id) + ">");
    }
    // console->println(F("sendTurntableEntryRequest(): end"));
}

//private
void DCCEXProtocol::sendTurntableIndexEntryRequest(int id) {
    // console->println(F("sendTurntableIndexEntryRequest()"));
    if (delegate) {
        sendCommand("<JP " + String(id) + ">");
    }
    // console->println(F("sendTurntableIndexEntryRequest() end"));
}

//private
void DCCEXProtocol::processTurntableEntry(LinkedList<String> &args) {  // <jO id type position position_count "[desc]">
    // console->println(F("processTurntableEntry(): "));
    if (delegate) {
        //find the Turntable entry to update
        if (turntables.size()>0) { 
            for (int i=0; i<turntables.size(); i++) {
                int id = args.get(1).toInt();
                if (turntables.get(i)->getTurntableId()==id) {
                    String name = "Unkown";
                    TurntableType type = TurntableTypeUnknown;
                    int position = 0;
                    int indexCount = 0;
                    if (args.size() > 3) {  // server did not find the id
                        name = args.get(5);
                        type = args.get(2).toInt();
                        position = args.get(3).toInt();
                        indexCount = args.get(4).toInt();
                    }
                    turntables.get(i)->setTurntableName(stripLeadAndTrailQuotes(name));
                    turntables.get(i)->setTurntableType(type);
                    turntables.get(i)->setTurntableCurrentPosition(position);
                    turntables.get(i)->setTurntableIndexCount(indexCount);
                    turntables.get(i)->setHasReceivedDetails();
                }
            }
        } 
    }
    // console->println(F("processTurntableEntry(): end"));
}

//private
void DCCEXProtocol::processTurntableIndexEntry(LinkedList<String> &args) { // <jP id index angle "[desc]">
    // console->println(F("processTurntableIndexEntry(): "));
    if (delegate) {
        if (args.size() > 3) {  // server did not find the index
            //find the Turntable entry to update
            if (turntables.size()>0) { 
                for (int i=0; i<turntables.size(); i++) {
                    int id = args.get(1).toInt();
                    if (turntables.get(i)->getTurntableId()==id) {
                        //this assumes we are always starting from scratch, not updating indexes
                        int index = args.get(2).toInt();
                        int angle = args.get(3).toInt();
                        String name = args.get(4);

                        turntables.get(i)->turntableIndexes.add(new TurntableIndex(index, stripLeadAndTrailQuotes(name), angle));
                        break;
                    }
                }

                bool rslt = true;
                for (int i=0; i<turntables.size(); i++) {
                    if (!turntables.get(i)->getHasReceivedDetails()) {
                        console->print(F("processTurntableIndexEntry(): not received yet: ~")); console->print(turntables.get(i)->getTurntableName()); console->print(F("~ ")); console->println(turntables.get(i)->getTurntableId());
                        rslt = false;
                        break;
                    } else { // got the main entry. check the index entries as well
                        if (turntables.get(i)->turntableIndexes.size() != turntables.get(i)->getTurntableIndexCount()) {
                            console->print(F("processTurntableIndexEntry(): not received index entry yet: ~")); console->print(turntables.get(i)->getTurntableName()); console->print(F("~ ")); console->println(turntables.get(i)->getTurntableId());
                            rslt = false;
                            break;
                        }
                    }
                }
                if (rslt) {
                    turntableListFullyReceived = true;
                    console->println(F("processTurntableIndexEntry(): received all"));
                    delegate->receivedTurntableList(turntables.size());
                }  
                
            }
        }
    }
    // console->println(F("processTurntableIndexEntry(): end"));
}

//private
void DCCEXProtocol::processTurntableAction(LinkedList<String> &args) { // <i id position moving>
    // console->println(F("processTurntableAction(): "));
    if (delegate) {
        int id = args.get(1).toInt();
        int newPos = args.get(2).toInt();
        TurntableState state = args.get(3).toInt();
        int pos = findTurntableListPositionFromId(id);
        if (pos!=newPos) {
            turntables.get(pos)->actionTurntableExternalChange(newPos, state);
        }
        delegate->receivedTurntableAction(id, newPos, state);
    }
    // console->println(F("processTurntableAction(): end"));
}

//private
void DCCEXProtocol::processSensorEntry(LinkedList<String> &args) {  // <jO id type position position_count "[desc]">
    console->println(F("processSensorEntry(): "));
    if (delegate) {
        //????????????????? TODO
    }
    console->println(F("processSensorEntry(): end"));
}

// ****************
// Locos

//private
bool DCCEXProtocol::processLocoAction(LinkedList<String> &args) { //<l cab reg speedByte functMap>
    // console->println(F("processLocoAction()"));
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
                FunctionState fnStates[MAX_FUNCTIONS];
                getFunctionStatesFromFunctionMap(fnStates, functMap);
                for (uint i=0; i<MAX_FUNCTIONS; i++) {
                    if (fnStates[i] != throttleConsists[throttleNo].consistGetLocoAtPosition(0)->locoFunctions.getFunctionState(i)) {
                        console->print(i);
                        console->print(" - ");
                        console->print(fnStates[i]);
                        console->print(" - ");
                        console->println(throttleConsists[throttleNo].consistGetLocoAtPosition(0)->locoFunctions.getFunctionState(i));

                        delegate->receivedFunction(throttleNo, i, fnStates[i]);
                        throttleConsists[throttleNo].consistGetLocoAtPosition(0)->locoFunctions.setFunctionState(i, fnStates[i]);
                    }
                }
                throttleConsists[throttleNo].actionConsistExternalChange(speed, dir, fnStates);

                delegate->receivedSpeed(throttleNo, speed);
                delegate->receivedDirection(throttleNo, dir);

            }
        } else {
            console->println(F("processLocoAction(): unknown loco"));
            return false;
        }
    }
    // console->println(F("processLocoAction() end"));
    return true;
}

// ******************************************************************************************************
// server commands

bool DCCEXProtocol::sendServerDetailsRequest() {
    // console->println(F("sendServerDetailsRequest(): "));
    if (delegate) {
       sendCommand("<s>");	
    }
    // console->println(F("sendServerDetailsRequest(): end"));
    return true; 
}

// ******************************************************************************************************
// power commands

bool DCCEXProtocol::sendTrackPower(TrackPower state) {
    // console->println(F("sendTrackPower(): "));
    if (delegate) {
       sendCommand("<" + String(state) + ">");	
    }
    // console->println(F("sendTrackPower(): end"));
    return true;
}

bool DCCEXProtocol::sendTrackPower(TrackPower state, char track) {
    // console->println(F("sendTrackPower(): "));
    if (delegate) {
        sendCommand("<" + String(state) + " " + String(track) + ">");	
    }
    // console->println(F("sendTrackPower(): end"));
    return true;
}

// ******************************************************************************************************

void DCCEXProtocol::sendEmergencyStop() {
    // console->println(F("emergencyStop(): "));
    if (delegate) {
            sendCommand("<!>");
    }
    // console->println(F("emergencyStop(): end"));
}

// ******************************************************************************************************

bool DCCEXProtocol::sendFunction(int throttle, int funcNum, bool pressed) {
    // console->println(F("sendFunction(): "));
    if (delegate) {
        ConsistLoco* conLoco = throttleConsists[throttle].consistGetLocoAtPosition(0);
        int address = conLoco->getLocoAddress();
        if (address>=0) {
            sendCommand("<F " + String(address) + " " + String(funcNum) + " " + String(pressed) + ">");
        }
    }
    // console->println(F("sendFunction(): end")); 
    return true;
}

bool DCCEXProtocol::sendFunction(int throttle, String address, int funcNum, bool pressed) {
    // console->println(F("sendFunction(): "));
    if (delegate) {
        sendCommand("<F " + String(address) + " " + String(funcNum) + " " + String(pressed) + ">");
    }
    // console->println(F("sendFunction(): end")); 
    return true;
}


// ******************************************************************************************************
// throttle

bool DCCEXProtocol::sendThrottleAction(int throttle, int speed, Direction direction) {
    // console->println(F("sendThrottleAction(): "));
    if (delegate) {
        if (throttleConsists[throttle].consistGetNumberOfLocos()>0) {
            throttleConsists[throttle].consistSetSpeed(speed);
            throttleConsists[throttle].consistSetDirection(direction);
            for (int i=0; i<throttleConsists[throttle].consistGetNumberOfLocos(); i++) {
                ConsistLoco* conLoco = throttleConsists[throttle].consistGetLocoAtPosition(i);
                int address = conLoco->getLocoAddress();
                Direction dir = direction;
                if (conLoco->getConsistLocoFacing()==Reverse) {
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
        sendCommand("<t " + String(address) + ">");
    }
    // console->println(F("sendLocoUpdateRequest() end"));
    return true;
}

bool DCCEXProtocol::sendLocoAction(int address, int speed, Direction direction) {
    // console->print(F("sendLocoAction(): ")); console->println(address);
    if (delegate) {
        sendCommand("<t " + String(address) + " " + String(speed) + " " + String(direction) + ">");
    }
    // console->println(F("sendLocoAction(): end"));
    return true;
}

// ******************************************************************************************************

bool DCCEXProtocol::sendRouteAction(int routeId) {
    // console->println(F("sendRouteAction()"));
    if (delegate) {
        sendCommand("</START " + String(routeId) + ">");
    }
    // console->println(F("sendRouteAction() end"));
    return true;
}

bool DCCEXProtocol::sendPauseRoutes() {
    // console->println(F("sendPauseRoutes()"));
    if (delegate) {
        sendCommand("</PAUSE>");
    }
    // console->println(F("sendPauseRoutes() end"));
    return true;
}

bool DCCEXProtocol::sendResumeRoutes() {
    // console->println(F("sendResumeRoutes()"));
    if (delegate) {
        sendCommand("</RESUME>");
    }
    // console->println(F("sendResumeRoutes() end"));
    return true;
}

// ******************************************************************************************************


bool DCCEXProtocol::sendTurntableAction(int turntableId, int position, int activity) {
    // console->println(F("sendTurntable()"));
    if (delegate) {
        sendCommand("<I " + String(turntableId) + " " + String(position) + ">");
    }
    // console->println(F("sendTurntable() end"));
    return true;
}

bool DCCEXProtocol::sendAccessoryAction(int accessoryAddress, int activate) {
    // console->println(F("sendAccessory()"));
    if (delegate) {
        sendCommand("<a " + String(accessoryAddress) + " " + String(activate) + ">");
    }
    // console->println(F("sendAccessory() end"));
    return true;
}

bool DCCEXProtocol::sendAccessoryAction(int accessoryAddress, int accessorySubAddr, int activate) {
    // console->println(F("sendAccessory()"));
    if (delegate) {
        sendCommand("<a " + String(accessoryAddress) + " " + String(accessorySubAddr) + " " + String(activate) + ">");
    }
    // console->println(F("sendAccessory() end"));
    return true;
}

// ******************************************************************************************************

bool DCCEXProtocol::getRoster() {
    // console->println(F("getRoster()"));
    if (delegate) {
        sendCommand("<JR>");
    }
    // console->println(F("getRoster() end"));
    return true;
}

bool DCCEXProtocol::isRosterFullyReceived() {
    // console->println(F("isRosterFullyReceived()"));
    // if (rosterFullyReceived) console->println(F("true"));
    return rosterFullyReceived;
}

bool DCCEXProtocol::getTurnouts() {
    // console->println(F("getTurnouts()"));
    if (delegate) {
        sendCommand("<JT>");
    }
    // console->println(F("getTurnouts() end"));
    return true;
}

bool DCCEXProtocol::isTurnoutListFullyReceived() {
    return turnoutListFullyReceived;
}

bool DCCEXProtocol::getRoutes() {
    // console->println(F("getRoutes()"));
    if (delegate) {
        sendCommand("<JA>");
    }
    // console->println(F("getRoutes() end"));
    return true;
}

bool DCCEXProtocol::isRouteListFullyReceived() {
    return routeListFullyReceived;
}

bool DCCEXProtocol::getTurntables() {
    // console->println(F("getTurntables()"));
    if (delegate) {
        sendCommand("<JO>");
    }
    // console->println(F("getTurntables() end"));
    return true;
}

bool DCCEXProtocol::isTurntableListFullyReceived() {
    return turntableListFullyReceived;
}

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// helper functions

// private
// find which, if any, throttle has this loco selected
int DCCEXProtocol::findThrottleWithLoco(int address) {
    for (uint i=0; i<MAX_THROTTLES; i++) {
        if (throttleConsists[i].consistGetNumberOfLocos()>0) {
            int pos = throttleConsists[i].consistGetLocoPosition(address);

            // console->print(F("checking consist: ")); console->print(i); console->print(" found: "); console->println(pos);
            // console->print(F("in consist: ")); console->println(throttleConsists[i].consistGetNumberOfLocos()); 

            // for (int j=0; j<throttleConsists[i].consistGetNumberOfLocos(); j++ ) {
            //      console->print(F("checking consist X: ")); console->print(j); console->print(" is: "); console->println(throttleConsists[i].consistLocos.get(i)->getLocoAddress());
            // }    

            if (pos>=0) {
                return i;
            }
        }
    }
    return -1;  //not found
}

int DCCEXProtocol::findTurnoutListPositionFromId(int id) {
    if (turnouts.size()>0) {
        for (int i=0; i<turnouts.size(); i++) {
            if (turnouts.get(i)->getTurnoutId()==id) {
                return i;
            }
        }
    }
    return -1;
}

int DCCEXProtocol::findRouteListPositionFromId(int id) {
    if (routes.size()>0) {
        for (int i=0; i<routes.size(); i++) {
            if (routes.get(i)->getRouteId()==id) {
                return i;
            }
        }
    }
    return -1;
}

int DCCEXProtocol::findTurntableListPositionFromId(int id) {
    if (turntables.size()>0) {
        for (int i=0; i<turntables.size(); i++) {
            if (turntables.get(i)->getTurntableId()==id) {
                return i;
            }
        }
    }
    return -1;
}

bool DCCEXProtocol::splitValues(LinkedList<String> &args, char *cmd) {
    byte parameterCount = 0;
    
    int currentCharIndex = 0; // pointer to the next character to process
    splitState state = FIND_START;
    String currentArg = "";
    byte currentArgLength = 0;
    
    while (parameterCount < MAX_COMMAND_PARAMS)
    {
        char currentChar = cmd[currentCharIndex];
        // In this switch, 'break' will go on to next char but 'continue' will rescan the current char. 
        switch (state) {
        case FIND_START: // looking for '<'
            if (currentChar == COMMAND_START) state = SKIP_SPACES;   // '<'
            break;

        case SKIP_SPACES: // skipping spaces before a param
            if (currentChar == ' ') break; // ignore
            state = CHECK_FOR_LEADING_QUOTE;
            continue;

        case CHECK_FOR_LEADING_QUOTE: // checking for quotes at the start of the parameter
            if (currentChar == '"') {
                state = BUILD_QUOTED_PARAM;
                continue;
            } 
            state = BUILD_PARAM;
            continue;

        case BUILD_QUOTED_PARAM: 
            if (currentChar == '>') {
                state = CHECK_FOR_END;
                continue;
            }
            currentArg = currentArg + String(currentChar);
            // console->println(currentArg);
            currentArgLength++;
            if ( (currentArgLength>1) && (currentChar == '"') ) { // trailing quote
                args.add( currentArg );
                currentArg = "";
                currentArgLength = 0;
                state = SKIP_SPACES;
            }
            break;

        case BUILD_PARAM: // building a parameter
            if (currentChar == COMMAND_END) {  // '>'
                state = CHECK_FOR_END;
                continue;
            }
            if (currentChar != ' ') {
                currentArg = currentArg + String(currentChar);
                // console->println(currentArg);
                currentArgLength++;
                break;
            }

            // space - end of parameter detected 
            args.add( currentArg );
            currentArg = "";
            currentArgLength = 0;
            parameterCount++;
            state = SKIP_SPACES;
            continue;
        
        case CHECK_FOR_END:
            if ( (currentChar=='\0') || (currentChar == '>') ) {
                if (currentArgLength>0) { // in case there was a param we started but didn't find the proper end
                    args.add( currentArg );
                }
                return true;  
            }
            break;
        }
        currentCharIndex++;
    }
    return true;
    
}

bool DCCEXProtocol::splitFunctions(LinkedList<String> &args, char *cmd) {
    byte parameterCount = 0;
    
    int currentCharIndex = 0; // pointer to the next character to process
    splitState state = FIND_START;
    String currentArg = "";
    byte currentArgLength = 0;
    
    while (parameterCount < MAX_FUNCTIONS)
    {
        char currentChar = cmd[currentCharIndex];
        // console->print("..."); console->print(currentChar); console->println("...");
        // In this switch, 'break' will go on to next char but 'continue' will rescan the current char. 
        switch (state) {
        case CHECK_FOR_LEADING_QUOTE:
        case FIND_START: // looking for leading '"'
            if (currentChar == '"') state = SKIP_SPACES;
            break;

        case SKIP_SPACES: // skipping spaces or slashes before a param
            if ( (currentChar == ' ') || (currentChar == '/') ) break; // ignore
            state = BUILD_PARAM;
            continue;

        case BUILD_QUOTED_PARAM:
        case BUILD_PARAM: // building a parameter
            if (currentChar == '"') {
                state = CHECK_FOR_END;
                continue;
            }
            if (currentChar != '/') {
                currentArg = currentArg + String(currentChar);
                // console->println(currentArg);
                currentArgLength++;
                break;
            }

            // end of parameter detected  '/')
            args.add( currentArg );
            currentArg = "";
            currentArgLength = 0;
            parameterCount++;
            state = SKIP_SPACES;
            continue;
        
        case CHECK_FOR_END:
            if ( (currentChar == '\0') || (currentChar == '"') ) {
                if (currentArgLength>0) { // in case there was a param we started but didn't find the proper end
                    args.add( currentArg );
                }
                return true;  
            }
            break;
        }
        currentCharIndex++;
    }
    return true;
}

String DCCEXProtocol::stripLeadAndTrailQuotes(String text) {
    String s = text;
    if (text.charAt(0)=='"' && text.charAt(text.length()-1)=='"') {
        s = s.substring(1, text.length()-1);
    }
    return s;
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
        functionState[functionNumber] = state;
        return true;
    }
    bool Functions::actionFunctionStateExternalChange(int functionNumber, FunctionState state) {
        //????????????????? TODO
        //  This may not be needed
        return true;
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

    Loco::Loco(int address, String name, LocoSource source) {
        locoAddress = address;
        locoName = name;
        locoSource = source;
        locoDirection = Forward;
        locoSpeed = 0;
        rosterReceivedDetails = false;
        for (uint i=0; i<MAX_FUNCTIONS; i++) {
            locoFunctions.initFunction(i, "", FunctionLatchingFalse, FunctionStateOff);
        }
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
    bool Loco::setLocoName(String name) {
        locoName = name;
        return true;
    }
    String Loco::getLocoName() {
        return locoName;
    }
    bool Loco::setLocoSource(LocoSource source) {
        locoSource = source;
        return true;
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
    void Loco::setIsFromRosterAndReceivedDetails() {
        rosterReceivedDetails = true;
    }
    bool Loco::getIsFromRosterAndReceivedDetails() {
        if (locoSource==LocoSourceRoster && rosterReceivedDetails) {
            return true;
        }
        return false;
    }

// class ConsistLoco : public Loco

    ConsistLoco::ConsistLoco(int address, String name, LocoSource source, Facing facing) 
    : Loco::Loco(address, name, source) {
         consistLocoFacing = facing;
    }
    bool ConsistLoco::setConsistLocoFacing(Facing facing) {
        consistLocoFacing = facing;
        return true;
    }
    Facing ConsistLoco::getConsistLocoFacing() {
        return consistLocoFacing;
    }

// class Consist

    Consist::Consist(String name) {
        consistName = name;
    }
    bool Consist::consistAddLoco(Loco loco, Facing facing) {
        int address = loco.getLocoAddress();
        String name = loco.getLocoName();
        LocoSource source = loco.getLocoSource();
        Facing correctedFacing = facing;
        int rslt = consistGetLocoPosition(address);
        if (rslt<0) { // not already in the list, so add it
            if (consistGetNumberOfLocos() == 0) correctedFacing = FacingForward; // first loco in consist is always forward
            consistLocos.add(new ConsistLoco(address, name, source, correctedFacing));
            return true;
        }
        return false;
    }
    bool Consist::consistReleaseAllLocos()  {
        if (consistLocos.size()>0) {
            consistLocos.clear();
        }
        return true;
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
    }
    ConsistLoco* Consist::consistGetLocoAtPosition(int position) {
        if (position<consistLocos.size()) {
            return consistLocos.get(position);
        }
        return {};
    }
    int Consist::consistGetLocoPosition(int locoAddress) {
        for (int i=0; i<consistLocos.size(); i++) {
            if (consistLocos.get(i)->getLocoAddress() == locoAddress) {
                return i;
            }
        }
        return -1;
    }
    // set the position of the loco in the consist
    // Assumes the loco is already in the consist
    bool Consist::consistSetLocoPosition(int locoAddress, int position) {
        int currentPosition = consistGetLocoPosition(locoAddress);
        if (currentPosition < 0  || currentPosition == position)  {
            return false;
        } else {
            ConsistLoco* loco = consistGetLocoAtPosition(currentPosition);
            consistLocos.remove(currentPosition);
            int address = loco->getLocoAddress();
            String name = loco->getLocoName();
            LocoSource source = loco->getLocoSource();
            Facing correctedFacing = loco->getConsistLocoFacing();
            // if (consistGetNumberOfLocos() == 0 || position == 0) {
            //     correctedFacing = FacingForward; // first loco in consist is always forward
            // }
            consistLocos.add(position, new ConsistLoco(address, name, source, correctedFacing));

            consistGetLocoAtPosition(0)->setConsistLocoFacing(FacingForward); // first loco in consist is always forward
        }
        return true;
    }
    bool Consist::consistSetSpeed(int speed) {
        if (consistLocos.size()>0) {
            if (consistSpeed!=speed) {
                for (int i=0; i<consistLocos.size(); i++) {
                    consistLocos.get(i)->setLocoSpeed(speed);
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
                    if (consistLocos.get(i)->getConsistLocoFacing()!=FacingForward) { // lead loco 'facing' is always assumed to be forward
                        if (direction == Forward) {
                            locoDir = Reverse;
                        } else {
                            locoDir = Forward;
                        }
                    }
                    consistLocos.get(i)->setLocoSpeed(consistSpeed);
                    consistLocos.get(i)->setLocoDirection(locoDir);
                }
            }
        }
        consistDirection = direction;
        return true;
    }
    bool Consist::actionConsistExternalChange(int speed, Direction direction, FunctionState fnStates[]) {
        if (consistLocos.size()>0) {
            if ( (consistDirection != direction) || (consistSpeed != speed) ) {
                for (int i=0; i<consistLocos.size(); i++) {
                    Direction locoDir = direction;
                    if (consistLocos.get(i)->getConsistLocoFacing()!=FacingForward) { // lead loco 'facing' is always assumed to be forward
                        if (direction == Forward) {
                            locoDir = Reverse;
                        } else {
                            locoDir = Forward;
                        }
                    }
                    consistLocos.get(i)->setLocoSpeed(consistSpeed);
                    consistLocos.get(i)->setLocoDirection(locoDir);

                    // if (i==0) {
                    //     FunctionState fnStates[MAX_FUNCTIONS];
                    //     for (uint i=0; i<MAX_FUNCTIONS; i++) {
                    //         fnStates[i] = bitExtracted(fnStates,1,i+1);
                    //         // if (fStates)
                    //         //????????????????? TODO
                    //     }
                    // }
                }
            }
        }

        return true;
    }

    Direction Consist::consistGetDirection() {
        return consistDirection;
    }
    bool Consist::consistSetFunction(int functionNo, FunctionState state) {
        //????????????????? TODO
        return true;
    }
    bool Consist::consistSetFunction(int address, int functionNo, FunctionState state) {
        //????????????????? TODO
        return true;
    }

    String Consist::getConsistName() {
        return consistName;
    }

// class Turnout

    Turnout::Turnout(int id, String name, TurnoutState state) {
        turnoutId = id;
        turnoutName = name;
        turnoutState = state;
        hasReceivedDetail = false;
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
    bool Turnout::setTurnoutId(int id) {
        turnoutId = id;
        return true;
    }
    int Turnout::getTurnoutId() {
        return turnoutId;
    }
    bool Turnout::setTurnoutName(String name) {
        turnoutName = name;
        return true;
    }
    String Turnout::getTurnoutName() {
        return turnoutName;
    }
    TurnoutState Turnout::getTurnoutState() {
        return turnoutState;
    }
    void Turnout::setHasReceivedDetails() {
        hasReceivedDetail = true;
    }
    bool Turnout::getHasReceivedDetails() {
        return hasReceivedDetail;
    }

// class Route

    Route::Route(int id, String name) {
        routeId = id;
        routeName = name;
        hasReceivedDetail = false;
    }
    int Route::getRouteId() {
        return routeId;
    }
    bool Route::setRouteName(String name) {
        routeName = name;
        return true;
    }
    String Route::getRouteName() {
        return routeName;
    }
    bool Route::setRouteType(String type) {
        routeType = type;
        return true;
    }
    RouteType Route::getRouteType() {
        return routeType;
    }
    void Route::setHasReceivedDetails() {
        hasReceivedDetail = true;
    }
    bool Route::getHasReceivedDetails() {
        return hasReceivedDetail;
    }

// class TurntableIndex

    TurntableIndex::TurntableIndex(int index, String name, int angle) {
        turntableIndexIndex = index;
        turntableIndexName = name;
        turntableIndexAngle = angle;
    }
    String TurntableIndex::getTurntableIndexName() {
        return turntableIndexName;
    }
    int TurntableIndex::getTurntableIndexId() {
        return turntableIndexId;
    }
    int TurntableIndex::getTurntableIndexIndex() {
        return turntableIndexIndex;
    }

// class Turntable

    Turntable::Turntable(int id, String name, TurntableType type, int position, int indexCount) {
        turntableId = id;
        turntableName = name;
        turntableType = type;
        turntableCurrentPosition = position;
        turnTableIndexCount = indexCount;
    }
    int Turntable::getTurntableId() {
        return turntableId;
    }
    bool Turntable::setTurntableName(String name) {
        turntableName = name;
        return true;
    }
    String Turntable::getTurntableName() {
        return turntableName;
    }
    bool Turntable::setTurntableType(TurntableType type) {
        turntableType = type;
        return true;
    }
    TurntableType Turntable::getTurntableType() {
        return turntableType;
    }
    bool Turntable::addTurntableIndex(int index, String indexName, int indexAngle) {
        turntableIndexes.add(new TurntableIndex(index, indexName, indexAngle));
        return true;
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
    bool Turntable::setTurntableIndexCount(int indexCount) {  // what was listed in the original definition
        turnTableIndexCount = indexCount;
        return true;
    }
    int Turntable::getTurntableIndexCount() { // what was listed in the original definition
        return turnTableIndexCount;
    }
    int Turntable::getTurntableNumberOfIndexes() { // actual count
        return turntableIndexes.size();
    }
    TurntableIndex* Turntable::getTurntableIndexAt(int positionInLinkedList) {
        return turntableIndexes.get(positionInLinkedList);
    }
    TurntableIndex* Turntable::getTurntableIndex(int indexId) {
        for (int i=0; i<turntableIndexes.size(); i++) {
            if (turntableIndexes.get(i)->getTurntableIndexId()==indexId) {
                return turntableIndexes.get(i);
            }
        }
        return {};
    }
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
    void Turntable::setHasReceivedDetails() {
        hasReceivedDetail = true;
    }
    bool Turntable::getHasReceivedDetails() {
        return hasReceivedDetail;
    }