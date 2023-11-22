/* -*- c++ -*-
 *
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
 * This library is aimed at making thinges easier for throttle developers writing software for
 * Arduino based hardware throttles that wish to use DCC-EX EX-CommandStation native API
 * commands.
 * 
 * For more information, refer to the included README file, and the DCC-EX website.
 * https://www.dcc-ex.com
 */

#ifndef DCCEXPROTOCOL_H
#define DCCEXPROTOCOL_H

#include <Arduino.h>
#include "DCCEXInbound.h"
#include "DCCEXLoco.h"
#include "DCCEXRoutes.h"
#include "DCCEXTurnouts.h"
#include "DCCEXTurntables.h"

const int MAX_OUTBOUND_COMMAND_LENGTH=100;          // Max number of bytes for outbound commands
const int MAX_SERVER_DESCRIPTION_PARAM_LENGTH=100;  // Max number of bytes for <s> server details response
const int MAX_COMMAND_PARAMS=50;                    // Max number of params to parse via DCCEXInbound parser
const int MAX_COMMAND_BUFFER=500;                   // Max number of bytes for the inbound command buffer

// Valid track power state values
enum TrackPower {
    PowerOff = 0,
    PowerOn = 1,
    PowerUnknown = 2,
};

// Valid TrackManager types
enum TrackManagerMode {
  MAIN,   // Normal DCC track mode
  PROG,   // Programming DCC track mode
  DC,     // DC mode
  DCX,    // Reverse polarity DC mode
  OFF,    // Track is off
};

/// @brief Nullstream class for initial DCCEXProtocol instantiation
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

/// @brief Delegate responses and broadcast events to the client software to enable custom event handlers
class DCCEXProtocolDelegate {
public:
  /// @brief Notify when the server version has been received
  /// @param major Major version of EX-CommandStation (eg. 5.0.7 returns 5)
  /// @param minor Minor version of EX-CommandStation (eg. 5.0.7 returns 0)
  /// @param patch Patch version of EX-CommandStation (eg. 5.0.7 returns 7)
  virtual void receivedServerVersion(int major, int minor, int patch) {}

  /// @brief Notify when the roster list is received
  virtual void receivedRosterList() {}
  
  /// @brief Notify when the turnout list is received
  virtual void receivedTurnoutList() {}    
  
  /// @brief Notify when the route list is received
  virtual void receivedRouteList() {}
  
  /// @brief Notify when the turntable list is received
  virtual void receivedTurntableList() {}    

  /// @brief Notify when speed for a throttle is received
  /// @param throttleNo Number of the throttle (0 to maxThrottles - 1)
  /// @param speed Speed value (0 - 126)
  virtual void receivedSpeed(int throttleNo, int speed) {}
  
  /// @brief Notify when direction for a throttle is received
  /// @param throttleNo Number of the throttle (0 to maxThrottles - 1)
  /// @param dir Direction received (Forward|Reverse)
  virtual void receivedDirection(int throttleNo, Direction dir) {}
  
  /// @brief Notify when a function state change for a throttle is received
  /// @param throttleNo Number of the throttle (0 to maxThrottles - 1)
  /// @param func Function number (0 - 27)
  /// @param state On or off (true|false)
  virtual void receivedFunction(int throttleNo, int func, bool state) {}

  /// @brief Notify when a track power state change is received
  /// @param state Power state received (PowerOff|PowerOn|PowerUnknown)
  virtual void receivedTrackPower(TrackPower state) {}

  /// @brief Notify when a turnout state change is received
  /// @param turnoutId ID of the turnout
  /// @param thrown Wether it is thrown or not (true|false)
  virtual void receivedTurnoutAction(int turnoutId, bool thrown) {}

  /// @brief Notify when a turntable index change is received
  /// @param turntableId ID of the turntable
  /// @param position Index of the position it is moving (or has moved) to
  /// @param moving Whether it is moving or not (true|false)
  virtual void receivedTurntableAction(int turntableId, int position, bool moving) {}

  /// @brief Notify when a loco address is read from the programming track
  /// @param address DCC address read from the programming track, or -1 for a failure to read
  virtual void receivedReadLoco(int address) {}
};

/// @brief Main class for the DCCEXProtocol library
class DCCEXProtocol {
  public:
    // Protocol and server methods

    /// @brief Constructor for the DCCEXProtocol object
    /// @param maxThrottles The number of throttles to create, default is 6
    // DCCEXProtocol(int maxThrottles=6, bool server=false);
    DCCEXProtocol(int maxThrottles=6);

    /// @brief Set the delegate object for callbacks
    /// @param delegate 
    void setDelegate(DCCEXProtocolDelegate* delegate);
    
    /// @brief Set the stream object for console output
    /// @param console 
    void setLogStream(Stream* console);

    /// @brief Connect the stream object to interact with DCC-EX
    /// @param stream 
    void connect(Stream* stream);
    
    /// @brief Disconnect from DCC-EX
    void disconnect();

    /// @brief Check for incoming DCC-EX broadcasts/responses and parse them
    void check();

    /// @brief Check if server version has been received
    /// @return 
    bool receivedVersion();

    // Consist/Loco methods
    
    /// @brief Set the specified throttle to the provided speed and direction
    /// @param throttle The throttle containing the loco(s) to control (0 to number of throttles - 1)
    /// @param speed The speed (0 - 126)
    /// @param direction The direction (Forward|Reverse)
    void setThrottle(int throttle, int speed, Direction direction);
    
    /// @brief Set provided function on or off for the specified throttle
    /// @param throttle The throttle containing the loco(s) to control (0 to number of throttles - 1)
    /// @param functionNumber The number of the function (0 - 27)
    /// @param pressed True|False to turn the function on or off
    void setFunction(int throttle, int functionNumber, bool pressed);
    
    /// @brief Query if a specific function is on for the specified throttle
    /// @param throttle Throttle to query (0 to numThrottles - 1)
    /// @param functionNumber Function number (0 - 27)
    /// @return On or off (true|false)
    bool functionOn(int throttle, int functionNumber);
    
    /// @brief Explicitly request an update for the specified loco
    /// @param address DCC address of the loco
    void requestLocoUpdate(int address);
    
    /// @brief Initiate reading a loco address from the programming track, response will be a delegate notification
    void readLoco();


    

    

    Consist* throttle;
    Loco* roster=nullptr;
    Turnout* turnouts=nullptr;
    Route* routes=nullptr;
    Turntable* turntables=nullptr;

    //helper functions
    
    
    
    // *******************

    /// @brief Returns the Loco object for the specified address if found
    /// @param address 
    /// @return 
    Loco* findLocoInRoster(int address);
    
    

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
    // Methods
    void _sendCommand();
    void _setLoco(int address, int speed, Direction direction);
    Direction _getDirectionFromSpeedByte(int speedByte);
    int _getSpeedFromSpeedByte(int speedByte);
    int _getValidFunctionMap(int functionMap);

    // Attributes
    int _rosterCount = 0;
    int _turnoutsCount = 0;
    int _routesCount = 0;
    int _turntablesCount = 0;

    char* _serverDescription;
    int _majorVersion;
    int _minorVersion;
    int _patchVersion;

    int _maxThrottles;
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
    bool _receivedVersion = false;

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
    char* _nextServerDescriptionParam(int startAt, bool lookingAtVersionNumber);
};

#endif // DCCEXPROTOCOL_H