#include <Arduino.h>
#include "DCCEXTurntables.h"

// class TurntableIndex

TurntableIndex::TurntableIndex(int index, char* name, int angle) {
    turntableIndexIndex = index;
    turntableIndexAngle = angle;

    char *dynName;
    dynName = (char *) malloc(strlen(name)+1);
    // strcpy(dynName, name);
    sprintf(dynName,"%s",name);

    turntableIndexName = dynName;
}
char* TurntableIndex::getTurntableIndexName() {
    return turntableIndexName;
}
int TurntableIndex::getTurntableIndexId() {
    return turntableIndexId;
}
int TurntableIndex::getTurntableIndexIndex() {
    return turntableIndexIndex;
}

int TurntableIndex::getTurntableIndexAngle() {
    return turntableIndexAngle;
}

// class Turntable

Turntable::Turntable(int id, TurntableType type, int position, int indexCount) {
    turntableId = id;
    turntableType = type;
    turntableCurrentPosition = position;
    turnTableIndexCount = indexCount;
    turntableName = nullptr;
    hasReceivedDetail = false;
    hasReceivedIndexes = false;
}
int Turntable::getTurntableId() {
    return turntableId;
}
void Turntable::setTurntableName(char* name) {
    turntableName = name;
}
char* Turntable::getTurntableName() {
    return turntableName;
}
bool Turntable::setTurntableType(TurntableType type) {
    turntableType = type;
    return true;
}
TurntableType Turntable::getTurntableType() {
    return turntableType;
}
bool Turntable::addTurntableIndex(int index, char* indexName, int indexAngle) {
    turntableIndexes.add(new TurntableIndex(index, indexName, indexAngle));
    return true;
}
bool Turntable::setTurntableCurrentPosition(int index) {
    if (turntableCurrentPosition != index) {
        turntableCurrentPosition = index;
        turntableIsMoving = TurntableMoving;
        return true;
    }
    return false;
}    
int Turntable::getTurntableCurrentPosition() {
    return turntableCurrentPosition;
}
bool Turntable::setTurntableIndexCount(int indexCount) {  // what was listed in the original definition
    turnTableIndexCount = indexCount;
    return true;
}
int Turntable::getTurntableIndexCount() { // what was listed in the original definition
    return turnTableIndexCount;
}
int Turntable::getTurntableNumberOfIndexes() { // actual count
    return turntableIndexes.size();
}
TurntableIndex* Turntable::getTurntableIndexAt(int positionInLinkedList) {
    return turntableIndexes.get(positionInLinkedList);
}
TurntableIndex* Turntable::getTurntableIndex(int indexId) {
    for (int i=0; i<turntableIndexes.size(); i++) {
        if (turntableIndexes.get(i)->getTurntableIndexId()==indexId) {
            return turntableIndexes.get(i);
        }
    }
    return {};
}
bool Turntable::actionTurntableExternalChange(int index, TurntableState state) {
    turntableCurrentPosition = index;
    turntableIsMoving = state;
    return true;
}
TurntableState Turntable::getTurntableState() {
    TurntableState rslt = TurntableStationary;
    if (turntableIsMoving) {
        rslt = TurntableMoving;
    }
    return rslt;
}
void Turntable::setHasReceivedDetails() {
    hasReceivedDetail = true;
}
bool Turntable::getHasReceivedDetails() {
    return hasReceivedDetail;
}

void Turntable::setHasReceivedIndexes() {
    hasReceivedIndexes = true;
}

bool Turntable::getHasReceivedIndexes() {
    return hasReceivedIndexes;
}