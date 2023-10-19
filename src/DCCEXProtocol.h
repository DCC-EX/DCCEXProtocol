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
#include "DCCEXLoco.h"
#include "DCCEXRoutes.h"
#include "DCCEXTurnouts.h"
#include "DCCEXTurntables.h"

static const int MAX_THROTTLES = 6;
#define MAX_OUTBOUND_COMMAND_LENGTH 100

// DCCEXInbound params
const int MAX_COMMAND_PARAMS = 50;
const int MAX_COMMAND_BUFFER = 500;

// Protocol special characters
// #define NEWLINE 			'\n'
// #define CR 					'\r'
// #define COMMAND_START       '<'
// #define COMMAND_END         '>'

static const char NAME_UNKNOWN[] = "Unknown";
// static const char NAME_BLANK[] = "";

// enum splitState {FIND_START, SKIP_SPACES, CHECK_FOR_LEADING_QUOTE, BUILD_QUOTED_PARAM, BUILD_PARAM, CHECK_FOR_END};
// enum splitFunctionsState {FIND_FUNCTION_START, SKIP_FUNCTION_LEADING_SLASH_SPACES, SKIP_FUNCTION_SPACES, BUILD_FUNCTION_PARAM, CHECK_FOR_FUNCTION_END};

// *****************************************************************

enum TrackPower {
    PowerOff = 0,
    PowerOn = 1,
    PowerUnknown = 2,
};

typedef int TrackMode;
#define TrackModeMain "MAIN"
#define TrackModeProg "PROG"
#define TrackModeDC "DC"
#define TrackModeDCX "DCX"
#define TrackModeOff "OFF"

// *****************************************************************

// used to split the function labels
class FunctionArgument {
    public:
        char* arg;
        FunctionArgument(char* argValue);
        bool clearFunctionArgument();
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

    virtual void receivedTurnoutAction(int turnoutId, TurnoutStates state) { }
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

    // LinkedList<CommandArgument*> argz = LinkedList<CommandArgument*>();
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
    bool sendTurnoutAction(int turnoutId, TurnoutStates action);

    Turntable* getTurntableById(int turntableId);

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
    // bool splitValues(char *cmd);
    bool splitFunctions(char *functionNames);
    // bool splitFunctions(char *cmd);
    // bool stripLeadAndTrailQuotes(char* rslt, char* text);
    // char* substituteCharBetweenQuotes(char* text, char searchChar, char substituteChar);
};

#endif // DCCEXPROTOCOL_H