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
    strcpy(outboundCommand, "<U DISCONNECT>");
    sendCommand();
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

void DCCEXProtocol::getFunctionStatesFromFunctionMap(FunctionState fnStates[], int functionMap) {
 
    for (uint i=0; i<MAX_FUNCTIONS; i++) {
        fnStates[i] = (bitExtracted(functionMap, 1, i+1)==0) ? FunctionStateOff : FunctionStateOn;
    }
}

int DCCEXProtocol::bitExtracted(int number, int k, int p)
{
    return (((1 << k) - 1) & (number >> (p - 1)));
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

        strcpy(outboundCommand, ""); // clear it once it has been sent
    }
}

//private
bool DCCEXProtocol::processCommand(char* c, int len) {
    // console->println(F("processCommand()"));

    lastServerResponseTime = millis()/1000;

    console->print("<== "); console->println(c);

    // console->print("processing: "); console->println(c);

    for( int i=0; i<argz.size(); i++) { argz.get(i)->clearCommandArgument(); }
    argz.clear();

    splitValues(c);
    
    // console->print("number of args: "); console->println(argz.size());
    // for (uint i=0; i<argz.size();i++) {
    //     console->print("arg"); console->print(i); console->print(": ~"); console->print(argz.get(i)->arg); console->println("~");
    // }

    char char0 = argz.get(0)->arg[0];
    char char1 = argz.get(0)->arg[1];
    char arg1[MAX_SINGLE_COMMAND_PARAM_LENGTH];
    char arg2[MAX_SINGLE_COMMAND_PARAM_LENGTH];
    strcpy(arg1, "\0");
    strcpy(arg2, "\0");
    if (argz.size()>1) {
        strcpy(arg1, argz.get(1)->arg); strcat(arg1, "\0");
        if (argz.size()>2) {
            strcpy(arg2, argz.get(2)->arg); strcat(arg2, "\0");
        }
    }
    int noOfParameters = argz.size();

    bool processed = false;
    if (delegate) {
        if (char0=='i') {
            if (strcmp(argz.get(0)->arg,"iDCC-EX") == 0) {
                //<iDCCEX version / microprocessorType / MotorControllerType / buildNumber>
                processServerDescription();
                processed = true;
            } else {
                //<i id position>   or   <i id position moving>
                TurntableState state = TurntableStationary;
                if (argz.size()<3) { 
                    state = TurntableMoving;
                    processed = true;
                }
                processTurntableAction();
                delegate->receivedTurntableAction(atoi(arg1), atoi(arg2), state);
                processed = true;
            }

        } else if (char0 == 'p') {
            //<p onOff>
            processTrackPower();
            processed = true;

        } else if (char0 == 'l') {
            //<l cab reg speedByte functMap>
            processLocoAction();
            processed = true;

        } else if (char0 == 'j') {
            if( char1 == 'R' ) {
                if (noOfParameters == 1) {  // empty roster
                    rosterFullyReceived = true;
                    processed = true;
                } else if (noOfParameters > 1) { 
                    if ( (noOfParameters<3) || (argz.get(2)->arg[0] != '"') ) {  // loco list
                        //<jR [id1 id2 id3 ...]>
                        processRosterList(); 
                        processed = true;
                    } else { // individual
                        //<jR id ""|"desc" ""|"funct1/funct2/funct3/...">
                        processRosterEntry(); 
                        processed = true;
                    }
                }
            } else if (char1 == 'T') {
                if (noOfParameters == 1) { // empty list
                    turnoutListFullyReceived = true;
                    processed = true;
                } else if (noOfParameters > 1) { 
                    if ( ((noOfParameters == 4) && (argz.get(3)->arg[0] == '"')) 
                    || ((noOfParameters == 3) && (argz.get(2)->arg[0] == 'X')) ) {
                        //<jT id state |"[desc]">   or    <jT id X">
                        processTurnoutEntry(); 
                        processed = true;
                    } else {
                        //<jT [id1 id2 id3 ...]>
                        processTurnoutList(); 
                        processed = true;
                    }
                }

            } else if (char1=='O') {
                if (noOfParameters == 1) {  // empty list
                    turntableListFullyReceived = true;
                    processed = true;
                } else if (noOfParameters>1) { 
                    if ( (noOfParameters == 6) && (argz.get(5)->arg[0] == '"') ) { 
                        //<jO id type position position_count "[desc]">
                        processTurntableEntry(); 
                            processed = true;
                    } else {
                        //<jO [id1 id2 id3 ...]>
                        processTurntableList(); 
                            processed = true;
                    } 
                }
            } else if  (char1=='P') {
                if (noOfParameters>1) { 
                    // <jP id index angle "[desc]">
                    processTurntableIndexEntry(); 
                    processed = true;
                }

            } else if (char1 == 'A') {
                if (noOfParameters > 1) { 
                    if ( ((noOfParameters == 4) && (argz.get(3)->arg[0] == '"'))
                    || ((noOfParameters == 3) && (argz.get(2)->arg[0] == 'X')) ) {
                        //<jA id type |"desc">   or  <jA id X>
                        processRouteEntry(); 
                        processed = true;
                    } else {
                        //<jA [id0 id1 id2 ..]>
                        processRouteList(); 
                        processed = true;
                    }
                }
            }

        } else if (char0 == 'H') {
            if (argz.size()==3) {
                processTurnoutAction();
                delegate->receivedTurnoutAction(atoi(arg1), atoi(arg2));
                processed = true;
            }

        } else if (char0 == 'q') {
            // <q id>
            processSensorEntry();
            processed = true;

        } else if (char0 == 'X') {
            // error   Nothing we can do with it as we don't know what it is for
            processed = true;
        } 

        if (!processed) {
            processUnknownCommand(c);
        }
    }

    // console->println(F("processCommand() end"));
    return true;
}

