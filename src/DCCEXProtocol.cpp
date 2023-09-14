/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCC-EX protocol connection,
 * allow a device to communicate with a DCC-EX EX CommnadStation
 *
 * Copyright © 2018-2019 Blue Knobby Systems Inc.
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
#include <vector>  //https://github.com/arduino-libraries/Arduino_AVRSTL

#include "DCCEXProtocol.h"

static const int MIN_SPEED = 0;
static const int MAX_SPEED = 126;
static const char *rosterSegmentDesc[] = {"Name", "Address", "Length"};


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
	
	// init heartbeat
	// heartbeatTimer = millis();
    // heartbeatPeriod = 0;
	
	// init fasttime
	fastTimeTimer = millis();
    currentFastTime = 0.0;
    currentFastTimeRate = 0.0;
    
	// init global variables
    for (int multiThrottleIndex=0; multiThrottleIndex<6; multiThrottleIndex++) {
        locomotiveSelected[multiThrottleIndex] = false;
        currentSpeed[multiThrottleIndex] = 0;
        speedSteps[multiThrottleIndex] = 0;
        currentDirection[multiThrottleIndex] = Forward;
        locomotives[multiThrottleIndex].resize(0);
        locomotivesFacing[multiThrottleIndex].resize(0);
    }

    //last Response time
    lastServerResponseTime = millis() /1000;
	
	// init change flags
    resetChangeFlags();

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


void DCCEXProtocol::resetChangeFlags() {
    clockChanged = false;
    heartbeatChanged = false;
}

void DCCEXProtocol::connect(Stream *stream) {
    init();
    this->stream = stream;
}

void DCCEXProtocol::disconnect() {
    String command = "Q";
    sendCommand(command);
    this->stream = NULL;
}

void DCCEXProtocol::setDeviceName(String deviceName) {
    currentDeviceName = deviceName;
    String command = "N" + deviceName;
    sendCommand(command);
}

