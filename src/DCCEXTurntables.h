#ifndef DCCEXTURNTABLES_H
#define DCCEXTURNTABLES_H

#include <Arduino.h>
#include <LinkedList.h>

enum TurntableState {
    TurntableStationary = 0,
    TurntableMoving = 1,
};

enum TurntableType {
    TurntableTypeDCC = 0,
    TurntableTypeEXTT = 1,
    TurntableTypeUnknown = 9,
};

class TurntableIndex {
    public:
        int turntableIndexId;
        int turntableIndexIndex;
        char* turntableIndexName;
        int turntableIndexAngle;
        bool hasReceivedDetail;

        TurntableIndex() {}
        TurntableIndex(int index, char* name, int angle);
        char* getTurntableIndexName();
        int getTurntableIndexId();
        int getTurntableIndexIndex();
        int getTurntableIndexAngle();
};

class Turntable {
    public:
        Turntable() {}
        // Turntable(int id, char* name, TurntableType type, int position, int indexCount);
        Turntable(int id, TurntableType type, int position, int indexCount);
        bool addTurntableIndex(int index, char* indexName, int indexAngle);
        LinkedList<TurntableIndex*> turntableIndexes = LinkedList<TurntableIndex*>();
        bool setTurntableIndexCount(int indexCount); // what was listed in the original definition
        int getTurntableIndexCount(); // what was listed in the original definition
 
        int getTurntableId();
        // bool setTurntableName(char* name);
        void setTurntableName(char* name);
        char* getTurntableName();
        bool setTurntableCurrentPosition(int index);
        bool setTurntableType(TurntableType type);
        TurntableType getTurntableType();
        int getTurntableCurrentPosition();
        int getTurntableNumberOfIndexes();
        TurntableIndex* getTurntableIndexAt(int positionInLinkedList);
        TurntableIndex* getTurntableIndex(int indexId);
        TurntableState getTurntableState();
        bool actionTurntableExternalChange(int index, TurntableState state);
        void setHasReceivedDetails();
        bool getHasReceivedDetails();
        void setHasReceivedIndexes();
        bool getHasReceivedIndexes();

    private:
        int turntableId;
        TurntableType turntableType;
        char* turntableName;
        int turntableCurrentPosition;
        bool turntableIsMoving;
        bool hasReceivedDetail;
        bool hasReceivedIndexes;
        int turnTableIndexCount; // what was listed in the original definition
};

#endif