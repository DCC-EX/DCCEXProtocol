/* -*- c++ -*-
 *
 * Copyright © 2026 Peter Cole
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

#ifndef DCCEXCSCONSIST_H
#define DCCEXCSCONSIST_H

#include "DCCEXLoco.h"

/**
 * @brief Class for managing members of a command station consist
 */
class CSConsistMember {
public:
  /**
   * @brief Construct a new CSConsistMember object
   * @param loco Pointer to a Loco object in the consist
   * @param reversed True if loco is reversed compared to lead loco, otherwise false
   */
  CSConsistMember(Loco *loco, bool reversed);

  /**
   * @brief Get the Loco object associated with this member
   * @return Loco* Pointer to the Loco object
   */
  Loco *getLoco();

  /**
   * @brief Test if this Loco is reversed compared to the lead loco
   * @return true True if reversed compared to lead loco
   * @return false If not reversed
   */
  bool isReversed();

  /**
   * @brief Set the Next object
   * @param next Pointer to the next CSConsistMember object in the list
   */
  void setNext(CSConsistMember *next);

  /**
   * @brief Get the Next object
   * @return CSConsistMember* Pointer to the next CSConsistMember object in the list
   */
  CSConsistMember *getNext();

  /**
   * @brief Destroy the CSConsistMember object
   */
  ~CSConsistMember();

private:
  Loco *_loco;
  bool _reversed;
  CSConsistMember *_next;
};

/**
 * @brief Class to assist managing command station consists
 * @details You must provide a lead loco object or address when creating a CSConsist instance. In order for the command
 * station to accept a CSConsist, at least one additional loco is required. Additional locos are CSConsistMember
 * objects, enabling them to be operated in reverse compared to the lead loco. Each CSConsist instance is in a linked
 * list accessible via CSConsist::getFirst().
 */
class CSConsist {
public:
  /**
   * @brief Construct a new CSConsist object
   * @param leadLoco Pointer to the lead Loco object
   */
  CSConsist(Loco *leadLoco);

  /**
   * @brief Construct a new CSConsist object
   * @param leadLocoAddress DCC address of the lead loco
   */
  CSConsist(int leadLocoAddress);

  /**
   * @brief Get the First object
   * @return CSConsist* Pointer to the first CSConsist object
   */
  static CSConsist *getFirst();

  /**
   * @brief Get the Next object
   * @return CSConsist* Pointer to the next CSConsist object
   */
  CSConsist *getNext();

  /**
   * @brief Add a Loco to the consist
   * @param loco Pointer to the Loco object to be added
   * @param reversed True if loco is reversed compared to lead loco, otherwise false
   */
  void addMember(Loco *loco, bool reversed);

  /**
   * @brief Add a loco by address to the consist
   * @param address DCC address of the loco to be added
   * @param reversed True if loco is reversed compared to lead loco, otherwise false
   */
  void addMember(int address, bool reversed);

  /**
   * @brief Remove a Loco from the consist
   * @param loco Pointer to the Loco object to be removed
   */
  void removeMember(Loco *loco);

  /**
   * @brief Remove a loco by address from the consist
   * @param address DCC address of the loco to be removed
   */
  void removeMember(int address);

  /**
   * @brief Get the Lead Loco object
   * @return Loco* Pointer to the lead Loco object
   */
  Loco *getLeadLoco();

  /**
   * @brief Get the First Member object
   * @return CSConsistMember* Pointer to the first CSConsistMember object
   */
  CSConsistMember *getFirstMember();

  /**
   * @brief Get the Member object
   * @param loco Pointer to the Loco associated with the member
   * @return CSConsistMember* Pointer to the CSConsistMember object
   */
  CSConsistMember *getMember(Loco *loco);

  /**
   * @brief Get the Member Loco object
   * @param address DCC address of the loco associated with the member
   * @return CSConsistMember* Pointer to the CSConsistMember object
   */
  CSConsistMember *getMember(int address);

  /**
   * @brief Check if a Loco is in this consist
   * @param loco Pointer to the Loco object
   * @return true If Loco in consist
   * @return false If not
   */
  bool isInConsist(Loco *loco);

  /**
   * @brief Check by address if a loco is in this consist
   * @param address DCC address of the loco
   * @return true If loco address is in consist
   * @return false If not
   */
  bool isInConsist(int address);

  /**
   * @brief Check if the Loco is reversed in this consist
   * @param loco Pointer to the Loco object
   * @return true If loco is in consist and reversed
   * @return false If loco is not in consist or is not reversed
   */
  bool isReversed(Loco *loco);

  /**
   * @brief Check by address if the loco is reversed in this consist
   * @param address DCC address of the loco
   * @return true If loco address is in consist and reversed
   * @return false If loco address is not in consist or is not reversed
   */
  bool isReversed(int address);

  /**
   * @brief Destroy the CSConsist object
   */
  ~CSConsist();

private:
  Loco *_leadLoco;
  CSConsistMember *_firstMember;
  CSConsist *_next;
  static CSConsist *_first;

  /**
   * @brief Helper to add CSConsist to the linked list
   * @param csConsist Pointer to the CSConsist to add
   */
  void _addToConsistList(CSConsist *csConsist);

  /**
   * @brief Helper to add to the CSConsistMember list
   * @param member CSConsistMember to add
   */
  void _addToMemberList(CSConsistMember *member);

  /**
   * @brief Helper to remove from the CSConsistMember list
   * @param member Loco to remove
   */
  void _removeFromMemberList(Loco *loco);
};

#endif // DCCEXCSCONSIST_H
