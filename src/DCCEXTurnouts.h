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

#ifndef DCCEXTURNOUTS_H
#define DCCEXTURNOUTS_H

#include <Arduino.h>

/// @brief Class to contain and maintain the various Turnout/Point attributes and methods
class Turnout {
public:
  /// @brief Constructor for a Turnout object
  /// @param id Turnout ID
  /// @param thrown true (thrown)|false (closed)
  Turnout(int id, bool thrown);

  /// @brief Set thrown state (true thrown, false closed)
  /// @param thrown true|false
  void setThrown(bool thrown);

  /// @brief Set turnout name
  /// @param _name Name to set the turnout
  void setName(const char *_name);

  /// @brief Get turnout Id
  /// @return ID of the turnout
  int getId();

  /// @brief Get turnout name
  /// @return Current name of the turnout
  const char *getName();

  /// @brief Get thrown state (true thrown, false closed)
  /// @return true|false
  bool getThrown();

  /// @brief Get first turnout object
  /// @return Pointer to the first Turnout object
  static Turnout *getFirst();

  /// @brief Set the next turnout in the list
  /// @param turnout Pointer to the next Turnout
  void setNext(Turnout *turnout);

  /// @brief Get next turnout object
  /// @return Pointer to the next Turnout object
  Turnout *getNext();

  /// @brief Get turnout object by turnout ID
  /// @param id ID of the turnout to retrieve
  /// @return Pointer to the turnout object or nullptr if not found
  static Turnout *getById(int id);

  /// @brief Clear the list of turnouts
  static void clearTurnoutList();

  /// @brief Destructor for a Turnout
  ~Turnout();

private:
  static Turnout *_first;
  Turnout *_next;
  int _id;
  char *_name;
  bool _thrown;

  /// @brief Remove the turnout from the list
  /// @param turnout Pointer to the turnout to remove
  void _removeFromList(Turnout *turnout);
};

#endif