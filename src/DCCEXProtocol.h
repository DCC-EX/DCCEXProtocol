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
#define COMMAND_START       '<'
#define COMMAND_END         '>'

#define UNKNOWN "Unknown"
#define UnknownIdResponse "X"

// *****************************************************************

typedef int Direction;
#define Reverse 1
#define Forward 0

typedef int  TrackPower;
#define PowerOff 0
#define PowerOn 1
#define PowerUnknown 2

typedef int TurnoutState;
#define TurnoutClosed 0
#define TurnoutThrown 1
#define TurnoutResponseClosed "C"
#define TurnoutResponseThrown "T"

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
#define TrackModeMain "MAIN"
#define TrackModeProg "PROG"
#define TrackModeDC "DC"
#define TrackModeDCX "DCX"
#define TrackModeOff "OFF"

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

typedef String RouteType;
#define RouteTypeRoute "R"
#define RouteTypeAutomation "A"

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
        Loco() {}
        Loco(int address, String name, LocoSource source);
        Functions locoFunctions;

        bool setLocoSpeed(int speed);
        bool setLocoDirection(Direction direction);
        
        int getLocoAddress();
        bool setLocoName(String name);
        String getLocoName();
        bool setLocoSource(LocoSource source);
        LocoSource getLocoSource();
        int  getLocoSpeed();
        Direction getLocoDirection();
        void setIsFromRosterAndReceivedDetails();
        bool getIsFromRosterAndReceivedDetails();

    private:
        int locoAddress;
        String locoName;
        int locoSpeed;
        Direction locoDirection;
        LocoSource locoSource;
        bool rosterReceivedDetails;
};

class ConsistLoco : public Loco {
    public:
        // ConsistLoco(int address, String name, LocoSource source, Facing facing)
        //  : Loco(address, name, source) {};
        ConsistLoco() {};
        ConsistLoco(int address, String name, LocoSource source, Facing facing);
        bool setConsistLocoFacing(Facing facing);
        Facing getConsistLocoFacing();

    private:
        Facing consistLocoFacing;
};

class Consist {
    public:
        Consist() {}
        Consist(String name);
        bool consistAddLoco(Loco loco, Facing facing);
        bool consistReleaseAllLocos();
        bool consistReleaseLoco(int locoAddress);
        int consistGetNumberOfLocos();
        ConsistLoco* consistGetLocoAtPosition(int position);
        int consistGetLocoPosition(int locoAddress);

        bool actionConsistExternalChange(int speed, Direction direction, FunctionState fnStates[]);

        bool consistSetSpeed(int speed);
        int consistGetSpeed();
        bool consistSetDirection(Direction direction);
        Direction consistGetDirection();
        bool consistSetFunction(int functionNo, FunctionState state);
        bool consistSetFunction(int address, int functionNo, FunctionState state);

        String getConsistName();
        LinkedList<ConsistLoco*> consistLocos = LinkedList<ConsistLoco*>();

    private:
        int consistSpeed;
        Direction consistDirection;
        String consistName;
};

class Turnout {
    public:
        Turnout() {}
        Turnout(int id, String name, TurnoutState state);
        bool setTurnoutState(TurnoutAction action);
        TurnoutState getTurnoutState();
        bool setTurnoutId(int id);
        int getTurnoutId();
        bool setTurnoutName(String name);
        String getTurnoutName();
        void setHasReceivedDetails();
        bool getHasReceivedDetails();

    private:
        int turnoutId;
        String turnoutName;
        TurnoutState turnoutState;
        bool hasReceivedDetail;
};

class Route {
    public:
        Route() {}
        Route(int id, String name);
        int getRouteId();
        bool setRouteName(String name);
        String getRouteName();
        bool setRouteType(String type);
        RouteType getRouteType();

        void setHasReceivedDetails();
        bool getHasReceivedDetails();
        
    private:
        int routeId;
        String routeName;
        String routeType;
        bool hasReceivedDetail;
};


class TurntableIndex {
    public:
        int turntableIndexId;
        int turntableIndexIndex;
        String turntableIndexName;
        int turntableIndexAngle;
        bool hasReceivedDetail;

