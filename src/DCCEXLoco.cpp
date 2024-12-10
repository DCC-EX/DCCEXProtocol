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

DCCEXLoco::DCCEXLoco(int address, bool inRoster) : _address(address) {
  if (inRoster) {
    _source = LocoSource::LocoSourceRoster;
  } else {
    _source = LocoSource::LocoSourceEntry;
  }
  for (int i = 0; i < MAX_FUNCTIONS; i++) {
    _functionNames[i] = nullptr;
  }
  _direction = Forward;
  _speed = 0;
  _name = nullptr;
  _functionStates = 0;
  _momentaryFlags = 0;
  _next = nullptr;
}

int DCCEXLoco::getAddress() { return _address; }

void DCCEXLoco::setName(const char *name) {
  if (_name) {
    free(_name);
  }
  _name = (char *)malloc(strlen(name) + 1);
  strcpy(_name, name);
}

const char *DCCEXLoco::getName() { return _name; }

void DCCEXLoco::setSpeed(int speed) { _speed = speed; }

int DCCEXLoco::getSpeed() { return _speed; }

void DCCEXLoco::setDirection(Direction direction) { _direction = direction; }

Direction DCCEXLoco::getDirection() { return (Direction)_direction; }

LocoSource DCCEXLoco::getSource() { return (LocoSource)_source; }

void DCCEXLoco::setupFunctions(const char *functionNames) {
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

bool DCCEXLoco::isFunctionOn(int function) { return _functionStates & 1 << function; }

void DCCEXLoco::setFunctionStates(int functionStates) { _functionStates = functionStates; }

int DCCEXLoco::getFunctionStates() { return _functionStates; }

const char *DCCEXLoco::getFunctionName(int function) { return _functionNames[function]; }

bool DCCEXLoco::isFunctionMomentary(int function) { return _momentaryFlags & 1 << function; }

DCCEXLoco *DCCEXLoco::getNext() { return _next; }

DCCEXLoco::~DCCEXLoco() {
  if (_name) {
    free(_name);
    _name = nullptr;
  }

  for (int i = 0; i < MAX_FUNCTIONS; i++) {
    if (_functionNames[i]) {
      free(_functionNames[i]);
    }
  }
  _next = nullptr;
}