bool DCCEXProtocol::check() {
    bool changed = false;
    resetChangeFlags();

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


bool DCCEXProtocol::processLocomotiveAction(char multiThrottle, char *c, int len) {
    int multiThrottleIndex = getMultiThrottleIndex(multiThrottle);
    String remainder(c);  // the leading "MTA" was not passed to this method

    console->printf("processLocomotiveAction: remainder at first is %s\n", remainder.c_str());

    if (currentAddress[multiThrottleIndex].equals("")) {
        console->printf("  skipping due to no selected address\n");
        return true;
    }
    else {
        console->printf("  currentAddress is '%s'\n", currentAddress[multiThrottleIndex].c_str());
    }

    String addrCheck = currentAddress[multiThrottleIndex] + PROPERTY_SEPARATOR;
    String allCheck = "*";
    allCheck.concat(PROPERTY_SEPARATOR);
    if (remainder.startsWith(addrCheck)) {
        remainder.remove(0, addrCheck.length());
    }
    else if (remainder.startsWith(allCheck)) {
        remainder.remove(0, allCheck.length());
    }

    console->printf("processLocomotiveAction: after separator is %s\n", remainder.c_str());

    if (remainder.length() > 0) {
        char action = remainder[0];

        switch (action) {
            case 'F':
                //console->printf("processing function state\n");
                processFunctionState(multiThrottle, remainder);
                break;
            case 'V':
                processSpeed(multiThrottle, remainder);
                break;
            case 's':
                processSpeedSteps(multiThrottle, remainder);
                break;
            case 'R':
                processDirection(multiThrottle, remainder);
                break;
            default:
                console->printf("unrecognized action '%c'\n", action);
                // no processing on unrecognized actions
                break;
        }
        return true;
    }
    else {
        console->printf("insufficient action to process\n");
        return false;
    }
}

bool DCCEXProtocol::processRosterFunctionList(char multiThrottle, char *c, int len) {
    int multiThrottleIndex = getMultiThrottleIndex(multiThrottle);
    String remainder(c);  // the leading "MTL" was not passed to this method

    console->printf("processRosterFunctionList: remainder at first is %s\n", remainder.c_str());

    if (currentAddress[multiThrottleIndex].equals("")) {
        console->printf("  skipping due to no selected address\n");
        return true;
    }
    else {
        console->printf("  currentAddress is '%s'\n", currentAddress[multiThrottleIndex].c_str());
    }

    String addrCheck = currentAddress[multiThrottleIndex] + PROPERTY_SEPARATOR;
    String allCheck = "*";
    allCheck.concat(PROPERTY_SEPARATOR);
    if (remainder.startsWith(addrCheck)) {
        remainder.remove(0, addrCheck.length());
    }
    else if (remainder.startsWith(allCheck)) {
        remainder.remove(0, allCheck.length());
    }

    console->printf("processRosterFunctionList: after separator is %s\n", remainder.c_str());

    if (remainder.length() > 0) {
        char action = remainder[0];

        if (action == ']') {
            processRosterFunctionListEntries(multiThrottle, remainder);
        } else {
            console->printf("unrecognized L action '%c'\n", action);
            // no processing on unrecognized actions
        }
        return true;
    }
    else {
        console->printf("insufficient action to process\n");
        return false;
    }
}

bool DCCEXProtocol::processCommand(char *c, int len) {
    bool changed = false;

    console->print("<== ");
    console->println(c);

    lastServerResponseTime = millis()/1000;

    // we regularly get this string as part of the data sent
    // by a Digitrax LnWi.  Remove it, and try again.
    const char *ignoreThisGarbage = "AT+CIPSENDBUF=";
    while (strncmp(c, ignoreThisGarbage, strlen(ignoreThisGarbage)) == 0) {
        console->printf("removed one instance of %s\n", ignoreThisGarbage);
        c += strlen(ignoreThisGarbage);
        changed = true;
    }

    if (changed) {
        console->printf("input string is now: '%s'\n", c);
    }

    if (len > 3 && c[0]=='P' && c[1]=='F' && c[2]=='T') {
        return processFastTime(c+3, len-3);
    }
    else if (len > 3 && c[0]=='P' && c[1]=='P' && c[2]=='A') {
        processTrackPower(c+3, len-3);
        return true;
    }
    else if (len > 1 && c[0]=='*') {
        return processHeartbeat(c+1, len-1);
    }
    else if (len > 2 && c[0]=='V' && c[1]=='N') {
        processProtocolVersion(c+2, len-2);
        return true;
    }
    else if (len > 2 && c[0]=='H' && c[1]=='T') {
        processServerType(c+2, len-2);
        return true;
    }
    else if (len > 2 && c[0]=='H' && c[1]=='t') {
        processServerDescription(c+2, len-2);
        return true;
    }	
    else if (len > 2 && c[0]=='P' && c[1]=='W') {
        processWebPort(c+2, len-2);
        return true;
    }
    else if (len > 2 && c[0]=='R' && c[1]=='L') {
        processRosterList(c+2, len-2);
        return true;
    }	
    else if (len > 3 && c[0]=='P' && c[1]=='T' && c[2]=='L') {
        processTurnoutList(c+2, len-2);
        return true;
    }	
    else if (len > 3 && c[0]=='P' && c[1]=='R' && c[2]=='L') {
        processRouteList(c+2, len-2);
        return true;
    }	
    else if (len > 6 && c[0]=='M' && c[2]=='S') {
        processStealNeeded(c[1], c+3, len-3);
        return true;
    }
    else if (len > 6 && c[0]=='M' && (c[2]=='+' || c[2]=='-')) {
        // we want to make sure the + or - is passed in as part of the string to process
        processAddRemove(c[1], c+2, len-2);
        return true;
    }
    else if (len > 8 && c[0]=='M' && c[2]=='A') {
        return processLocomotiveAction(c[1], c+3, len-3);
    }
    else if (len > 8 && c[0]=='M' && c[2]=='L') {
        return processRosterFunctionList(c[1], c+3, len-3);
    }
    else if (len > 5 && c[0]=='P' && c[1]=='T' && c[2]=='A') {
        processTurnoutAction(c+3, len-3);
        return true;
    }
    else if (len > 4 && c[0]=='P' && c[1]=='R' && c[2]=='A') {
        processRouteAction(c+3, len-3);
        return true;
    }
    else if (len > 3 && c[0]=='A' && c[1]=='T' && c[2]=='+') {
        // this is an AT+.... command that the LnWi sometimes emits and we
        // ignore these commands altogether
    }
    else {
        console->printf("unknown command '%s'\n", c);
        // all other commands are explicitly ignored
    }
    return false;
}


void DCCEXProtocol::setCurrentFastTime(const String& s) {
    int t = s.toInt();
    if (currentFastTime == 0.0) {
        console->print("set fast time to "); console->println(t);
    }
    else {
        console->print("updating fast time (should be "); console->print(t);
        console->print(" is "); console->print(currentFastTime);  console->println(")");
        console->printf("currentTime is %ld\n", millis());
    }
    currentFastTime = t;
}


void DCCEXProtocol::processServerDescription(char *c, int len) {
	
    if (delegate && len > 0) {
        String serverDescription = String(c);
        delegate->receivedServerDescription(serverDescription);
    }
}


void DCCEXProtocol::processRosterList(char *c, int len) {
    console->println("processRosterList()");

	String s(c);

	// get the number of entries
    int indexSeperatorPosition = s.indexOf(ENTRY_SEPARATOR,1);
	int entries = s.substring(0, indexSeperatorPosition).toInt();
	console->print("Entries in roster: "); console->println(entries);
	
	// if set, call the delegate method
	if (delegate) delegate->receivedRosterEntries(entries);	
	
	// loop
	int entryStartPosition = 4; //ignore the first entry separator
	for(int i = 0; i < entries; i++) {
	
		// get element
		int entrySeparatorPosition = s.indexOf(ENTRY_SEPARATOR, entryStartPosition);
		String entry = s.substring(entryStartPosition, entrySeparatorPosition);
		console->print("Roster Entry: "); console->println(i + 1);
		
		// split element in segments and parse them		
		String name;
		int address = 0;
		char length = 0;
		int segmentStartPosition = 0;
		for(int j = 0; j < 3; j++) {
		
			// get segment
			int segmentSeparatorPosition = entry.indexOf(SEGMENT_SEPARATOR, segmentStartPosition);
			String segment = entry.substring(segmentStartPosition, segmentSeparatorPosition);
			console->print(rosterSegmentDesc[j]); console->print(": "); console->println(segment);
			segmentStartPosition = segmentSeparatorPosition + 3;
			
			// parse the segments
			if(j == 0) name = segment;
			else if(j == 1) address = segment.toInt();
			else if(j == 2) length = segment[0];
		}
		
		// if set, call the delegate method
		if(delegate) delegate->receivedRosterEntry(i, name, address, length);
		
		entryStartPosition = entrySeparatorPosition + 3;
	}

    console->println("processRosterList(): end");
}


void DCCEXProtocol::processTurnoutList(char *c, int len) {
    console->println("processTurnoutList()");

  	String s(c);

    // loop
    int entries = -1;
    boolean entryFound = true;
	int entryStartPosition = 4; //ignore the first entry separator
    if (sizeof(c) <= 3) entryFound =false;

    while (entryFound) {
	    entries++;

		// get element
		int entrySeparatorPosition = s.indexOf(ENTRY_SEPARATOR, entryStartPosition);
        if (entrySeparatorPosition==-1) entrySeparatorPosition = s.length();
		String entry = s.substring(entryStartPosition, entrySeparatorPosition);
		console->print("Turnout Entry: "); console->println(entries + 1);
		
		// split element in segments and parse them		
		String sysName;
		String userName;
        int state = 0;
		int segmentStartPosition = 0;
		for(int j = 0; j < 3; j++) {
		
			// get segment
			int segmentSeparatorPosition = entry.indexOf(SEGMENT_SEPARATOR, segmentStartPosition);
			String segment = entry.substring(segmentStartPosition, segmentSeparatorPosition);
			console->print(rosterSegmentDesc[j]); console->print(": "); console->println(segment);
			segmentStartPosition = segmentSeparatorPosition + 3;
			
			// parse the segments
			if(j == 0) sysName = segment;
			else if(j == 1) userName = segment;
			else if(j == 2) state = segment.toInt();
		}
		
		// if set, call the delegate method
		if(delegate) delegate->receivedTurnoutEntry(entries, sysName, userName, state);
		
		entryStartPosition = entrySeparatorPosition + 3;
        console->println(entryStartPosition);
        console->println(s.length());
        if (entryStartPosition >= s.length()) entryFound = false;
	}

	// get the number of entries
	console->print("Entries in Turnouts List: "); console->println(entries+1);
	// if set, call the delegate method
	if (delegate) delegate->receivedTurnoutEntries(entries+1);	

    console->println("processTurnoutList(): end");
}


void DCCEXProtocol::processRouteList(char *c, int len) {
    console->println("processRouteList()");
  	String s(c);

    // loop
    int entries = -1;
    boolean entryFound = true;
	int entryStartPosition = 4; //ignore the first entry separator
    if (sizeof(c) <= 3) entryFound =false;

    while (entryFound) {
	    entries++;

		// get element
		int entrySeparatorPosition = s.indexOf(ENTRY_SEPARATOR, entryStartPosition);
        if (entrySeparatorPosition==-1) entrySeparatorPosition = s.length();
		String entry = s.substring(entryStartPosition, entrySeparatorPosition);
		console->print("Route Entry: "); console->println(entries + 1);
		
		// split element in segments and parse them		
		String sysName;
		String userName;
        int state = 0;
		int segmentStartPosition = 0;
		for(int j = 0; j < 3; j++) {
		
			// get segment
			int segmentSeparatorPosition = entry.indexOf(SEGMENT_SEPARATOR, segmentStartPosition);
			String segment = entry.substring(segmentStartPosition, segmentSeparatorPosition);
			console->print(rosterSegmentDesc[j]); console->print(": "); console->println(segment);
			segmentStartPosition = segmentSeparatorPosition + 3;
			
			// parse the segments
			if(j == 0) sysName = segment;
			else if(j == 1) userName = segment;
			else if(j == 2) state = segment.toInt();
		}
		
		// if set, call the delegate method
		if(delegate) delegate->receivedRouteEntry(entries, sysName, userName, state);
		
		entryStartPosition = entrySeparatorPosition + 3;
        console->println(entryStartPosition);
        console->println(s.length());
        if (entryStartPosition >= s.length()) entryFound = false;
	}

	// get the number of entries
	console->print("Entries in Turnouts List: "); console->println(entries+1);
	// if set, call the delegate method
	if (delegate) delegate->receivedRouteEntries(entries+1);	

    console->println("processRouteList(): end");
}

// supported multiThrottle codes are 'T' '0' '1' '2' '3' '4' '5' only.
int 
DCCEXProtocol::getMultiThrottleIndex(char multiThrottle) {
    console->print("getMultiThrottleIndex(): "); console->println(multiThrottle);
    int mThrottle = multiThrottle - '0';


    if ((mThrottle >= 0) && (mThrottle<=5)) {
        return mThrottle;
    } else {
        return 0;
    }
}


// the string passed in will look 'F03' (meaning turn off Function 3) or
// 'F112' (turn on function 12)
void
DCCEXProtocol::processFunctionState(char multiThrottle, const String& functionData) {
    console->print("processFunctionState(): "); console->println(multiThrottle);

    // F[0|1]nn - where nn is 0-28
    if (delegate && functionData.length() >= 3) {
        bool state = functionData[1]=='1' ? true : false;

        String funcNumStr = functionData.substring(2);
        uint8_t funcNum = funcNumStr.toInt();

        if (funcNum == 0 && funcNumStr != "0") {
            // error in parsing
        }
        else {
            if (multiThrottle == DEFAULT_MULTITHROTTLE) {
                delegate->receivedFunctionState(funcNum, state);
            } else {
                delegate->receivedFunctionStateMultiThrottle(multiThrottle,funcNum, state);
            }
        }
    }
    console->println("processFunctionState(): end");
}


// the string passed in will look ']\[Headlight]\[Bell]\[Whistle]\[Short Whistle]\[Steam Release]\[FX5 Light]\[FX6 Light]\[Dimmer]\[Mute]\[Water Stop]\[Injectors]\[Brake Squeal]\[Coupler]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\['
void
DCCEXProtocol::processRosterFunctionListEntries(char multiThrottle, const String& s) {
    console->print("processRosterFunctionListEntries(): "); console->println(multiThrottle);

    String functions[28];

    // loop
    int entries = -1;
    boolean entryFound = true;
	int entryStartPosition = 3; //ignore the first entry separator
    if (s.length() <= 3) entryFound =false;

    while ((entryFound) && (entries < 28)) {
	    entries++;

		// get element
		int entrySeparatorPosition = s.indexOf(ENTRY_SEPARATOR, entryStartPosition);
        if (entrySeparatorPosition == -1) entrySeparatorPosition = s.length();
		String entry = s.substring(entryStartPosition, entrySeparatorPosition);
        functions[entries] = entry;
		console->print("Function Entry: "); console->print(entries); console->print(" - "); console->println(entry);
        
        entryStartPosition = entrySeparatorPosition + 3;
    }

    console->print("Functions for roster entry: "); console->println(entries);

    for(int i = entries+1; i < 28; i++) {
        functions[i] = "";
    } 

    if (multiThrottle == DEFAULT_MULTITHROTTLE) {
        delegate->receivedRosterFunctionList(functions);
    } else {
        delegate->receivedRosterFunctionListMultiThrottle(multiThrottle, functions);
    }

    console->println("processRosterFunctionListEntries(): end");
}


void
DCCEXProtocol::processSpeed(char multiThrottle, const String& speedData) {
    console->print("processSpeed(): "); console->println(multiThrottle);

    if (delegate && speedData.length() >= 2) {
        String speedStr = speedData.substring(1);
        int speed = speedStr.toInt();

        if ((speed < MIN_SPEED) || (speed > MAX_SPEED)) {
            speed = 0;
        }

        if (multiThrottle == DEFAULT_MULTITHROTTLE) {
            delegate->receivedSpeed(speed);
        } else {
            delegate->receivedSpeedMultiThrottle(multiThrottle, speed);
        }
    }

    console->println("processSpeed(): end");
}


void
DCCEXProtocol::processDirection(char multiThrottle, const String& directionStr) {
    console->print("processDirection(): "); console->println(multiThrottle);

    int multiThrottleIndex = getMultiThrottleIndex(multiThrottle);
    console->print("DIRECTION STRING: ");
    console->println(directionStr);
    console->print("LENGTH: ");
    console->println(directionStr.length());


    // R[0|1]
    if (delegate && directionStr.length() == 2) {
        if (directionStr.charAt(1) == '0') {
            currentDirection[multiThrottleIndex] = Reverse;
        }
        else {
            currentDirection[multiThrottleIndex] = Forward;
        }

        if (multiThrottle == DEFAULT_MULTITHROTTLE) {
            delegate->receivedDirection(currentDirection[multiThrottleIndex]);
        } else {
            delegate->receivedDirectionMultiThrottle(multiThrottle, currentDirection[multiThrottleIndex]);
        }
    }

    console->println("processDirection(): end"); 
}



void
DCCEXProtocol::processTrackPower(char *c, int len) {
    console->println("processTrackPower()");

    if (delegate) {
        if (len > 0) {
            TrackPower state = PowerUnknown;
            if (c[0]=='0') {
                state = PowerOff;
            }
            else if (c[0]=='1') {
                state = PowerOn;
            }

            delegate->receivedTrackPower(state);
        }
    }
}


void
DCCEXProtocol::processAddRemove(char multiThrottle, char *c, int len) {
    console->print("processAddRemove(): "); console->println(multiThrottle);

    if (!delegate) {
        // If no one is listening, don't do the work to parse the string
        return;
    }

    //console->printf("processing add/remove command %s\n", c);

    String s(c);

    bool add = (c[0] == '+');
    bool remove = (c[0] == '-');

    int p = s.indexOf(PROPERTY_SEPARATOR);
    if (p > 0) {
        String address = s.substring(1, p);
        String entry   = s.substring(p+3);

        address.trim();
        entry.trim();

        if (add) {
            if (multiThrottle == DEFAULT_MULTITHROTTLE) {
                delegate->addressAdded(address, entry);
            } else {
                delegate->addressAddedMultiThrottle(multiThrottle, address, entry);
            }
        }
        if (remove) {
            if (entry.equals("d\n") || entry.equals("r\n")) {
                delegate->addressRemoved(address, entry);
            }
            else {
                console->printf("malformed address removal: command is %s\n", entry.c_str());
                console->printf("entry length is %d\n", entry.length());
                for (int i = 0; i < entry.length(); i++) {
                    console->printf("  char at %d is %d\n", i, entry.charAt(i));
                }
            }
        }
    }

    console->println("processAddRemove(): end"); 
}


void
DCCEXProtocol::processTurnoutAction(char *c, int len) {
    if (delegate) {
        String s(c);
        String systemName = s.substring(1,s.length()-1);
        TurnoutState state = TurnoutUnknown;
        if (c[0]=='2') {
            state = TurnoutClosed;
        }
        else if (c[0]=='4') {
            state = TurnoutThrown;
        }
        else if (c[0]=='1') {
            state = TurnoutUnknown;
        }
        else if (c[0]=='8') {
            state = TurnoutInconsistent;
        }

        delegate->receivedTurnoutAction(systemName, state);
    }
}


void
DCCEXProtocol::processRouteAction(char *c, int len) {
    if (delegate) {
        String s(c);
        String systemName = s.substring(1,s.length()-1);
        RouteState state = RouteInconsistent;
        if (c[0]=='2') {
            state = RouteActive;
        }
        else if (c[0]=='4') {
            state = RouteInactive;
        }

        delegate->receivedRouteAction(systemName, state);
    }
}


// ******************************************************************************************************

bool
DCCEXProtocol::addLocomotive(char multiThrottle, String address) {
    console->print("addLocomotive(): "); console->print(multiThrottle); console->print(" : "); console->println(address);

    int multiThrottleIndex = getMultiThrottleIndex(multiThrottle);
    bool ok = false;

    if (address[0] == 'S' || address[0] == 'L') {
        String rosterName = address; 
        String cmd = "M" + String(multiThrottle) + "+" + address + PROPERTY_SEPARATOR + rosterName;
        sendCommand(cmd);

        boolean locoAlreadyInList = false;
        for(int i=0;i<locomotives[multiThrottleIndex].size();i++) {
            if (locomotives[multiThrottleIndex][i].equals(address)) {
                locoAlreadyInList = true;
                break;
            }
        } 
        if (!locoAlreadyInList) {
            locomotives[multiThrottleIndex].push_back(address);
            currentAddress[multiThrottleIndex] = locomotives[multiThrottleIndex].front();
            locomotivesFacing[multiThrottleIndex].push_back(Forward);
            locomotiveSelected[multiThrottleIndex] = true;
        }
        ok = true;
    }

    console->print("addLocomotive(): end : ");  console->println(ok); 
    return ok;
}

// ******************************************************************************************************

bool
DCCEXProtocol::stealLocomotive(char multiThrottle, String address) {
    console->print("stealLocomotive(): "); console->print(multiThrottle); console->print(" : "); console->println(address);

    bool ok = false;

    if (releaseLocomotive(multiThrottle, address)) {
        ok = addLocomotive(multiThrottle, address);
    }

    return ok;
}

// ******************************************************************************************************

bool
DCCEXProtocol::releaseLocomotive(char multiThrottle, String address) {
    console->print("releaseLocomotive(): "); console->print(multiThrottle); console->print(" : "); console->println(address);

    int multiThrottleIndex = getMultiThrottleIndex(multiThrottle);
    // MT-*<;>r
    String cmd = "M" + String(multiThrottle) + "-";
    cmd.concat(address);
    cmd.concat(PROPERTY_SEPARATOR);
    cmd.concat("r");
    sendCommand(cmd);

    if (address.equals(ALL_LOCOS_ON_THROTTLE)) {
            locomotives[multiThrottleIndex].clear();
            locomotivesFacing[multiThrottleIndex].clear();
    } else {
        for(int i=0;i<locomotives[multiThrottleIndex].size();i++) {
            if (locomotives[multiThrottleIndex][i].equals(address)) {
                locomotives[multiThrottleIndex].erase(locomotives[multiThrottleIndex].begin()+i);
                locomotivesFacing[multiThrottleIndex].erase(locomotivesFacing[multiThrottleIndex].begin()+i);
                break;
            }
        } 
    }
    
    if (locomotives[multiThrottleIndex].size()==0) { 
        locomotiveSelected[multiThrottleIndex] = false;
        currentAddress[multiThrottleIndex] = "";
    } else {        
        currentAddress[multiThrottleIndex] = locomotives[multiThrottleIndex].front();
    }

    // console->println("releaseLocomotive(): end"); 
    return true;
}

// ******************************************************************************************************

String
DCCEXProtocol::getLeadLocomotive(char multiThrottle) {
    console->print("getLeadLocomotive(): "); console->println(multiThrottle);

    int multiThrottleIndex = getMultiThrottleIndex(multiThrottle);
    if (locomotives[multiThrottleIndex].size()>0) { 
        return locomotives[multiThrottleIndex].front();
    }
    return {};
}

// ******************************************************************************************************

String
DCCEXProtocol::getLocomotiveAtPosition(char multiThrottle, int position) {
    console->print("getLocomotiveAtPosition(): "); console->print(multiThrottle); console->print(" : "); console->println(position);

    int multiThrottleIndex = getMultiThrottleIndex(multiThrottle);
    // console->print("getLocomotiveAtPosition(): vector size: "); console->println(locomotives[multiThrottleIndex].size());
    if (locomotives[multiThrottleIndex].size()>0) { 
        // console->print("getLocomotiveAtPosition(): return: "); console->println(locomotives[multiThrottleIndex][position]);
        return locomotives[multiThrottleIndex][position];
    }
    return {};
}

// ******************************************************************************************************

int
DCCEXProtocol::getNumberOfLocomotives(char multiThrottle) {
    console->print("getNumberOfLocomotives(): "); console->println(multiThrottle);

    int multiThrottleIndex = getMultiThrottleIndex(multiThrottle);

    int size = locomotives[multiThrottleIndex].size();
    console->print("getNumberOfLocomotives(): end "); console->println(size);
    return size;
}

// ******************************************************************************************************

bool
DCCEXProtocol::setSpeed(char multiThrottle, int speed) {
    console->print("setSpeed(): "); console->print(multiThrottle); console->print(" : "); console->println(speed);

    int multiThrottleIndex = getMultiThrottleIndex(multiThrottle);
    if (speed < 0 || speed > 126) {
        return false;
    }
    if (!locomotiveSelected[multiThrottleIndex]) {
        return false;
    }

    if (speed != currentSpeed[multiThrottleIndex]) {
        String cmd = "M" + String(multiThrottle) + "A*";
        cmd.concat(PROPERTY_SEPARATOR);
        cmd.concat("V");
        cmd.concat(String(speed));
        sendCommand(cmd);
        currentSpeed[multiThrottleIndex] = speed;
    }
    return true;
}

// ******************************************************************************************************

int
DCCEXProtocol::getSpeed(char multiThrottle) {
    console->print("getSpeed(): "); console->println(multiThrottle);

    int multiThrottleIndex = getMultiThrottleIndex(multiThrottle);
    return currentSpeed[multiThrottleIndex];
}

// ******************************************************************************************************

bool
DCCEXProtocol::setDirection(char multiThrottle, Direction direction) {
    return setDirection(multiThrottle, ALL_LOCOS_ON_THROTTLE, direction);
}

bool
DCCEXProtocol::setDirection(char multiThrottle, String address, Direction direction) {
    console->print("setDirection(): "); console->print(multiThrottle); console->print(" : "); console->println(direction);

    int multiThrottleIndex = getMultiThrottleIndex(multiThrottle);
    if (!locomotiveSelected[multiThrottleIndex]) {
        return false;
    }


    String cmd = "M" + String(multiThrottle) + "A" + address;
    cmd.concat(PROPERTY_SEPARATOR);
    cmd.concat("R");
    if (direction == Reverse) {
        cmd += "0";
    }
    else {
        cmd += "1";
    }
    sendCommand(cmd);

    if (address.equals(ALL_LOCOS_ON_THROTTLE)) {
        currentDirection[multiThrottleIndex] = direction;
    } else {
        for(int i=0;i<locomotives[multiThrottleIndex].size();i++) {
            if (locomotives[multiThrottleIndex][i].equals(address)) {
                locomotivesFacing[multiThrottleIndex][i] = direction;
                break;
            }
        }
    }
    return true;
}

// ******************************************************************************************************

Direction
DCCEXProtocol::getDirection(char multiThrottle) {
    return getDirection(multiThrottle, ALL_LOCOS_ON_THROTTLE);
}

Direction
DCCEXProtocol::getDirection(char multiThrottle, String address) {
    console->print("getDirection(): "); console->println(multiThrottle);

    int multiThrottleIndex = getMultiThrottleIndex(multiThrottle);

    if (address.equals(ALL_LOCOS_ON_THROTTLE)) {
        return currentDirection[multiThrottleIndex];
    } else {
        Direction individualDirection = currentDirection[multiThrottleIndex];
        for(int i=0;i<locomotives[multiThrottleIndex].size();i++) {
            if (locomotives[multiThrottleIndex][i].equals(address)) {
                individualDirection = locomotivesFacing[multiThrottleIndex][i];
                break;
            }
        }
        return individualDirection;
    }
}

// ******************************************************************************************************

void DCCEXProtocol::emergencyStop() {
    emergencyStop(DEFAULT_MULTITHROTTLE, ALL_LOCOS_ON_THROTTLE);
}


// ******************************************************************************************************

void DCCEXProtocol::setFunction(char multiThrottle, int funcNum, bool pressed) {
    setFunction(multiThrottle, "", funcNum, pressed) ;
}

void DCCEXProtocol::setFunction(char multiThrottle, String address, int funcNum, bool pressed) {
    console->print("setFunction(): "); console->print(multiThrottle); console->print(" : "); console->println(funcNum);

    int multiThrottleIndex = getMultiThrottleIndex(multiThrottle);
    if (!locomotiveSelected[multiThrottleIndex]) {
        console->println("setFunction(): end - not selected");
        return;
    }

    if (funcNum < 0 || funcNum > 28) {
        return;
    }

    String cmd = "M" + String(multiThrottle) + "A";
    if (address.equals("")) {
        cmd.concat(currentAddress[multiThrottleIndex]);
    } else {
        cmd.concat(address);
    }

    cmd.concat(PROPERTY_SEPARATOR);
    cmd.concat("F");

    if (pressed) {
        cmd += "1";
    }
    else {
        cmd += "0";
    }
    cmd += funcNum;
    sendCommand(cmd);

    console->println("setFunction(): end"); 
}

// ******************************************************************************************************

void DCCEXProtocol::setTrackPower(TrackPower state) {

    String cmd = "PPA";
    cmd.concat(state);

    sendCommand(cmd);	
}

bool DCCEXProtocol::setTurnout(String turnoutSystemName, TurnoutAction action) {  // address is turnout system name
    String s = "T";
    if (action == TurnoutClose) {
        s = "C";
    } 
    else if (action == TurnoutToggle) {
        s = "2";
    }
    String cmd = "PTA" + s + turnoutSystemName;
    sendCommand(cmd);

    return true;
}

bool DCCEXProtocol::setRoute(String routeSystemName) {  // address is turnout system name
    String cmd = "PRA2" + routeSystemName;
    sendCommand(cmd);

    return true;
}

long DCCEXProtocol::getLastServerResponseTime() {
  return lastServerResponseTime;   
}

