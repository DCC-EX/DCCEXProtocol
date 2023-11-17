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
#include "DCCEXTurntables.h"

// class TurntableIndex

TurntableIndex* TurntableIndex::_first=nullptr;

TurntableIndex::TurntableIndex(int ttId, int id, int angle, char* name) {
  _ttId=ttId;
  _id=id;
  _angle=angle;
  _name=name;
  _nextIndex=nullptr;
}

int TurntableIndex::getTTId() {
  return _ttId;
}

int TurntableIndex::getId() {
  return _id;
}

int TurntableIndex::getAngle() {
  return _angle;
}

char* TurntableIndex::getName() {
  return _name;
}

TurntableIndex* TurntableIndex::getNextIndex() {
  return _nextIndex;
}

// class Turntable

Turntable* Turntable::_first=nullptr;

Turntable::Turntable(int id) {
  _id=id;
  _type=TurntableTypeUnknown;
  _index=0;
  _numberOfIndexes=0;
  _name=nullptr;
  _firstIndex=nullptr;
  _indexCount=0;
  _next=nullptr;
  if (!_first) {
    _first=this;
  } else {
    Turntable* current=_first;
    while (current->_next!=nullptr) {
      current=current->_next;
    }
    current->_next=this;
  }
  _count++;
}

int Turntable::getId() {
  return _id;
}

void Turntable::setType(TurntableType type) {
  _type=type;
}

TurntableType Turntable::getType() {
  return (TurntableType)_type;
}

void Turntable::setIndex(int index) {
  _index=index;
}

int Turntable::getIndex() {
  return _index;
}

void Turntable::setNumberOfIndexes(int numberOfIndexes) {
  _numberOfIndexes=numberOfIndexes;
}

int Turntable::getNumberOfIndexes() {
  return _numberOfIndexes;
}

void Turntable::setName(char* name) {
  _name=name;
}

char* Turntable::getName() {
  return _name;
}

void Turntable::setMoving(bool moving) {
  _isMoving=moving;
}

bool Turntable::isMoving() {
  return _isMoving;
}

int Turntable::getCount() {
  return _count;
}

int Turntable::getIndexCount() {
  return _indexCount;
}

Turntable* Turntable::getFirst() {
  return _first;
}

Turntable* Turntable::getNext() {
  return _next;
}

void Turntable::addIndex(TurntableIndex* index) {
  if (this->_firstIndex==nullptr) {
    this->_firstIndex=index;
  } else {
    TurntableIndex* current=this->_firstIndex;
    while (current->_nextIndex!=nullptr) {
      current=current->_nextIndex;
    }
    current->_nextIndex=index;
  }
  _indexCount++;
}

TurntableIndex* Turntable::getFirstIndex() {
  return _firstIndex;
}

Turntable* Turntable::getById(int id) {
  for (Turntable* tt=getFirst(); tt; tt=tt->getNext()) {
    if (tt->getId()==id) {
      return tt;
    }
  }
  return nullptr;
}

TurntableIndex* Turntable::getIndexById(int id) {
  for (TurntableIndex* tti=getFirstIndex(); tti; tti=tti->getNextIndex()) {
    if (tti->getId()==id) {
      return tti;
    }
  }
  return nullptr;
}
