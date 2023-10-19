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

typedef char FunctionState;
#define FunctionStateOff '0'
#define FunctionStateOn '1'

typedef char FunctionLatching;
#define FunctionLatchingTrue '1'
#define FunctionLatchingFalse '0'

enum LocoSource {
    LocoSourceRoster = 0,
    LocoSourceEntry = 1,
};

typedef char Facing;
#define FacingForward '0'
#define FacingReversed '1'

class Functions {
    public:
        bool initFunction(int functionNumber, char* label, FunctionLatching latching, FunctionState state);
        bool setFunctionState(int functionNumber, FunctionState state);
        bool setFunctionName(int functionNumber, char* label);
        char* getFunctionName(int functionNumber);
        FunctionState getFunctionState(int functionNumber);
        FunctionLatching getFunctionLatching(int functionNumber);
        bool clearFunctionNames();
    
    private:
        char* functionName[MAX_FUNCTIONS];
        FunctionState functionState[MAX_FUNCTIONS];
        int functionLatching[MAX_FUNCTIONS];

        bool actionFunctionStateExternalChange(int functionNumber, FunctionState state);
};

class Loco {
    public:
        Loco() {}
        Loco(int address, char* name, LocoSource source);
        Functions locoFunctions;
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

    private:
        int locoAddress;
        char* locoName;
        int locoSpeed;
        Direction locoDirection;
        LocoSource locoSource;
        bool rosterReceivedDetails;
};

class ConsistLoco : public Loco {
    public:
        ConsistLoco() {};
        ConsistLoco(int address, char* name, LocoSource source, Facing facing);
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
        bool consistAddLocoFromAddress(int address, char* name, Facing facing);
        bool consistReleaseAllLocos();
        bool consistReleaseLoco(int locoAddress);
        int consistGetNumberOfLocos();
        ConsistLoco* consistGetLocoAtPosition(int position);
        int consistGetLocoPosition(int locoAddress);
        bool consistSetLocoPosition(int locoAddress, int position);

        bool actionConsistExternalChange(int speed, Direction direction, FunctionState fnStates[]);

        bool consistSetSpeed(int speed);
        int consistGetSpeed();
        bool consistSetDirection(Direction direction);
        Direction consistGetDirection();
        bool consistSetFunction(int functionNo, FunctionState state);
        bool consistSetFunction(int address, int functionNo, FunctionState state);
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