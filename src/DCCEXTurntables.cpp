#include <Arduino.h>
#include "DCCEXTurntables.h"

// class TurntableIndex

TurntableIndex* TurntableIndex::_first=nullptr;

TurntableIndex::TurntableIndex(int ttId, int id, int angle, char* name) {
  _ttId=ttId;
  _id=id;
  _angle=angle;
  _name=name;
  _next=nullptr;
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

TurntableIndex* TurntableIndex::getNext() {
  return _next;
}

// class Turntable

Turntable* Turntable::_first=nullptr;

Turntable::Turntable(int id) {
  _id=id;
  _type=TurntableTypeUnknown;
  _index=0;
  _numberOfIndexes=0;
  _name=nullptr;
  _indexList=nullptr;
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
  if (this->_indexList==nullptr) {
    this->_indexList=index;
  } else {
    TurntableIndex* current=this->_indexList;
    while (current->_next!=nullptr) {
      current=current->_next;
    }
    current->_next=index;
  }
  _indexCount++;
}

TurntableIndex* Turntable::getIndexList() {
  return _indexList;
}