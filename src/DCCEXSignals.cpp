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

#include "DCCEXSignals.h"
#include <Arduino.h>

Signal *Signal::_first = nullptr;

Signal::Signal(int id) {
  _id = id;
  _state = SignalStateInvalid;
  _aspect = InvalidAspect;
  _next = nullptr;
  _name = nullptr;
  if (!_first) {
    _first = this;
  } else {
    Signal *current = _first;
    while (current->_next != nullptr) {
      current = current->_next;
    }
    current->_next = this;
  }
}

int Signal::getId() { return _id; }

void Signal::setState(SignalState state) { _state = state; }

SignalState Signal::getState() { return _state; }

void Signal::setAspect(int aspect) { _aspect = aspect; }

int Signal::getAspect() { return _aspect; }

void Signal::setName(const char *name) {
  if (!name)
    return;

  if (_name) {
    delete[] _name;
    _name = nullptr;
  }
  int nameLength = strlen(name);
  _name = new char[nameLength + 1];
  strcpy(_name, name);
}

const char *Signal::getName() { return _name; }

Signal *Signal::getFirst() { return _first; }

void Signal::setNext(Signal *signal) { _next = signal; }

Signal *Signal::getNext() { return _next; }

Signal *Signal::getById(int id) {
  for (Signal *t = Signal::getFirst(); t; t = t->getNext()) {
    if (t->getId() == id) {
      return t;
    }
  }
  return nullptr;
}

SignalState Signal::getStateFromName(const char name) {
  switch (name) {
    case 'R':
      return SignalStateRed;
    case 'A':
      return SignalStateAmber;
    case 'G':
      return SignalStateGreen;
    default:
      return SignalStateInvalid;
  }
}

void Signal::clearSignalList() {
  // Count Signals in list
  int signalCount = 0;
  Signal *currentSignal = Signal::getFirst();
  while (currentSignal != nullptr) {
    signalCount++;
    currentSignal = currentSignal->getNext();
  }

  // Store Signal pointers in an array for clean up
  Signal **deleteSignals = new Signal *[signalCount];
  currentSignal = Signal::getFirst();
  for (int i = 0; i < signalCount; i++) {
    deleteSignals[i] = currentSignal;
    currentSignal = currentSignal->getNext();
  }

  // Delete each Signal
  for (int i = 0; i < signalCount; i++) {
    delete deleteSignals[i];
  }

  // Clean up the array of pointers
  delete[] deleteSignals;

  // Reset first pointer
  Signal::_first = nullptr;
}

Signal::~Signal() {
  _removeFromList(this);
  _next = nullptr;

  if (_name) {
    delete[] _name;
    _name = nullptr;
  }
}

// Private methods
void Signal::_removeFromList(Signal *signal) {
  if (!signal) {
    return;
  }

  if (getFirst() == signal) {
    _first = signal->getNext();
  } else {
    Signal *currentSignal = _first;
    while (currentSignal && currentSignal->getNext() != signal) {
      currentSignal = currentSignal->getNext();
    }
    if (currentSignal) {
      currentSignal->setNext(signal->getNext());
    }
  }
}
