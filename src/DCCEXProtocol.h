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

    /// @brief Request DCC-EX object lists (Roster, Turnouts, Routes, Turntables)
    /// @param rosterRequired Request the roster list (true|false)
    /// @param turnoutListRequired Request the turnout list (true|false)
    /// @param routeListRequired Request the route list (true|false)
    /// @param turntableListRequired Request the turntable list (true|false)
    void getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired);

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

    // Roster methods

    /// @brief Get the number of roster entries
    /// @return Number of roster entries
    int getRosterCount();
    
    /// @brief 
    /// @param entryNo 
    /// @return 
    Loco* getRosterEntryNo(int entryNo);
    
    /// @brief Check if roster has been received
    /// @return true|false
    bool rosterReceived();

    // Turnout methods

    /// @brief Get the number of turnouts
    /// @return Number of turnouts defined
    int getTurnoutCount();
    
    /// @brief 
    /// @param entryNo 
    /// @return 
    Turnout* getTurnoutEntryNo(int entryNo);
    
    /// @brief Check if turnout list has been received
    /// @return true|false
    bool turnoutListReceived();
    

    

    

    //helper functions
    
    
    
    // *******************

    /// @brief Returns the Loco object for the specified address if found
    /// @param address 
    /// @return 
    Loco* findLocoInRoster(int address);
    
    

    // *******************

    /// @brief Request server details
    void sendServerDetailsRequest();

    
    
    
    
    
    
    int getRoutesCount();
    Route* getRoutesEntryNo(int entryNo);
    
    bool isRouteListFullyReceived();
    
    int getTurntablesCount();
    
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

    // Attributes

    Consist* throttle;              // Consist object for the throttle
    Loco* roster=nullptr;           // Linked list of locos for the roster
    Turnout* turnouts=nullptr;      // Linked list of turnouts
    Route* routes=nullptr;          // Linked list of routes
    Turntable* turntables=nullptr;  // Linked list of turntables

  private:
    // Methods
    void _sendCommand();
    void _setLoco(int address, int speed, Direction direction);
    Direction _getDirectionFromSpeedByte(int speedByte);
    int _getSpeedFromSpeedByte(int speedByte);
    int _getValidFunctionMap(int functionMap);
    void _getRoster();
    void _getTurnouts();
    void _getRoutes();
    void _getTurntables();
    bool isRosterRequested();
    bool isTurnoutListRequested();
    bool isRouteListRequested();
    bool isTurntableListRequested();
    
    // Attributes
    int _rosterCount = 0;     // Count of roster items received
    int _turnoutsCount = 0;   // Count of turnout objects received
    int _routesCount = 0;     // Count of route objects received
    int _turntablesCount = 0; // Count of turntable objects received
    char* _serverDescription; // Char array for EX-CommandStation server description <s>
    int _majorVersion;        // EX-CommandStation major version X.y.z
    int _minorVersion;        // EX-CommandStation minor version x.Y.z
    int _patchVersion;        // EX-CommandStation patch version x.y.Z
    int _maxThrottles;        // Number of throttles to support
    Stream* _stream;          // Stream object where commands are sent/received
    Stream* _console;         // Stream object for console output
    NullStream _nullStream;   // Send streams to null if no object provided
    int _bufflen;             // Used to ensure command buffer size not exceeded
    char _cmdBuffer[MAX_COMMAND_BUFFER];  // Char array for inbound command buffer
    char _outboundCommand[MAX_OUTBOUND_COMMAND_LENGTH]; // Char array for outbound commands
    DCCEXProtocolDelegate* _delegate = nullptr; // Pointer to the delegate for notifications
    long _lastServerResponseTime; // Records the timestamp of the last server response
    char _inputBuffer[512];   // Char array for input buffer
    ssize_t _nextChar;        // where the next character to be read goes in the buffer
    bool _receivedVersion = false;  // Flag that server version has been received
    bool _receivedLists = false;  // Flag if all requested lists have been received
    bool _rosterRequested = false;          // Flag that roster has been requested
    bool _rosterReceived = false;           // Flag that roster has been received
    bool _turnoutListRequested = false;     // Flag that turnout list requested
    bool _turnoutListReceived = false;      // Flag that turnout list received
    bool _routeListRequested = false;       // Flag that route list requested
    bool _routeListReceived = false;        // Flag that route list received
    bool _turntableListRequested = false;   // Flag that turntable list requested
    bool _turntableListReceived = false;    // Flag that turntable list received
    
    

    void init();

    void processCommand();

    void processServerDescription();	
    

    void processTrackPower();

    // *******************

    
    void processRosterEntry();
    void processRosterList();
    void sendRosterEntryRequest(int address);
    void processReadLoco();

    
    void processTurnoutEntry();
    void processTurnoutList();
    void processTurnoutAction();
    void sendTurnoutEntryRequest(int id);

    
    void processRouteList();
    void processRouteEntry();
    void sendRouteEntryRequest(int id);
    // void processRouteAction();

    
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