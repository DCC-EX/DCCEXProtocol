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

/* how to use this:
   1) setup is done once with your expected max parameter count.
   2) Call parse with your command input buffer.
      If it returns trua... you have results.
      
    3) Use the get... functions to access the parameters.
    These parameters are ONLY VALID until you next call parse.  */
class DCCEXInbound {
public: 
   /// @brief setup parser once with enough space to handle the maximum number of 
   ///  parameters expected from the command station.
   /// @param maxParameterValues 
   static void setup(int16_t maxParameterValues);

   /// @brief  pass in a command string to parse
   /// @param command 
   /// @return true if parsed ok, false if badly terminated command or too many parameters
   static bool parse(char* command);

   /// @brief gets the dccex opcode of parsed command (the first char after the <)
   static byte getOpcode();

   /// @brief gets number of parameters detected after opcode  <JR 1 2 3> is 4 parameters!
   /// @return 
   static int16_t getParameterCount();

   /// @brief   gets a numeric parameter (or hashed keyword) from parsed command
   static int32_t getNumber(int16_t parameterNumber);

   /// @brief checks if a parameter is actually text rather than numeris 
   /// @param parameterNumber 
   /// @return 
   static bool isTextParameter(int16_t parameterNumber);

   /// @brief gets address of text type parameter.
   ///         does not create permenant copy 
   /// @param parameterNumber 
   /// @return 
   static char * getText(int16_t parameterNumber);
   
   /// @brief gets address of a heap copy of text type parameter.
   /// @param parameterNumber 
   /// @return 
   static char * getSafeText(int16_t parameterNumber);

   /// @brief dump list of parameters obtained
   /// @param  Address of output e.g. &Serial
   static void dump(Print *);

private:
    static int16_t maxParams;
    static int16_t parameterCount;
    static byte opcode;
    static int32_t * parameterValues;
    static char* cmdBuffer;  
    static bool isTextInternal(int16_t n);
};

#endif
