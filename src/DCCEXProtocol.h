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

#include <Arduino.h>
#include <LinkedList.h>  // https://github.com/ivanseidel/LinkedList


static const int MAX_THROTTLES = 6;
static const int MAX_FUNCTIONS = 28;

// Protocol special characters
#define NEWLINE 			'\n'
#define CR 					'\r'

// *****************************************************************

typedef int Direction;
#define Reverse 0
#define Forward 1

typedef int  TrackPower;
#define PowerOff 0
#define PowerOn 1
#define PowerUnknown 2

typedef int TurnoutState;
#define TurnoutClosed 0
#define TurnoutThrown 1
#define TurnoutUnknownId -1

typedef int TurnoutAction;
#define TurnoutClose 0
#define TurnoutThrow 1
#define TurnoutToggle 2
#define TurnoutExamine 9     // "X" needs to be sent

typedef int RouteState;
#define RouteActive 2
#define RouteInactive 4
#define RouteInconsistent 8

typedef int TrackMode;
static const String TrackModeMain = "MAIN";
static const String  TrackModeProg = "PROG";
static const String  TrackModeDC = "DC";
static const String  TrackModeDCX = "DCX";
static const String TrackModeOff = "OFF";

typedef int TurntableState;
#define TurntableStationary 0
#define TurntableMoving 1

typedef int TurntableType;
#define TurntableTypeDCC 0
#define TurntableTypeEXTT 1
#define TurntableTypeUnknown 9 // returns 'X'

typedef int FunctionState;
#define FunctionStateOff 0
#define FunctionStateOn 1

typedef int FunctionLatching;
#define FunctionLatchingTrue 1
#define FunctionLatchingFalse 0

typedef int LocoSource;
#define LocoSourceRoster 0
#define LocoSourceEntry 1

typedef int Facing;
#define FacingForward 0
#define FacingReversed 1

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
};

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
};

class ConsistLoco : public Loco {
    public:
        bool setConsistLocoFacing(Facing facing);
        Facing getConsistLocoFacing();
        bool initConsistLoco(Loco loco, Facing facing);
    private:
        Facing consistLocoFacing;
};

class Consist {
    public:

        bool initConsist(String name);
        bool consistAddLoco(Loco loco, Facing facing);
        bool consistReleaseAllLocos();
        bool consistReleaseLoco(int locoAddress);
        int consistGetNumberOfLocos();
        ConsistLoco consistGetLocoAtPosition(int position);
        int consistGetLocoPosition(int locoAddress);

        bool actionConsistExternalChange(int speed, Direction direction, Functions functions);

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
};

class Turnout {
    public:
        bool initTurnout(int id, String name, TurnoutState state);
        // bool sendTurnoutState(TurnoutAction action);
        bool setTurnoutState(TurnoutAction action);
        TurnoutState getTurnoutState();
        int getTurnoutId();
        String getTurnoutName();
//        bool actionTurnoutExternalChange(TurnoutState state);

    private:
        int turnoutId;
        String turnoutName;
        TurnoutState turnoutState;
};

class Route {
    public:
        bool initRoute(int id, String name);
        int getRouteId();
    private:
        int routeId;
        String routeName;
};


class TurntableIndex {
    public:
        int turntableIndexId;
        int turntableIndexIndex;
        String turntableIndexName;
        int turntableIndexAngle;

        bool initTurntableIndex(int index, String name, int angle);
};

class Turntable {
    public:
        bool initTurntable(int id, String name, TurntableType type, int position);
        bool addTurntableIndex(int index, String indexName, int indexAngle);
        // bool sendTurntableRotateTo(int index);
        LinkedList<TurntableIndex> turntableIndexes = LinkedList<TurntableIndex>();

        int getTurntableId();
        String getTurntableName();
        bool setTurntableCurrentPosition(int index);
        int getTurntableCurrentPosition();
        int getTurntableNumberOfIndexes();
        TurntableIndex getTurntableIndexAt(int positionInLinkedList);
        TurntableIndex getTurntableIndex(int indexId);
        TurntableState getTurntableState();
        bool actionTurntableExternalChange(int index, TurntableState state);

    private:
        int turntableId;
        TurntableType turntableType;
        String turntableName;
        int turntableCurrentPosition;
        bool turntableIsMoving;
};

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
  
    virtual void receivedServerDescription(String microprocessor, String version) {}
  
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

// *******************

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

    //helper functions
    Direction getDirectionFromSpeedByte(int speedByte);
    int getSpeedFromSpeedByte(int speedByte);
    Functions getFunctionStatesFromFunctionMap(int * states, int functionMap);

    // *******************

    void sendCommand(String cmd);

    bool sendThrottleAction(int throttle, int speed, Direction direction);
    bool sendLocoAction(int address, int speed, Direction direction);
    bool sendFunction(int throttle, String address, int funcNum, bool pressed);
    bool sendLocoUpdateRequest(int address);

    // *******************

    bool sendServerDetailsRequest();

    bool getRoster();
    bool getTurnouts();
    bool getRoutes();
    bool getTurntables();

    long getLastServerResponseTime();  // seconds since Arduino start

    void sendEmergencyStop();

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

    // *******************

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
    void processUnknownCommand(String unknownCommand);

    void processServerDescription(LinkedList<String> args);	

    void processTrackPower(LinkedList<String> args);

    // *******************

    void processRosterEntry(LinkedList<String> args);
    void processRosterList(LinkedList<String> args);
    void sendRosterEntryRequest(int address);

    void processTurnoutEntry(LinkedList<String> args);
    void processTurnoutList(LinkedList<String> args);
    void processTurnoutAction(LinkedList<String> args);
    void sendTurnoutEntryRequest(int id);

    void processRouteList(LinkedList<String> args);
    void processRouteEntry(LinkedList<String> args);
    void sendRouteEntryRequest(int id);
    // void processRouteAction(LinkedList<String> args);

    void processTurntableEntry(LinkedList<String> args);
    void processTurntableList(LinkedList<String> args);
    void processTurntableIndexEntry(LinkedList<String> args);
    void processTurntableAction(LinkedList<String> args);
    void sendTurntableEntryRequest(int id);
    void sendTurntableIndexEntryRequest(int id);

    bool processLocoAction(LinkedList<String> args);

    //helper functions
    int findThrottleWithLoco(int address);
    int findTurnoutListPositionFromId(int id);
    int findRouteListPositionFromId(int id);
    int findTurntableListPositionFromId(int id);
    LinkedList<String> splitCommand(String text, char splitChar);
    int countSplitCharacters(String text, char splitChar);

};

#endif // DCCEXPROTOCOL_H