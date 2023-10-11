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
#include "DCCEXInbound.h"

static const int MAX_THROTTLES = 6;
static const int MAX_FUNCTIONS = 28;
// #define MAX_COMMAND_PARAMS 100
#define MAX_SINGLE_COMMAND_PARAM_LENGTH 500  // Unfortunately includes the function list for an individual loco
#define MAX_SINGLE_FUNCTION_LENGTH 30 
#define MAX_OBJECT_NAME_LENGTH 30  // including Loco name, Turnout/Point names, Route names, etc. names
#define MAX_OUTBOUND_COMMAND_LENGTH 100

// DCCEXInbound params
const int MAX_COMMAND_PARAMS = 50;
const int MAX_COMMAND_BUFFER = 500;

// Protocol special characters
#define NEWLINE 			'\n'
#define CR 					'\r'
#define COMMAND_START       '<'
#define COMMAND_END         '>'

static const char NAME_UNKNOWN[] = "Unknown";
static const char NAME_BLANK[] = "";
#define UnknownIdResponse "X"

enum splitState {FIND_START, SKIP_SPACES, CHECK_FOR_LEADING_QUOTE, BUILD_QUOTED_PARAM, BUILD_PARAM, CHECK_FOR_END};
enum splitFunctionsState {FIND_FUNCTION_START, SKIP_FUNCTION_LEADING_SLASH_SPACES, SKIP_FUNCTION_SPACES, BUILD_FUNCTION_PARAM, CHECK_FOR_FUNCTION_END};

// *****************************************************************

typedef char Direction;
#define Reverse '1'
#define Forward '0'

// typedef char TrackPower;
// #define PowerOff '0'
// #define PowerOn '1'
// #define PowerUnknown '2'

enum TrackPower {
    PowerOff = 0,
    PowerOn = 1,
    PowerUnknown = 2,
};

typedef char TurnoutState;
#define TurnoutClosed '0'
#define TurnoutThrown '1'
#define TurnoutResponseClosed 'C'
#define TurnoutResponseThrown 'T'

typedef char TurnoutAction;
#define TurnoutClose '0'
#define TurnoutThrow '1'
#define TurnoutToggle '2'
#define TurnoutExamine '9'     // "X" needs to be sent

typedef char RouteState;
#define RouteActive '2'
#define RouteInactive '4'
#define RouteInconsistent '8'

typedef int TrackMode;
#define TrackModeMain "MAIN"
#define TrackModeProg "PROG"
#define TrackModeDC "DC"
#define TrackModeDCX "DCX"
#define TrackModeOff "OFF"

typedef char TurntableState;
#define TurntableStationary '0'
#define TurntableMoving '1'

typedef char TurntableType;
#define TurntableTypeDCC '0'
#define TurntableTypeEXTT '1'
#define TurntableTypeUnknown '9' // returns 'X'

typedef char FunctionState;
#define FunctionStateOff '0'
#define FunctionStateOn '1'

typedef char FunctionLatching;
#define FunctionLatchingTrue '1'
#define FunctionLatchingFalse '0'

typedef char LocoSource;
#define LocoSourceRoster '0'
#define LocoSourceEntry '1'

typedef char Facing;
#define FacingForward '0'
#define FacingReversed '1'

typedef char RouteType;
#define RouteTypeRoute 'R'
#define RouteTypeAutomation 'A'

// *****************************************************************

// used to split the command arguments
class CommandArgument {
    public:
        char* arg;
        CommandArgument(char* argValue);
        bool clearCommandArgument();
};

// used to split the function labels
class FunctionArgument {
    public:
        char* arg;
        FunctionArgument(char* argValue);
        bool clearFunctionArgument();
};

class Functions {
    public:
        bool initFunction(int functionNumber, char* label, FunctionLatching latching, FunctionState state);
        bool setFunctionState(int functionNumber, FunctionState state);
        bool setFunctionName(int functionNumber, char* label);
        char* getFunctionName(int functionNumber);
        FunctionState getFunctionState(int functionNumber);
        FunctionLatching getFunctionLatching(int functionNumber);
        bool clearFunctionNames();
    
    private:
        char* functionName[MAX_FUNCTIONS];
        FunctionState functionState[MAX_FUNCTIONS];
        int functionLatching[MAX_FUNCTIONS];

        bool actionFunctionStateExternalChange(int functionNumber, FunctionState state);
};

class Loco {
    public:
        Loco() {}
        Loco(int address, char* name, LocoSource source);
        Functions locoFunctions;
        bool isFunctionOn(int functionNumber);

        bool setLocoSpeed(int speed);
        bool setLocoDirection(Direction direction);
        
