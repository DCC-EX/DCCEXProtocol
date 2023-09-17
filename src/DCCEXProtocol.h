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

#include <Arduino.h>
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
    TurnoutUnknownId = -1   // "X" is sent
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

typedef enum TurntableType {
    TurntableTypeDCC = 0,
    TurntableTypeEXTT = 1,
    TurntableTypeUnknown = 0 // returns 'X'
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

    virtual void receivedTurnoutAction(int turnoutId, TurnoutState state) { }
    virtual void receivedRouteAction(int routeId, RouteState state) { }
    virtual void receivedTurntableAction(int turntableId, int position, TurntableState turntableState) { }
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

    Consist throttleConsists[MAX_THROTTLES];
    LinkedList<Loco> roster = LinkedList<Loco>();
    LinkedList<Turnout> turnouts = LinkedList<Turnout>();
    LinkedList<Route> routes = LinkedList<Route>();
    LinkedList<Turntable> turntables =LinkedList<Turntable>();

    // *******************

    bool sendServerDetailsRequest();

    bool getRoster();
    bool getTurnouts();
    bool getRoutes();
    bool getTurntables();

    long getLastServerResponseTime();  // seconds since Arduino start

    bool sendEmergencyStop();

    Consist getThrottleConsist(int throttleNo);

	bool sendTrackPower(TrackPower state);
	bool sendTrackPower(TrackPower state, char track);

    bool sendTurnoutAction(int turnoutId, TurnoutAction action);

    bool sendRouteAction(int routeId);
    bool sendPauseRoutes();
    bool sendResumeRoutes();

    bool sendTurntableAction(int turntableId, int position, int activity);

    bool sendAccessoryAction(int accessoryAddress, int activate);
    bool sendAccessoryAction(int accessoryAddress, int accessorySubAddr, int activate);


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

    void sendCommand(String cmd);
    bool processCommand(char *c, int len);
    void processUnknownCommand(const String& unknownCommand);

    void processServerDescription(String args[], char *c, int len);	

    void processTrackPower(char *c, int len);

    // *******************

    bool sendLocoUpdateRequest(int address);
    bool sendLocoAction(int address, int speed, Direction direction);

    // *******************

    void processRosterList(String args[], char *c, int len);
    void processRosterEntry(String args[], char *c, int len);
    void requestRosterEntry(int id);
    void sendRosterEntryRequest(int address);

    void processTurnoutList(String args[], char *c, int len);
    void processTurnoutEntry(String args[], char *c, int len);
    void processTurnoutAction(String args[], char *c, int len);
    void sendTurnoutEntryRequest(int address);

    void processRouteList(String args[], char *c, int len);
    void processRouteEntry(String args[], char *c, int len);
    void sendRouteEntryRequest(int id);
    // void processRouteAction(String args[], char *c, int len);

    void processTurntableList(String args[], char *c, int len);
    void processTurntableEntry(String args[], char *c, int len);
    void processTurntableIndexEntry(String args[], char *c, int len);
    void processTurntableAction(String args[], char *c, int len);
    void sendTurntableEntryRequest(int id);
    void sendTurntableIndexEntryRequest(int id);

    bool processLocoAction(String args[], cchar *c, int len);

    //helper functions
    int findThrottleWithLoco(int address);
    int findTurnoutListPositionFromId(int id);
    int findRouteListPositionFromId(int id);
    int findTurntableListPositionFromId(int id);
};

// *****************************************************************

class Functions {
    public:
        bool initFunction(int functionNumber, String label, FunctionLatching latching, FunctionState state);
        bool setFunctionState(int functionNumber, FunctionState state);
        String getFunctionName(int functionNumber);
        FunctionState getFunctionState(int functionNumber);
        FunctionLatching getFunctionLatching(int functionNumber);
       
    private:
        String functionName[MAX_FUNCTIONS];
        int functionState[MAX_FUNCTIONS];
        int functionLatching[MAX_FUNCTIONS];

        bool actionFunctionStateExternalChange(int functionNumber, FunctionState state);
}

class Loco {
    public:
        Functions locoFunctions;
        bool initLoco(int address, String name, LocoSource source);
        bool setLocoSpeed(int speed);
        bool setLocoDirection(Direction direction);

        int getLocoAddress();
        String getLocoName();
        LocoSource getLocoSource();
        int  getLocoSpeed();
        Direction getLocoDirection();

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
        Facing consistLocoFacing;
}

class Consist {
    public:

        bool initConsist(String name);
        bool consistAddLoco(Loco loco, Facing, facing);
        bool consistReleaseAllLocos();
        bool consistReleaseLoco(int locoAddress);
        int consistGetNumberOfLocos();
        Loco consistGetLocoAtPosition(int position);
        int consistGetLocoPosition(int locoAddress);

        bool actionConsistExternalChange(int speed, Direction, direction, Functions functions);

        bool consistSetSpeed(int speed);
        int consistGetSpeed();
        bool consistSetDirection(Direction direction);
        Direction consistGetDirection();
        bool consistSetFunction(int functionNo, FunctionState state);
        bool consistSetFunction(int address, int functionNo, FunctionState state);

        String getConsistName();

    private:
        LinkedList<ConsistLoco> consistLocos = LinkedList<ConsistLoco>();
        int consistSpeed;
        Direction consistDirection;
        String consistName;
}

class Turnout {
    public:
        bool initTurnout(int id, String name, TurnoutState state);
        bool sendTurnoutState(TurnoutAction action);
        TurnoutState getTurnoutState();
        int getTurnoutId();
        String getTurnoutName();

    private:
        int turnoutId;
        String turnoutName;
        TurnoutState turnoutState;
        bool actionTurnoutExternalChange(TurnoutState state);
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
        int turntableIndexAngle;

        bool initTurntableIndex(int id, String name, TurntableType type, int angle);
}

class Turntable {
    public:
        bool initTurntable(int id, String name, TurntableType type, int position);
        bool addTurntableIndex(int turntableIndexId, String turntableIndexName, int turntableAngle);
        bool sendTurntableRotateTo(int index);

        int getTurntableId();
        String getTurntableName();
        int getTurntableCurrentPosition();
        int getTurntableNumberOfIndexes();
        TurntableIndex getTurntableIndexAt(int positionInLinkedList);
        TurntableIndex getTurntableIndex(int indexId);
        TurntableState getTurntableState();

    private
        int turntableId;
        TurntableType turntableType;
        String turntableName;
        int turntableCurrentPosition;
        LinkedList<TurntableIndex> turntableIndexes = LinkedList<TurntableIndex>;
        bool turntableIsMoving;
        bool actionTurntableExternalChange(int index, TurntableMoving moving);
}

#endif // DCCEXPROTOCOL_H