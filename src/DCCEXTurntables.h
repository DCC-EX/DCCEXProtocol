#ifndef DCCEXTURNTABLES_H
#define DCCEXTURNTABLES_H

#include <Arduino.h>
#include <LinkedList.h>

enum TurntableType {
  TurntableTypeDCC = 0,
  TurntableTypeEXTT = 1,
  TurntableTypeUnknown = 9,
};

class TurntableIndex {
public:
  /// @brief Constructor
  /// @param id Index ID
  TurntableIndex(int id, int angle, char* name);

  /// @brief Get index ID (0 is always home)
  /// @return 
  int getId();

  /// @brief Get angle of the index from home
  /// @return 
  int getAngle();

  /// @brief Get index name
  char* getName();

  /// @brief Get first TurntableIndex object
  /// @return 
  static TurntableIndex* getFirst();

  /// @brief Get next TurntableIndex object
  /// @return 
  TurntableIndex* getNext();

private:
  int _id;
  int _angle;
  char* _name;
  static TurntableIndex* _first;
  TurntableIndex* _next;

  friend class Turntable;

};

class Turntable {
public:
  /// @brief Constructor
  /// @param id Turntable ID
  Turntable(int id);

  /// @brief Get turntable ID
  /// @return 
  int getId();

  /// @brief Set turntable type
  /// @param type TurntableTypeDCC, TurntableTypeEXTT
  void setType(TurntableType type);

  /// @brief Get turntable type
  /// @return 
  TurntableType getType();

  /// @brief Set the current index for the turntable
  /// @param index 
  void setIndex(int index);

  /// @brief Get the current index for the turntable
  /// @return 
  int getIndex();

  /// @brief Set the number of indexes the turntable has defined
  /// @param indexCount 
  void setNumberOfIndexes(int numberOfIndexes);

  /// @brief Get the number of indexes defined for the turntable
  /// @return 
  int getNumberOfIndexes();

  /// @brief Set the turntable name
  /// @param name 
  void setName(char* name);

  /// @brief  Get the turntable name
  /// @return 
  char* getName();

  /// @brief Set the movement state (0 stationary, 1 moving)
  /// @param moving 
  void setMoving(bool moving);

  /// @brief Get movement state (0 stationary, 1 moving)
  /// @return 
  bool isMoving();

  /// @brief Get the number of turntables
  /// @return 
  int getCount();

  /// @brief Get the count of indexes added to the index list
  /// @return 
  int getIndexCount();

  /// @brief Get the first turntable object
  /// @return 
  static Turntable* getFirst();

  /// @brief Get the next turntable object
  /// @return 
  Turntable* getNext();

  /// @brief Add a turntable index object to the index list for this turntable
  /// @param index - a TurntableIndex object
  /// @return 
  void addIndex(TurntableIndex* index);

  /// @brief Get associated turntable index list
  /// @return 
  TurntableIndex* getIndexList();

private:
  int _id;
  TurntableType _type;
  int _index;
  int _numberOfIndexes;
  char* _name;
  bool _isMoving;
  int _count=0;
  int _indexCount;
  static Turntable* _first;
  Turntable* _next;
  TurntableIndex* _indexList;

};

#endif