        int getLocoAddress();
        bool setLocoName(char* name);
        char* getLocoName();
        bool setLocoSource(LocoSource source);
        LocoSource getLocoSource();
        int  getLocoSpeed();
        Direction getLocoDirection();
        void setIsFromRosterAndReceivedDetails();
        bool getIsFromRosterAndReceivedDetails();
        bool clearLocoNameAndFunctions();

    private:
        int locoAddress;
        char* locoName;
        int locoSpeed;
        Direction locoDirection;
        LocoSource locoSource;
        bool rosterReceivedDetails;
};

class ConsistLoco : public Loco {
    public:
        ConsistLoco() {};
        ConsistLoco(int address, char* name, LocoSource source, Facing facing);
        bool setConsistLocoFacing(Facing facing);
        Facing getConsistLocoFacing();

    private:
        Facing consistLocoFacing;
};

class Consist {
    public:
        Consist() {}
        Consist(char* name);
        bool consistAddLoco(Loco loco, Facing facing);
        bool consistAddLocoFromRoster(LinkedList<Loco*> roster, int address, Facing facing);
        bool consistAddLocoFromAddress(int address, char* name, Facing facing);
        bool consistReleaseAllLocos();
        bool consistReleaseLoco(int locoAddress);
        int consistGetNumberOfLocos();
        ConsistLoco* consistGetLocoAtPosition(int position);
        int consistGetLocoPosition(int locoAddress);
        bool consistSetLocoPosition(int locoAddress, int position);

        bool actionConsistExternalChange(int speed, Direction direction, FunctionState fnStates[]);

        bool consistSetSpeed(int speed);
        int consistGetSpeed();
        bool consistSetDirection(Direction direction);
        Direction consistGetDirection();
        bool consistSetFunction(int functionNo, FunctionState state);
        bool consistSetFunction(int address, int functionNo, FunctionState state);
        bool isFunctionOn(int functionNumber);
        bool setConsistName(char* name);
        char* getConsistName();
        LinkedList<ConsistLoco*> consistLocos = LinkedList<ConsistLoco*>();

    private:
        int consistSpeed;
        Direction consistDirection;
        char* consistName;
};

class Turnout {
    public:
        Turnout() {}
        Turnout(int id, char* name, TurnoutState state);
        bool setTurnoutState(TurnoutAction action);
        TurnoutState getTurnoutState();
        bool throwTurnout();
        bool closeTurnout();
        bool toggleTurnout();
        bool setTurnoutId(int id);
        int getTurnoutId();
        bool setTurnoutName(char* name);
        char* getTurnoutName();
        void setHasReceivedDetails();
        bool getHasReceivedDetails();

    private:
        int turnoutId;
        char* turnoutName;
        TurnoutState turnoutState;
        bool hasReceivedDetail;
};

class Route {
    public:
        Route() {}
        Route(int id, char* name);
        int getRouteId();
        bool setRouteName(char* name);
        char* getRouteName();
        bool setRouteType(RouteType type);
        RouteType getRouteType();

        void setHasReceivedDetails();
        bool getHasReceivedDetails();
        
    private:
        int routeId;
        char* routeName;
        char routeType;
        bool hasReceivedDetail;
};


class TurntableIndex {
    public:
        int turntableIndexId;
        int turntableIndexIndex;
        char* turntableIndexName;
        int turntableIndexAngle;
        bool hasReceivedDetail;

        TurntableIndex() {}
        TurntableIndex(int index, char* name, int angle);
        void setHasReceivedDetails(); //????????????????? Probably not needed
        bool getHasReceivedDetails(); //????????????????? Probably not needed
        char* getTurntableIndexName();
        int getTurntableIndexId();
        int getTurntableIndexIndex();
};

class Turntable {
    public:
        Turntable() {}
        Turntable(int id, char* name, TurntableType type, int position, int indexCount);
        bool addTurntableIndex(int index, char* indexName, int indexAngle);
        LinkedList<TurntableIndex*> turntableIndexes = LinkedList<TurntableIndex*>();
        bool setTurntableIndexCount(int indexCount); // what was listed in the original definition
        int getTurntableIndexCount(); // what was listed in the original definition
 
        int getTurntableId();
        bool setTurntableName(char* name);
        char* getTurntableName();
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
        char* turntableName;
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
    virtual void receivedServerDescription(char* version) {}
  
    virtual void receivedRosterList(int rosterSize) {}
    virtual void receivedTurnoutList(int turnoutListSize) {}    
    virtual void receivedRouteList(int routeListSize) {}
    virtual void receivedTurntableList(int turntablesListSize) {}    

    virtual void receivedSpeed(int throttleNo, int speed) { }
    virtual void receivedDirection(int throttleNo, Direction dir) { }
    virtual void receivedFunction(int throttleNo, int func, FunctionState state) { }

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

