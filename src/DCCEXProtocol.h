/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
 *
 * Copyright © 2023 Peter Akers
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

#ifndef DCCEXPROTOCOL_H
#define DCCEXPROTOCOL_H

static const int MAX_THROTTLES 6
static const int MAX_ROSTER 20
static const int MAX_TURNOUTS 20
static const int MAX_ROUTES 20
static const int MAX_TURNTABLES 2
static const int MAX_TURNTABLES_INDEXES 10

#include "Arduino.h"

// *****************************************************************

typedef enum Direction {
    Reverse = 0,
    Forward = 1
} Direction;

typedef enum TrackPower {
    PowerOff = 0,
    PowerOn = 1,
    PowerUnknown = 2
} TrackPower;

typedef enum TurnoutState {
    TurnoutClosed = 0,
    TurnoutThrown = 1,
    TurnoutUnknownId = 9   // "X" is sent
} TurnoutState;

typedef enum TurnoutAction {
    TurnoutClose = 0,
    TurnoutThrow = 1,
    TurnoutToggle = 2,
    TurnoutExamine = 9     // "X" needs to be sent
} TurnoutAction;

typedef enum RouteState {
    RouteActive = 2,
    RouteInactive = 4,
    RouteInconsistent = 8
} RouteState;

typedef enum TrackMode {
    TrackModeMain = "MAIN",
    TrackModeProg = "PROG",
    TrackModeDC = "DC",
    TrackModeDCX = "DCX",
    TrackModeOff = "OFF"
} TrackMode;

typedef enum TurntableState {
    TurntableMoving = 1,
    TuerntableStationary = 0
} TurntableState;

typedef enum FunctionState {
    FunctionStateOn = 1,
    FunctionStateOff = 0
} FunctionState;

typedef enum FunctionLatching {
    FunctionLatchingTrue = 1,
    FunctionLatchingFalse = 0
} FunctionLatching;

typedef enum LocoSource {
    LocoSourceRoster = 0,
    LocoSourceEntry = 1
} LocoSource;

typedef enum Facing {
    FacingForward = 0,
    FacingReversed = 1
} Facing;

// *****************************************************************

class NullStream : public Stream {
  
  public:
	NullStream() {}
	int available() { return 0; }
	void flush() {}
	int peek() { return -1; }
	int read() { return -1; }
	size_t write(uint8_t c) { return 1; }
	size_t write(const uint8_t *buffer, size_t size) { return size; }
};

class DCCEXProtocolDelegate {
  public:
  
    virtual void receivedServerDescription(String description, String verion) {}
  
    virtual void receivedRosterList(int rosterSize) {}
    virtual void receivedTurnoutList(int turnoutListSize) {}    
    virtual void receivedRouteList(int routeListSize) {}
    virtual void receivedTurntablesList(int turntablesListSize) {}    

    virtual void receivedSpeed(int throttleNo, int speed) { }
    virtual void receivedDirection(int throttleNo, Direction dir) { }
    virtual void receivedFunction(int throttleNo, uint8_t func, bool state) { }

    virtual void receivedTrackPower(TrackPower state) { }

    // virtual void receivedLocoAdded(int throttleNo, String address, String entry) { }
    // virtual void receivedLocoRemoved(int throttleNo, String address, String command) { }

    virtual void receivedTurnoutAction(String systemName, TurnoutState state) { } //  PTAturnoutstatesystemname
    virtual void receivedRouteAction(String systemName, RouteState state) { } //  PTAturnoutstatesystemname
};


class DCCEXProtocol {
  public:
    
    DCCEXProtocol(bool server = false);

    void setDelegate(DCCEXProtocolDelegate *delegate);
    void setLogStream(Stream *console);

    void connect(Stream *stream);
    void disconnect();

    bool check();

    String currentDeviceName;

    // *******************

