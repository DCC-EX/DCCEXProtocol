#include <Arduino.h>
#include "DCCEXLoco.h"

// class Loco

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

bool Loco::isFunctionOn(int function) {
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


/* OLD LOCO
// Loco::Loco(int address, LocoSource source) {
//     locoAddress = address;
//     locoSource = source;
//     locoDirection = Forward;
//     locoSpeed = 0;
//     rosterReceivedDetails = false;
//     locoName = nullptr;
//     _functionStates = 0;
//     _momentaryFlags = 0;
// }

// bool Loco::isFunctionOn(int functionNumber) {
//     return _functionStates & 1<<functionNumber;
// }

// bool Loco::setLocoSpeed(int speed) {
//     if (locoSpeed!=speed) {
//         locoSpeed = speed;
//         // sendLocoAction(locoAddress, speed, locoDirection);
//     }
//     locoSpeed = speed;
//     return true;
// }

// bool Loco::setLocoDirection(Direction direction) {
//     if (locoDirection!=direction) {
//         locoDirection = direction;
//     }
//     locoDirection = direction;
//     return true;
// }

// int Loco::getLocoAddress() {
//     return locoAddress;
// }

// bool Loco::setLocoName(char* name) {
//     locoName = name;
//     return true;
// }

// char* Loco::getLocoName() {
//     return locoName;
// }

// bool Loco::setLocoSource(LocoSource source) {
//     locoSource = source;
//     return true;
// }

// LocoSource Loco::getLocoSource() {
//     return locoSource;
// }

// int  Loco::getLocoSpeed() {
//     return locoSpeed;
// }

// Direction Loco::getLocoDirection() {
//     return locoDirection;
// }

// void Loco::setIsFromRosterAndReceivedDetails() {
//     rosterReceivedDetails = true;
// }

// bool Loco::getIsFromRosterAndReceivedDetails() {
//     if (locoSource==LocoSourceRoster && rosterReceivedDetails) {
//         return true;
//     }
//     return false;
// }

// bool Loco::clearLocoNameAndFunctions() {
//     if (locoName != nullptr) {
//         free(locoName);
//         locoName=nullptr; 
//     }
//     // locoFunctions.clearFunctionNames();
//     for (uint i=0; i<MAX_FUNCTIONS; i++) {
//         if (_functionNames[i] != nullptr) {
//             free(_functionNames[i]);
//             _functionNames[i] = nullptr;
//         }
//     }
//     return true;
// }

// void Loco::setupFunctions(char *functionNames) {
//     // Importtant note: 
//     // The functionNames string is modified in place. 
//     //   console->print(F("Splitting \""));
//     //   console->print(functionNames);
//     //   console->println(F("\""));
//     char * t=functionNames;
//     int fkey=0;
    
//     while(*t) {
//         bool momentary=false;
//         if(*t=='*')  {
//             momentary=true;
//             t++;
//         }
//         char * fName=t;  // function name starts here
//         while(*t) { // loop completes at end of name ('/' or 0)
//             if (*t=='/') {
//             // found end of name
//             *t='\0'; // mark name ends here 
//             t++;
//             break;
//             }
//             t++;
//         }

//         // At this point we have a function key
//         // int fkey = function number 0....
//         // bool momentary = is it a momentary
//         // fName = pointer to the function name 
//         _functionNames[fkey] = fName;
//         if (momentary) {
//             _momentaryFlags |= 1<<fkey;
//         } else {
//             _momentaryFlags &= ~1<<fkey;
//         }
//         //    console->print("Function ");
//         //    console->print(fkey);
//         //    console->print(momentary ? F("  Momentary ") : F(""));
//         //    console->print(" ");
//         //    console->println(fName);
//         fkey++;
//     }
// }

// int Loco::getFunctionStates() {
//     return _functionStates;
// }

// void Loco::setFunctionStates(int functionStates) {
//     _functionStates=functionStates;
// }
OLD LOCO END */

// class ConsistLoco : public Loco

ConsistLoco::ConsistLoco(int address, LocoSource source, Facing facing)
: Loco::Loco(address, source) {
  _facing=facing;
}

void ConsistLoco::setFacing(Facing facing) {
  _facing=facing;
}

Facing ConsistLoco::getFacing() {
  return (Facing)_facing;
}

/* OLD CONSISTLOCO
ConsistLoco::ConsistLoco(int address, LocoSource source, Facing facing) 
: Loco::Loco(address, source) {
      consistLocoFacing = facing;
}

bool ConsistLoco::setConsistLocoFacing(Facing facing) {
    consistLocoFacing = facing;
    return true;
}

Facing ConsistLoco::getConsistLocoFacing() {
    return consistLocoFacing;
}
END OLD CONSISTLOCO */

// class Consist

Consist::Consist(char* name) {
  _name=name;
  _count=0;
  _speed=0;
  _direction=Forward;
  _consistLocos=nullptr;
}

char* Consist::getName() {
  return _name;
}

void Consist::addFromRoster(Loco* loco, Facing facing) {
  _addLoco(loco, facing);
}

void Consist::addFromEntry(int address, Facing facing) {
  Loco* loco = new Loco(address, LocoSourceEntry);
  _addLoco(loco, facing);
}

void Consist::releaseAll() {
  
}

void Consist::releaseLoco(int address) {

}

int Consist::getLocoCount() {
  return _count;
}

ConsistLoco* Consist::getLocoAtPosition(int position) {

}

int Consist::getLocoPosition(int address) {
  return -1;
}

void Consist::setLocoPosition(int address, int position) {

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

// private functions

void Consist::_addLoco(Loco* loco, Facing facing) {
  int address=loco->getAddress();
  LocoSource source=loco->getSource();
  if (getLocoPosition(address)>=0) return;  // Already in the consist
  if (_count==0) facing=FacingForward;  // Force forward facing for the first loco added
  ConsistLoco* conLoco=new ConsistLoco(address, source, facing);
}

/* OLD CONSIST
Consist::Consist(char* name) {
    consistName = name;
}

bool Consist::consistAddLoco(Loco loco, Facing facing) {
    int address = loco.getLocoAddress();
    LocoSource source = loco.getLocoSource();
    Facing correctedFacing = facing;
    int rslt = consistGetLocoPosition(address);
    if (rslt<0) { // not already in the list, so add it
        if (consistGetNumberOfLocos() == 0) correctedFacing = FacingForward; // first loco in consist is always forward
        consistLocos.add(new ConsistLoco(address, source, correctedFacing));

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
bool Consist::consistAddLocoFromAddress(int address, Facing facing) {
    Loco loco = Loco(address, LocoSourceEntry);
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
        LocoSource source = loco->getLocoSource();
        Facing correctedFacing = loco->getConsistLocoFacing();
        // if (consistGetNumberOfLocos() == 0 || position == 0) {
        //     correctedFacing = FacingForward; // first loco in consist is always forward
        // }
        // consistLocos.add(position, new ConsistLoco(address, name, source, correctedFacing));
        consistLocos.add(position, new ConsistLoco(address, source, correctedFacing));

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

bool Consist::actionConsistExternalChange(int speed, Direction direction, int functionStates) {
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
bool Consist::consistSetFunction(int functionNo, bool state) {
    if (consistLocos.size()>0) {
        // ConsistLoco* loco = consistGetLocoAtPosition(0);
        //????????????????? TODO
        return true;
    }
    return false;
}

// set a function on a specific loco on a throttle
bool Consist::consistSetFunction(int address, int functionNo, bool state) {
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
    consistName = name;
    return true;
}

char* Consist::getConsistName() {
    return consistName;
}
END OLD CONSIST */