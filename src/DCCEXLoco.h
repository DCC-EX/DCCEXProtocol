/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
 *
 * Copyright © 2024 Peter Cole
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

#ifndef DCCEXLOCO_H
#define DCCEXLOCO_H

#include <Arduino.h>

static const int MAX_FUNCTIONS = 32;
const int MAX_OBJECT_NAME_LENGTH = 30;      // including Loco name, Turnout/Point names, Route names, etc. names
#define MAX_SINGLE_COMMAND_PARAM_LENGTH 500 // Unfortunately includes the function list for an individual loco

enum Direction {
  Reverse = 0,
  Forward = 1,
};

enum LocoSource {
  LocoSourceRoster = 0,
  LocoSourceEntry = 1,
};

enum Facing {
  FacingForward = 0,
  FacingReversed = 1,
};

/// @brief Class for a DCCEXLoco object representing a DCC addressed locomotive
class DCCEXLoco {
public:
  /// @brief Constructor for a Loco instance
  /// @param address DCC address of this Loco
  /// @param inRoster true sets source to LocoSourceRoster, false sets source to LocoSourceEntry (default)
  DCCEXLoco(int address, bool inRoster = false);

  /// @brief Get loco address
  /// @return DCC address of loco
  int getAddress();

  /// @brief Set loco name
  /// @param name Name of the loco
  void setName(const char *name);

  /// @brief Get loco name
  /// @return Name of the loco
  const char *getName();

  /// @brief Set loco speed
  /// @param speed Valid speed (0 - 126)
  void setSpeed(int speed);

  /// @brief Get loco speed
  /// @return Speed (0 - 126)
  int getSpeed();

  /// @brief Set loco direction (enums Forward, Reverse)
  /// @param direction Direction to set (Forward|Reverse)
  void setDirection(Direction direction);

  /// @brief Get loco direction (enums Forward, Reverse)
  /// @return Current direction (Forward|Reverse)
  Direction getDirection();

  /// @brief Get loco source (enums LocoSourceRoster, LocoSourceEntry)
  /// @return Source of loco (LocoSourceRoster|LocoSourceEntry)
  LocoSource getSource();

  /// @brief Setup functions for the loco
  /// @param functionNames Char array of function names
  void setupFunctions(const char *functionNames);

  /// @brief Test if function is on
  /// @param function Number of the function to test
  /// @return true|false
  bool isFunctionOn(int function);

  /// @brief Set function states
  /// @param functionStates Integer representing all function states
  void setFunctionStates(int functionStates);

  /// @brief Get function states
  /// @return Integer representing current function states
  int getFunctionStates();

  /// @brief Get the name/label for a function
  /// @param function Number of the function to return the name/label of
  /// @return char* representing the function name/label
  const char *getFunctionName(int function);

  /// @brief Get the name/label for a function
  /// @param function Number of the function to return the name/label of
  /// @return char* representing the function name/label
  bool isFunctionMomentary(int function);

  /// @brief Get next Loco object
  /// @return Pointer to the next Loco object
  DCCEXLoco *getNext();

  /// @brief Destructor for a Loco
  ~DCCEXLoco();

private:
  int _address;
  char *_name;
  int _speed;
  Direction _direction;
  LocoSource _source;
  char *_functionNames[MAX_FUNCTIONS];
  int32_t _functionStates;
  int32_t _momentaryFlags;
  DCCEXLoco *_next;
};

#endif // DCCEXLOCO_H
