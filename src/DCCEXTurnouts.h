#ifndef DCCEXTURNOUTS_H
#define DCCEXTURNOUTS_H

#include <Arduino.h>

// enum TurnoutStates {
//   TurnoutClosed = 0,
//   TurnoutThrown = 1,
//   TurnoutResponseClosed = 'C',
//   TurnoutResponseThrown = 'T',
//   TurnoutClose = 0,
//   TurnoutThrow = 1,
//   TurnoutToggle = 2,
//   TurnoutExamine = 9,
// };

class Turnout {
public:
  /// @brief Constructor
  /// @param id 
  /// @param _Thrown 
  Turnout(int id, bool thrown);
  
  /// @brief Set thrown state (1 thrown, 0 closed)
  /// @param isThrown 
  void setThrown(bool thrown);
  
  /// @brief Set turnout name
  /// @param _name 
  void setName(char* _name);

  /// @brief Get turnout Id
  /// @return 
  int getId();

  /// @brief Get turnout name
  /// @return 
  char* getName();

  /// @brief Get thrown state (1 thrown, 0 closed)
  /// @return 
  bool getThrown();

  /// @brief Get first turnout object
  /// @return 
  static Turnout* getFirst();

  /// @brief Get next turnout object
  /// @return 
  Turnout* getNext();

  /// @brief Get the number of turnouts
  /// @return 
  int getCount();
  
private:
  static Turnout* _first;
  Turnout* _next;
  int _id;
  char* _name;
  bool _thrown;
  int _count=0;
  
};

#endif