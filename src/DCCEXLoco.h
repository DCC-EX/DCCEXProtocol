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

/// @brief Class for a Loco object representing a DCC addressed locomotive
class Loco {
public:
  /// @brief Constructor
  /// @param address DCC address of loco
  /// @param source LocoSourceRoster (from roster) or LocoSourceEntry (from user input)
  Loco(int address, LocoSource source);

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

  /// @brief Get first Loco object
  /// @return Pointer to the first Loco object
  static Loco *getFirst();

  /// @brief Set the next loco in the roster list
  /// @param loco Pointer to the next Loco object
  void setNext(Loco *loco);

  /// @brief Get next Loco object
  /// @return Pointer to the next Loco object
  Loco *getNext();

  /// @brief Get Loco object by its DCC address
  /// @param address DCC address of the loco to get
  /// @return Loco object or nullptr if it doesn't exist
  static Loco *getByAddress(int address);

  /// @brief Clear all Locos from the roster
  static void clearRoster();

  /// @brief Destructor for the Loco object
  ~Loco();

private:
  int _address;
  char *_name;
  int _speed;
  Direction _direction;
  LocoSource _source;
  char *_functionNames[MAX_FUNCTIONS];
  int32_t _functionStates;
  int32_t _momentaryFlags;
  static Loco *_first;
  Loco *_next;

  /// @brief Method to remove this loco from the roster list
  /// @param loco Pointer to the Loco to remove
  static void _removeFromList(Loco *loco);

  friend class Consist;
};

/// @brief Class to add an additional attribute to a Loco object to specify the direction it is facing in a consist
class ConsistLoco {
public:
  /// @brief Constructor
  /// @param loco Pointer to the Loco object to add
  /// @param facing Direction loco is facing in the consist (FacingForward|FacingReversed)
  ConsistLoco(Loco *loco, Facing facing);

  /// @brief Get the associated Loco object for this consist entry
  /// @return Pointer to the Loco object
  Loco *getLoco();

  /// @brief Set which way the loco is facing in the consist (FacingForward, FacingReversed)
  /// @param facing FacingForward|FacingReversed
  void setFacing(Facing facing);

  /// @brief Get which way the loco is facing in the consist (FacingForward, FacingReversed)
  /// @return FacingForward|FacingReversed
  Facing getFacing();

  /// @brief Get the next consist loco object
  /// @return Pointer to the next ConsistLoco object
  ConsistLoco *getNext();

  /// @brief Set the next consist loco object
  /// @param consistLoco Pointer to the ConsistLoco object
  void setNext(ConsistLoco *consistLoco);

  /// @brief Destructor for a ConsistLoco
  ~ConsistLoco();

private:
  Loco *_loco;
  Facing _facing;
  ConsistLoco *_next;

  friend class Consist;
};

/// @brief Class to create a software consist of one or more ConsistLoco objects
class Consist {
public:
  /// @brief Constructor
  Consist();

  /// @brief Set consist name
  /// @param name Name to set for the consist
  void setName(const char *name);

  /// @brief Get consist name
  /// @return Current name of the consist
  const char *getName();

  /// @brief Add a loco to the consist using a Loco object
  /// @param loco Pointer to a loco object
  /// @param facing Direction the loco is facing (FacingForward|FacingReversed)
  void addLoco(Loco *loco, Facing facing);

  /// @brief Add a loco to the consist using a DCC address
  /// @param address DCC address of the loco to add
  /// @param facing Direction the loco is facing (FacingForward|FacingReversed)
  void addLoco(int address, Facing facing);

  /// @brief Remove a loco from the consist - Loco objects with LocoSourceEntry will also be deleted
  /// @param loco Pointer to a loco object to remove
  void removeLoco(Loco *loco);

  /// @brief Remove all locos from a consist - Loco objects with LocoSourceEntry will also be deleted
  void removeAllLocos();

  /// @brief Update the direction of a loco in the consist
  /// @param loco Pointer to the loco object to update
  /// @param facing Direction to set it to (FacingForward|FacingReversed)
  void setLocoFacing(Loco *loco, Facing facing);

  /// @brief Get the count of locos in the consist
  /// @return Count of locos
  int getLocoCount();

  /// @brief Check if the provided loco is in the consist
  /// @param loco Pointer to the loco object to check
  /// @return true|false
  bool inConsist(Loco *loco);

  /// @brief Check if the loco with the provided address is in the consist
  /// @param address DCC address of loco to check
  /// @return true|false
  bool inConsist(int address);

  /// @brief Get consist speed - obtained from first linked loco
  /// @return Current speed (0 - 126)
  int getSpeed();

  /// @brief Get consist direction - obtained from first linked loco
  /// @return Current direction (Forward|Reverse)
  Direction getDirection();

  /// @brief Get the first loco in the consist
  /// @return Pointer to the first ConsistLoco object
  ConsistLoco *getFirst();

  /// @brief Get the loco in the consist with the specified address
  /// @param address DCC address of loco to retrieve
  /// @return Pointer to the first ConsistLoco object
  ConsistLoco *getByAddress(int address);

  /// @brief Destructor for a Consist
  ~Consist();

private:
  char *_name;
  int _locoCount;
  ConsistLoco *_first;

  void _addLocoToConsist(ConsistLoco *consistLoco);
};

#endif