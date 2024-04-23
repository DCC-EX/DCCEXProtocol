/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
 *
 * Copyright © 2023 Chris Harlow
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

#ifndef DCCEXINBOUND_H
#define DCCEXINBOUND_H

#include <Arduino.h>

/* How to use this:
  1) setup is done once with your expected max parameter count.
  2) Call parse with your command input buffer.
    If it returns true... you have results.

  3) Use the get... functions to access the parameters.
  These parameters are ONLY VALID until you next call parse.
*/

/// @brief Inbound DCC-EX command parser class to parse commands and provide interpreted parameters
class DCCEXInbound {
public:
  /// @brief Setup parser once with enough space to handle the maximum number of
  ///  parameters expected from the command station.
  /// @param maxParameterValues Maximum parameter values to accommodate
  static void setup(int16_t maxParameterValues);

  /// @brief Cleanup parser
  static void cleanup();

  /// @brief Pass in a command string to parse
  /// @param command Char array of command to parse
  /// @return True if parsed ok, false if badly terminated command or too many parameters
  static bool parse(char *command);

  /// @brief Gets the DCC-EX OPCODE of the parsed command (the first char after the <)
  static byte getOpcode();

  /// @brief Gets number of parameters detected after OPCODE  <JR 1 2 3> is 4 parameters!
  /// @return Number of parameters
  static int16_t getParameterCount();

  /// @brief Gets a numeric parameter (or hashed keyword) from parsed command
  /// @return The numeric parameter
  static int32_t getNumber(int16_t parameterNumber);

  /// @brief Checks if a parameter is actually text rather than numeric
  /// @param parameterNumber The number of the parameter to check
  /// @return true|false
  static bool isTextParameter(int16_t parameterNumber);

  /// @brief Gets address of text type parameter.
  ///         does not create permanent copy
  /// @param parameterNumber The number of the parameter to retrieve
  /// @return Char array of text (use once and discard)
  static char *getTextParameter(int16_t parameterNumber);

  /// @brief gets address of a heap copy of text type parameter.
  /// @param parameterNumber
  /// @return
  static char *copyTextParameter(int16_t parameterNumber);

  /// @brief dump list of parameters obtained
  /// @param out Address of output e.g. &Serial
  static void dump(Print *);

private:
  static int16_t _maxParams;
  static int16_t _parameterCount;
  static byte _opcode;
  static int32_t *_parameterValues;
  static char *_cmdBuffer;
  static bool _isTextInternal(int16_t n);
};

#endif
