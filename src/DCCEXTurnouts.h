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

class Turnout {
public:
  /// @brief Constructor
  /// @param id Turnout ID
  /// @param thrown Bool for turnout thrown (0 closed, 1 thrown)
  Turnout(int id, bool thrown);
  
  /// @brief Set thrown state (1 thrown, 0 closed)
  /// @param isThrown 
  void setThrown(bool thrown);
  
  /// @brief Set turnout name
  /// @param _name 
  void setName(char* _name);

  /// @brief Get turnout Id
  /// @return 
  int getId();

  /// @brief Get turnout name
  /// @return 
  char* getName();

  /// @brief Get thrown state (1 thrown, 0 closed)
  /// @return 
  bool getThrown();

  /// @brief Get first turnout object
  /// @return 
  static Turnout* getFirst();

  /// @brief Get next turnout object
  /// @return 
  Turnout* getNext();

  /// @brief Get the number of turnouts
  /// @return 
  int getCount();

  /// @brief Get turnout object by turnout ID
  /// @param id 
  /// @return 
  Turnout* getById(int id);
  
private:
  static Turnout* _first;
  Turnout* _next;
  int _id;
  char* _name;
  bool _thrown;
  int _count=0;
  
};

#endif