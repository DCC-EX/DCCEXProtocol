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
    int sLen = s.length();

    if (delegate) {
        if (len > 3 && s.charAt(1)=='i') {
            serverVersion = args[1];
            serverMicroprocessorType = args[2];
            serverMotorcontrollerType = args [3];
            serverBuildNumber = args[4];
        } 
    } else if (len > 3 && s.charAt(1)=='i') {
    }

    console->println("processCommand() end");
    return false;
}

void DCCEXProtocol::processUnknownCommand(const String& unknownCommand) {
    console->println("processUnknownCommand()");
    if (delegate) {
    }
    console->println("processUnknownCommand() end");
}

bool DCCEXProtocol::processLocoAction(char *c, int len) {
    console->println("processLocoAction()");
    if (delegate) {
    }
    console->println("processLocoAction() end");
}


void DCCEXProtocol::processServerDescription(char *c, int len) {
    if (delegate && len > 0) {
        String serverDescription = String(c);
        delegate->receivedServerDescription(serverDescription);
    }
}


void DCCEXProtocol::processRosterList(char *c, int len) {
    console->println("processRosterList()");
    if (delegate) {
    }
    console->println("processRosterList(): end");
}


void DCCEXProtocol::processTurnoutList(char *c, int len) {
    console->println("processTurnoutList()");
    if (delegate) {
    }
    console->println("processTurnoutList(): end");
}


void DCCEXProtocol::processRouteList(char *c, int len) {
    console->println("processRouteList()");
    if (delegate) {
    }
    console->println("processRouteList(): end");
}


bool DCCEXProtocol::pauseRoutes() {
    console->println("pauseRoutes()");
    if (delegate) {
    }
    console->println("pauseRoutes() end");
}

bool DCCEXProtocol::resumeRoutes() {
    console->println("resumeRoutes()");
    if (delegate) {
    }
    console->println("resumeRoutes() end");
}

void DCCEXProtocol::processTrackPower(char *c, int len) {
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


void DCCEXProtocol::processTurnoutAction(char *c, int len) {
    console->println("processTurnoutAction(): ");
    if (delegate) {

    }
    console->println("processTurnoutAction(): end");
}


void DCCEXProtocol::processRouteAction(char *c, int len) {
    console->println("processRouteAction(): ");
    if (delegate) {

    }
    console->println("processRouteAction(): end");
}


void DCCEXProtocol::processTurntableAction(char *c, int len) {
    console->println("processTurntableAction(): ");
    if (delegate) {

    }
    console->println("processTurntableAction(): end");
}


// ******************************************************************************************************

bool DCCEXProtocol::addLocomotive(int throttle, int address) {
    console->print("addLocomotive(): "); console->print(throttle); console->print(" : "); console->println(address);
    bool ok = false;
    return ok;
}

// ******************************************************************************************************

bool DCCEXProtocol::releaseLocomotive(int throttle, String address) {
    console->print("releaseLocomotive(): "); console->print(throttle); console->print(" : "); console->println(address);
    return true;
}

// ******************************************************************************************************

int DCCEXProtocol::getLocomotiveAtPosition(int throttle, int position) {
    console->print("getLocomotiveAtPosition(): "); console->print(throttle); console->print(" : "); console->println(position);
    return {};
}

// ******************************************************************************************************

int DCCEXProtocol::getNumberOfLocos(int throttle) {
    console->print("getNumberOfLocos(): "); console->println(throttle);
    int size = 0;
    console->print("getNumberOfLocomotives(): end "); console->println(size);
    return size;
}

// ******************************************************************************************************

bool DCCEXProtocol::setSpeed(int throttle, int speed) {
    console->print("setSpeed(): "); console->print(throttle); console->print(" : "); console->println(speed);
    console->println("setSpeed(): end"); 
    return true;
}

// ******************************************************************************************************

void DCCEXProtocol::requestLocoUpdate(int address) {

}

int DCCEXProtocol::setLoco(int address, int speed, Direction direction) {
    console->print("setLoco(): "); console->println(address);
    console->println("setLoco(): end");
}


// ******************************************************************************************************

void DCCEXProtocol::emergencyStop() {
    emergencyStop(DEFAULT_MULTITHROTTLE, ALL_LOCOS_ON_THROTTLE);
}


// ******************************************************************************************************

void DCCEXProtocol::setFunction(int throttle, String address, int funcNum, bool pressed) {
    console->print("setFunction(): "); console->print(multiThrottle); console->print(" : "); console->println(funcNum);

    sendCommand(cmd);

    console->println("setFunction(): end"); 
}

// ******************************************************************************************************

void DCCEXProtocol::setTrackPower(TrackPower state) {

    String cmd = "PPA";
    cmd.concat(state);

    sendCommand(cmd);	
}

bool DCCEXProtocol::setTurnout(int turnoutId, TurnoutAction action) {  // address is turnout system name
    sendCommand(cmd);

    return true;
}

bool DCCEXProtocol::setRoute(int routeId) {  // address is turnout system name
    String cmd = "";
    sendCommand(cmd);
    return true;
}

long DCCEXProtocol::getLastServerResponseTime() {
  return lastServerResponseTime;   
}


bool DCCEXProtocol::setTurntable(int TurntableId, int position, int activity) {

}

bool DCCEXProtocol::setAccessory(int accessoryAddress, int activate) {

}

bool DCCEXProtocol::setAccessory(int accessoryAddress, int accessorySubAddr, int activate) {

}


void DCCEXProtocol::processRosterEntry(char *c, int len) {

}

void DCCEXProtocol::processTurnoutEntry(char *c, int len) {

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


class DCCEXProtocol::Functions {
    public:
        String functionLabel[28];
        int functionState[28];
        int functionLatching[28];
        int functionState[28];

    private:
}

class DCCEXProtocol::Loco {
    public:
        int locoAddress;
        String locoName;
        int locoSpeed;
        Direction locoDirection;
        Functions locoFunctions;
        LocoSource locoSource;

    private:

}

class DCCEXProtocol::Consist {
    public:
        Loco consistLocos[10];
        int consistLocosFacing[10];
        int consistSpeed;
        Direction consistDirection;

        bool consistAddLoco(int locoAddress) {

        }

        bool consistReleaseLoco() {

        }
        
        bool consistReleaseLoco(int locoAddress) {

        }
        
        bool consistGetNumberOfLocos() {

        }
        
        bool consistGetLocoAtPosition(int position) {

        }
        
        bool consistGetLocoPosition(int locoAddress) {

        }
        

        bool consistSetSpeed(int speed) {

        }
        
        int consistGetSpeed() {

        }
        
        bool consistSetDirection(Direction direction) {

        }
        
        bool consistSetFunction(int functionNo, FunctionState functionState) {

        }
        
        bool consistSetFunction(int address, int functionNo, FunctionState functionState) {

        }
        

    private:

}

class DCCEXProtocol::Turnout {
    int turnoutId;
    String turnoutName;
    TurnoutState turnoutState;
}

class DCCEXProtocol::Route {
    int routeId;
    String routeName;
}


class DCCEXProtocol::TurntableIndex {
    int turntableIndexId;
    String turntableIndexName;
    int turntableValue;
    int turntableAngle;
}

class DCCEXProtocol::Turntable {
    int turntableId;
    String turntableName;
    int turntablePosition;
    TurntableIndex turntableIndexes[MAX_TURNTABLE_INDEXES];
    bool turntableIsMoving;

    int turntableGetCurrentIndex() {

    }
}