    Consist throttle[MAX_THROTTLES];
    Loco roster[MAX_ROSTER];
    Turnout turnouts[MAX_TURNOUTS];
    Route routes[MAX_ROUTES];
    Turntable turntables[MAX_TURNTABLES];

    // *******************

    bool getServer();

    bool getRoster();
    bool getTurnouts();
    bool getRoutes();
    bool getTurntable();
  
    setPower(TrackPower powerState, int track);

    bool emergencyStop();

    Consist getThrottleConsist(int throttleNo);

	void setTrackPower(TrackPower state);

    bool setTurnout(int turnoutId, TurnoutAction action);
    bool setRoute(int routeId);
    bool pauseRoutes();
    bool resumeRoutes();
    bool setTurntable(int TurntableId, int position, int activity);

    bool setAccessory(int accessoryAddress, int activate);
    bool setAccessory(int accessoryAddress, int accessorySubAddr, int activate);

    long getLastServerResponseTime();  // seconds since Arduino start

  private:
  
    bool server;
    
    Stream *stream;
    Stream *console;
    NullStream nullStream;
	
    DCCEXProtocolDelegate *delegate = NULL;

    long lastServerResponseTime;

    char inputbuffer[32767];
    ssize_t nextChar;  // where the next character to be read goes in the buffer

    void init();

    bool processCommand(char *c, int len);
    void sendCommand(String cmd);

    void processUnknownCommand(const String& unknownCommand);

    void processServerDescription(char *c, int len);	

    void processTrackPower(char *c, int len);

    // *******************

    void requestLocoUpdate(int address);
    void setLoco(int address, int speed, Direction direction);

    bool processLocoAction(char *c, int len);

    // *******************

    void processRosterList(char *c, int len);
    void processRosterEntry(char *c, int len);

    void processTurnoutList(char *c, int len);
    void processTurnoutEntry(char *c, int len);
    void processTurnoutAction(char *c, int len);

    void processRouteList(char *c, int len);
    void processRouteEntry(char *c, int len);
    void processRouteAction(char *c, int len);

    void processTurntableList(char *c, int len);
    void processTurntableEntry(char *c, int len);
    void processTurntableIndexEntry(char *c, int len);
    void processTurntableAction(char *c, int len);

};

class Functions {
    public:
        String functionLabel[28];
        int functionState[28];
        int functionLatching[28];
        int functionState[28];

    private:
}

class Loco {
    public:
        int locoAddress;
        String locoName;
        int locoSpeed;
        Direction locoDirection;
        Functions locoFunctions;
        LocoSource locoSource;

    private:

}

class Consist {
    public:
        Loco consistLocos[10];
        int consistLocosFacing[10];
        int consistSpeed;
        Direction consistDirection;

        bool consistAddLoco(int locoAddress);
        bool consistReleaseLoco();   //all
        bool consistReleaseLoco(int locoAddress);
        bool consistGetNumberOfLocos();
        bool consistGetLocoAtPosition(int position);
        bool consistGetLocoPosition(int locoAddress);

        bool consistSetSpeed(int speed);
        int consistGetSpeed();
        bool consistSetDirection(Direction direction);
        bool consistSetFunction(int functionNo, FunctionState functionState);
        bool consistSetFunction(int address, int functionNo, FunctionState functionState);

    private:

}

class Turnout {
    public:
        int turnoutId;
        String turnoutName;
        TurnoutState turnoutState;
    private:
}

class Route {
    public:
        int routeId;
        String routeName;
    private:
}


class TurntableIndex {
    public:
        int turntableIndexId;
        String turntableIndexName;
        int turntableValue;
        int turntableAngle;
    private:
}

class Turntable {
    public:
        int turntableId;
        String turntableName;
        int turntablePosition;
        TurntableIndex turntableIndexes[MAX_TURNTABLE_INDEXES];
        int turntableGetCurrentIndex();

    private:
        bool turntableIsMoving;
}

#endif // DCCEXPROTOCOL_H