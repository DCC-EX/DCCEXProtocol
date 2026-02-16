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

#include <Arduino.h>

/**
 * @brief Structure for a CSConsistMember
 */
struct CSConsistMember {
  uint16_t address : 15;
  uint16_t reversed : 1;
  CSConsistMember *next;

  /**
   * @brief Construct a new CSConsistMember object
   * @param address DCC address of the member
   * @param reversed True if reversed to normal direction of travel
   */
  CSConsistMember(int address, bool reversed) : address(address), reversed(reversed), next(nullptr) {}
};

/**
 * @brief Class to assist managing command station consists
 * @details In order for the command station to accept a CSConsist, at least two locos are required. Each member loco is
 * a CSConsistMember object, enabling them to be operated in reverse compared to normal direction of travel.
 * Each CSConsist instance is in a linked list accessible via CSConsist::getFirst(). It is recommended to always create
 * and delete CSConsist objects using the methods provided in DCCEXProtocol class to ensure your command station is
 * updated appropriately.
 */
class CSConsist {
public:
  /**
   * @brief Construct a new CSConsist object
   */
  CSConsist();

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
   * @param reversed True if loco is reversed to normal direction of travel
   */
  // void addMember(Loco *loco, bool reversed);

  /**
   * @brief Add a loco by address to the consist
   * @details This method attempts to find an existing Loco object and will use that instead of creating a new object.
   * @param address DCC address of the loco to be added
   * @param reversed True if loco is reversed to normal direction of travel
   */
  void addMember(int address, bool reversed);

  /**
   * @brief Remove a loco by address from the consist
   * @param address DCC address of the loco to be removed
   */
  void removeMember(int address);

  /**
   * @brief Remove all CSConsistMember objects from this consist
   */
  void removeAllMembers();

  /**
   * @brief Get the First Member object
   * @return CSConsistMember* Pointer to the first CSConsistMember object
   */
  CSConsistMember *getFirstMember();

  /**
   * @brief Get the Member Loco object
   * @param address DCC address of the loco associated with the member
   * @return CSConsistMember* Pointer to the CSConsistMember object
   */
  CSConsistMember *getMember(int address);

  /**
   * @brief Check by address if a loco is in this consist
   * @param address DCC address of the loco
   * @return true If loco address is in consist
   * @return false If not
   */
  bool isInConsist(int address);

  /**
   * @brief Check by address if the loco is reversed in this consist
   * @param address DCC address of the loco
   * @return true If loco address is in consist and reversed
   * @return false If loco address is not in consist or is not reversed
   */
  bool isReversed(int address);

  /**
   * @brief Set the flag for this consist's state in the command station
   * @param created True if it has been created in the command station, otherwise false
   */
  void setCreatedInCS(bool created);

  /**
   * @brief Check if this consist has been created in the command station
   * @return true If it is created
   * @return false If it is not
   */
  bool isCreatedInCS();

  /**
   * @brief Set the flag if this consist needs to be deleted in the command station
   * @param pending True if it needs to be deleted, otherwise false
   */
  void setDeleteCSPending(bool pending);

  /**
   * @brief Check if this consist needs to be deleted in the command station
   * @return true If it needs to be deleted
   * @return false If it does not
   */
  bool isDeleteCSPending();

  /**
   * @brief Check if this is a valid consist with more than 1 member
   * @return true Valid CSConsist
   * @return false Invalid (less than 2 members)
   */
  bool isValid();

  /**
   * @brief Clear all CSConsists from the list
   */
  static void clearCSConsists();

  /**
   * @brief Get the CSConsist the provided address is lead loco of
   * @param address DCC address of the lead loco to check for
   * @return CSConsist* Pointer to the CSConsist object, or nullptr if none found
   */
  static CSConsist *getLeadLocoCSConsist(int address);

  /**
   * @brief Get the CSConsist the provided address is a member of
   * @param address DCC address of the member loco to check for
   * @return CSConsist* Pointer to the CSConsist object, or nullptr if none found
   */
  static CSConsist *getMemberCSConsist(int address);

  /**
   * @brief Destroy the CSConsist object
   */
  ~CSConsist();

private:
  CSConsistMember *_firstMember;
  CSConsist *_next;
  bool _createdInCS;
  bool _deleteCSPending;
  int _memberCount;
  static CSConsist *_first;
};

#endif // DCCEXCSCONSIST_H