//private
void DCCEXProtocol::processUnknownCommand(char* unknownCommand) {
    console->println(F("processUnknownCommand()"));
    if (delegate) {
        console->print("unknown command '"); console->print("'"); console->println(unknownCommand);
    }
    console->println(F("processUnknownCommand() end"));
}

char* DCCEXProtocol::charToCharArray(char c) {
    charToCharArrayVal[0] = c;
    charToCharArrayVal[1] = '\0';
    return charToCharArrayVal;
}

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// Process responses from the CS

//private
void DCCEXProtocol::processServerDescription() { //<iDCCEX version / microprocessorType / MotorControllerType / buildNumber>
    // console->println(F("processServerDescription()"));
    if (delegate) {
        char *_serverVersion;
        _serverVersion = (char *) malloc(strlen(argz.get(1)->arg)+1);
        strcpy(_serverVersion, argz.get(1)->arg);
        serverVersion = _serverVersion;

        char *_serverMicroprocessorType;
        _serverMicroprocessorType = (char *) malloc(strlen(argz.get(3)->arg)+1);
        strcpy(_serverMicroprocessorType, argz.get(3)->arg);
        serverMicroprocessorType = _serverMicroprocessorType;

        char *_serverMotorcontrollerType;
        _serverMotorcontrollerType = (char *) malloc(strlen(argz.get(5)->arg)+1);
        strcpy(_serverMotorcontrollerType, argz.get(5)->arg);
        serverMotorcontrollerType = _serverMotorcontrollerType;

        char *_serverBuildNumber;
        _serverBuildNumber = (char *) malloc(strlen(argz.get(6)->arg)+1);
        strcpy(_serverBuildNumber, argz.get(6)->arg);
        serverBuildNumber = _serverBuildNumber;        

        // strcpy(serverVersion, argz.get(1)->arg);
        // strcpy(serverMicroprocessorType, argz.get(3)->arg);
        // strcpy(serverMotorcontrollerType, argz.get(5)->arg);
        // strcpy(serverBuildNumber, argz.get(6)->arg);

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
        if (argz.get(0)->arg[0]=='0') {
            state = PowerOff;
        } else if (argz.get(0)->arg[1]=='1') {
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
    if (delegate) {
        if (roster.size()>0) { // already have a roster so this is an update
            // roster.clear();
            console->println(F("processRosterList(): roster list already received. Ignoring this!"));
            return;
        } 
        char arg[MAX_OBJECT_NAME_LENGTH];
        char name[MAX_OBJECT_NAME_LENGTH];
        for (int i=1; i<argz.size(); i++) {
            strcpy(arg, argz.get(i)->arg); strcat(arg, "\0");
            int address = atoi(arg);
            strcpy(name, NAME_UNKNOWN);

            roster.add(new Loco(address, name, LocoSourceRoster));
            sendRosterEntryRequest(address);
        }
    }
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
    // console->println(F("processRosterEntry()"));
    if (delegate) {
        //find the roster entry to update
        if (roster.size()>0) { 
            char arg[MAX_SINGLE_COMMAND_PARAM_LENGTH];
            char name[MAX_OBJECT_NAME_LENGTH];
            char cleanName[MAX_OBJECT_NAME_LENGTH];

            for (int i=0; i<roster.size(); i++) {
                strcpy(arg, argz.get(1)->arg); strcat(arg, "\0");
                int address = atoi(arg);

                if (roster.get(i)->getLocoAddress() == address) {
                    // console->print("processRosterEntry(): found: "); console->println(address);
                    strcpy(name, argz.get(2)->arg);
                    strcat(name, "\0");
                    stripLeadAndTrailQuotes(cleanName, name);
                    roster.get(i)->setLocoName(cleanName);
                    roster.get(i)->setLocoSource(LocoSourceRoster);
                    roster.get(i)->setIsFromRosterAndReceivedDetails();

                    char functions[MAX_SINGLE_COMMAND_PARAM_LENGTH];
                    strcpy(functions, argz.get(3)->arg);

                    for( int i=0; i<functionArgs.size(); i++) { functionArgs.get(i)->clearFunctionArgument(); }
                    functionArgs.clear();

                    splitFunctions(functions);
                    int noOfParameters = functionArgs.size();

                    // for (uint i=0; i<functionArgs.size();i++) {
                    //     console->print("fn"); console->print(i); console->print(": ~"); console->print(functionArgs.get(i)->arg); console->println("~");
                    // }

                    // console->print("processing Functions: "); console->println(functions);

                    for (int j=0; (j<noOfParameters && j<MAX_FUNCTIONS); j++ ) {
                        // console->print("functionArgs"); console->print(j); console->print(": ~"); console->print(functionArgs.get(j)->arg); console->println("~");
                        char* functionName = functionArgs.get(j)->arg;
                        FunctionState state = FunctionStateOff;
                        FunctionLatching latching = FunctionLatchingTrue;
                        if (functionName[0]=='*') {
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
void DCCEXProtocol::processTurnoutList() {
    // console->println(F("processTurnoutList()"));
    if (delegate) {
        if (turnouts.size()>0) { // already have a turnouts list so this is an update
            // turnouts.clear();
            console->println(F("processTurnoutList(): Turnout/Points list already received. Ignoring this!"));
            return;
        } 
        char val[MAX_OBJECT_NAME_LENGTH];
        char name[MAX_OBJECT_NAME_LENGTH];

        for (int i=1; i<argz.size(); i++) {
            strcpy(val, argz.get(i)->arg); strcat(val, "\0");
            int id = atoi(val);
            strcpy(name, NAME_UNKNOWN);
            
            turnouts.add(new Turnout(id, name, 0));
            sendTurnoutEntryRequest(id);
        }
    }
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
    // console->println(F("processTurnoutEntry()"));
    if (delegate) {
        //find the turnout entry to update
        if (turnouts.size()>0) { 
            char val[MAX_OBJECT_NAME_LENGTH];
            char name[MAX_OBJECT_NAME_LENGTH];
            char cleanName[MAX_OBJECT_NAME_LENGTH];

            for (int i=0; i<turnouts.size(); i++) {
                strcpy(val, argz.get(1)->arg);
                strcat(val, "\0");
                int id = atoi(val);
                if (turnouts.get(i)->getTurnoutId()==id) {
                    TurnoutState state = TurnoutClosed;
                    strcpy(name, NAME_UNKNOWN);

                    if (strcmp(argz.get(2)->arg,UnknownIdResponse) != 0 ) {
                        strcpy(name, argz.get(3)->arg);
                        if (argz.get(2)->arg[0] == TurnoutResponseClosed) {
                            state = TurnoutClosed;
                        } else {
                            state = TurnoutThrown;
                        }
                    }
                    turnouts.get(i)->setTurnoutId(id);
                    stripLeadAndTrailQuotes(cleanName, name);
                    turnouts.get(i)->setTurnoutName(cleanName);
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

// find the turnout/point in the turnout list by id. return a pointer or null is not found
Turnout* DCCEXProtocol::getTurnoutById(int turnoutId) {
    for (int i = 0; i < turnouts.size(); i++) {
        Turnout* turnout = turnouts.get(i);
        if (turnout->getTurnoutId() == turnoutId) {
            return turnout;
        }
    }
    return nullptr;  // not found
}

bool DCCEXProtocol::sendTurnoutAction(int turnoutId, TurnoutAction action) {
    if (delegate) {
        sprintf(outboundCommand, "<T %d %c>", turnoutId, action);

        sendCommand();
    }
    return true;
}

//private
void DCCEXProtocol::processTurnoutAction() { //<H id state>
    // console->println(F("processTurnoutAction(): "));
    if (delegate) {
        //find the Turnout entry to update
        if (turnouts.size()>0) { 
            char val[MAX_OBJECT_NAME_LENGTH];

            for (int i=0; i<turnouts.size(); i++) {
                strcpy(val, argz.get(1)->arg); strcat(val, "\0");
                int id = atoi(val);
                
                if (turnouts.get(i)->getTurnoutId()==id) {
                    strcpy(val, argz.get(2)->arg); strcat(val, "\0");
                    TurnoutState state = atoi(val);
                    if (argz.size() >= 3) {
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
void DCCEXProtocol::processRouteList() {
    // console->println(F("processRouteList()"));
    if (delegate) {
        if (routes.size()>0) { // already have a routes list so this is an update
            // routes.clear();
            console->println(F("processRouteList(): Routes/Automation list already received. Ignoring this!"));
            return;
        } 
        char val[MAX_OBJECT_NAME_LENGTH];
        char name[MAX_OBJECT_NAME_LENGTH];

        for (int i=1; i<argz.size(); i++) {
            strcpy(val, argz.get(i)->arg); strcat(val, "\0");
            int id = atoi(val);
            strcpy(name, NAME_UNKNOWN);

            routes.add(new Route(id, name));
            sendRouteEntryRequest(id);
        }
    }
    // console->println(F("processRouteList(): end"));
}

//private
void DCCEXProtocol::sendRouteEntryRequest(int address) {
    // console->println(F("sendRouteEntryRequest()"));
    if (delegate) {
        sprintf(outboundCommand, "<JA %d>", address);

        sendCommand();
    }
    // console->println(F("sendRouteEntryRequest() end"));
}

//private
void DCCEXProtocol::processRouteEntry() {
    // console->println(F("processRouteEntry()"));
    if (delegate) {
        //find the Route entry to update
        if (routes.size()>0) { 
            char val[MAX_OBJECT_NAME_LENGTH];
            char name[MAX_OBJECT_NAME_LENGTH];
            char cleanName[MAX_OBJECT_NAME_LENGTH];

            for (int i=0; i<routes.size(); i++) {
                strcpy(val, argz.get(1)->arg);
                strcat(val, "\0");
                int id = atoi(val);
                if (routes.get(i)->getRouteId()==id) {
                    char type = RouteTypeRoute;
                    strcpy(name, NAME_UNKNOWN);

                    if (strcmp(argz.get(2)->arg,"X") != 0) {
                        type = argz.get(2)->arg[0];
                        strcpy(name, argz.get(3)->arg);
                    }
                    stripLeadAndTrailQuotes(cleanName, name);
                    routes.get(i)->setRouteName(cleanName);
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

// void DCCEXProtocol::processRouteAction() {
//     console->println(F("processRouteAction(): "));
//     if (delegate) {

//     }
//     console->println(F("processRouteAction(): end"));
// }

// ****************
// Turntables

void DCCEXProtocol::processTurntableList() {  // <jO [id1 id2 id3 ...]>
    // console->println(F("processTurntableList(): "));
    if (delegate) {
        if (turntables.size()>0) { // already have a turntables list so this is an update
            // turntables.clear();
            console->println(F("processTurntableList(): Turntable list already received. Ignoring this!"));
            return;
        } 
        char val[MAX_OBJECT_NAME_LENGTH];
        char name[MAX_OBJECT_NAME_LENGTH];

        for (int i=1; i<argz.size(); i++) {
            strcpy(val, argz.get(i)->arg); strcat(val, "\0");
            int id = atoi(val);
            strcpy(name, NAME_UNKNOWN);

            turntables.add(new Turntable(id, name, TurntableTypeUnknown, 0, 0));
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
    if (delegate) {
        //find the Turntable entry to update
        if (turntables.size()>0) { 
            char val[MAX_OBJECT_NAME_LENGTH];
            char name[MAX_OBJECT_NAME_LENGTH];
            char cleanName[MAX_OBJECT_NAME_LENGTH];

            for (int i=0; i<turntables.size(); i++) {
                strcpy(val, argz.get(1)->arg); strcat(val, "\0");
                int id = atoi(val);
                if (turntables.get(i)->getTurntableId()==id) {
                    strcpy(name, NAME_UNKNOWN);
                    TurntableType type = TurntableTypeUnknown;
                    int position = 0;
                    int indexCount = 0;

                    if (argz.size() > 3) {  // server did not find the id
                        strcpy(name, argz.get(5)->arg);
                        strcpy(val, argz.get(2)->arg); strcat(val, "\0");
                        type = atoi(val);
                        strcpy(val, argz.get(3)->arg); strcat(val, "\0");
                        position = atoi(val);
                        strcpy(val, argz.get(4)->arg); strcat(val, "\0");
                        indexCount = atoi(val);
                    }
                    stripLeadAndTrailQuotes(cleanName, name);
                    turntables.get(i)->setTurntableName(cleanName);
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
void DCCEXProtocol::processTurntableIndexEntry() { // <jP id index angle "[desc]">
    // console->println(F("processTurntableIndexEntry(): "));
    if (delegate) {
        if (argz.size() > 3) {  // server did not find the index
            //find the Turntable entry to update
            if (turntables.size()>0) { 
                char val[MAX_OBJECT_NAME_LENGTH];
                char name[MAX_OBJECT_NAME_LENGTH];
                char cleanName[MAX_OBJECT_NAME_LENGTH];

                for (int i=0; i<turntables.size(); i++) {
                    strcpy(val, argz.get(1)->arg); strcat(val, "\0");
                    int id = atoi(val);

                    if (turntables.get(i)->getTurntableId()==id) {
                        //this assumes we are always starting from scratch, not updating indexes
                        strcpy(val, argz.get(2)->arg); strcat(val, "\0");
                        int index = atoi(val);
                        strcpy(val, argz.get(3)->arg); strcat(val, "\0");
                        int angle = atoi(val);
                        strcpy(name, argz.get(4)->arg);

                        stripLeadAndTrailQuotes(cleanName, name);
                        turntables.get(i)->turntableIndexes.add(new TurntableIndex(index, cleanName, angle));
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
void DCCEXProtocol::processTurntableAction() { // <i id position moving>
    // console->println(F("processTurntableAction(): "));
    if (delegate) {
        char val[MAX_OBJECT_NAME_LENGTH];
        strcpy(val, argz.get(1)->arg); strcat(val, "\0");
        int id = atoi(val);
        strcpy(val, argz.get(2)->arg); strcat(val, "\0");
        int newPos = atoi(val);
        TurntableState state = argz.get(3)->arg[0];

        int pos = findTurntableListPositionFromId(id);
        if (pos!=newPos) {
            turntables.get(pos)->actionTurntableExternalChange(newPos, state);
        }
        delegate->receivedTurntableAction(id, newPos, state);
    }
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
    if (delegate) {
        char val[MAX_OBJECT_NAME_LENGTH];
        strcpy(val, argz.get(1)->arg); strcat(val, "\0");
        int address = atoi(val);
        strcpy(val, argz.get(3)->arg); strcat(val, "\0");
        int speedByte = atoi(val);
        strcpy(val, argz.get(4)->arg); strcat(val, "\0");
        int functMap = atoi(val);

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

                        throttleConsists[throttleNo].consistGetLocoAtPosition(0)->locoFunctions.setFunctionState(i, fnStates[i]);

                        // console->print(i);
                        // console->print(" - ");
                        // console->print(charToCharArray(fnStates[i]));
                        // console->print(" - ");
                        // console->print(charToCharArray(throttleConsists[throttleNo].consistGetLocoAtPosition(0)->locoFunctions.getFunctionState(i)));
                        // console->println(" - ");

                        delegate->receivedFunction(throttleNo, i, fnStates[i]);
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
        strcpy(outboundCommand, "<s>");
        sendCommand();        
    }
    // console->println(F("sendServerDetailsRequest(): end"));
    return true; 
}

// ******************************************************************************************************
// power commands

bool DCCEXProtocol::sendTrackPower(TrackPower state) {
    // console->println(F("sendTrackPower(): "));
    if (delegate) {
        char _state[2];
        _state[0] = state;
        _state[1] = '\0';

        strcpy(outboundCommand, "<");
        strcat(outboundCommand, _state);
        strcat(outboundCommand, ">");

        sendCommand();
    }
    // console->println(F("sendTrackPower(): end"));
    return true;
}

bool DCCEXProtocol::sendTrackPower(TrackPower state, char track) {
    // console->println(F("sendTrackPower(): "));
    if (delegate) {
        char _state[2];
        _state[0] = state;
        _state[1] = '\0';

        char _track[2];
        _track[0] = track;
        _track[1] = '\0';

        strcpy(outboundCommand, "<");
        strcat(outboundCommand, _state);
        strcat(outboundCommand, " ");
        strcat(outboundCommand, _track);
        strcat(outboundCommand, ">");

        sendCommand();
    }
    // console->println(F("sendTrackPower(): end"));
    return true;
}

// ******************************************************************************************************

void DCCEXProtocol::sendEmergencyStop() {
    // console->println(F("emergencyStop(): "));
    if (delegate) {
        strcpy(outboundCommand, "<!>");
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
        return throttleConsists[throttleNo];
    }
    // console->println(F("getThrottleConsist(): end"));
    return {};
}


// ******************************************************************************************************

// by default only send to the lead loco
bool DCCEXProtocol::sendFunction(int throttle, int functionNumber, FunctionState pressed) {
    // console->println(F("sendFunction(): "));
    if (delegate) {
        ConsistLoco* conLoco = throttleConsists[throttle].consistGetLocoAtPosition(0);
        int address = conLoco->getLocoAddress();
        if (address>=0) {
            sendFunction(throttle, address, functionNumber, pressed);
        }
    }
    // console->println(F("sendFunction(): end")); 
    return true;
}

// send to a specific address on the throttle
bool DCCEXProtocol::sendFunction(int throttle, int address, int functionNumber, FunctionState pressed) { // throttle is ignored
    // console->println(F("sendFunction(): "));
    if (delegate) {
        char _address[6];
        itoa(address, _address, 10);
        
        char _functionNumber[3];
        itoa(functionNumber, _functionNumber, 10);

        char _pressed[2];
        _pressed[0] = pressed;
        _pressed[1] = '\0';

        strcpy(outboundCommand, "<F ");
        strcat(outboundCommand, _address);
        strcat(outboundCommand, " ");
        strcat(outboundCommand, _functionNumber);
        strcat(outboundCommand, " ");
        strcat(outboundCommand, _pressed);
        strcat(outboundCommand, ">");

        sendCommand();
    }
    // console->println(F("sendFunction(): end")); 
    return true;
}

// by default only check the lead loco on the throttle
bool DCCEXProtocol::isFunctionOn(int throttle, int functionNumber) {
    if (delegate) {
        ConsistLoco* conLoco = throttleConsists[throttle].consistGetLocoAtPosition(0);
        int address = conLoco->getLocoAddress();
        if (address>=0) {
            console->print(" '");
            console->print(conLoco->locoFunctions.getFunctionState(functionNumber));
            console->print("' ");
            conLoco->isFunctionOn(functionNumber);
        }
        return (conLoco->locoFunctions.getFunctionState(functionNumber)==FunctionStateOn) ? true : false;
    }
    return false;
}

// ******************************************************************************************************
// throttle

bool DCCEXProtocol::sendThrottleAction(int throttle, int speed, Direction direction) {
    console->println(F("sendThrottleAction(): "));
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
    console->println(F("sendThrottleAction(): end"));
    return true;
}



// ******************************************************************************************************
// individual locos
// 

bool DCCEXProtocol::sendLocoUpdateRequest(int address) {
    // console->println(F("sendLocoUpdateRequest()"));
    if (delegate) {
        char val[6];
        itoa(address, val, 10);
        
        strcpy(outboundCommand, "<t ");
        strcat(outboundCommand, val);
        strcat(outboundCommand, ">");

        sendCommand();
    }
    // console->println(F("sendLocoUpdateRequest() end"));
    return true;
}

bool DCCEXProtocol::sendLocoAction(int address, int speed, Direction direction) {
    console->print(F("sendLocoAction(): ")); console->println(address);
    if (delegate) {
        char val[6];
        itoa(address, val, 10);
        
        char val2[3];
        itoa(speed, val2, 10);

        char _direction[2];
        _direction[0] = direction;
        _direction[1] = '\0';

        strcpy(outboundCommand, "<t ");
        strcat(outboundCommand, val);
        strcat(outboundCommand, " ");
        strcat(outboundCommand, val2);
        strcat(outboundCommand, " ");
        strcat(outboundCommand, _direction);
        strcat(outboundCommand, ">");

        sendCommand();
    }
    console->println(F("sendLocoAction(): end"));
    return true;
}

// ******************************************************************************************************

bool DCCEXProtocol::sendRouteAction(int routeId) {
    // console->println(F("sendRouteAction()"));
    if (delegate) {
        // char val[6];
        // itoa(routeId, val, 10);

        // strcpy(outboundCommand, "</START ");
        // strcat(outboundCommand, val);
        // strcat(outboundCommand, ">");

        sprintf(outboundCommand, "</START  %d >", routeId);
        sendCommand();
    }
    // console->println(F("sendRouteAction() end"));
    return true;
}

bool DCCEXProtocol::sendPauseRoutes() {
    // console->println(F("sendPauseRoutes()"));
    if (delegate) {
        // strcpy(outboundCommand, "</PAUSE>");
        sprintf(outboundCommand, "</PAUSE>");
        sendCommand();
    }
    // console->println(F("sendPauseRoutes() end"));
    return true;
}

bool DCCEXProtocol::sendResumeRoutes() {
    // console->println(F("sendResumeRoutes()"));
    if (delegate) {
        // strcpy(outboundCommand, "</RESUME>");
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
        // char val[6];
        // itoa(turntableId, val, 10);
        
        // char val2[3];
        // itoa(activity, val2, 10);

        // char val3[3];
        // itoa(position, val3, 10);

        // strcpy(outboundCommand, "<I ");
        // strcat(outboundCommand, val);
        // strcat(outboundCommand, " ");
        // strcat(outboundCommand, val2);
        // strcat(outboundCommand, " ");
        // strcat(outboundCommand, val3);
        // strcat(outboundCommand, ">");

        sprintf(outboundCommand, "<I %d %d %d>", turntableId, position, activity);

        sendCommand();
    }
    // console->println(F("sendTurntable() end"));
    return true;
}

bool DCCEXProtocol::sendAccessoryAction(int accessoryAddress, int activate) {
    // console->println(F("sendAccessory()"));
    if (delegate) {
        // char val[6];
        // itoa(accessoryAddress, val, 10);
        
        // char val2[3];
        // itoa(activate, val2, 10);

        // strcpy(outboundCommand, "<a ");
        // strcat(outboundCommand, val);
        // strcat(outboundCommand, " ");
        // strcat(outboundCommand, val2);
        // strcat(outboundCommand, ">");

        sprintf(outboundCommand, "<a %d %d %d>", accessoryAddress, activate);

        sendCommand();
    }
    // console->println(F("sendAccessory() end"));
    return true;
}

bool DCCEXProtocol::sendAccessoryAction(int accessoryAddress, int accessorySubAddr, int activate) {
    // console->println(F("sendAccessory()"));
    if (delegate) {
        // char val[6];
        // itoa(accessoryAddress, val, 10);
        
        // char val2[3];
        // itoa(accessorySubAddr, val2, 10);

        // char val3[3];
        // itoa(activate, val3, 10);

        // strcpy(outboundCommand, "<a ");
        // strcat(outboundCommand, val);
        // strcat(outboundCommand, " ");
        // strcat(outboundCommand, val2);
        // strcat(outboundCommand, " ");
        // strcat(outboundCommand, val3);
        // strcat(outboundCommand, ">");

        sprintf(outboundCommand, "<a %d %d %d>", accessoryAddress, accessorySubAddr, activate);
        sendCommand();
    }
    // console->println(F("sendAccessory() end"));
    return true;
}

// ******************************************************************************************************

// sequentially request and get the required lists. To avoid overloading the buffer
bool DCCEXProtocol::getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired) {

    if (!allRequiredListsReceived) {
        if (!rosterRequested) {
            getRoster();
        } else { 
            if (rosterFullyReceived) {

                if (!turnoutListRequested) {
                    getTurnouts();
                } else { 
                    if (turnoutListFullyReceived) {

                        if (!routeListRequested) {
                            getRoutes();
                        } else { 
                            if (routeListFullyReceived) {

                                if (!turntableListRequested) {
                                    getTurntables();
                                } else { 
                                    if (turntableListFullyReceived) {

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
  return true;
}

bool DCCEXProtocol::getRoster() {
    // console->println(F("getRoster()"));
    if (delegate) {
        // strcpy(outboundCommand, "<JR>");
        sprintf(outboundCommand, "<JR>");
        sendCommand();
        rosterRequested = true;
    }
    // console->println(F("getRoster() end"));
    return true;
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
        // strcpy(outboundCommand, "<JT>");
        sprintf(outboundCommand, "<JT>");
        sendCommand();
        turnoutListRequested = true;
    }
    // console->println(F("getTurnouts() end"));
    return true;
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
        // strcpy(outboundCommand, "<JA>");
        sprintf(outboundCommand, "<JA>");
        sendCommand();
        routeListRequested = true;
    }
    // console->println(F("getRoutes() end"));
    return true;
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
        // strcpy(outboundCommand, "<JO>");
        sprintf(outboundCommand, "<JO>");
        sendCommand();
        turntableListRequested = true;
    }
    // console->println(F("getTurntables() end"));
    return true;
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
Loco DCCEXProtocol::findLocoInRoster(int address) {
    if (roster.size()>0) { 
        for (int i=0; i<roster.size(); i++) {
            if (roster.get(i)->getLocoAddress() == address) {
                return *roster.get(i);
            }
        }
    }
    return {};
}

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

bool DCCEXProtocol::splitValues(char *cmd) {
    // console->println(F("splitValues(): "));
    byte parameterCount = 0;
    
    int currentCharIndex = 0; // pointer to the next character to process
    splitState state = FIND_START;
    char currentArg[MAX_SINGLE_COMMAND_PARAM_LENGTH];
    currentArg[0]='\0';
    int currentArgLength = 0;
    
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
            if (currentArgLength < (MAX_SINGLE_COMMAND_PARAM_LENGTH-1)) {
                currentArg[currentArgLength] = currentChar;
                currentArg[currentArgLength+1]='\0';
                // console->println(currentArg);
                currentArgLength++;
            }
            if ( (currentArgLength>1) && (currentChar == '"') ) { // trailing quote
                argz.add( new CommandArgument(currentArg) );
                currentArg[0]='\0';
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
                if (currentArgLength < (MAX_SINGLE_COMMAND_PARAM_LENGTH-1)) {
                    currentArg[currentArgLength] = currentChar;
                    currentArg[currentArgLength+1]='\0';
                    // console->println(currentArg);
                    currentArgLength++;
                }
                break;
            }

            // space - end of parameter detected 
            argz.add( new CommandArgument(currentArg) );

            currentArg[0]='\0';
            currentArgLength = 0;
            parameterCount++;
            state = SKIP_SPACES;

                // for (uint i=0; i<argz.size();i++) {
                //     // console->print("args b "); console->print(i); console->print(": ~"); console->print(args.get(i)); console->println("~");
                //     console->print("argz b "); console->print(i); console->print(": ~"); console->print(argz.get(i)->arg); console->println("~");
                // }

            continue;
        
        case CHECK_FOR_END:
            if ( (currentChar=='\0') || (currentChar == '>') ) {
                if (currentArgLength>0) { // in case there was a param we started but didn't find the proper end
                    argz.add( new CommandArgument(currentArg));
                }
                // console->println(F("splitValues(): end"));
                return true;  
            }
            break;
        }
        currentCharIndex++;
    }

    // console->println(F("splitValues(): end"));
    return true;
    
}

bool DCCEXProtocol::splitFunctions(char *cmd) {
    // console->println(F("splitFunctions(): "));
    byte parameterCount = 0;
    
    int currentCharIndex = 0; // pointer to the next character to process
    splitFunctionsState state = FIND_FUNCTION_START;
    char currentArg[MAX_SINGLE_COMMAND_PARAM_LENGTH];
    currentArg[0]='\0';
    int currentArgLength = 0;
    
    while (parameterCount < MAX_FUNCTIONS)
    {
        char currentChar = cmd[currentCharIndex];
        // console->print("..."); console->print(currentChar); console->println("...");
        // In this switch, 'break' will go on to next char but 'continue' will rescan the current char. 
        switch (state) {
        case FIND_FUNCTION_START: // looking for leading '"'
            if (currentChar == '"') state = SKIP_FUNCTION_SPACES;
            break;

        case SKIP_FUNCTION_LEADING_SLASH_SPACES: // skipping only 1 "/" before a param
            state = SKIP_FUNCTION_SPACES;
            if (currentChar == '/') break; // ignore
            continue;

        case SKIP_FUNCTION_SPACES: // skipping spaces before a param
            if (currentChar == ' ') break; // ignore
            state = BUILD_FUNCTION_PARAM;
            continue;

        case BUILD_FUNCTION_PARAM: // building a parameter
            if (currentChar == '"') {
                state = CHECK_FOR_FUNCTION_END;
                continue;
            }
            if (currentChar != '/') {
                currentArg[currentArgLength] = currentChar;
                currentArg[currentArgLength+1]='\0';
                // console->println(currentArg);
                currentArgLength++;
                break;
            }

            // end of parameter detected  '/')
            functionArgs.add( new FunctionArgument(currentArg));
            currentArg[0]='\0';
            currentArgLength = 0;
            parameterCount++;
            state = SKIP_FUNCTION_LEADING_SLASH_SPACES;
            continue;
        
        case CHECK_FOR_FUNCTION_END:
            if ( (currentChar == '\0') || (currentChar == '"') ) {
                // trailing function
                functionArgs.add( new FunctionArgument(currentArg));
                return true;  
            }
            break;
        }
        currentCharIndex++;
    }
    
    console->println(F("splitFunctions(): end"));
    return true;
}

bool DCCEXProtocol::stripLeadAndTrailQuotes(char* rslt, char* text) {
    if (text[0]=='"' && text[strlen(text)-1]=='"') {
        for(size_t i=1; i<strlen(text)-1; i++) {
            rslt[i-1] = text[i];
        }
        rslt[strlen(text)-2]='\0';
    } else {
        for(size_t i=0; i<strlen(text); i++) {
            rslt[i] = text[i];
        }
        rslt[strlen(text)]='\0';
    }
    return true;
}

// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************
// subsidary classes

    CommandArgument::CommandArgument(char* argValue) {
        char *dynName;
        dynName = (char *) malloc(strlen(argValue)+1);
        strcpy(dynName, argValue);

        arg = dynName;
    }
    bool CommandArgument::clearCommandArgument() {
        free(arg);
        arg = nullptr;

        return true;
    }

    FunctionArgument::FunctionArgument(char* argValue) {
        char *dynName;
        dynName = (char *) malloc(strlen(argValue)+1);
        strcpy(dynName, argValue);

        arg = dynName;        
    }
    bool FunctionArgument::clearFunctionArgument() {
        free(arg);
        arg = nullptr;

        return true;
    }

// class Functions

    bool Functions::initFunction(int functionNumber, char* label, FunctionLatching latching, FunctionState state) {
        functionLatching[functionNumber] = latching;
        functionState[functionNumber] = state;
        
        char *dynName;
        dynName = (char *) malloc(strlen(label)+1);
        strcpy(dynName, label);
        functionName[functionNumber] = dynName;
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
    bool Functions::setFunctionName(int functionNumber,char* label) {
        if (functionName[functionNumber]!=nullptr) {
            free(functionName[functionNumber]);
            functionName[functionNumber]=nullptr; 
        }
        char *dynName;
        dynName = (char *) malloc(strlen(label)+1);
        strcpy(dynName, label);

        functionName[functionNumber] = dynName;
        return true;
    }
    char* Functions::getFunctionName(int functionNumber) {
        return functionName[functionNumber];
    }
    FunctionState Functions::getFunctionState(int functionNumber) {
        return functionState[functionNumber];
    }
    FunctionLatching Functions::getFunctionLatching(int functionNumber) {
        return functionLatching[functionNumber];
    }
    // private
    bool Functions::clearFunctionNames() {
        for (uint i=0; i<MAX_FUNCTIONS; i++) {
            if (functionName[i] != nullptr) {
                free(functionName[i]);
                functionName[i] = nullptr; 
            }
        }
        return true;
    }
// class Loco

    Loco::Loco(int address, char* name, LocoSource source) {
        locoAddress = address;
        locoSource = source;
        locoDirection = Forward;
        locoSpeed = 0;
        rosterReceivedDetails = false;
        char fnName[MAX_OBJECT_NAME_LENGTH];

        char *dynName;
        dynName = (char *) malloc(strlen(name)+1);
        strcpy(dynName, name);
        locoName = dynName;

        for (uint i=0; i<MAX_FUNCTIONS; i++) {
            strcpy(fnName, NAME_BLANK);
            locoFunctions.initFunction(i, fnName, FunctionLatchingFalse, FunctionStateOff);
        }
    }
    bool Loco::isFunctionOn(int functionNumber) {
        return (locoFunctions.getFunctionState(functionNumber)==FunctionStateOn) ? true : false;
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
    bool Loco::setLocoName(char* name) {
        if (locoName != nullptr) {
            free(locoName);
            locoName = nullptr; 
        }
        char *dynName;
        dynName = (char *) malloc(strlen(name)+1);
        strcpy(dynName, name);

        locoName = dynName;
        return true;
    }
    char* Loco::getLocoName() {
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
    bool Loco::clearLocoNameAndFunctions() {
        if (locoName != nullptr) {
            free(locoName);
            locoName=nullptr; 
        }
        locoFunctions.clearFunctionNames();
        return true;
    }

// class ConsistLoco : public Loco

    ConsistLoco::ConsistLoco(int address, char* name, LocoSource source, Facing facing) 
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

    Consist::Consist(char* name) {
        char *_name;
        _name = (char *) malloc(strlen(name)+1);
        strcpy(_name, name);
        consistName = _name;
    }
    bool Consist::consistAddLoco(Loco loco, Facing facing) {
        int address = loco.getLocoAddress();
        char name[MAX_SINGLE_COMMAND_PARAM_LENGTH];
        strcpy(name, loco.getLocoName());
        LocoSource source = loco.getLocoSource();
        Facing correctedFacing = facing;
        int rslt = consistGetLocoPosition(address);
        if (rslt<0) { // not already in the list, so add it
            if (consistGetNumberOfLocos() == 0) correctedFacing = FacingForward; // first loco in consist is always forward
            consistLocos.add(new ConsistLoco(address, name, source, correctedFacing));

            //fix the name of the consist
            char _consistName[MAX_SINGLE_COMMAND_PARAM_LENGTH];
            strcpy(_consistName, consistLocos.get(0)->getLocoName());
            if (rslt>1) { // must be more than two now
                for (int i=1; i<consistLocos.size(); i++) {
                    strcat(_consistName, ", ");
                    strcat(_consistName, consistLocos.get(i)->getLocoName());
                }
                setConsistName(_consistName);
            }
            return true;
        }
        return false;
    }
    // create from a DCC Address
    bool Consist::consistAddLocoFromRoster(LinkedList<Loco*> roster, int address, Facing facing) {
        if (roster.size()>0) { 
            for (int i=0; i<roster.size(); i++) {
                if (roster.get(i)->getLocoAddress() == address) {
                    consistAddLoco(*roster.get(i), facing);
                    return  true;
                }
            }
        }
        return false;
    }
    // create from a DCC Address
    bool Consist::consistAddLocoFromAddress(int address, char* name, Facing facing) {
        Loco loco = Loco(address, name, LocoSourceEntry);
        consistAddLoco(loco, facing);
        return true;
    }
    bool Consist::consistReleaseAllLocos()  {
        if (consistLocos.size()>0) {
            for (int i=1; i<consistLocos.size(); i++) {
                consistLocos.get(i)->clearLocoNameAndFunctions();
            }
            consistLocos.clear();
            char _consistName[1];
            strcpy(_consistName, "\0");
            setConsistName(_consistName);
        }
        return true;
    }
    bool Consist::consistReleaseLoco(int locoAddress)  {
        int rslt = consistGetLocoPosition(locoAddress);
        if (rslt>=0) {
            consistLocos.get(rslt)->clearLocoNameAndFunctions();
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
            char* name = loco->getLocoName();
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

                    //????????????????? TODO

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
    // by default only set the function on the lead loco
    bool Consist::consistSetFunction(int functionNo, FunctionState state) {
        if (consistLocos.size()>0) {
            // ConsistLoco* loco = consistGetLocoAtPosition(0);
            //????????????????? TODO
            return true;
        }
        return false;
    }
    // set a function on a specific loco on a throttle
    bool Consist::consistSetFunction(int address, int functionNo, FunctionState state) {
        //????????????????? TODO
        // individual loco
        return true;
    }
    bool Consist::isFunctionOn(int functionNumber) {
        if (consistLocos.size()>0) {
            ConsistLoco* loco = consistGetLocoAtPosition(0);
            return loco->isFunctionOn(functionNumber);
        }
        return false;
    }
    bool Consist::setConsistName(char* name) {
        if (consistName != nullptr) {
            free(consistName);
            consistName = nullptr; 
        }
        char *_name;
        _name = (char *) malloc(strlen(name)+1);
        strcpy(_name, name);
        consistName = _name;
        return true;
    }
    char* Consist::getConsistName() {
        return consistName;
    }

// class Turnout

    Turnout::Turnout(int id, char* name, TurnoutState state) {
        turnoutId = id;
        turnoutState = state;
        hasReceivedDetail = false;

        char *dynName;
        dynName = (char *) malloc(strlen(name)+1);
        strcpy(dynName, name);
        turnoutName = dynName;
    }
    bool Turnout::throwTurnout() {
        setTurnoutState(TurnoutThrow);
        return true;
    }
    bool Turnout::closeTurnout() {
        setTurnoutState(TurnoutClose);
        return true;
    }
    bool Turnout::toggleTurnout() {
        setTurnoutState(TurnoutToggle);
        return true;
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
    bool Turnout::setTurnoutName(char* name) {
        if (turnoutName != nullptr) {
            free(turnoutName);
            turnoutName=nullptr; 
        }
        char *dynName;
        dynName = (char *) malloc(strlen(name)+1);
        strcpy(dynName, name);

        turnoutName = dynName;
        return true;
    }
    char* Turnout::getTurnoutName() {
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

    Route::Route(int id, char* name) {
        routeId = id;
        hasReceivedDetail = false;

        char *dynName;
        dynName = (char *) malloc(strlen(name)+1);
        strcpy(dynName, name);
        routeName = dynName;
    }
    int Route::getRouteId() {
        return routeId;
    }
    bool Route::setRouteName(char* name) {
        if (routeName != nullptr) {
            free(routeName);
            routeName=nullptr; 
        }
        char *dynName;
        dynName = (char *) malloc(strlen(name)+1);
        strcpy(dynName, name);

        routeName = dynName;
        return true;
    }
    char* Route::getRouteName() {
        return routeName;
    }
    bool Route::setRouteType(RouteType type) {
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

    TurntableIndex::TurntableIndex(int index, char* name, int angle) {
        turntableIndexIndex = index;
        turntableIndexAngle = angle;

        char *dynName;
        dynName = (char *) malloc(strlen(name)+1);
        strcpy(dynName, name);

        turntableIndexName = dynName;
    }
    char* TurntableIndex::getTurntableIndexName() {
        return turntableIndexName;
    }
    int TurntableIndex::getTurntableIndexId() {
        return turntableIndexId;
    }
    int TurntableIndex::getTurntableIndexIndex() {
        return turntableIndexIndex;
    }

// class Turntable

    Turntable::Turntable(int id, char* name, TurntableType type, int position, int indexCount) {
        turntableId = id;
        turntableType = type;
        turntableCurrentPosition = position;
        turnTableIndexCount = indexCount;

        char *dynName;
        dynName = (char *) malloc(strlen(name)+1);
        strcpy(dynName, name);
        turntableName = dynName;
    }
    int Turntable::getTurntableId() {
        return turntableId;
    }
    bool Turntable::setTurntableName(char* name) {
        if (turntableName != nullptr) {
            free(turntableName);
            turntableName=nullptr; 
        }
        char *dynName;
        dynName = (char *) malloc(strlen(name)+1);
        strcpy(dynName, name);

        turntableName = dynName;
        return true;
    }
    char* Turntable::getTurntableName() {
        return turntableName;
    }
    bool Turntable::setTurntableType(TurntableType type) {
        turntableType = type;
        return true;
    }
    TurntableType Turntable::getTurntableType() {
        return turntableType;
    }
    bool Turntable::addTurntableIndex(int index, char* indexName, int indexAngle) {
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