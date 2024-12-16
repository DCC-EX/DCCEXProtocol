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

#include "DCCEXLoco.h"
#include <Arduino.h>

// class Loco
// Public methods

Loco *Loco::_first = nullptr;

Loco::Loco(int address, LocoSource source) : _address(address), _source(source) {
  for (int i = 0; i < MAX_FUNCTIONS; i++) {
    _functionNames[i] = nullptr;
  }
  _direction = Forward;
  _speed = 0;
  _name = nullptr;
  _functionStates = 0;
  _momentaryFlags = 0;
  _next = nullptr;
  if (_source == LocoSource::LocoSourceRoster) {
    if (!_first) {
      _first = this;
    } else {
      Loco *current = _first;
      while (current->_next != nullptr) {
        current = current->_next;
      }
      current->_next = this;
    }
  }
}

int Loco::getAddress() { return _address; }

void Loco::setName(const char *name) {
  if (_name) {
    delete[] _name;
    _name = nullptr;
  }
  int nameLength = strlen(name);
  _name = new char[nameLength + 1];
  strcpy(_name, name);
}

const char *Loco::getName() { return _name; }

void Loco::setSpeed(int speed) { _speed = speed; }

int Loco::getSpeed() { return _speed; }

void Loco::setDirection(Direction direction) { _direction = direction; }

Direction Loco::getDirection() { return (Direction)_direction; }

LocoSource Loco::getSource() { return (LocoSource)_source; }

void Loco::setupFunctions(const char *functionNames) {
  if (functionNames == nullptr) {
    return;
  }
  // Copy functionNames so we can clean up later
  char *fNames = new char[strlen(functionNames) + 1];
  if (fNames == nullptr) {
    return; // Bail out if malloc failed
  }
  strcpy(fNames, functionNames); // Copy names

  // Remove any existing names first
  for (int nameIndex = 0; nameIndex < MAX_FUNCTIONS; nameIndex++) {
    if (_functionNames[nameIndex] != nullptr) {
      delete[] _functionNames[nameIndex];
      _functionNames[nameIndex] = nullptr;
    }
  }

  int fNameIndex = 0;                // Index for each function name
  int fNamesLength = strlen(fNames); // Length of all names for sizing later
  int fNameStartChar = 0;            // Position of the first char in the name

  // Iterate through the fNames char array to look for names
  for (int charIndex = 0; charIndex <= fNamesLength; charIndex++) {
    // End of name is either / or null terminator
    // Start of name will be in char array index fNameStart
    if (fNames[charIndex] == '/' || fNames[charIndex] == '\0') {
      // Make sure we're at a sane index
      if (fNameIndex < MAX_FUNCTIONS) {
        bool momentary = false;
        // If start is *, it's momentary, name starts at following index
        if (fNames[fNameStartChar] == '*') {
          momentary = true;
          fNameStartChar++;
        }
        int nameLength = charIndex - fNameStartChar;           // Calculate length of name
        _functionNames[fNameIndex] = new char[nameLength + 1]; // Allocate mem + null terminator
        if (_functionNames[fNameIndex] != nullptr) {
          // Copy the name to the array index and null terminate it
          strncpy(_functionNames[fNameIndex], &fNames[fNameStartChar], nameLength);
          _functionNames[fNameIndex][nameLength] = '\0';
        }
        // Set the momentary flag
        if (momentary) {
          _momentaryFlags |= 1 << fNameIndex;
        } else {
          _momentaryFlags &= ~(1 << fNameIndex);
        }
        // Move to the next index
        fNameIndex++;
      } else {
        break;
      }
      fNameStartChar = charIndex + 1; // Calculate the start index of the next name
    }
  }
  delete[] fNames; // Clean up fNames
}

bool Loco::isFunctionOn(int function) { return _functionStates & 1 << function; }

void Loco::setFunctionStates(int functionStates) { _functionStates = functionStates; }

int Loco::getFunctionStates() { return _functionStates; }

const char *Loco::getFunctionName(int function) { return _functionNames[function]; }

