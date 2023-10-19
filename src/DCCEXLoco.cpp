#include <Arduino.h>
#include "DCCEXLoco.h"

// class Functions

bool Functions::initFunction(int functionNumber, char* label, FunctionLatching latching, FunctionState state) {
    functionLatching[functionNumber] = latching;
    functionState[functionNumber] = state;
    
    char *dynName;
    dynName = (char *) malloc(strlen(label)+1);
    sprintf(dynName,"%s",label);
    functionName[functionNumber] = dynName;
    return true;
}

bool Functions::setFunctionState(int functionNumber, FunctionState state) {
    functionState[functionNumber] = state;
    return true;
}

bool Functions::actionFunctionStateExternalChange(int functionNumber, FunctionState state) {
    //????????????????? TODO
    //  This may not be needed
    return true;
}

bool Functions::setFunctionName(int functionNumber,char* label) {
    if (functionName[functionNumber]!=nullptr) {
        free(functionName[functionNumber]);
        functionName[functionNumber]=nullptr; 
    }
    char *dynName;
    dynName = (char *) malloc(strlen(label)+1);
    sprintf(dynName,"%s",label);

    functionName[functionNumber] = dynName;
    return true;
}

char* Functions::getFunctionName(int functionNumber) {
    return functionName[functionNumber];
}

FunctionState Functions::getFunctionState(int functionNumber) {
    return functionState[functionNumber];
}

FunctionLatching Functions::getFunctionLatching(int functionNumber) {
    return functionLatching[functionNumber];
}

// private
bool Functions::clearFunctionNames() {
    for (uint i=0; i<MAX_FUNCTIONS; i++) {
        if (functionName[i] != nullptr) {
            free(functionName[i]);
            functionName[i] = nullptr; 
        }
    }
    return true;
}

// class Loco

Loco::Loco(int address, char* name, LocoSource source) {
    locoAddress = address;
    locoSource = source;
    locoDirection = Forward;
    locoSpeed = 0;
    rosterReceivedDetails = false;
    char fnName[MAX_OBJECT_NAME_LENGTH];

    char *dynName;
    dynName = (char *) malloc(strlen(name)+1);
    sprintf(dynName,"%s",name);
    locoName = dynName;

    for (uint i=0; i<MAX_FUNCTIONS; i++) {
        *fnName = 0;  // blank
        locoFunctions.initFunction(i, fnName, FunctionLatchingFalse, FunctionStateOff);
    }
}

bool Loco::isFunctionOn(int functionNumber) {
    return (locoFunctions.getFunctionState(functionNumber)==FunctionStateOn) ? true : false;
}

bool Loco::setLocoSpeed(int speed) {
    if (locoSpeed!=speed) {
        locoSpeed = speed;
        // sendLocoAction(locoAddress, speed, locoDirection);
    }
    locoSpeed = speed;
    return true;
}

bool Loco::setLocoDirection(Direction direction) {
    if (locoDirection!=direction) {
        locoDirection = direction;
    }
    locoDirection = direction;
    return true;
}

int Loco::getLocoAddress() {
    return locoAddress;
}

bool Loco::setLocoName(char* name) {
    if (locoName != nullptr) {
        free(locoName);
        locoName = nullptr; 
    }
    char *dynName;
    dynName = (char *) malloc(strlen(name)+1);
    sprintf(dynName,"%s",name);

    locoName = dynName;
    return true;
}

char* Loco::getLocoName() {
    return locoName;
}

bool Loco::setLocoSource(LocoSource source) {
    locoSource = source;
    return true;
}

LocoSource Loco::getLocoSource() {
    return locoSource;
}

int  Loco::getLocoSpeed() {
    return locoSpeed;
}

Direction Loco::getLocoDirection() {
    return locoDirection;
}

void Loco::setIsFromRosterAndReceivedDetails() {
    rosterReceivedDetails = true;
}

bool Loco::getIsFromRosterAndReceivedDetails() {
    if (locoSource==LocoSourceRoster && rosterReceivedDetails) {
        return true;
    }
    return false;
}

