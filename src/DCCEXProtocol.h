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
static const int MAX_FUNCTIONS 28

#include "Arduino.h"
#include <LinkedList.h>  // https://github.com/ivanseidel/LinkedList

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
    TurntableStationary = 0
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
  
    virtual void receivedServerDescription(String description, String version) {}
  
    virtual void receivedRosterList(int rosterSize) {}
    virtual void receivedTurnoutList(int turnoutListSize) {}    
    virtual void receivedRouteList(int routeListSize) {}
    virtual void receivedTurntablesList(int turntablesListSize) {}    

    virtual void receivedSpeed(int throttleNo, int speed) { }
    virtual void receivedDirection(int throttleNo, Direction dir) { }
    virtual void receivedFunction(int throttleNo, int func, bool state) { }

    virtual void receivedTrackPower(TrackPower state) { }

    virtual void receivedTurnoutAction(int turnoutId, TurnoutState state) { } //  PTAturnoutstatesystemname
    virtual void receivedRouteAction(int routeId, RouteState state) { } //  PTAturnoutstatesystemname
    virtual void receivedTurntableAction(int turntableId, int position, TurntableState turntableState) { } //  PTAturnoutstatesystemname
};


class DCCEXProtocol {
  public:
    
    DCCEXProtocol(bool server = false);

    void setDelegate(DCCEXProtocolDelegate *delegate);
    void setLogStream(Stream *console);

    void connect(Stream *stream);
    void disconnect();

    bool check();

    String serverVersion;
    String serverMicroprocessorType;
    String serverMotorcontrollerType;
    String serverBuildNumber;

    // *******************

    Consist throttle[MAX_THROTTLES];
    LinkedList<Loco> roster = LinkedList<Loco>();
    LinkedList<Turnout> turnouts = LinkedList<Turnout>();
    LinkedList<Route> routes = LinkedList<Route>();
    LinkedList<Turntable> turntables =LinkedList<Turntable>();

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

    void processServerDescription(String args[], char *c, int len);	

    void processTrackPower(char *c, int len);

    // *******************

    void requestLocoUpdate(int address);
    void sendLoco(int address, int speed, Direction direction);

    bool processLocoAction(String args[], cchar *c, int len);

    // *******************

    void processRosterList(String args[], char *c, int len);
    void processRosterEntry(String args[], char *c, int len);
    void requestRosterEntry(int id);

    void processTurnoutList(String args[], char *c, int len);
    void processTurnoutEntry(String args[], char *c, int len);
    void processTurnoutAction(String args[], char *c, int len);

    void processRouteList(String args[], char *c, int len);
    void processRouteEntry(String args[], char *c, int len);
    void processRouteAction(String args[], char *c, int len);

    void processTurntableList(String args[], char *c, int len);
    void processTurntableEntry(String args[], char *c, int len);
    void processTurntableIndexEntry(String args[], char *c, int len);
    void processTurntableAction(String args[], char *c, int len);

};

// *****************************************************************

class Functions {
    public:
        bool setFunction(int functionNumber, String label, FunctionLatching latching, FunctionState state);
        bool setFunctionState(int functionNumber, FunctionState state);
        FunctionState getFunctionState(int functionNumber);
       
    private:
        String functionLabel[MAX_FUNCTIONS];
        int functionState[MAX_FUNCTIONS];
        int functionLatching[MAX_FUNCTIONS];
        int functionState[MAX_FUNCTIONS];
}

class Loco {
    public:
        Functions locoFunctions;
        bool setLoco(int address, String name, LocoSource source);
        String getLocoName(int address);
        LocoSource getLocoSource(int address);
        bool setLocoSpeed(int address, int speed);
        bool setLocoDirection(int address, Direction direction);
        int  getLocoSpeed(int address);
        Direction getLocoDirection(int address);

    private:
        int locoAddress;
        String locoName;
        int locoSpeed;
        Direction locoDirection;
        LocoSource locoSource;
}

class ConsistLoco : public Loco {
    public:
        bool setConsistLocoFacing(Facing facing);
        Facing getConsistLocoFacing();
    private:
        Facing consisLocoFacing;
}

class Consist {
    public:

        bool consistAddLoco(Loco loco, Facing, facing);
        bool consistReleaseLoco();   //all
        bool consistReleaseLoco(int locoAddress);
        bool consistGetNumberOfLocos();
        bool consistGetLocoAtPosition(int position);
        bool consistGetLocoPosition(int locoAddress);

        bool consistSetSpeed(int speed);
        int consistGetSpeed();
        bool consistSetDirection(Direction direction);
        Direction consistGetDirection();
        bool consistSetFunction(int functionNo, FunctionState state);
        bool consistSetFunction(int address, int functionNo, FunctionState state);

    private:
        LinkedList<ConsistLoco> consistLocos = LinkedList<ConsistLoco>();
        int consistSpeed;
        Direction consistDirection;
}

class Turnout {
    public:
        bool setTurnout(int id, String name, TurnoutState state);
        bool setTurnoutState(int id, TurnoutState state);
        TurnoutState getTurnoutState(int id);

    private:
        int turnoutId;
        String turnoutName;
        TurnoutState turnoutState;
}

class Route {
    public:
        bool setRoute(int id, String name);
    private:
        int routeId;
        String routeName;
}


class TurntableIndex {
    public:
        int turntableIndexId;
        String turntableIndexName;
        int turntableAngle;
}

class Turntable {
    public:
        bool addTurntableIndex(int turntableIndexId, String turntableIndexName, int turntableAngle);
        int getTurntableCurrentPosition();
        int getTurntableNumberOfIndexes();
        TurntableIndex getTurntableIndexAt(int positionInLinkedList);
        TurntableIndex getTurntableIndex(int indexId);
        bool rotateTurntableTo(int index);
        TurntableState getTurntableState();

    private
        int turntableId;
        String turntableName;
        int turntableCurrentPosition;
        LinkedList<TurntableIndex> turntableIndexes = LinkedList<TurntableIndex>;
        bool turntableIsMoving;
}

#endif // DCCEXPROTOCOL_H