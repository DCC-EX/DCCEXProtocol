#ifndef DCCEXLOCO_H
#define DCCEXLOCO_H

#include <Arduino.h>

static const int MAX_FUNCTIONS = 28;
const int MAX_OBJECT_NAME_LENGTH = 30;  // including Loco name, Turnout/Point names, Route names, etc. names
#define MAX_SINGLE_COMMAND_PARAM_LENGTH 500  // Unfortunately includes the function list for an individual loco

enum Direction {
  Reverse = 0,
  Forward = 1,
};

enum LocoSource {
  LocoSourceRoster=0,
  LocoSourceEntry=1,
};

enum Facing {
  FacingForward=0,
  FacingReversed=1,
};

class Loco {
public:
  /// @brief Constructor
  /// @param address 
  /// @param source 
  Loco(int address, LocoSource source);

  /// @brief Get loco address
  /// @return 
  int getAddress();

  /// @brief Set loco name
  /// @param name 
  void setName(char* name);

  /// @brief Get loco name
  /// @return 
  char* getName();

  /// @brief Set loco speed
  /// @param speed 
  void setSpeed(int speed);

  /// @brief Get loco speed
  /// @return 
  int getSpeed();

  /// @brief Set loco direction (enums Forward, Reverse)
  /// @param direction 
  void setDirection(Direction direction);

  /// @brief Get loco direction (enums Forward, Reverse)
  /// @return 
  Direction getDirection();

  /// @brief Get loco source (enums LocoSourceRoster, LocoSourceEntry)
  /// @return 
  LocoSource getSource();

  /// @brief Setup functions for the loco
  /// @param functionNames 
  void setupFunctions(char* functionNames);

  /// @brief Test if function is on
  /// @param function 
  /// @return 
  bool isFunctionOn(int function);

  /// @brief Set function states
  /// @param functionStates 
  void setFunctionStates(int functionStates);

  /// @brief Get function states
  /// @return 
  int getFunctionStates();

  /// @brief Get count of locos
  /// @return 
  int getCount();

  /// @brief Get first Loco object
  /// @return 
  static Loco* getFirst();

  /// @brief Get next Loco object
  /// @return 
  Loco* getNext();

  /// @brief Get Loco object by its DCC address
  /// @param address 
  /// @return 
  Loco* getByAddress(int address);

private:
  int _address;
  char* _name;
  int _speed;
  Direction _direction;
  LocoSource _source;
  char* _functionNames[MAX_FUNCTIONS];
  int32_t _functionStates;
  int32_t _momentaryFlags;
  static Loco* _first;
  Loco* _next;
  int _count=0;

  friend class Consist;
};

class ConsistLoco : public Loco {
public:
  /// @brief  Constructor
  /// @param address 
  /// @param source 
  /// @param facing 
  ConsistLoco(int address, LocoSource source, Facing facing);
  
  /// @brief Set which way the loco is facing in the consist (FacingForward, FacingReversed)
  /// @param facing 
  void setFacing(Facing facing);

  /// @brief Get which way the loco is facing in the consist (FacingForward, FacingReversed)
  /// @return 
  Facing getFacing();

  /// @brief Get the next consist loco object
  /// @return 
  ConsistLoco* getNext();

private:
  Facing _facing;
  ConsistLoco* _next;

  friend class Consist;

};

class Consist {
public:
  Consist();
  
  /// @brief Constructor
  /// @param name 
  // Consist(char* name);

  /// @brief Set consist name
  /// @param name 
  void setName(char* name);

  /// @brief Get consist name
  /// @return 
  char* getName();

  /// @brief Add a loco to the consist from a roster entry
  /// @param loco 
  /// @param facing 
  void addFromRoster(Loco* loco, Facing facing);

  /// @brief Add a loco to the consist from a user entering the address
  /// @param address 
  /// @param facing 
  void addFromEntry(int address, Facing facing);

  /// @brief Release all locos from the consist
  void releaseAll();

  /// @brief Release a specific loco from the consist
  /// @param address 
  void releaseLoco(int address);

  /// @brief Get the count of locos in the consist
  /// @return 
  int getLocoCount();

  /// @brief Check if the provided loco address is in the consist
  /// @param address 
  /// @return 
  bool inConsist(int address);

  /// @brief Set speed for the consist
  /// @param speed 
  void setSpeed(int speed);

  /// @brief Get consist speed
  /// @return 
  int getSpeed();

  /// @brief Set direction for the consist
  /// @param direction 
  void setDirection(Direction direction);

  /// @brief Get consist direction
  /// @return 
  Direction getDirection();

  /// @brief Get the list of locos in the consist
  /// @return 
  ConsistLoco* getFirst();

private:
  int _speed;
  Direction _direction;
  char* _name;
  int _locoCount;
  ConsistLoco* _first;

  /// @brief Add a loco object to the consist
  /// @param loco 
  /// @param facing 
  void _addLoco(Loco* loco, Facing facing);

};

#endif