bool Loco::isFunctionMomentary(int function) { return _momentaryFlags & 1 << function; }

Loco *Loco::getFirst() { return _first; }

void Loco::setNext(Loco *loco) { _next = loco; }

Loco *Loco::getNext() { return _next; }

Loco *Loco::getByAddress(int address) {
  for (Loco *l = getFirst(); l; l = l->getNext()) {
    if (l->getAddress() == address) {
      return l;
    }
  }
  return nullptr;
}

void Loco::clearRoster() {
  // Count Locos in roster
  int locoCount = 0;
  Loco *currentLoco = Loco::getFirst();
  while (currentLoco != nullptr) {
    locoCount++;
    currentLoco = currentLoco->getNext();
  }

  // Store Loco pointers in an array for clean up
  Loco **deleteLocos = new Loco *[locoCount];
  currentLoco = Loco::getFirst();
  for (int i = 0; i < locoCount; i++) {
    deleteLocos[i] = currentLoco;
    currentLoco = currentLoco->getNext();
  }

  // Delete each Loco
  for (int i = 0; i < locoCount; i++) {
    delete deleteLocos[i];
  }

  // Clean up the array of pointers
  delete[] deleteLocos;

  // Reset first pointer
  Loco::_first = nullptr;
}

Loco::~Loco() {
  _removeFromList(this);

  if (_name) {
    delete[] _name;
    _name = nullptr;
  }

  for (int i = 0; i < MAX_FUNCTIONS; i++) {
    if (_functionNames[i]) {
      delete[] _functionNames[i];
      _functionNames[i] = nullptr;
    }
  }

  _next = nullptr;
}

// Private methods

void Loco::_removeFromList(Loco *loco) {
  if (!loco) {
    return;
  }

  if (getFirst() == loco) {
    _first = loco->getNext();
  } else {
    Loco *currentLoco = _first;
    while (currentLoco && currentLoco->getNext() != loco) {
      currentLoco = currentLoco->getNext();
    }
    if (currentLoco) {
      currentLoco->setNext(loco->getNext());
    }
  }
}

// class ConsistLoco
// Public methods

ConsistLoco::ConsistLoco(Loco *loco, Facing facing) {
  _loco = loco;
  _facing = facing;
  _next = nullptr;
}

Loco *ConsistLoco::getLoco() { return _loco; }

void ConsistLoco::setFacing(Facing facing) { _facing = facing; }

Facing ConsistLoco::getFacing() { return (Facing)_facing; }

ConsistLoco *ConsistLoco::getNext() { return _next; }

void ConsistLoco::setNext(ConsistLoco *consistLoco) { _next = consistLoco; }

ConsistLoco::~ConsistLoco() {
  if (_loco && _loco->getSource() == LocoSource::LocoSourceEntry) {
    delete _loco;
    _loco = nullptr;
  }
  _next = nullptr;
}

// class Consist
// Public methods

Consist::Consist() {
  _name = nullptr;
  _locoCount = 0;
  _first = nullptr;
}

void Consist::setName(const char *name) {
  if (name == nullptr) {
    return;
  }
  if (_name) {
    delete[] _name;
    _name = nullptr;
  }
  int nameLength = strlen(name);
  _name = new char[nameLength + 1];
  if (_name == nullptr) {
    return;
  }
  strcpy(_name, name);
}

const char *Consist::getName() { return _name; }

void Consist::addLoco(Loco *loco, Facing facing) {
  if (inConsist(loco))
    return; // Already in the consist
  if (_locoCount == 0) {
    facing = FacingForward; // Force forward facing for the first loco added
    if (_name == nullptr) {
      setName(loco->getName()); // Set consist name to the first loco name if not already set
    }
  }
  ConsistLoco *conLoco = new ConsistLoco(loco, facing);
  _addLocoToConsist(conLoco);
}

