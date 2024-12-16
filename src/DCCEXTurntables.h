/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
 *
 * Copyright © 2023 Peter Akers
 * Copyright © 2023 Peter Cole
 *
 * This work is licensed under the Creative Commons Attribution-ShareAlike
 * 4.0 International License. To view a copy of this license, visit
 * http://creativecommons.org/licenses/by-sa/4.0/ or send a letter to
 * Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
 *
 * Attribution — You must give appropriate credit, provide a link to the
 * license, and indicate if changes were made. You may do so in any
 * reasonable manner, but not in any way that suggests the licensor
 * endorses you or your use.
 *
 * ShareAlike — If you remix, transform, or build upon the material, you
 * must distribute your contributions under the same license as the
 * original.
 *
 * All other rights reserved.
 *
 */

#ifndef DCCEXTURNTABLES_H
#define DCCEXTURNTABLES_H

#include <Arduino.h>

enum TurntableType {
  TurntableTypeDCC = 0,
  TurntableTypeEXTT = 1,
  TurntableTypeUnknown = 9,
};

/// @brief Class to contain and maintain the various Turntable Index attributes and methods associated with a Turntable
class TurntableIndex {
public:
  /// @brief Constructor
  /// @param ttId ID of the turntable the index is associated with
  /// @param id ID of the index
  /// @param angle Angle from home for this index (0 - 3600)
  /// @param name Name of the index
  TurntableIndex(int ttId, int id, int angle, const char *name);

  /// @brief Get the turntable ID
  /// @return ID of the turntable this index is associated with
  int getTTId();

  /// @brief Get index ID (0 is always home)
  /// @return ID of this index
  int getId();

  /// @brief Get angle of the index from home
  /// @return Angle of this index from home (0 - 3600)
  int getAngle();

  /// @brief Get index name
  /// @return Current name of the index
  const char *getName();

  /// @brief Get next TurntableIndex object
  /// @return Pointer to the next TurntableIndex object
  TurntableIndex *getNextIndex();

  /// @brief Destructor for an index
  ~TurntableIndex();

private:
  int _ttId;
  int _id;
  int _angle;
  char *_name;
  TurntableIndex *_nextIndex;

  friend class Turntable;
};

/// @brief Class to contain and maintain the various Turntable attributes and methods
class Turntable {
public:
  /// @brief Constructor
  /// @param id ID of the turntable
  Turntable(int id);

  /// @brief Get turntable ID
  /// @return ID of the turntable
  int getId();

  /// @brief Set turntable type
  /// @param type TurntableTypeDCC|TurntableTypeEXTT|TurntableTypeUnknown
  void setType(TurntableType type);

  /// @brief Get turntable type
  /// @return TurntableTypeDCC|TurntableTypeEXTT|TurntableTypeUnknown
  TurntableType getType();

  /// @brief Set the current index for the turntable
  /// @param index ID of the index to set
  void setIndex(int index);

  /// @brief Get the current index for the turntable
  /// @return ID of the current index
  int getIndex();

  /// @brief Set the number of indexes the turntable has defined (from the \<JT id\> command response)
  /// @param numberOfIndexes Count of the indexes defined for the turntable including home
  void setNumberOfIndexes(int numberOfIndexes);

  /// @brief Get the number of indexes defined for the turntable
  /// @return Count of the indexes defined
  int getNumberOfIndexes();

  /// @brief Set the turntable name
  /// @param name Name to set for the turntable
  void setName(const char *name);

  /// @brief  Get the turntable name
  /// @return Current name of the turntable
  const char *getName();

  /// @brief Set the movement state (false stationary, true moving)
  /// @param moving true|false
  void setMoving(bool moving);

  /// @brief Get movement state (false stationary, true moving)
  /// @return true|false
  bool isMoving();

  /// @brief Get the count of indexes added to the index list (counted from the \<JP id\> command response)
  /// @return Count of indexes received for this turntable including home
  int getIndexCount();

  /// @brief Get the first turntable object
  /// @return Pointer to the first Turntable object
  static Turntable *getFirst();

  /// @brief Set the next turntable in the list
  /// @param turntable Pointer to the next turntable
  void setNext(Turntable *turntable);

  /// @brief Get the next turntable object
  /// @return Pointer to the next Turntable object
  Turntable *getNext();

  /// @brief Add a turntable index object to the index list for this turntable
  /// @param index TurntableIndex object to add
  void addIndex(TurntableIndex *index);

  /// @brief Get the first associated turntable index
  /// @return Pointer to the first associated TurntableIndex object
  TurntableIndex *getFirstIndex();

  /// @brief Get a turntable object by its ID
  /// @param id ID of the turntable to retrieve
  /// @return Pointer to the Turntable object or nullptr if not found
  static Turntable *getById(int id);

  /// @brief Get TurntableIndex object by its ID
  /// @param id ID of the index to retrieve
  /// @return Pointer to the TurntableIndex object or nullptr if not found
  TurntableIndex *getIndexById(int id);

  /// @brief Clear the list of turntables
  static void clearTurntableList();

  /// @brief Destructor for a turntable
  ~Turntable();

private:
  int _id;
  TurntableType _type;
  int _index;
  int _numberOfIndexes;
  char *_name;
  bool _isMoving;
  int _indexCount;
  static Turntable *_first;
  Turntable *_next;
  TurntableIndex *_firstIndex;

  /// @brief Remove the turntable from the list
  /// @param turntable Pointer to the turntable to remove
  void _removeFromList(Turntable *turntable);
};

#endif