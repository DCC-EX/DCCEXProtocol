#include <Arduino.h>
#include "DCCEXTurntables.h"

// class TurntableIndex

TurntableIndex* TurntableIndex::_first=nullptr;

TurntableIndex::TurntableIndex(int id, int angle, char* name) {
  _id=id;
  _angle=angle;
  _name=name;
  _next=nullptr;
  _count++;
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

int TurntableIndex::getCount() {
  return _count;
}

TurntableIndex* TurntableIndex::getFirst() {
  return _first;
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
}

TurntableIndex* Turntable::getIndexList() {
  return _indexList;
}

// TurntableIndex::TurntableIndex(int index, char* name, int angle) {
//     turntableIndexIndex = index;
//     turntableIndexAngle = angle;

//     char *dynName;
//     dynName = (char *) malloc(strlen(name)+1);
//     // strcpy(dynName, name);
//     sprintf(dynName,"%s",name);

//     turntableIndexName = dynName;
// }
// char* TurntableIndex::getTurntableIndexName() {
//     return turntableIndexName;
// }
// int TurntableIndex::getTurntableIndexId() {
//     return turntableIndexId;
// }
// int TurntableIndex::getTurntableIndexIndex() {
//     return turntableIndexIndex;
// }

// int TurntableIndex::getTurntableIndexAngle() {
//     return turntableIndexAngle;
// }

// class Turntable

// Turntable::Turntable(int id, TurntableType type, int position, int indexCount) {
//     turntableId = id;
//     turntableType = type;
//     turntableCurrentPosition = position;
//     turnTableIndexCount = indexCount;
//     turntableName = nullptr;
//     hasReceivedDetail = false;
//     hasReceivedIndexes = false;
// }
// int Turntable::getTurntableId() {
//     return turntableId;
// }
// void Turntable::setTurntableName(char* name) {
//     turntableName = name;
// }
// char* Turntable::getTurntableName() {
//     return turntableName;
// }
// bool Turntable::setTurntableType(TurntableType type) {
//     turntableType = type;
//     return true;
// }
// TurntableType Turntable::getTurntableType() {
//     return turntableType;
// }
// bool Turntable::addTurntableIndex(int index, char* indexName, int indexAngle) {
//     turntableIndexes.add(new TurntableIndex(index, indexName, indexAngle));
//     return true;
// }
// bool Turntable::setTurntableCurrentPosition(int index) {
//     if (turntableCurrentPosition != index) {
//         turntableCurrentPosition = index;
//         turntableIsMoving = TurntableMoving;
//         return true;
//     }
//     return false;
// }    
// int Turntable::getTurntableCurrentPosition() {
//     return turntableCurrentPosition;
// }
// bool Turntable::setTurntableIndexCount(int indexCount) {  // what was listed in the original definition
//     turnTableIndexCount = indexCount;
//     return true;
// }
// int Turntable::getTurntableIndexCount() { // what was listed in the original definition
//     return turnTableIndexCount;
// }
// int Turntable::getTurntableNumberOfIndexes() { // actual count
//     return turntableIndexes.size();
// }
// TurntableIndex* Turntable::getTurntableIndexAt(int positionInLinkedList) {
//     return turntableIndexes.get(positionInLinkedList);
// }
// TurntableIndex* Turntable::getTurntableIndex(int indexId) {
//     for (int i=0; i<turntableIndexes.size(); i++) {
//         if (turntableIndexes.get(i)->getTurntableIndexId()==indexId) {
//             return turntableIndexes.get(i);
//         }
//     }
//     return {};
// }
// bool Turntable::actionTurntableExternalChange(int index, TurntableState state) {
//     turntableCurrentPosition = index;
//     turntableIsMoving = state;
//     return true;
// }
// TurntableState Turntable::getTurntableState() {
//     TurntableState rslt = TurntableStationary;
//     if (turntableIsMoving) {
//         rslt = TurntableMoving;
//     }
//     return rslt;
// }
// void Turntable::setHasReceivedDetails() {
//     hasReceivedDetail = true;
// }
// bool Turntable::getHasReceivedDetails() {
//     return hasReceivedDetail;
// }

// void Turntable::setHasReceivedIndexes() {
//     hasReceivedIndexes = true;
// }

// bool Turntable::getHasReceivedIndexes() {
//     return hasReceivedIndexes;
// }