void Consist::addLoco(int address, Facing facing) {
  if (inConsist(address))
    return;
  if (_locoCount == 0) {
    facing = FacingForward;
    if (_name == nullptr) {
      int addressLength = (address == 0) ? 1 : log10(address) + 1;
      char *newName = new char[addressLength + 1];
      snprintf(newName, addressLength + 1, "%d", address);
      setName(newName);
      delete[] newName;
    }
  }
  Loco *loco = new Loco(address, LocoSourceEntry);
  ConsistLoco *conLoco = new ConsistLoco(loco, facing);
  _addLocoToConsist(conLoco);
}

void Consist::removeLoco(Loco *loco) {
  // Start with no previous, and the first CL
  ConsistLoco *previousCL = nullptr;
  ConsistLoco *currentCL = _first;
  while (currentCL) {
    // If the currentCL is our loco to remove, process it
    if (currentCL->getLoco() == loco) {
      // If there is a previous, set its next to the current's next to skip currentCL
      ConsistLoco *nextCL = currentCL->getNext();
      if (previousCL) {
        previousCL->setNext(nextCL);
        // Otherwise the first is now the next
      } else {
        _first = nextCL;
      }
      // Delete the currentCL and decrement the count of locos
      delete currentCL;
      _locoCount--;
      currentCL = nextCL;
      // Otherwise move to the next one in the list
    } else {
      previousCL = currentCL;
      currentCL = currentCL->getNext();
    }
  }
  // When we're finished, if this was the last one, clean up
  if (!_first) {
    _first = nullptr;
    _locoCount = 0;
  }
}

void Consist::removeAllLocos() {
  // Clean up the linked list
  ConsistLoco *currentCL = _first;
  while (currentCL != nullptr) {
    // Capture the next one
    ConsistLoco *nextCL = currentCL->getNext();
    // Delete the current one
    delete currentCL;
    // Set next as current
    currentCL = nextCL;
  }
  // Set _first to nullptr
  _first = nullptr;
  _locoCount = 0;
}

void Consist::setLocoFacing(Loco *loco, Facing facing) {
  for (ConsistLoco *cl = _first; cl; cl = cl->getNext()) {
    if (cl->getLoco() == loco) {
      cl->setFacing(facing);
    }
  }
}

int Consist::getLocoCount() { return _locoCount; }

bool Consist::inConsist(Loco *loco) {
  for (ConsistLoco *cl = _first; cl; cl = cl->_next) {
    if (cl->getLoco() == loco) {
      return true;
    }
  }
  return false;
}

bool Consist::inConsist(int address) {
  for (ConsistLoco *cl = _first; cl; cl = cl->_next) {
    if (cl->getLoco()->getAddress() == address) {
      return true;
    }
  }
  return false;
}

int Consist::getSpeed() {
  ConsistLoco *cl = _first;
  if (!cl)
    return 0;
  return cl->getLoco()->getSpeed();
}

Direction Consist::getDirection() {
  ConsistLoco *cl = _first;
  if (!cl)
    return Forward;
  return cl->getLoco()->getDirection();
}

ConsistLoco *Consist::getFirst() { return _first; }

ConsistLoco *Consist::getByAddress(int address) {
  for (ConsistLoco *cl = _first; cl; cl = cl->_next) {
    if (cl->getLoco()->getAddress() == address) {
      return cl;
    }
  }
  return nullptr;
}

Consist::~Consist() {
  // Clean up the name
  if (_name) {
    delete[] _name;
    _name = nullptr;
  }

  // Clean up the linked list
  ConsistLoco *currentCL = _first;
  while (currentCL != nullptr) {
    // Capture the next one
    ConsistLoco *nextCL = currentCL->getNext();
    // Delete the current one
    delete currentCL;
    // Set next as current
    currentCL = nextCL;
  }
  // Set _first to nullptr
  _first = nullptr;
  _locoCount = 0;
}

// Private methods

void Consist::_addLocoToConsist(ConsistLoco *conLoco) {
  if (this->_first == nullptr) {
    this->_first = conLoco;
  } else {
    ConsistLoco *current = this->_first;
    while (current->_next != nullptr) {
      current = current->_next;
    }
    current->_next = conLoco;
  }
  _locoCount++;
}
