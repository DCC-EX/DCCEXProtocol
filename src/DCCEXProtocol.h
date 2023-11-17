/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
 *
 * Copyright © 2023 Peter Akers
 * Copyright © 2023 Peter Cole
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
#include "DCCEXInbound.h"
#include "DCCEXLoco.h"
#include "DCCEXRoutes.h"
#include "DCCEXTurnouts.h"
#include "DCCEXTurntables.h"

const int MAX_OUTBOUND_COMMAND_LENGTH=100;
const int MAX_SERVER_DESCRIPTION_PARAM_LENGTH=100;

// DCCEXInbound params
const int MAX_COMMAND_PARAMS=50;
const int MAX_COMMAND_BUFFER=500;

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
    /// @brief Callback when server description is received
    /// @param version 
    virtual void receivedServerDescription(char* version) {}
  
    /// @brief Callback when roster list received
    /// @param rosterSize 
    virtual void receivedRosterList(int rosterSize) {}
    
    /// @brief Callback when received turnout list
    /// @param turnoutListSize 
    virtual void receivedTurnoutList(int turnoutListSize) {}    
    
    /// @brief Callback when received route list
    /// @param routeListSize 
    virtual void receivedRouteList(int routeListSize) {}
    
    /// @brief  Callback when received turntable list
    /// @param turntablesListSize 
    virtual void receivedTurntableList(int turntablesListSize) {}    

    /// @brief Callback when received speed for a throttle
    /// @param throttleNo 
    /// @param speed 
    virtual void receivedSpeed(int throttleNo, int speed) { }
    
    /// @brief Callback when received direction for a throttle
    /// @param throttleNo 
    /// @param dir 
    virtual void receivedDirection(int throttleNo, Direction dir) { }
    
    /// @brief Callback when received function state change for a throttle
    /// @param throttleNo 
    /// @param func 
    /// @param state 
    virtual void receivedFunction(int throttleNo, int func, bool state) { }

    /// @brief Callback when received a track power state change
    /// @param state 
    virtual void receivedTrackPower(TrackPower state) { }

    /// @brief Callback when received a turnout state change
    /// @param turnoutId 
    /// @param thrown 
    virtual void receivedTurnoutAction(int turnoutId, bool thrown) { }

    /// @brief Callback when received a turntable index change
    /// @param turntableId 
    /// @param position 
    /// @param turntableState 
    virtual void receivedTurntableAction(int turntableId, int position, bool moving) { }

    /// @brief Callback when a loco address is read from the programming track
    /// @param address 
    virtual void receivedReadLoco(int address) {}
};

// *******************

class DCCEXProtocol {
  public:
    
    /// @brief Constructor
    /// @param server 
    DCCEXProtocol(int maxThrottles=6, bool server=false);

    /// @brief Set the delegate object for callbacks
    /// @param delegate 
    void setDelegate(DCCEXProtocolDelegate *delegate);
    
    /// @brief Set the stream object for console output
    /// @param console 
    void setLogStream(Stream *console);

    /// @brief Connect the stream object to interact with DCC-EX
    /// @param stream 
    void connect(Stream *stream);
    
    /// @brief Disconnect from DCC-EX
    void disconnect();

    /// @brief Check for incoming DCC-EX broadcasts/responses
    /// @return 
    bool check();

    char *serverDescription;
    char *serverVersion;
    char *serverVersionMajor;
    char *serverVersionMinor;
    char *serverVersionPatch;
    char *serverMicroprocessorType;
    char *serverMotorcontrollerType;
    char *serverBuildNumber;
    bool isServerDetailsReceived();

    // *******************

    // Consist throttle[maxThrottles];
    Consist *throttle;
    Loco* roster=nullptr;
    Turnout* turnouts=nullptr;
    Route* routes=nullptr;
    Turntable* turntables=nullptr;

    //helper functions
    
    /// @brief Get direct enum name from the speed byte
    /// @param speedByte 
    /// @return 
    Direction getDirectionFromSpeedByte(int speedByte);
    
    /// @brief Get the speed from the speed byte
    /// @param speedByte 
    /// @return 
    int getSpeedFromSpeedByte(int speedByte);
    
    /// @brief Masks off any functions above the first 28
    /// @param functionMap 
    /// @return 
    int getValidFunctionMap(int functionMap);
    
    // *******************

    /// @brief Send the command in the outbound command buffer to DCC-EX
    void sendCommand();

    /// @brief Returns the Loco object for the specified address if found
    /// @param address 
    /// @return 
    Loco* findLocoInRoster(int address);
    
    bool sendThrottleAction(int throttle, int speed, Direction direction);
    bool sendLocoAction(int address, int speed, Direction direction);
    bool sendFunction(int throttle, int functionNumber, bool pressed);
    bool sendFunction(int throttle, int address, int functionNumber, bool pressed);
    bool isFunctionOn(int throttle, int functionNumber);
    bool sendLocoUpdateRequest(int address);
    void sendReadLoco();

    // *******************

    /// @brief Request server details
    void sendServerDetailsRequest();

    /// @brief Request object lists (Roster, Turnouts, Routes, Turntables)
    /// @param rosterRequired 
    /// @param turnoutListRequired 
    /// @param routeListRequired 
    /// @param turntableListRequired 
    void getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired);
    bool getRoster();
    int getRosterCount();
    Loco* getRosterEntryNo(int entryNo);
    bool isRosterRequested();
    bool isRosterFullyReceived();
    bool getTurnouts();
    int getTurnoutsCount();
    Turnout* getTurnoutsEntryNo(int entryNo);
    bool isTurnoutListRequested();
    bool isTurnoutListFullyReceived();
    bool getRoutes();
    int getRoutesCount();
    Route* getRoutesEntryNo(int entryNo);
    bool isRouteListRequested();
    bool isRouteListFullyReceived();
    bool getTurntables();
    int getTurntablesCount();
    bool isTurntableListRequested();
    bool isTurntableListFullyReceived();
    bool isAllListsReceived();

    long getLastServerResponseTime();  // seconds since Arduino start

    void sendEmergencyStop();

    Consist getThrottleConsist(int throttleNo);

	  bool sendTrackPower(TrackPower state);
	  bool sendTrackPower(TrackPower state, char track);

    Turnout* getTurnoutById(int turnoutId);
    void closeTurnout(int turnoutId);
    void throwTurnout(int turnoutId);
    void toggleTurnout(int turnoutId);

    Turntable* getTurntableById(int turntableId);

    bool sendRouteAction(int routeId);
    bool sendPauseRoutes();
    bool sendResumeRoutes();

    bool sendTurntableAction(int turntableId, int position, int activity);

    bool sendAccessoryAction(int accessoryAddress, int activate);
    bool sendAccessoryAction(int accessoryAddress, int accessorySubAddr, int activate);

    // *******************

  private:
  
    int _rosterCount = 0;
    int _turnoutsCount = 0;
    int _routesCount = 0;
    int _turntablesCount = 0;

    int _maxThrottles;
    bool server;
    Stream *stream;
    Stream *console;
    NullStream nullStream;

    int bufflen;
    char cmdBuffer[MAX_COMMAND_BUFFER];
	
    char outboundCommand[MAX_OUTBOUND_COMMAND_LENGTH];

    DCCEXProtocolDelegate *delegate = NULL;

    long lastServerResponseTime;
    
    char inputbuffer[512];    
    ssize_t nextChar;  // where the next character to be read goes in the buffer

    void init();

    void processCommand();

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
    void processReadLoco();

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
    char* nextServerDescriptionParam(int startAt, bool lookingAtVersionNumber);
};

#endif // DCCEXPROTOCOL_H