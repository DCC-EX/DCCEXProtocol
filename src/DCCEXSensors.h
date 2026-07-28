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

#ifndef DCCEXSENSORS_H
#define DCCEXSENSORS_H

#include <Arduino.h>

/// @brief Class to contain and maintain the various Sensor attributes and methods
class Sensor {
public:
  /// @brief Constructor for a Sensor object
  /// @param id sensor ID
  /// @param active true (active)|false (non-active)
  Sensor(int id, bool active);

  /// @brief Set active state (true active, false non-active)
  /// @param active true|false
  void setActive(bool active);

  /// @brief Set sensor name
  /// @param _name Name to set the sensor
  void setName(const char *_name);

  /// @brief Get sensor Id
  /// @return ID of the sensor
  int getId();

  /// @brief Get sensor name
  /// @return Current name of the sensor
  const char *getName();

  /// @brief Get active state
  /// @return true|false
  bool getActive();

  /// @brief Get first sensor object
  /// @return Pointer to the first Sensor object
  static Sensor *getFirst();

  /// @brief Set the next sensor in the list
  /// @param sensor Pointer to the next Sensor
  void setNext(Sensor *sensor);

  /// @brief Get next sensor object
  /// @return Pointer to the next sensor object
  Sensor *getNext();

  /// @brief Get sensor object by sensor ID
  /// @param id ID of the sensor to retrieve
  /// @return Pointer to the sensor object or nullptr if not found
  static Sensor *getById(int id);

  /// @brief Clear the list of sensor
  static void clearSensorList();

  /// @brief Destructor for a Sensor
  ~Sensor();

private:
  static Sensor *_first;
  Sensor *_next;
  int _id;
  char *_name;
  bool _active;

  /// @brief Remove the sensor from the list
  /// @param sensor Pointer to the sensor to remove
  void _removeFromList(Sensor *sensor);
};

#endif