bool Loco::clearLocoNameAndFunctions() {
    if (locoName != nullptr) {
        free(locoName);
        locoName=nullptr; 
    }
    locoFunctions.clearFunctionNames();
    return true;
}

// class ConsistLoco : public Loco

ConsistLoco::ConsistLoco(int address, char* name, LocoSource source, Facing facing) 
: Loco::Loco(address, name, source) {
      consistLocoFacing = facing;
}

bool ConsistLoco::setConsistLocoFacing(Facing facing) {
    consistLocoFacing = facing;
    return true;
}

Facing ConsistLoco::getConsistLocoFacing() {
    return consistLocoFacing;
}

// class Consist

Consist::Consist(char* name) {
    char *_name;
    _name = (char *) malloc(strlen(name)+1);
    sprintf(_name,"%s",name);
    consistName = _name;
}

bool Consist::consistAddLoco(Loco loco, Facing facing) {
    int address = loco.getLocoAddress();
    char name[MAX_SINGLE_COMMAND_PARAM_LENGTH];
    sprintf(name,"%s",loco.getLocoName());
    LocoSource source = loco.getLocoSource();
    Facing correctedFacing = facing;
    int rslt = consistGetLocoPosition(address);
    if (rslt<0) { // not already in the list, so add it
        if (consistGetNumberOfLocos() == 0) correctedFacing = FacingForward; // first loco in consist is always forward
        consistLocos.add(new ConsistLoco(address, name, source, correctedFacing));

        //fix the name of the consist
        char _consistName[MAX_SINGLE_COMMAND_PARAM_LENGTH];
          sprintf(_consistName,"%s",consistLocos.get(0)->getLocoName());
        if (rslt>1) { // must be more than one now
            for (int i=1; i<consistLocos.size(); i++) {
                sprintf(_consistName,", %s",consistLocos.get(i)->getLocoName());
            }
            setConsistName(_consistName);
        }
        return true;
    }
    return false;
}

// create from a DCC Address
bool Consist::consistAddLocoFromRoster(LinkedList<Loco*> roster, int address, Facing facing) {
    if (roster.size()>0) { 
        for (int i=0; i<roster.size(); i++) {
            if (roster.get(i)->getLocoAddress() == address) {
                consistAddLoco(*roster.get(i), facing);
                return  true;
            }
        }
    }
    return false;
}

// create from a DCC Address
bool Consist::consistAddLocoFromAddress(int address, char* name, Facing facing) {
    Loco loco = Loco(address, name, LocoSourceEntry);
    consistAddLoco(loco, facing);
    return true;
}

bool Consist::consistReleaseAllLocos()  {
    if (consistLocos.size()>0) {
        for (int i=1; i<consistLocos.size(); i++) {
            consistLocos.get(i)->clearLocoNameAndFunctions();
        }
        consistLocos.clear();
        char _consistName[1];
        // strcpy(_consistName, "\0");
        *_consistName = 0;
        setConsistName(_consistName);
    }
    return true;
}

bool Consist::consistReleaseLoco(int locoAddress)  {
    int rslt = consistGetLocoPosition(locoAddress);
    if (rslt>=0) {
        consistLocos.get(rslt)->clearLocoNameAndFunctions();
        consistLocos.remove(rslt);
        return true;
    }
    return false;
}

int Consist::consistGetNumberOfLocos() {
    return consistLocos.size();
}

ConsistLoco* Consist::consistGetLocoAtPosition(int position) {
    if (position<consistLocos.size()) {
        return consistLocos.get(position);
    }
    return {};
}

int Consist::consistGetLocoPosition(int locoAddress) {
    for (int i=0; i<consistLocos.size(); i++) {
        if (consistLocos.get(i)->getLocoAddress() == locoAddress) {
            return i;
        }
    }
    return -1;
}

