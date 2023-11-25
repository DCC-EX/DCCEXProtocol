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

#include <Arduino.h>
#include "DCCEXLoco.h"

// class Loco
// Public methods

Loco* Loco::_first=nullptr;

Loco::Loco(int address, LocoSource source) {
  _address=address;
  _source=source;
  _direction=Forward;
  _speed=0;
  _name=nullptr;
  _functionStates=0;
  _momentaryFlags=0;
  _next=nullptr;
  if (!_first) {
    _first=this;
  } else {
    Loco* current=_first;
    while (current->_next!=nullptr) {
      current=current->_next;
    }
    current->_next=this;
  }
  _count++;
}

int Loco::getAddress() {
  return _address;
}

void Loco::setName(char* name) {
  _name=name;
}

char* Loco::getName() {
  return _name;
}

void Loco::setSpeed(int speed) {
  _speed=speed;
}

int Loco::getSpeed() {
  return _speed;
}

void Loco::setDirection(Direction direction) {
  _direction=direction;
}

Direction Loco::getDirection() {
  return (Direction)_direction;
}

LocoSource Loco::getSource() {
  return (LocoSource)_source;
}

void Loco::setupFunctions(char *functionNames) {
  // Importtant note: 
  // The functionNames string is modified in place. 
  //   console->print(F("Splitting \""));
  //   console->print(functionNames);
  //   console->println(F("\""));
  char * t=functionNames;
  int fkey=0;

  while(*t) {
    bool momentary=false;
    if(*t=='*')  {
      momentary=true;
      t++;
    }
    char * fName=t;  // function name starts here
    while(*t) { // loop completes at end of name ('/' or 0)
      if (*t=='/') {
      // found end of name
      *t='\0'; // mark name ends here 
      t++;
      break;
      }
      t++;
    }

    // At this point we have a function key
    // int fkey = function number 0....
    // bool momentary = is it a momentary
    // fName = pointer to the function name 
    _functionNames[fkey] = fName;
    if (momentary) {
      _momentaryFlags |= 1<<fkey;
    } else {
      _momentaryFlags &= ~1<<fkey;
    }
    //    console->print("Function ");
    //    console->print(fkey);
    //    console->print(momentary ? F("  Momentary ") : F(""));
    //    console->print(" ");
    //    console->println(fName);
    fkey++;
  }
}

bool Loco::functionOn(int function) {
  return _functionStates & 1<<function;
}

void Loco::setFunctionStates(int functionStates) {
  _functionStates=functionStates;
}

int Loco::getFunctionStates() {
  return _functionStates;
}

int Loco::getCount() {
  return _count;
}

Loco* Loco::getFirst() {
  return _first;
}

Loco* Loco::getNext() {
  return _next;
}

Loco* Loco::getByAddress(int address) {
  for (Loco* l=getFirst(); l; l=l->getNext()) {
    if (l->getAddress() == address) {
      return l;
    }
  }
  return nullptr;
}

// class ConsistLoco : public Loco
// Public methods

ConsistLoco::ConsistLoco(int address, LocoSource source, Facing facing)
: Loco::Loco(address, source) {
  _facing=facing;
  _next=nullptr;
}

void ConsistLoco::setFacing(Facing facing) {
  _facing=facing;
}

Facing ConsistLoco::getFacing() {
  return (Facing)_facing;
}

ConsistLoco* ConsistLoco::getNext() {
  return _next;
}

// class Consist
// Public methods

Consist::Consist() {
  _name=nullptr;
  _locoCount=0;
  _speed=0;
  _direction=Forward;
  _first=nullptr;
}

void Consist::setName(char* name) {
  _name=name;
}

char* Consist::getName() {
  return _name;
}

void Consist::addFromRoster(Loco* loco, Facing facing) {
  _addLoco(loco, facing);
}

void Consist::addFromEntry(int address, Facing facing) {
  Loco* loco=new Loco(address, LocoSourceEntry);
  _addLoco(loco, facing);
}

void Consist::releaseAll() {
  ConsistLoco* current=_first;
  while (current!=nullptr) {
    ConsistLoco* temp=current;
    current=current->_next;
    delete temp;  // Delete the ConsistLoco object
  }
  _first=nullptr; // Reset the linked list
  _locoCount=0; // Reset the loco count
}

void Consist::releaseLoco(int address) {
  ConsistLoco* current=_first;
  ConsistLoco* next=nullptr;

  while (current!=nullptr) {
    if (current->getAddress()==address) {
      if (next==nullptr) {
        // The matching ConsistLoco is the first one in the list
        _first=current->_next;
      } else {
        next->_next=current->_next;
      }

      delete current; // Delete the specific ConsistLoco object
      _locoCount--; // Decrement loco count
      return; // Exit the function once the loco is found and released
    }

    next=current;
    current=current->_next;
  }
}

int Consist::getLocoCount() {
  return _locoCount;
}

bool Consist::inConsist(int address) {
  for (ConsistLoco* cl=_first; cl; cl=cl->_next) {
    if (cl->getAddress()==address) {
      return true;
    }
  }
  return false;
}

void Consist::setSpeed(int speed) {
  _speed=speed;
}

int Consist::getSpeed() {
  return _speed;
}

void Consist::setDirection(Direction direction) {
  _direction=direction;
}

Direction Consist::getDirection() {
  return(Direction)_direction;
}

ConsistLoco* Consist::getFirst() {
  return _first;
}

// Private methods

void Consist::_addLoco(Loco* loco, Facing facing) {
  int address=loco->getAddress();
  LocoSource source=loco->getSource();
  if (inConsist(address)) return;  // Already in the consist
  if (_locoCount==0) {
    facing=FacingForward;  // Force forward facing for the first loco added
    _name=loco->getName();  // Set consist name to the first loco name
  }
  ConsistLoco* conLoco=new ConsistLoco(address, source, facing);
  if (this->_first==nullptr) {
    this->_first=conLoco;
  } else {
    ConsistLoco* current=this->_first;
    while (current->_next!=nullptr) {
      current=current->_next;
    }
    current->_next=conLoco;
  }
  _locoCount++;
}
