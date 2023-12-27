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
    if (fkey < MAX_FUNCTIONS) {
      _functionNames[fkey] = fName;
      if (momentary) {
        _momentaryFlags |= 1<<fkey;
      } else {
        _momentaryFlags &= ~1<<fkey;
      }
    }
    //  Serial.print("Function ");
    //  Serial.print(fkey);
    //  Serial.print(momentary ? F("  Momentary ") : F(""));
    //  Serial.print(" ");
    //  Serial.println(fName);
    //  Serial.println(_functionNames[fkey]); 
    fkey++;
  }
  if (fkey<MAX_FUNCTIONS) {
    for (int i=fkey; i<MAX_FUNCTIONS; i++) {_functionNames[i] = nullptr;}
  }
}

bool Loco::isFunctionOn(int function) {
  return _functionStates & 1<<function;
}

void Loco::setFunctionStates(int functionStates) {
  _functionStates=functionStates;
}

int Loco::getFunctionStates() {
  return _functionStates;
}

char* Loco::getFunctionName(int function) {
  return _functionNames[function];
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

// class ConsistLoco
// Public methods

ConsistLoco::ConsistLoco(Loco* loco, Facing facing) {
  _loco=loco;
  _facing=facing;
  _next=nullptr;
}

Loco* ConsistLoco::getLoco() {
  return _loco;
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

void ConsistLoco::setNext(ConsistLoco* consistLoco) {
  _next=consistLoco;
}

// class Consist
// Public methods

Consist::Consist() {
  _name=nullptr;
  _locoCount=0;
  _first=nullptr;
}

void Consist::setName(char* name) {
  _name=name;
}

char* Consist::getName() {
  return _name;
}

void Consist::addLoco(Loco* loco, Facing facing) {
  if (inConsist(loco)) return;  // Already in the consist
  if (_locoCount==0) {
    facing=FacingForward;  // Force forward facing for the first loco added
    _name=loco->getName();  // Set consist name to the first loco name
  }
  ConsistLoco* conLoco=new ConsistLoco(loco, facing);
  _addLocoToConsist(conLoco);
}

void Consist::addLoco(int address, Facing facing) {
  if (inConsist(address)) return;
  if (_locoCount==0) {
    facing=FacingForward;
    char temp[6];
    snprintf(temp, 6, "%d", address);
    _name=temp;
  }
  Loco* loco=new Loco(address, LocoSourceEntry);
  ConsistLoco* conLoco=new ConsistLoco(loco, facing);
  _addLocoToConsist(conLoco);
}

void Consist::removeLoco(Loco* loco) {
  ConsistLoco* previous=nullptr;
  ConsistLoco* current=_first;
  while (current) {
    if (current->getLoco()==loco) {
      if (loco->getSource()==LocoSourceEntry) {
        delete loco;
      }
      if (previous) {
        previous->setNext(current->getNext());
      } else {
        _first=current->getNext();
      }
      delete current;
      _locoCount--;
      break;
    }
    previous=current;
    current=current->getNext();
  }
  if (!_first) {
    _first=nullptr;
    _locoCount = 0;
  }
}

void Consist::removeAllLocos() {
  ConsistLoco* current=_first;
  while (current) {
    ConsistLoco* next=current->getNext();
    Loco* loco=current->getLoco();
    if (loco->getSource()==LocoSourceEntry) {
      delete loco;
    }
    delete current;
    current=next;
  }
  _first=nullptr;
  _locoCount = 0;
}

void Consist::setLocoFacing(Loco* loco, Facing facing) {
  for (ConsistLoco* cl=_first; cl; cl=cl->getNext()) {
    if (cl->getLoco()==loco) {
      cl->setFacing(facing);
    }
  }
}

int Consist::getLocoCount() {
  return _locoCount;
}

bool Consist::inConsist(Loco* loco) {
  for (ConsistLoco* cl=_first; cl; cl=cl->_next) {
    if (cl->getLoco()==loco) {
      return true;
    }
  }
  return false;
}

bool Consist::inConsist(int address) {
  for (ConsistLoco* cl=_first; cl; cl=cl->_next) {
    if (cl->getLoco()->getAddress()==address) {
      return true;
    }
  }
  return false;
}

int Consist::getSpeed() {
  ConsistLoco* cl=_first;
  if (!cl) return 0;
  return cl->getLoco()->getSpeed();
}

Direction Consist::getDirection() {
  ConsistLoco* cl=_first;
  if (!cl) return Forward;
  return cl->getLoco()->getDirection();
}

ConsistLoco* Consist::getFirst() {
  return _first;
}

ConsistLoco* Consist::getByAddress(int address) {
  for (ConsistLoco* cl=_first; cl; cl=cl->_next) {
    if (cl->getLoco()->getAddress()==address) {
      return cl;
    }
  }
  return nullptr;
}

// Private methods

void Consist::_addLocoToConsist(ConsistLoco* conLoco) {
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
