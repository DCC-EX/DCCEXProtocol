#ifndef DCCEXTURNOUTS_H
#define DCCEXTURNOUTS_H

#include <Arduino.h>

class Turnout {
public:
  /// @brief Constructor
  /// @param id Turnout ID
  /// @param thrown Bool for turnout thrown (0 closed, 1 thrown)
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

  /// @brief Get turnout object by turnout ID
  /// @param id 
  /// @return 
  Turnout* getById(int id);
  
private:
  static Turnout* _first;
  Turnout* _next;
  int _id;
  char* _name;
  bool _thrown;
  int _count=0;
  
};

#endif