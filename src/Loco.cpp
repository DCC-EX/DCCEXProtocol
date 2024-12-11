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

#include "Loco.h"
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

int Loco::getAddress() { return _address; }

void Loco::setName(char *name) {
  if (_name) {
    delete[] _name;
    _name = nullptr;
  }
  int nameLength = strlen(name);
  _name = new char[nameLength + 1];
  strcpy(_name, name);
}

char *Loco::getName() { return _name; }

void Loco::setSpeed(int speed) { _speed = speed; }

int Loco::getSpeed() { return _speed; }

void Loco::setDirection(Direction direction) { _direction = direction; }

Direction Loco::getDirection() { return (Direction)_direction; }

LocoSource Loco::getSource() { return (LocoSource)_source; }

void Loco::setupFunctions(char *functionNames) {
  if (functionNames == nullptr) {
    return;
  }
  // Copy functionNames so fNames can be freed later
  char *fNames = (char *)malloc(strlen(functionNames) + 1);
  if (fNames == nullptr) {
    return;
  }
  strcpy(fNames, functionNames);
  int fIndex = 0;
  int fNamesLength = strlen(fNames);
  int fNameStart = 0;
  // Free any existing function names
  for (int i = 0; i < MAX_FUNCTIONS; i++) {
    if (_functionNames[i] != nullptr) {
      free(_functionNames[i]);
      _functionNames[i] = nullptr;
    }
  }
  for (int i = 0; i <= fNamesLength; i++) {
    if (fNames[i] == '/' || fNames[i] == '\0') { // Check if it's the end of the name
      if (fIndex < MAX_FUNCTIONS) {              // Make sure it's a sane index
        if (_functionNames[fIndex] != nullptr) { // If it exists already, free it
          free(_functionNames[fIndex]);
        }
        bool momentary = false;
        if (fNames[fNameStart] == '*') { // If * it's momentary, and skip to next index for name
          momentary = true;
          fNameStart++;
        }
        int nameLength = i - fNameStart;                         // Calculate the length of the name
        _functionNames[fIndex] = (char *)malloc(nameLength + 1); // Allocate mem
        if (_functionNames[fIndex] != nullptr) {
          strncpy(_functionNames[fIndex], &fNames[fNameStart], nameLength); // Copy name
          _functionNames[fIndex][nameLength] = '\0';                        // Null terminate it
        }
        if (momentary) {
          _momentaryFlags |= 1 << fIndex;
        } else {
          _momentaryFlags &= ~(1 << fIndex);
        }
        fIndex++;
      } else {
        break;
      }
      fNameStart = i + 1; // Move to the next name
    }
  }
  free(fNames);
}

bool Loco::isFunctionOn(int function) { return _functionStates & 1 << function; }

void Loco::setFunctionStates(int functionStates) { _functionStates = functionStates; }

int Loco::getFunctionStates() { return _functionStates; }

char *Loco::getFunctionName(int function) { return _functionNames[function]; }

bool Loco::isFunctionMomentary(int function) { return _momentaryFlags & 1 << function; }

Loco *Loco::getFirst() { return _first; }

Loco *Loco::getNext() { return _next; }

Loco *Loco::getByAddress(int address) {
  for (Loco *l = getFirst(); l; l = l->getNext()) {
    if (l->getAddress() == address) {
      return l;
    }
  }
  return nullptr;
}

void Loco::setFirst(Loco *firstLoco) { _first = firstLoco; }

Loco::~Loco() {
  if (_name) {
    delete[] _name;
    _name = nullptr;
  }

  for (int i = 0; i < MAX_FUNCTIONS; i++) {
    if (_functionNames[i]) {
      free(_functionNames[i]);
    }
  }

  if (Loco::getFirst() == this) {
    Loco::setFirst(_next);
  } else {
    Loco *currentLoco = _first;
    while (currentLoco && currentLoco->_next != this) {
      currentLoco = currentLoco->_next;
    }
    if (currentLoco) {
      currentLoco->_next = _next;
    }
  }
  _next = nullptr;
}

// class ConsistLoco
// Public methods

ConsistLoco::ConsistLoco(Loco *loco, Facing facing) : _loco(loco), _facing(facing), _next(nullptr) {}

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

Consist::Consist() : _name(nullptr), _locoCount(0), _first(nullptr) {}

void Consist::setName(char *name) {
  if (name == nullptr) {
    return;
  }

  if (_name) {
    delete[] _name;
  }
  int nameLength = strlen(name);
  _name = new char[nameLength + 1];
  strcpy(_name, name);
}

char *Consist::getName() { return _name; }

void Consist::addLoco(Loco *loco, Facing facing) {
  if (inConsist(loco))
    return; // Already in the consist
  if (_locoCount == 0) {
    facing = FacingForward; // Force forward facing for the first loco added
    if (_name == nullptr) {
      setName(loco->getName()); // Set consist name to the first loco name if no consist name
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
    if (_name == nullptr) { // If no consist name, set to address of first loco
      int digits = (address == 0) ? 1 : log10(address) + 1;
      char *temp = new char[digits + 1];
      snprintf(temp, digits + 1, "%d", address);
      setName(temp);
      delete[] temp;
    }
  }
  Loco *loco = new Loco(address, LocoSourceEntry);
  ConsistLoco *conLoco = new ConsistLoco(loco, facing);
  _addLocoToConsist(conLoco);
}

void Consist::removeLoco(Loco *loco) {
  ConsistLoco *previous = nullptr;
  ConsistLoco *current = _first;
  while (current) {
    if (current->getLoco() == loco) {
      if (loco->getSource() == LocoSourceEntry) {
        // delete loco;
      }
      if (previous) {
        previous->setNext(current->getNext());
      } else {
        _first = current->getNext();
      }
      // delete current;
      _locoCount--;
      break;
    }
    previous = current;
    current = current->getNext();
  }
  if (!_first) {
    _first = nullptr;
    _locoCount = 0;
  }
}

void Consist::removeAllLocos() {
  ConsistLoco *current = _first;
  while (current) {
    ConsistLoco *next = current->getNext();
    Loco *loco = current->getLoco();
    if (loco->getSource() == LocoSourceEntry) {
      // delete loco;
    }
    // delete current;
    current = next;
  }
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
    // Store the next CL
    ConsistLoco *nextCL = currentCL->getNext();
    // Delete the current CL
    delete currentCL;
    // Move to the next CL
    currentCL = nextCL;
  }
  // Set _first to nullptr after cleanup
  _first = nullptr;
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
