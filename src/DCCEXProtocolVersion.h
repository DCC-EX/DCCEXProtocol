/* -*- c++ -*-
 *
 * Copyright © 2026 Peter Cole
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
 * This library is aimed at making things easier for throttle developers writing software for
 * Arduino based hardware throttles that wish to use DCC-EX EX-CommandStation native API
 * commands.
 *
 * For more information, refer to the included README file, and the DCC-EX website.
 * https://www.dcc-ex.com
 */

#ifndef DCCEXPROTOCOLVERSION_H
#define DCCEXPROTOCOLVERSION_H

#define DCCEX_PROTOCOL_VERSION "1.3.1"

/*
Version information:

1.3.1   - Fix bug where function 28 is masked off incorrectly and not received in Loco updates
1.3.0   - Introduce queued throttle updates to prevent buffer overloads and broadcast storms
        - Additional constructor attribute "userChangeDelay" enables user configuration of the queue time
        - Bug fix GitHub issue #39 sending unnecessary CRLF using println instead of print
        - Move version history to this file and expose via DCCEXProtocol::getLibraryVersion()
        - Major clean up of irrelevant dependencies on a DCCEXProtocolDelegate instance
        - Add missing guards when calling DCCEXProtocolDelegate methods if no instance is set
        - Add support for command station consists in favour of the current in-throttle consists
        - DEPRECATE existing in-throttle consists, these will be REMOVED in 2.0.0:
        - ALL current Consist METHODS and CLASSES are now deprecated
        - New CSConsist methods:
                - Accessible via csConsists->getFirst()
                - requestCSConsists()
                - createCSConsist(int leadLoco, bool reversed)
                - addCSConsistMember(CSConsist *csConsist, int address, bool reversed)
                - getCSConsistByLeadLoco(int address)
                - getCSConsistByLeadLoco(Loco *loco)
                - getCSConsistByMemberLoco(int address)
                - getCSConsistByMemberLoco(Loco *loco)
                - removeCSConsistMember(CSConsist *csConsist, int address)
                - deleteCSConsist(int leadLoco)
                - deleteCSConsist(CSConsist *csConsist)
                - clearCSConsists()
                - setThrottle(CSConsist *csConsist, int speed, Direction direction)
                - functionOn(CSConsist *csConsist, int function)
                - functionOff(CSConsist *csConsist, int function)
                - isFunctionOn(CSConsist *csConsist, int function)
        - DCCEXProtocolDelegate new method receivedCSConsist(int leadLoco, CSConsist *csConsist)
        - Add ability to replicate functions across CSConsist members (does not act like CV19/21/22)
                - CSConsist *csConsist = new CSConsist(true)
                - createCSConsist(int leadLoco, bool reversed, bool replicateFunctions)
                - When enabled, setting a function for the CSConsist sets the same across all members
        - Add support for momentum, new methods:
                - setMomentumAlgorithm(MomentumAlgorithm algorithm)
                - setDefaultMomentum(int momentum)
                - setDefaultMomentum(int accelerating, int braking)
                - setMomentum(int address, int momentum)
                - setMomentum(Loco *loco, int momentum)
                - setMomentum(int address, int accelerating, int braking)
                - setMomentum(Loco *loco, int accelerating, int braking)
        - Add method to enable/disable debug output to console, default is off (false)
                - setDebug(bool debug)
        - Add support for fast clock with new delegate methods:
                - setFastClock(int minutes, int speedFactor)
                - requestFastClockTime()
                - receivedSetFastClock(int minutes, int speedFactor)
                - receivedFastClockTime(int minutes)
        - Add support for track gauges (current limits) and track current:
                - requestTrackCurrentGauges()
                - requestTrackCurrents()
                - receivedTrackCurrentGauge(char track, int limit)
                - receivedTrackCurrent(char track, int current)
        - Fix compiler warnings by change char * to const char * in these methods:
                - sendCommand(const char *cmd)
                - receivedScreenUpdate(int screen, int row, const char *message)
1.2.1   - Refactor Consist::addLoco to use itoa instead of snprintf for Flash savings
        - Refactor all DCCEXProtocol outbound commands to remove sprintf
        - Add default true to getLists() so users can just call it without parameters to get all lists
        - Deprecate disconnect() method that does nothing and conflicts with user U command
        - Additional tests to improve future bug detection/breakages
        - Other non-functional tidy up changes
1.2.0   - Add loco hand off method handOffLoco(locoAddress, automationId)
        - Add readCV(cv) and validateCV(cv, value) methods with associated delegate method:
                receivedValidateCV(int cv, int value)
        - Add write loco address writeLocoAddress(address) with associated delegate method:
                receivedWriteLoco(int address)
        - Add validateCVBit(cv, bit, value) method with associated delegate method:
                receivedValidateCVBit(int cv, int bit, int value)
        - Add writeCV(cv, value) with delegate method receivedWriteCV(int cv, int value)
        - Add writeCVBit(cv, bit, value) - note there is no response for this due to parser limitations
        - Add writeCVOnMain(address, cv, value)
        - Add writeCVBitOnMain(address, cv, bit, value)
1.1.0   - Add new track power methods:
        - powerMainOn()/powerMainOff() - Control track power for MAIN track only
        - powerProgOn()/powerProgOff() - Control track power for PROG track only
        - joinProg() - Join PROG to MAIN
1.0.2   - No functional changes, updated examples to use receivedLocoBroadcast() for non-roster Loco objects
1.0.1   - Add additional receivedLocoBroadcast() delegate method to cater for non-roster updates
1.0.0   - First Production release
        - Add methods to clear and refresh the various lists
        - Various memory leak bugfixes
        - Fix bug where any Loco created was added to the roster, despite LocoSourceEntry being set
        - Fix bug where getById() for Turnout, Route, and Turntable was not a static method, causing runtime errors
        - Removed redundant count on Turnout, Route, and Turntable as these are available from getRosterCount,
                getTurnoutCount, getRouteCount, getTurntableCount
        - Updated all public methods setting and getting names from char * to const char * to remove compiler warnings
        - Enable configuring the max parameters parsed by DCCEXInbound via the DCCEXProtocol constructor
        - Implemented many new tests
0.0.17  - Fix typo in turntable example
        - Fix bug where the turntable isMoving() method always returned true
        - Add enableHeartbeat(heartbeatDelay) to send a heartbeat every x ms if a command is not sent
0.0.16  - add public sendCommand method
0.0.15  - any acquired loco is now retained in the roster
0.0.14  - add getNumberSupportedLocos()   used for the fake heartbeat
0.0.13  - Fix bug to allow compilation on AVR platforms, change ssize_t to int
        - Add serial connectivity example
        - Add support for SCREEN updates to delegate
        - Enhance buffer management to clear command buffer if full
0.0.12  - Improved memory management
0.0.11  - support for individual track power   receivedIndividualTrackPower(TrackPower state, int track)
        - improved logic for overall track power
0.0.10  - Add support for broadcast messages
0.0.9   - if loco is selected by address and that loco is in the roster (with the same DCC Address), updated and send
          speed commands for both
0.0.8   - No functional changes, add cross-platform and unit testing capabilities (credit to
          higaski)
0.0.7   - Add isFunctionMomentary(int function);
0.0.6   - Add getFunctionName(int function);
0.0.5   - Increase MAX_FUNCTIONS to 32.
        - Also add check to make sure the incoming does not exceed MAX_FUNCTIONS
0.0.4   - No functional changes, update author/maintainer and URL library properties
0.0.3   - Add getByAddress method to ConsistLoco
        - Fix bug when removing locos from a consist
        - Tidy setTrackType() method
0.0.2   - Add TrackManager configuration method and broadcast processing
        - Add TrackManager, SSID, and mDNS examples
0.0.1   - Initial library release via the Arduino Library Manager
*/

#endif // DCCEXPROTOCOLVERSION_H