        TurntableIndex() {}
        TurntableIndex(int index, String name, int angle);
        void setHasReceivedDetails();
        bool getHasReceivedDetails();
        String getTurntableIndexName();
        int getTurntableIndexId();
        int getTurntableIndexIndex();
};

class Turntable {
    public:
        Turntable() {}
        Turntable(int id, String name, TurntableType type, int position, int indexCount);
        bool addTurntableIndex(int index, String indexName, int indexAngle);
        LinkedList<TurntableIndex*> turntableIndexes = LinkedList<TurntableIndex*>();
        bool setTurntableIndexCount(int indexCount); // what was listed in the original definition
        int getTurntableIndexCount(); // what was listed in the original definition
 
        int getTurntableId();
        bool setTurntableName(String name);
        String getTurntableName();
        bool setTurntableCurrentPosition(int index);
        bool setTurntableType(TurntableType type);
        TurntableType getTurntableType();
        int getTurntableCurrentPosition();
        int getTurntableNumberOfIndexes();
        TurntableIndex* getTurntableIndexAt(int positionInLinkedList);
        TurntableIndex* getTurntableIndex(int indexId);
        TurntableState getTurntableState();
        bool actionTurntableExternalChange(int index, TurntableState state);
        void setHasReceivedDetails();
        bool getHasReceivedDetails();

    private:
        int turntableId;
        TurntableType turntableType;
        String turntableName;
        int turntableCurrentPosition;
        bool turntableIsMoving;
        bool hasReceivedDetail;
        int turnTableIndexCount; // what was listed in the original definition
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
    virtual void receivedTurntableList(int turntablesListSize) {}    

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
    LinkedList<Loco*> roster = LinkedList<Loco*>();
    LinkedList<Turnout*> turnouts = LinkedList<Turnout*>();
    LinkedList<Route*> routes = LinkedList<Route*>();
    LinkedList<Turntable*> turntables = LinkedList<Turntable*>();

    //helper functions
    Direction getDirectionFromSpeedByte(int speedByte);
    int getSpeedFromSpeedByte(int speedByte);
    void getFunctionStatesFromFunctionMap(FunctionState fnStates[], int functionMap);
    int bitExtracted(int number, int k, int p);

    // *******************

    void sendCommand(String cmd);

    bool sendThrottleAction(int throttle, int speed, Direction direction);
    bool sendLocoAction(int address, int speed, Direction direction);
    bool sendFunction(int throttle, int funcNum, bool pressed);
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

    void processServerDescription(LinkedList<String> &args);	

    void processTrackPower(LinkedList<String> &args);

    // *******************

    void processRosterEntry(LinkedList<String> &args);
    void processRosterList(LinkedList<String> &args);
    void sendRosterEntryRequest(int address);

    void processTurnoutEntry(LinkedList<String> &args);
    void processTurnoutList(LinkedList<String> &args);
    void processTurnoutAction(LinkedList<String> &args);
    void sendTurnoutEntryRequest(int id);

    void processRouteList(LinkedList<String> &args);
    void processRouteEntry(LinkedList<String> &args);
    void sendRouteEntryRequest(int id);
    // void processRouteAction(LinkedList<String> &args);

    void processTurntableEntry(LinkedList<String> &args);
    void processTurntableList(LinkedList<String> &args);
    void processTurntableIndexEntry(LinkedList<String> &args);
    void processTurntableAction(LinkedList<String> &args);
    void sendTurntableEntryRequest(int id);
    void sendTurntableIndexEntryRequest(int id);

    void processSensorEntry(LinkedList<String> &args);

    bool processLocoAction(LinkedList<String> &args);

    //helper functions
    int findThrottleWithLoco(int address);
    int findTurnoutListPositionFromId(int id);
    int findRouteListPositionFromId(int id);
    int findTurntableListPositionFromId(int id);
    // LinkedList<String> splitCommand(String text, char splitChar);
    bool splitCommand(LinkedList<String> &args, String text, char splitChar);
    int countSplitCharacters(String text, char splitChar);
    String stripLeadAndTrailQuotes(String text);
    String substituteCharBetweenQuotes(String text, char searchChar, char substituteChar);
};

#endif // DCCEXPROTOCOL_H