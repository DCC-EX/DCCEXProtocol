#ifndef DCCEXLOCO_H
#define DCCEXLOCO_H

#include <Arduino.h>
#include <LinkedList.h>

static const int MAX_FUNCTIONS = 28;
const int MAX_OBJECT_NAME_LENGTH = 30;  // including Loco name, Turnout/Point names, Route names, etc. names
#define MAX_SINGLE_COMMAND_PARAM_LENGTH 500  // Unfortunately includes the function list for an individual loco

enum Direction {
    Reverse = 0,
    Forward = 1,
};

// typedef char FunctionState;
// #define FunctionStateOff '0'
// #define FunctionStateOn '1'

// typedef char FunctionLatching;
// #define FunctionLatchingTrue '1'
// #define FunctionLatchingFalse '0'

enum LocoSource {
    LocoSourceRoster = 0,
    LocoSourceEntry = 1,
};

typedef char Facing;
#define FacingForward '0'
#define FacingReversed '1'

class Loco {
    public:
        Loco() {}
        Loco(int address, LocoSource source);
        bool isFunctionOn(int functionNumber);

        bool setLocoSpeed(int speed);
        bool setLocoDirection(Direction direction);
        
        int getLocoAddress();
        bool setLocoName(char* name);
        char* getLocoName();
        bool setLocoSource(LocoSource source);
        LocoSource getLocoSource();
        int  getLocoSpeed();
        Direction getLocoDirection();
        void setIsFromRosterAndReceivedDetails();
        bool getIsFromRosterAndReceivedDetails();
        bool clearLocoNameAndFunctions();
        void setupFunctions(char* functionNames);
        int getFunctionStates();
        void setFunctionStates(int functionStates);

    private:
        int locoAddress;
        char* locoName;
        int locoSpeed;
        Direction locoDirection;
        LocoSource locoSource;
        bool rosterReceivedDetails;
        char* _functionNames[MAX_FUNCTIONS];
        int32_t _functionStates;
        int32_t _momentaryFlags;
};

class ConsistLoco : public Loco {
    public:
        ConsistLoco() {};
        ConsistLoco(int address, LocoSource source, Facing facing);
        bool setConsistLocoFacing(Facing facing);
        Facing getConsistLocoFacing();

    private:
        Facing consistLocoFacing;
};

class Consist {
    public:
        Consist() {}
        Consist(char* name);
        bool consistAddLoco(Loco loco, Facing facing);
        bool consistAddLocoFromRoster(LinkedList<Loco*> roster, int address, Facing facing);
        bool consistAddLocoFromAddress(int address, Facing facing);
        bool consistReleaseAllLocos();
        bool consistReleaseLoco(int locoAddress);
        int consistGetNumberOfLocos();
        ConsistLoco* consistGetLocoAtPosition(int position);
        int consistGetLocoPosition(int locoAddress);
        bool consistSetLocoPosition(int locoAddress, int position);

        bool actionConsistExternalChange(int speed, Direction direction, int functionStates);

        bool consistSetSpeed(int speed);
        int consistGetSpeed();
        bool consistSetDirection(Direction direction);
        Direction consistGetDirection();
        bool consistSetFunction(int functionNo, bool state);
        bool consistSetFunction(int address, int functionNo, bool state);
        bool isFunctionOn(int functionNumber);
        bool setConsistName(char* name);
        char* getConsistName();
        LinkedList<ConsistLoco*> consistLocos = LinkedList<ConsistLoco*>();

    private:
        int consistSpeed;
        Direction consistDirection;
        char* consistName;
};

#endif