    char *serverVersion;
    char *serverMicroprocessorType;
    char *serverMotorcontrollerType;
    char *serverBuildNumber;
    bool isServerDetailsReceived();

    // *******************

    Consist throttleConsists[MAX_THROTTLES];
    LinkedList<Loco*> roster = LinkedList<Loco*>();
    LinkedList<Turnout*> turnouts = LinkedList<Turnout*>();
    LinkedList<Route*> routes = LinkedList<Route*>();
    LinkedList<Turntable*> turntables = LinkedList<Turntable*>();

    LinkedList<CommandArgument*> argz = LinkedList<CommandArgument*>();
    LinkedList<FunctionArgument*> functionArgs = LinkedList<FunctionArgument*>();

    //helper functions
    Direction getDirectionFromSpeedByte(int speedByte);
    int getSpeedFromSpeedByte(int speedByte);
    void getFunctionStatesFromFunctionMap(FunctionState fnStates[], int functionMap);
    int bitExtracted(int number, int k, int p);
    char* charToCharArray(char c);

    // *******************

    void sendCommand();

    Loco findLocoInRoster(int address);
    
    bool sendThrottleAction(int throttle, int speed, Direction direction);
    bool sendLocoAction(int address, int speed, Direction direction);
    bool sendFunction(int throttle, int functionNumber, FunctionState pressed);
    bool sendFunction(int throttle, int address, int functionNumber, FunctionState pressed);
    bool isFunctionOn(int throttle, int functionNumber);
    bool sendLocoUpdateRequest(int address);

    // *******************

    bool sendServerDetailsRequest();

    bool getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired);
    bool getRoster();
    bool isRosterRequested();
    bool isRosterFullyReceived();
    bool getTurnouts();
    bool isTurnoutListRequested();
    bool isTurnoutListFullyReceived();
    bool getRoutes();
    bool isRouteListRequested();
    bool isRouteListFullyReceived();
    bool getTurntables();
    bool isTurntableListRequested();
    bool isTurntableListFullyReceived();

    long getLastServerResponseTime();  // seconds since Arduino start

    void sendEmergencyStop();

    Consist getThrottleConsist(int throttleNo);

	bool sendTrackPower(TrackPower state);
	bool sendTrackPower(TrackPower state, char track);

    Turnout* getTurnoutById(int turnoutId);
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

    int bufflen;
    char cmdBuffer[MAX_COMMAND_BUFFER];
	
    char outboundCommand[MAX_OUTBOUND_COMMAND_LENGTH];

    DCCEXProtocolDelegate *delegate = NULL;

    long lastServerResponseTime;

    // char inputbuffer[32767];
    // char inputbuffer[16383];    
    char inputbuffer[512];    
    ssize_t nextChar;  // where the next character to be read goes in the buffer
    char charToCharArrayVal[2];  // common. used to convert sinle char to char array.

    void init();

    void processCommand();
    // bool processCommand(char *c, int len);
    void processUnknownCommand(char* unknownCommand);

    void processServerDescription();	
    bool haveReceivedServerDetails = false;

    void processTrackPower();

    // *******************

    bool allRequiredListsReceived = false;
    
    bool rosterRequested = false;
    bool rosterFullyReceived = false;
    void processRosterEntry();
    void processRosterList();
    void sendRosterEntryRequest(int address);

    bool turnoutListRequested = false;
    bool turnoutListFullyReceived = false;
    void processTurnoutEntry();
    void processTurnoutList();
    void processTurnoutAction();
    void sendTurnoutEntryRequest(int id);

    bool routeListRequested = false;
    bool routeListFullyReceived = false;
    void processRouteList();
    void processRouteEntry();
    void sendRouteEntryRequest(int id);
    // void processRouteAction();

    bool turntableListRequested = false;
    bool turntableListFullyReceived = false;
    void processTurntableEntry();
    void processTurntableList();
    void processTurntableIndexEntry();
    void processTurntableAction();
    void sendTurntableEntryRequest(int id);
    void sendTurntableIndexEntryRequest(int id);

    void processSensorEntry();

    bool processLocoAction();

    //helper functions
    int findThrottleWithLoco(int address);
    int findTurnoutListPositionFromId(int id);
    int findRouteListPositionFromId(int id);
    int findTurntableListPositionFromId(int id);
    bool splitValues(char *cmd);
    bool splitFunctions(char *cmd);
    bool stripLeadAndTrailQuotes(char* rslt, char* text);
    // char* substituteCharBetweenQuotes(char* text, char searchChar, char substituteChar);
};

#endif // DCCEXPROTOCOL_H