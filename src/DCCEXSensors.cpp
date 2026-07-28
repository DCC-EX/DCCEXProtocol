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

#include "DCCEXSensors.h"
#include <Arduino.h>

Sensor *Sensor::_first = nullptr;

Sensor::Sensor(int id, bool active) {
  _id = id;
  _active = active;
  _name = nullptr;
  _next = nullptr;
  if (!_first) {
    _first = this;
  } else {
    Sensor *current = _first;
    while (current->_next != nullptr) {
      current = current->_next;
    }
    current->_next = this;
  }
}

void Sensor::setActive(bool active) { _active = active; }

void Sensor::setName(const char *name) {
  if (_name) {
    delete[] _name;
    _name = nullptr;
  }
  int nameLength = strlen(name);
  _name = new char[nameLength + 1];
  strcpy(_name, name);
}

int Sensor::getId() { return _id; }

const char *Sensor::getName() { return _name; }

bool Sensor::getActive() { return _active; }

Sensor *Sensor::getFirst() { return _first; }

void Sensor::setNext(Sensor *sensor) { _next = sensor; }

Sensor *Sensor::getNext() { return _next; }

Sensor *Sensor::getById(int id) {
  for (Sensor *t = Sensor::getFirst(); t; t = t->getNext()) {
    if (t->getId() == id) {
      return t;
    }
  }
  return nullptr;
}

void Sensor::clearSensorList() {
  // Count Sensors in list
  int sensorCount = 0;
  Sensor *currentSensor = Sensor::getFirst();
  while (currentSensor != nullptr) {
    sensorCount++;
    currentSensor = currentSensor->getNext();
  }

  // Store Sensor pointers in an array for clean up
  Sensor **deleteSensors = new Sensor *[sensorCount];
  currentSensor = Sensor::getFirst();
  for (int i = 0; i < sensorCount; i++) {
    deleteSensors[i] = currentSensor;
    currentSensor = currentSensor->getNext();
  }

  // Delete each Sensor
  for (int i = 0; i < sensorCount; i++) {
    delete deleteSensors[i];
  }

  // Clean up the array of pointers
  delete[] deleteSensors;

  // Reset first pointer
  Sensor::_first = nullptr;
}

Sensor::~Sensor() {
  _removeFromList(this);

  if (_name) {
    delete[] _name;
    _name = nullptr;
  }

  _next = nullptr;
}

// Private methods

void Sensor::_removeFromList(Sensor *sensor) {
  if (!sensor) {
    return;
  }

  if (getFirst() == sensor) {
    _first = sensor->getNext();
  } else {
    Sensor *currentSensor = _first;
    while (currentSensor && currentSensor->getNext() != sensor) {
      currentSensor = currentSensor->getNext();
    }
    if (currentSensor) {
      currentSensor->setNext(sensor->getNext());
    }
  }
}
