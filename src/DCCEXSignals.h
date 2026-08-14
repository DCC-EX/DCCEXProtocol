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

#ifndef DCCEXSIGNALS_H
#define DCCEXSIGNALS_H

#include <Arduino.h>

enum SignalState {
  SignalStateRed = 0,
  SignalStateAmber = 1,
  SignalStateGreen = 2,
  SignalStateInvalid = 3
};

const int InvalidAspect = 100;


/// @brief Class to contain and maintain the various Signal attributes and methods
class Signal {
public:
  /// @brief Constructor for a Signal object
  /// @param id signal ID
  Signal(int id);

  /// @brief Get signal Id
  /// @return ID of the signal
  int getId();

  /// @brief Set signal state
  /// @param state: SignalState to set for the signal
  void setState(SignalState state);

  /// @brief Get signal state
  /// @return state of the signal
  SignalState getState();

  /// @brief Set signal aspect
  /// @param aspect: aspect to set for the signal
  void setAspect(int aspect);

  /// @brief Get aspect state
  /// @return aspect of the signal
  int getAspect();

  /// @brief Set signal name
  /// @param _name Name to set the signal
  void setName(const char *_name);

  /// @brief Get signal name
  /// @return Current name of the signal
  const char *getName();

  /// @brief Get first signal object
  /// @return Pointer to the first Signal object
  static Signal *getFirst();

  /// @brief Get next signal object
  /// @return Pointer to the next signal object
  Signal *getNext();

  /// @brief Set the next signal in the list
  /// @param signal Pointer to the next Signal
  void setNext(Signal *signal);

  /// @brief Get signal object by signal ID
  /// @param id ID of the signal to retrieve
  /// @return Pointer to the signal object or nullptr if not found
  static Signal *getById(int id);

  /// @brief Clear the list of signals
  static void clearSignalList();

  /// @brief Destructor for a Signal
  ~Signal();

  /// @cond
  static SignalState getStateFromName(const char name);
  /// @endcond

private:
  static Signal *_first;
  Signal *_next;
  int _id;
  SignalState _state;
  int _aspect;
  char *_name;

  /// @brief Remove the signal from the list
  /// @param signal Pointer to the signal to remove
  void _removeFromList(Signal *signal);
};

#endif
