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

#include "DCCEXTurnouts.h"
#include <Arduino.h>

Turnout *Turnout::_first = nullptr;

Turnout::Turnout(int id, bool thrown) {
  _id = id;
  _thrown = thrown;
  _name = nullptr;
  _next = nullptr;
  if (!_first) {
    _first = this;
  } else {
    Turnout *current = _first;
    while (current->_next != nullptr) {
      current = current->_next;
    }
    current->_next = this;
  }
}

void Turnout::setThrown(bool thrown) { _thrown = thrown; }

void Turnout::setName(const char *name) {
  if (_name) {
    delete[] _name;
    _name = nullptr;
  }
  int nameLength = strlen(name);
  _name = new char[nameLength + 1];
  strcpy(_name, name);
}

int Turnout::getId() { return _id; }

const char *Turnout::getName() { return _name; }

bool Turnout::getThrown() { return _thrown; }

Turnout *Turnout::getFirst() { return _first; }

void Turnout::setNext(Turnout *turnout) { _next = turnout; }

Turnout *Turnout::getNext() { return _next; }

Turnout *Turnout::getById(int id) {
  for (Turnout *t = Turnout::getFirst(); t; t = t->getNext()) {
    if (t->getId() == id) {
      return t;
    }
  }
  return nullptr;
}

void Turnout::clearTurnoutList() {
  // Count Turnouts in list
  int turnoutCount = 0;
  Turnout *currentTurnout = Turnout::getFirst();
  while (currentTurnout != nullptr) {
    turnoutCount++;
    currentTurnout = currentTurnout->getNext();
  }

  // Store Turnout pointers in an array for clean up
  Turnout **deleteTurnouts = new Turnout *[turnoutCount];
  currentTurnout = Turnout::getFirst();
  for (int i = 0; i < turnoutCount; i++) {
    deleteTurnouts[i] = currentTurnout;
    currentTurnout = currentTurnout->getNext();
  }

  // Delete each Turnout
  for (int i = 0; i < turnoutCount; i++) {
    delete deleteTurnouts[i];
  }

  // Clean up the array of pointers
  delete[] deleteTurnouts;

  // Reset first pointer
  Turnout::_first = nullptr;
}

Turnout::~Turnout() {
  _removeFromList(this);

  if (_name) {
    delete[] _name;
    _name = nullptr;
  }

  _next = nullptr;
}

// Private methods

void Turnout::_removeFromList(Turnout *turnout) {
  if (!turnout) {
    return;
  }

  if (getFirst() == turnout) {
    _first = turnout->getNext();
  } else {
    Turnout *currentTurnout = _first;
    while (currentTurnout && currentTurnout->getNext() != turnout) {
      currentTurnout = currentTurnout->getNext();
    }
    if (currentTurnout) {
      currentTurnout->setNext(turnout->getNext());
    }
  }
}
