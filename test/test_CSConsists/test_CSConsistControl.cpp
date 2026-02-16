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

#include "../setup/CSConsistTests.h"

/**
 * @brief Test requesting CSConsists
 */
TEST_F(CSConsistTests, TestRequestCSConsists) {
  _dccexProtocol.requestCSConsists();
  EXPECT_EQ(_stream.getOutput(), "<^>");
}

/**
 * @brief Test creating a CSConsist
 */
TEST_F(CSConsistTests, TestCreateCSConsist) {
  _dccexProtocol.createCSConsist(3, false);
  CSConsist *csConsist = _dccexProtocol.csConsists->getFirst();
  ASSERT_NE(csConsist, nullptr);
  EXPECT_EQ(csConsist->getFirstMember()->address, 3);
  EXPECT_FALSE(csConsist->getFirstMember()->reversed);
  EXPECT_FALSE(csConsist->isValid());
}

/**
 * @brief Test creating a CSConsist with a lead loco in another returns existing
 */
TEST_F(CSConsistTests, TestCreateCSConsistAlreadyExisting) {
  // Create the existing
  CSConsist *existing = new CSConsist();
  existing->addMember(3, false);

  // Create new
  _dccexProtocol.createCSConsist(3, false);
  CSConsist *first = _dccexProtocol.csConsists->getFirst();
  ASSERT_EQ(first, existing);
}

/**
 * @brief Test creating a CSConsist with the lead loco in another consist removes from the other
 */
TEST_F(CSConsistTests, TestCreateCSConsistLeadInOtherCSConsist) {
  // Create the existing
  CSConsist *existing = new CSConsist();
  existing->addMember(5, false);
  existing->addMember(3, true);

  // Create new and validate results
  _dccexProtocol.createCSConsist(3, false);
  CSConsist *second = _dccexProtocol.csConsists->getFirst()->getNext();
  ASSERT_NE(second, nullptr);
  EXPECT_FALSE(existing->isInConsist(3));
  EXPECT_EQ(second->getFirstMember()->address, 3);
}

/**
 * @brief Test adding a member triggers the CSConsist to be created in the command station
 */
TEST_F(CSConsistTests, TestAddMemberCreatesInCommandStation) {
  // Create the consist and validate
  _dccexProtocol.createCSConsist(3, false);
  CSConsist *csConsist = _dccexProtocol.csConsists->getFirst();
  ASSERT_NE(csConsist, nullptr);
  EXPECT_EQ(csConsist->getFirstMember()->address, 3);
  EXPECT_FALSE(csConsist->getFirstMember()->reversed);
  EXPECT_FALSE(csConsist->isValid());
  EXPECT_EQ(_stream.getOutput(), "");

  // Add another member and validate it sends the command
  _dccexProtocol.addCSConsistMember(csConsist, 5, true);
  EXPECT_TRUE(csConsist->isValid());
  EXPECT_EQ(_stream.getOutput(), "<^ 3 -5>");
}

/**
 * @brief Test removing the last member will delete the consist in the CS
 */
TEST_F(CSConsistTests, TestRemoveLastMemberDeletesConsist) {}