// set the position of the loco in the consist
// Assumes the loco is already in the consist
bool Consist::consistSetLocoPosition(int locoAddress, int position) {
    int currentPosition = consistGetLocoPosition(locoAddress);
    if (currentPosition < 0  || currentPosition == position)  {
        return false;
    } else {
        ConsistLoco* loco = consistGetLocoAtPosition(currentPosition);
        consistLocos.remove(currentPosition);
        int address = loco->getLocoAddress();
        char* name = loco->getLocoName();
        LocoSource source = loco->getLocoSource();
        Facing correctedFacing = loco->getConsistLocoFacing();
        // if (consistGetNumberOfLocos() == 0 || position == 0) {
        //     correctedFacing = FacingForward; // first loco in consist is always forward
        // }
        consistLocos.add(position, new ConsistLoco(address, name, source, correctedFacing));

        consistGetLocoAtPosition(0)->setConsistLocoFacing(FacingForward); // first loco in consist is always forward
    }
    return true;
}

bool Consist::consistSetSpeed(int speed) {
    if (consistLocos.size()>0) {
        if (consistSpeed!=speed) {
            for (int i=0; i<consistLocos.size(); i++) {
                consistLocos.get(i)->setLocoSpeed(speed);
            }
        }
    }
    consistSpeed = speed;
    return true;
}

int Consist::consistGetSpeed() {
    return consistSpeed;
}

bool Consist::consistSetDirection(Direction direction) {
    if (consistLocos.size()>0) {
        if (consistDirection!=direction) {
            for (int i=0; i<consistLocos.size(); i++) {
                Direction locoDir = direction;
                if (consistLocos.get(i)->getConsistLocoFacing()!=FacingForward) { // lead loco 'facing' is always assumed to be forward
                    if (direction == Forward) {
                        locoDir = Reverse;
                    } else {
                        locoDir = Forward;
                    }
                }
                consistLocos.get(i)->setLocoSpeed(consistSpeed);
                consistLocos.get(i)->setLocoDirection(locoDir);
            }
        }
    }
    consistDirection = direction;
    return true;
}

bool Consist::actionConsistExternalChange(int speed, Direction direction, FunctionState fnStates[]) {
    if (consistLocos.size()>0) {
        if ( (consistDirection != direction) || (consistSpeed != speed) ) {
            for (int i=0; i<consistLocos.size(); i++) {
                Direction locoDir = direction;
                if (consistLocos.get(i)->getConsistLocoFacing()!=FacingForward) { // lead loco 'facing' is always assumed to be forward
                    if (direction == Forward) {
                        locoDir = Reverse;
                    } else {
                        locoDir = Forward;
                    }
                }
                consistLocos.get(i)->setLocoSpeed(consistSpeed);
                consistLocos.get(i)->setLocoDirection(locoDir);

                //????????????????? TODO

                // if (i==0) {
                //     FunctionState fnStates[MAX_FUNCTIONS];
                //     for (uint i=0; i<MAX_FUNCTIONS; i++) {
                //         fnStates[i] = bitExtracted(fnStates,1,i+1);
                //         // if (fStates)
                //         //????????????????? TODO
                //     }
                // }
            }
        }
    }
    return true;
}

Direction Consist::consistGetDirection() {
    return consistDirection;
}

// by default only set the function on the lead loco
bool Consist::consistSetFunction(int functionNo, FunctionState state) {
    if (consistLocos.size()>0) {
        // ConsistLoco* loco = consistGetLocoAtPosition(0);
        //????????????????? TODO
        return true;
    }
    return false;
}

// set a function on a specific loco on a throttle
bool Consist::consistSetFunction(int address, int functionNo, FunctionState state) {
    //????????????????? TODO
    // individual loco
    return true;
}

bool Consist::isFunctionOn(int functionNumber) {
    if (consistLocos.size()>0) {
        ConsistLoco* loco = consistGetLocoAtPosition(0);
        return loco->isFunctionOn(functionNumber);
    }
    return false;
}

bool Consist::setConsistName(char* name) {
    if (consistName != nullptr) {
        free(consistName);
        consistName = nullptr; 
    }
    char *_name;
    _name = (char *) malloc(strlen(name)+1);
    sprintf(_name,"%s",name);
    consistName = _name;
    return true;
}

char* Consist::getConsistName() {
    return consistName;
}
