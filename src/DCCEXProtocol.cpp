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
}

void DCCEXProtocol::disconnect() {
    // strcpy(outboundCommand, "<U DISCONNECT>");
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
            
            /* Original check function from here:
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
            -- to here */
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

        // strcpy(outboundCommand, "\0"); // clear it once it has been sent
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

/* Old processCommand() starts here --
bool DCCEXProtocol::processCommand(char* c, int len) {
    console->println(F("processCommand()"));

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
    // strcpy(arg1, "\0");
    // strcpy(arg2, "\0");
    *arg1 = 0;
    *arg2 = 0;

    if (argz.size()>1) {
        // strcpy(arg1, argz.get(1)->arg); strcat(arg1, "\0");
        sprintf(arg1,"%s",argz.get(1)->arg);
        if (argz.size()>2) {
            // strcpy(arg2, argz.get(2)->arg); strcat(arg2, "\0");
            sprintf(arg1,"%s",argz.get(1)->arg);
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
                // delegate->receivedTurnoutAction(atoi(arg1), arg2[0]);
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

    console->println(F("processCommand() end"));
    return true;
}
-- old processCommand() ends here */

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
        console->print(F("Process server description with "));
        console->print(DCCEXInbound::getParameterCount());
        console->println(F(" params"));
        
        char *_serverVersion;
        // _serverVersion = (char *) malloc(strlen(argz.get(1)->arg)+1);
        // // strcpy(_serverVersion, argz.get(1)->arg);
        // sprintf(_serverVersion,"%s",argz.get(1)->arg);
        _serverVersion = (char *) malloc(strlen(DCCEXInbound::getText(0))+1);
        sprintf(_serverVersion,"%s",DCCEXInbound::getText(0));
        serverVersion = _serverVersion;
        console->println(serverVersion);

        // char *_serverMicroprocessorType;
        // _serverMicroprocessorType = (char *) malloc(strlen(argz.get(3)->arg)+1);
        // // strcpy(_serverMicroprocessorType, argz.get(3)->arg);
        // sprintf(_serverMicroprocessorType,"%s",argz.get(3)->arg);
        // serverMicroprocessorType = _serverMicroprocessorType;

        // char *_serverMotorcontrollerType;
        // _serverMotorcontrollerType = (char *) malloc(strlen(argz.get(5)->arg)+1);
        // // strcpy(_serverMotorcontrollerType, argz.get(5)->arg);
        // sprintf(_serverMotorcontrollerType,"%s",argz.get(5)->arg);
        // serverMotorcontrollerType = _serverMotorcontrollerType;

        // char *_serverBuildNumber;
        // _serverBuildNumber = (char *) malloc(strlen(argz.get(6)->arg)+1);
        // // strcpy(_serverBuildNumber, argz.get(6)->arg);
        // sprintf(_serverBuildNumber,"%s",argz.get(6)->arg);
        // serverBuildNumber = _serverBuildNumber;        

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
    if (delegate) {
        if (roster.size()>0) { // already have a roster so this is an update
            // roster.clear();
            console->println(F("processRosterList(): roster list already received. Ignoring this!"));
            return;
        } 
        // char arg[MAX_OBJECT_NAME_LENGTH];
        char name[MAX_OBJECT_NAME_LENGTH];
        for (int i=1; i<DCCEXInbound::getParameterCount(); i++) {
            int address = DCCEXInbound::getNumber(i);
            sprintf(name, "%s", NAME_UNKNOWN);
            roster.add(new Loco(address, name, LocoSourceRoster));
            sendRosterEntryRequest(address);
        }
        // for (int i=1; i<argz.size(); i++) {
        //     // strcpy(arg, argz.get(i)->arg); strcat(arg, "\0");
        //     sprintf(arg,"%s",argz.get(i)->arg);
        //     int address = atoi(arg);
        //     // strcpy(name, NAME_UNKNOWN);
        //     sprintf(name, "%s", NAME_UNKNOWN);

        //     roster.add(new Loco(address, name, LocoSourceRoster));
        //     sendRosterEntryRequest(address);
        // }
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
    //find the roster entry to update
    int address=DCCEXInbound::getNumber(1);
    char *name=DCCEXInbound::getSafeText(2);
    char *funcs=DCCEXInbound::getSafeText(3);
    bool missingRosters=false;
    
    for (int i=0; i<roster.size(); i++) {
        auto r=roster.get(i);
        if (r->getLocoAddress() == address) {
            // console->print("processRosterEntry(): found: "); console->println(address);

            r->setLocoName(name);
            r->setLocoSource(LocoSourceRoster);
            r->setIsFromRosterAndReceivedDetails();

            console->println(DCCEXInbound::getText(3));
            splitFunctions(funcs);

            /* STOP FUNCTIONS
            for( int i=0; i<functionArgs.size(); i++) { functionArgs.get(i)->clearFunctionArgument(); }
            functionArgs.clear();

            splitFunctions(functions);
            int noOfParameters = functionArgs.size();

            for (uint i=0; i<functionArgs.size();i++) {
                console->print("fn"); console->print(i); console->print(": ~"); console->print(functionArgs.get(i)->arg); console->println("~");
            }

            console->print("processing Functions: "); console->println(functions);

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
            STOP FUNCTIONS */
        } else {
            if (!r->getIsFromRosterAndReceivedDetails()) {
                console->print(F("processRosterEntry(): not received yet: ~"));
                console->print(r->getLocoName());
                console->print("~ ");
                console->println(r->getLocoAddress());
                missingRosters=true;
            }
        }
    }
    if (!missingRosters) {
        rosterFullyReceived = true;
        console->println(F("processRosterEntry(): received all"));
        delegate->receivedRosterList(roster.size());
    }
    // console->println(F("processRosterEntry(): end"));
}

// ****************
// Turnouts/Points

//private
void DCCEXProtocol::processTurnoutList() {
    // <jT id1 id2 id3 ...>
    console->println(F("processTurnoutList()"));
    if (turnouts.size()>0) { // already have a turnouts list so this is an update
        // turnouts.clear();
        console->println(F("processTurnoutList(): Turnout/Points list already received. Ignoring this!"));
        return;
    } 
    char name[MAX_OBJECT_NAME_LENGTH];

    for (int i=1; i<DCCEXInbound::getParameterCount(); i++) {
        int id = DCCEXInbound::getNumber(i);
        sprintf(name, "%s", NAME_UNKNOWN);
        
        turnouts.add(new Turnout(id, name, TurnoutClosed));
        sendTurnoutEntryRequest(id);
    }
    console->println(F("processTurnoutList(): end"));
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
    // console->println(F("processTurnoutEntry()"));
    //find the turnout entry to update
    int id=DCCEXInbound::getNumber(1);
    TurnoutStates state=DCCEXInbound::getNumber(2)==1 ? TurnoutThrown : TurnoutClosed;
    char* name=DCCEXInbound::getSafeText(3);

    for (int i=0; i<turnouts.size(); i++) {
        auto t=turnouts.get(i);
        if (t->getTurnoutId()==id) {
            t->setTurnoutId(id);
            t->setTurnoutState(state);
            t->setTurnoutName(name);
            t->setHasReceivedDetails();
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

bool DCCEXProtocol::sendTurnoutAction(int turnoutId, TurnoutStates action) {
    if (delegate) {
        sprintf(outboundCommand, "<T %d %d>", turnoutId, action);
        sendCommand();
    }
    return true;
}

//private
void DCCEXProtocol::processTurnoutAction() { //<H id state>
    // console->println(F("processTurnoutAction(): "));
    if (DCCEXInbound::getParameterCount()!=2) return;
    //find the Turnout entry to update
    int id = DCCEXInbound::getNumber(0);
    TurnoutStates state = DCCEXInbound::getNumber(1)==1 ? TurnoutThrown : TurnoutClosed;
    for (int i=0; i<turnouts.size(); i++) {
        auto t=turnouts.get(i);
        if (t->getTurnoutId()==id) {
            t->setTurnoutState(state);
            delegate->receivedTurnoutAction(id, state);
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
        // char val[MAX_OBJECT_NAME_LENGTH];
        char name[MAX_OBJECT_NAME_LENGTH];

        // for (int i=1; i<argz.size(); i++) {
        for (int i=1; i<DCCEXInbound::getParameterCount(); i++) {
            // strcpy(val, argz.get(i)->arg); strcat(val, "\0");
            // sprintf(val,"%s",argz.get(i)->arg);
            // int id = atoi(val);
            // strcpy(name, NAME_UNKNOWN);
            int id = DCCEXInbound::getNumber(i);
            sprintf(name, "%s", NAME_UNKNOWN);

            routes.add(new Route(id, name));
            sendRouteEntryRequest(id);
        }
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
    if (delegate) {
        //find the Route entry to update
        if (routes.size()>0) { 
            // char val[MAX_OBJECT_NAME_LENGTH];
            // char name[MAX_OBJECT_NAME_LENGTH];
            // char cleanName[MAX_OBJECT_NAME_LENGTH];

            for (int i=0; i<routes.size(); i++) {
                // strcpy(val, argz.get(1)->arg); strcat(val, "\0");
                // sprintf(val,"%s",argz.get(1)->arg);
                // int id = atoi(val);
                int id = DCCEXInbound::getNumber(1);
                if (routes.get(i)->getRouteId()==id) {
                    // char type = RouteTypeRoute;
                    // strcpy(name, NAME_UNKNOWN);
                    // sprintf(name, "%s", NAME_UNKNOWN);

                    // if (strcmp(argz.get(2)->arg,"X") != 0) {
                    //     type = argz.get(2)->arg[0];
                    //     // strcpy(name, argz.get(3)->arg);
                    //     sprintf(name,"%s",argz.get(3)->arg);
                    // }
                    // stripLeadAndTrailQuotes(cleanName, name);
                    // routes.get(i)->setRouteName(cleanName);
                    // routes.get(i)->setRouteType(type);
                    routes.get(i)->setRouteType((RouteType)DCCEXInbound::getNumber(2));
                    routes.get(i)->setRouteName(DCCEXInbound::getSafeText(3));
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
        // char val[MAX_OBJECT_NAME_LENGTH];
        char name[MAX_OBJECT_NAME_LENGTH];

        // for (int i=1; i<argz.size(); i++) {
        for (int i=1; i<DCCEXInbound::getParameterCount(); i++) {
            // strcpy(val, argz.get(i)->arg); strcat(val, "\0");
            // sprintf(val,"%s",argz.get(i)->arg);
            // int id = atoi(val);
            // strcpy(name, NAME_UNKNOWN);
            int id = DCCEXInbound::getNumber(i);
            sprintf(name, "%s", NAME_UNKNOWN);

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
            // char val[MAX_OBJECT_NAME_LENGTH];
            // char name[MAX_OBJECT_NAME_LENGTH];
            // char cleanName[MAX_OBJECT_NAME_LENGTH];

            for (int i=0; i<turntables.size(); i++) {
                // strcpy(val, argz.get(1)->arg); strcat(val, "\0");
                // sprintf(val,"%s",argz.get(1)->arg);
                // int id = atoi(val);
                int id = DCCEXInbound::getNumber(i);
                if (turntables.get(i)->getTurntableId()==id) {
                    // strcpy(name, NAME_UNKNOWN);
                    // sprintf(name, "%s", NAME_UNKNOWN);
                    // TurntableType type = TurntableTypeUnknown;
                    // int position = 0;
                    // int indexCount = 0;

                    // if (argz.size() > 3) {  // server did not find the id
                    //     // strcpy(name, argz.get(5)->arg);
                    //     sprintf(name,"%s",argz.get(5)->arg);
                    //     // strcpy(val, argz.get(2)->arg); strcat(val, "\0");
                    //     sprintf(val,"%s",argz.get(2)->arg);
                    //     type = atoi(val);
                    //     // strcpy(val, argz.get(3)->arg); strcat(val, "\0");
                    //     sprintf(val,"%s",argz.get(3)->arg);
                    //     position = atoi(val);
                    //     // strcpy(val, argz.get(4)->arg); strcat(val, "\0");
                    //     sprintf(val,"%s",argz.get(4)->arg);
                    //     indexCount = atoi(val);
                    // }
                    // stripLeadAndTrailQuotes(cleanName, name);
                    // turntables.get(i)->setTurntableName(cleanName);
                    // turntables.get(i)->setTurntableType(type);
                    // turntables.get(i)->setTurntableCurrentPosition(position);
                    // turntables.get(i)->setTurntableIndexCount(indexCount);
                    turntables.get(i)->setTurntableType((TurntableType)DCCEXInbound::getNumber(2));
                    turntables.get(i)->setTurntableCurrentPosition(DCCEXInbound::getNumber(3));
                    turntables.get(i)->setTurntableIndexCount(DCCEXInbound::getNumber(4));
                    turntables.get(i)->setTurntableName(DCCEXInbound::getSafeText(5));
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
        // if (argz.size() > 3) {  // server did not find the index
        if (DCCEXInbound::getParameterCount()==5) {
            //find the Turntable entry to update
            if (turntables.size()>0) { 
                // char val[MAX_OBJECT_NAME_LENGTH];
                // char name[MAX_OBJECT_NAME_LENGTH];
                // char cleanName[MAX_OBJECT_NAME_LENGTH];

                for (int i=0; i<turntables.size(); i++) {
                    // strcpy(val, argz.get(1)->arg); strcat(val, "\0");
                    // sprintf(val,"%s",argz.get(1)->arg);
                    // int id = atoi(val);
                    int id = DCCEXInbound::getNumber(i);

                    if (turntables.get(i)->getTurntableId()==id) {
                        //this assumes we are always starting from scratch, not updating indexes
                        // strcpy(val, argz.get(2)->arg); strcat(val, "\0");
                        // sprintf(val,"%s",argz.get(2)->arg);
                        // int index = atoi(val);
                        // // strcpy(val, argz.get(3)->arg); strcat(val, "\0");
                        // sprintf(val,"%s",argz.get(3)->arg);
                        // int angle = atoi(val);
                        // // strcpy(name, argz.get(4)->arg);
                        // sprintf(name,"%s",argz.get(4)->arg);

                        // stripLeadAndTrailQuotes(cleanName, name);
                        // turntables.get(i)->turntableIndexes.add(new TurntableIndex(index, cleanName, angle));
                        turntables.get(i)->turntableIndexes.add(new TurntableIndex(DCCEXInbound::getNumber(2),
                                                                                    DCCEXInbound::getSafeText(4),
                                                                                    DCCEXInbound::getNumber(3)));
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
void DCCEXProtocol::processTurntableAction() { // <I id position moving>
    // console->println(F("processTurntableAction(): "));
    if (delegate) {
        // char val[MAX_OBJECT_NAME_LENGTH];
        // strcpy(val, argz.get(1)->arg); strcat(val, "\0");
        // sprintf(val,"%s",argz.get(1)->arg);
        // int id = atoi(val);
        // strcpy(val, argz.get(2)->arg); strcat(val, "\0");
        // sprintf(val,"%s",argz.get(2)->arg);
        // int newPos = atoi(val);
        // TurntableState state = argz.get(3)->arg[0];
        int id = DCCEXInbound::getNumber(0);
        int newPos = DCCEXInbound::getNumber(1);
        TurntableState state = (TurntableState)DCCEXInbound::getNumber(2);

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
        // char val[MAX_OBJECT_NAME_LENGTH];
        // strcpy(val, argz.get(1)->arg); strcat(val, "\0");
        // sprintf(val,"%s",argz.get(1)->arg);
        // int address = atoi(val);
        int address = DCCEXInbound::getNumber(0);
        // strcpy(val, argz.get(3)->arg); strcat(val, "\0");
        // sprintf(val,"%s",argz.get(3)->arg);
        // int speedByte = atoi(val);
        int speedByte = DCCEXInbound::getNumber(2);
        // strcpy(val, argz.get(4)->arg); strcat(val, "\0");
        // sprintf(val,"%s",argz.get(4)->arg);
        // int functMap = atoi(val);
        int functMap = DCCEXInbound::getNumber(3);

        int throttleNo = findThrottleWithLoco(address);
        if (throttleNo>=0) {
            int rslt = throttleConsists[throttleNo].consistGetLocoPosition(address);
            if (rslt==0) {  // ignore everything that is not the lead loco
                int speed = getSpeedFromSpeedByte(speedByte);
                Direction dir = getDirectionFromSpeedByte(speedByte);
                FunctionState fnStates[MAX_FUNCTIONS];
                getFunctionStatesFromFunctionMap(fnStates, functMap);
                for (uint i=0; i<MAX_FUNCTIONS; i++) {
                    // console->print(F("processLocoAction(): checking function: ")); console->println(i);
                    if (fnStates[i] != throttleConsists[throttleNo].consistGetLocoAtPosition(0)->locoFunctions.getFunctionState(i)) {

                        throttleConsists[throttleNo].consistGetLocoAtPosition(0)->locoFunctions.setFunctionState(i, fnStates[i]);

                        // console->println(F("processLocoAction(): "));
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
            // console->println(F("processLocoAction(): unknown loco"));
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
        sprintf(outboundCommand, "<s>");
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
        sprintf(outboundCommand, "<F %d %d %c>", address, functionNumber, pressed);
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
bool DCCEXProtocol::getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired) {

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
  return true;
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
    // console->println(F("findThrottleWithLoco()"));
    for (uint i=0; i<MAX_THROTTLES; i++) {
        if (throttleConsists[i].consistGetNumberOfLocos()>0) {
            int pos = throttleConsists[i].consistGetLocoPosition(address);

            // console->print(F("checking consist: ")); console->print(i); console->print(" found: "); console->println(pos);
            // console->print(F("in consist: ")); console->println(throttleConsists[i].consistGetNumberOfLocos()); 

            // for (int j=0; j<throttleConsists[i].consistGetNumberOfLocos(); j++ ) {
            //      console->print(F("checking consist X: ")); console->print(j); console->print(" is: "); console->println(throttleConsists[i].consistLocos.get(i)->getLocoAddress());
            // }    

            if (pos>=0) {
                // console->println(F("findThrottleWithLoco(): end. found"));
                return i;
            }
        }
    }
    // console->println(F("findThrottleWithLoco(): end. not found"));
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

bool DCCEXProtocol::splitFunctions(char *functionNames) {
  // Importtant note: 
  // The functionNames string is modified in place. 
  console->print(F("Splitting \""));
  console->print(functionNames);
  console->println(F("\""));
  char * t=functionNames;
  int fkey=0;
  
  while(*t) {
       bool momentary=false;
       if(*t=='*')  {
        momentary=true;
        t++;
       }
       char * fName=t;  // function name starts here
       while(*t) { // loop completes at end of name ('/' or 0)
        if (*t=='/') {
          // found end of name
          *t='\0'; // mark name ends here 
          t++;
          break;
        }
        t++;
       }

       // At this point we have a function key
       // int fkey = function number 0....
       // bool momentary = is it a momentary
       // fName = pointer to the function name 
       console->print("Function ");
       console->print(fkey);
       console->print(momentary ? F("  Momentary ") : F(""));
       console->print(" ");
       console->println(fName);
       fkey++;
  }
  return true;
}

/* -- OLD SPLITFUNCTIONS
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
-- OLD SPLITFUNCTIONS */

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
        // strcpy(dynName, argValue);
        sprintf(dynName,"%s",argValue);

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
        // strcpy(dynName, argValue);
        sprintf(dynName,"%s",argValue);

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
        // strcpy(dynName, label);
        sprintf(dynName,"%s",label);
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
        // strcpy(dynName, label);
        sprintf(dynName,"%s",label);

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
        // strcpy(dynName, name);
        sprintf(dynName,"%s",name);
        locoName = dynName;

        for (uint i=0; i<MAX_FUNCTIONS; i++) {
            // strcpy(fnName, NAME_BLANK);
            *fnName = 0;  // blank
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
        // strcpy(dynName, name);
        sprintf(dynName,"%s",name);

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
        // strcpy(_name, name);
        sprintf(_name,"%s",name);
        consistName = _name;
    }
    bool Consist::consistAddLoco(Loco loco, Facing facing) {
        int address = loco.getLocoAddress();
        char name[MAX_SINGLE_COMMAND_PARAM_LENGTH];
        // strcpy(name, loco.getLocoName());
        sprintf(name,"%s",loco.getLocoName());
        LocoSource source = loco.getLocoSource();
        Facing correctedFacing = facing;
        int rslt = consistGetLocoPosition(address);
        if (rslt<0) { // not already in the list, so add it
            if (consistGetNumberOfLocos() == 0) correctedFacing = FacingForward; // first loco in consist is always forward
            consistLocos.add(new ConsistLoco(address, name, source, correctedFacing));

            //fix the name of the consist
            char _consistName[MAX_SINGLE_COMMAND_PARAM_LENGTH];
            // strcpy(_consistName, consistLocos.get(0)->getLocoName());
             sprintf(_consistName,"%s",consistLocos.get(0)->getLocoName());
            if (rslt>1) { // must be more than one now
                for (int i=1; i<consistLocos.size(); i++) {
                    // strcat(_consistName, ", ");
                    // strcat(_consistName, consistLocos.get(i)->getLocoName());
                    sprintf(_consistName,", %s",consistLocos.get(i)->getLocoName());
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
            // strcpy(_consistName, "\0");
            *_consistName = 0;
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
        // strcpy(_name, name);
        sprintf(_name,"%s",name);
        consistName = _name;
        return true;
    }
    char* Consist::getConsistName() {
        return consistName;
    }

// class Turnout

    Turnout::Turnout(int id, char* name, TurnoutStates state) {
        turnoutId = id;
        turnoutState = state;
        hasReceivedDetail = false;

        char *dynName;
        dynName = (char *) malloc(strlen(name)+1);
        // strcpy(dynName, name);
        sprintf(dynName,"%s",name);
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
    bool Turnout::setTurnoutState(TurnoutStates action) {
        TurnoutStates newState = action;
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
        // strcpy(dynName, name);
        sprintf(dynName,"%s",name);

        turnoutName = dynName;
        return true;
    }
    char* Turnout::getTurnoutName() {
        return turnoutName;
    }
    TurnoutStates Turnout::getTurnoutState() {
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
        // strcpy(dynName, name);
        sprintf(dynName,"%s",name);
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
        // strcpy(dynName, name);
        sprintf(dynName,"%s",name);

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
        return (RouteType)routeType;
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
        // strcpy(dynName, name);
        sprintf(dynName,"%s",name);

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
        // strcpy(dynName, name);
        sprintf(dynName,"%s",name);

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
        // strcpy(dynName, name);
        sprintf(dynName,"%s",name);

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