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

  /// @brief Notify when an update to a Loco object is received
  /// @param loco Pointer to the loco object
  virtual void receivedLocoUpdate(Loco* loco) {}
  
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
    DCCEXProtocol();

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

    /// @brief Request DCC-EX object lists (Roster, Turnouts, Routes, Turntables)
    /// @param rosterRequired Request the roster list (true|false)
    /// @param turnoutListRequired Request the turnout list (true|false)
    /// @param routeListRequired Request the route list (true|false)
    /// @param turntableListRequired Request the turntable list (true|false)
    void getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired);

    /// @brief Check if all lists have been received (roster, routes, turnouts, turntables)
    /// @return true|false
    bool receivedLists();

    /// @brief Request server version information
    void requestServerVersion();

    /// @brief Check if server version has been received
    /// @return 
    bool receivedVersion();

    /// @brief Retrieve the major version of EX-CommandStation
    /// @return Major version number eg. 5.y.z
    int getMajorVersion();

    /// @brief Retrieve the minor version of EX-Commandstation
    /// @return Minor version number eg. x.0.z
    int getMinorVersion();

    /// @brief Retreive the patch version of EX-CommandStation
    /// @return Patch version number eg. x.y.7
    int getPatchVersion();

    unsigned long getLastServerResponseTime();  // seconds since Arduino start

    // Consist/Loco methods
    
    /// @brief Set the provided loco to the specified speed and direction
    /// @param loco Pointer to a Loco object
    /// @param speed Speed (0 - 126)
    /// @param direction Direction (Forward|Reverse)
    void setThrottle(Loco* loco, int speed, Direction direction);

    /// @brief Set all locos in the provided consist to the specified speed and direction
    /// @param consist Pointer to a consist object
    /// @param speed Speed (0 - 126)
    /// @param direction Direction (Forward|Reverse) - reverse facing locos will be adjusted automatically
    void setThrottle(Consist* consist, int speed, Direction direction);

    /// @brief Turn the specified function on for the provided loco
    /// @param loco Pointer to a loco object
    /// @param function Function number (0 - 27)
    void functionOn(Loco* loco, int function);

    /// @brief Turn the specified function off for the provided loco
    /// @param loco Pointer to a loco object
    /// @param function Function number (0 - 27)
    void functionOff(Loco* loco, int function);

    /// @brief Test if the specified function for the provided loco is on
    /// @param loco Pointer to a loco object
    /// @param function Function number to test (0 - 27)
    /// @return true = on, false = off
    bool isFunctionOn(Loco* loco, int function);

    /// @brief Explicitly request an update for the specified loco
    /// @param address DCC address of the loco
    void requestLocoUpdate(int address);
    
    /// @brief Initiate reading a loco address from the programming track, response will be a delegate notification
    void readLoco();

    /// @brief Initiate an emergency stop
    void emergencyStop();

    // Roster methods

    /// @brief Get the number of roster entries
    /// @return Number of roster entries received
    int getRosterCount();
    
    /// @brief Check if roster has been received
    /// @return true|false
    bool receivedRoster();

    /// @brief Search for the specified DCC address in the roster
    /// @param address DCC address to search for
    /// @return Pointer to the Loco object
    Loco* findLocoInRoster(int address);

    // Turnout methods

    /// @brief Get the number of turnouts
    /// @return Number of turnouts received
    int getTurnoutCount();
    
    /// @brief Check if turnout list has been received
    /// @return true|false
    bool receivedTurnoutList();

    /// @brief Retrieve a turnout/point object by its ID
    /// @param turnoutId ID of the turnout/point
    /// @return The turnout/point object
    Turnout* getTurnoutById(int turnoutId);
    
    /// @brief Close the specified turnout/point
    /// @param turnoutId ID of the turnout/point
    void closeTurnout(int turnoutId);
    
    /// @brief Throw the specified turnout/point
    /// @param turnoutId ID of the turnout/point
    void throwTurnout(int turnoutId);
    
    /// @brief Toggle the specified turnout/point (if closed, will throw, and vice versa)
    /// @param turnoutId ID of the turnout/point
    void toggleTurnout(int turnoutId);
    
    // Route methods

    /// @brief Get the number of route entries
    /// @return Number of routes received
    int getRouteCount();
    
    /// @brief Check if route list has been received
    /// @return true|false
    bool receivedRouteList();

    /// @brief Start a route/automation
    /// @param routeId ID of the route/automation to start
    void startRoute(int routeId);
    
    /// @brief Pause all routes/automations
    void pauseRoutes();
    
    /// @brief Resume all routes/automations
    void resumeRoutes();

    // Turntable methods
    
    /// @brief Get the number of turntable entries
    /// @return Number of turntables received
    int getTurntableCount();
    
    /// @brief Check if turntable list has been received
    /// @return true|false
    bool receivedTurntableList();

    /// @brief Retrieve a turntable object by its ID
    /// @param turntableId ID of the turntable
    /// @return The turntable object
    Turntable* getTurntableById(int turntableId);

    /// @brief Rotate a turntable object
    /// @param turntableId ID of the turntable
    /// @param position Position index to rotate to
    /// @param activity Optional activity for EX-Turntable objects only
    void rotateTurntable(int turntableId, int position, int activity=0);
    
    // Track management methods
	  
    /// @brief Global track power on command
    void powerOn();

    /// @brief Global track power off command
    void powerOff();

    /// @brief Turn power on for the specified track
    /// @param track Track name (A - H)
    void powerTrackOn(char track);

    /// @brief Turn power off for the specified track
    /// @param track Track name (A - H)
    void powerTrackOff(char track);
    
    // DCC accessory methods

    /// @brief Activate DCC accessory at the specified address and subaddress
    /// @param accessoryAddress Address of the DCC accessory
    /// @param accessorySubAddr Subaddress of the DCC accessory
    void activateAccessory(int accessoryAddress, int accessorySubAddr);

    /// @brief Deactivate DCC accessory at the specified address and subaddress
    /// @param accessoryAddress Address of the DCC accessory
    /// @param accessorySubAddr Subaddress of the DCC accessory
    void deactivateAccessory(int accessoryAddress, int accessorySubAddr);
    
    /// @brief Activate DCC accessory at the specified linear address
    /// @param linearAddress Linear address of the DCC accessory
    void activateLinearAccessory(int linearAddress);

    /// @brief Deactivate DCC accessory at the specified linear address
    /// @param linearAddress Linear address of the DCC accessory
    void deactivateLinearAccessory(int linearAddress);

    // Attributes

    Loco* roster=nullptr;           // Linked list of locos for the roster
    Turnout* turnouts=nullptr;      // Linked list of turnouts
    Route* routes=nullptr;          // Linked list of routes
    Turntable* turntables=nullptr;  // Linked list of turntables

  private:
    // Methods
    // Protocol and server methods
    void _init();
    void _sendCommand();
    void _processCommand();
    void _processServerDescription();
    char* _nextServerDescriptionParam(char* description, int startAt, bool lookingAtVersionNumber);

    // Consist/loco methods
    void _processLocoBroadcast();
    int _getValidFunctionMap(int functionMap);
    int _getSpeedFromSpeedByte(int speedByte);
    Direction _getDirectionFromSpeedByte(int speedByte);
    void _setLoco(int address, int speed, Direction direction);
    void _processReadResponse();

    // Roster methods
    void _getRoster();
    bool _requestedRoster();
    void _processRosterList();
    void _requestRosterEntry(int address);
    void _processRosterEntry();

    // Turnout methods
    void _getTurnouts();
    bool _requestedTurnouts();
    void _processTurnoutList();
    void _requestTurnoutEntry(int id);
    void _processTurnoutEntry();
    void _processTurnoutBroadcast();
    
    // Route methods
    void _getRoutes();
    bool _requestedRoutes();
    void _processRouteList();
    void _requestRouteEntry(int id);
    void _processRouteEntry();

    // Turntable methods
    void _getTurntables();
    bool _requestedTurntables();
    void _processTurntableList();
    void _requestTurntableEntry(int id);
    void _processTurntableEntry();
    void _requestTurntableIndexEntry(int id);
    void _processTurntableIndexEntry();
    void _processTurntableBroadcast();    

    // Track management methods
    void _processTrackPower();
    
    // Attributes
    int _rosterCount=0;                 // Count of roster items received
    int _turnoutCount=0;                // Count of turnout objects received
    int _routeCount=0;                  // Count of route objects received
    int _turntableCount=0;              // Count of turntable objects received
    int _majorVersion=0;                // EX-CommandStation major version X.y.z
    int _minorVersion=0;                // EX-CommandStation minor version x.Y.z
    int _patchVersion=0;                // EX-CommandStation patch version x.y.Z
    Stream* _stream;                    // Stream object where commands are sent/received
    Stream* _console;                   // Stream object for console output
    NullStream _nullStream;             // Send streams to null if no object provided
    int _bufflen;                       // Used to ensure command buffer size not exceeded
    char _cmdBuffer[MAX_COMMAND_BUFFER];  // Char array for inbound command buffer
    char _outboundCommand[MAX_OUTBOUND_COMMAND_LENGTH]; // Char array for outbound commands
    DCCEXProtocolDelegate* _delegate=nullptr; // Pointer to the delegate for notifications
    unsigned long _lastServerResponseTime; // Records the timestamp of the last server response
    char _inputBuffer[512];             // Char array for input buffer
    ssize_t _nextChar;                  // where the next character to be read goes in the buffer
    bool _receivedVersion=false;        // Flag that server version has been received
    bool _receivedLists=false;          // Flag if all requested lists have been received
    bool _rosterRequested=false;        // Flag that roster has been requested
    bool _receivedRoster=false;         // Flag that roster has been received
    bool _turnoutListRequested=false;   // Flag that turnout list requested
    bool _receivedTurnoutList=false;    // Flag that turnout list received
    bool _routeListRequested=false;     // Flag that route list requested
    bool _receivedRouteList=false;      // Flag that route list received
    bool _turntableListRequested=false; // Flag that turntable list requested
    bool _receivedTurntableList=false;  // Flag that turntable list received

};

#endif // DCCEXPROTOCOL_H
