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
  CSConsist *csConsist = _dccexProtocol.createCSConsist(3, false);
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
  CSConsist *newConsist = _dccexProtocol.createCSConsist(3, false);
  ASSERT_EQ(newConsist, existing);
}

/**
 * @brief Test creating a CSConsist with the lead loco in another fails
 */
TEST_F(CSConsistTests, TestCreateCSConsistLeadInOtherCSConsist) {
  // Create the existing
  CSConsist *existing = new CSConsist();
  existing->addMember(5, false);
  existing->addMember(3, true);

  // Create new and validate results
  CSConsist *second = _dccexProtocol.createCSConsist(3, false);
  ASSERT_EQ(second, nullptr);
  EXPECT_TRUE(existing->isInConsist(3));
}

/**
 * @brief Test adding a member triggers the CSConsist to be created in the command station
 */
TEST_F(CSConsistTests, TestAddMemberCreatesInCommandStation) {
  // Create the consist and validate
  CSConsist *csConsist = _dccexProtocol.createCSConsist(3, false);
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
 * @brief Test adding a member that is in another consist fails
 */
TEST_F(CSConsistTests, TestAddMemberInOtherConsistFails) {
  // Create a consist
  CSConsist *existing = new CSConsist();
  existing->addMember(3, false);
  existing->addMember(5, true);

  // Test create with an existing member
  CSConsist *newConsist = _dccexProtocol.createCSConsist(9, false);
  ASSERT_NE(newConsist, nullptr);
  bool add = _dccexProtocol.addCSConsistMember(newConsist, 5, true);
  EXPECT_FALSE(add);
}

/**
 * @brief Test removing a consist member recreates the consist
 */
TEST_F(CSConsistTests, TestRemoveMember) {
  // Create a consist
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(3, false);
  csConsist->addMember(5, true);
  csConsist->addMember(7, false);

  // Remove middle member and validate
  bool remove = _dccexProtocol.removeCSConsistMember(csConsist, 5);
  EXPECT_TRUE(remove);
  EXPECT_EQ(_stream.getOutput(), "<^ 3 7>");
}

/**
 * @brief Test removing the last member will delete the consist in the CS and delete the CSConsist
 */
TEST_F(CSConsistTests, TestRemoveLastMemberDeletesConsist) {
  // Create a consist
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(3, false);
  csConsist->addMember(5, true);

  // Remove second member and validate
  bool remove = _dccexProtocol.removeCSConsistMember(csConsist, 5);
  EXPECT_TRUE(remove);
  EXPECT_EQ(_stream.getOutput(), "<^ 3>");
  EXPECT_EQ(_dccexProtocol.csConsists, nullptr);
}

/**
 * @brief Test creating, adding, and removing with invalid members fails sanely
 */
TEST_F(CSConsistTests, TestCreateAddRemoveInvalidMembers) {
  EXPECT_EQ(_dccexProtocol.createCSConsist(0, true), nullptr);
  ASSERT_EQ(_dccexProtocol.csConsists->getFirst(), nullptr);
  EXPECT_EQ(_dccexProtocol.createCSConsist(10240, true), nullptr);
  ASSERT_EQ(_dccexProtocol.csConsists->getFirst(), nullptr);
  CSConsist *csConsist = _dccexProtocol.createCSConsist(3, false);
  EXPECT_FALSE(_dccexProtocol.addCSConsistMember(csConsist, 0, true));
  EXPECT_FALSE(_dccexProtocol.addCSConsistMember(csConsist, 10240, true));
  EXPECT_FALSE(_dccexProtocol.addCSConsistMember(nullptr, 5, false));
  EXPECT_FALSE(_dccexProtocol.removeCSConsistMember(nullptr, 5));
  EXPECT_FALSE(_dccexProtocol.removeCSConsistMember(csConsist, 5));
}

/**
 * @brief Test removing a member from an empty consist deletes the CSConsist
 */
TEST_F(CSConsistTests, TestRemoveMemberFromEmptyConsist) {
  // Setup an empty consist
  CSConsist *empty = new CSConsist();
  ASSERT_NE(_dccexProtocol.csConsists->getFirst(), nullptr);

  // Attempt to delete a member
  bool remove = _dccexProtocol.removeCSConsistMember(empty, 3);
  EXPECT_FALSE(remove);

  // Should be false as member not found, but CSConsist should be deleted too
  EXPECT_EQ(_dccexProtocol.csConsists->getFirst(), nullptr);
}

/**
 * @brief Test deleting a consist by the lead loco address
 */
TEST_F(CSConsistTests, TestDeleteConsistByLeadLocoAddress) {
  // Create it
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(3, false);
  ASSERT_EQ(_dccexProtocol.csConsists->getFirst(), csConsist);

  // Delete and validate
  _dccexProtocol.deleteCSConsist(3);
  EXPECT_EQ(_dccexProtocol.csConsists->getFirst(), nullptr);
}

/**
 * @brief Test deleting a consist by a member address should fail
 */
TEST_F(CSConsistTests, TestDeleteConsistByMemberAddressFails) {
  // Create it
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(3, false);
  csConsist->addMember(5, true);
  ASSERT_EQ(_dccexProtocol.csConsists->getFirst(), csConsist);

  // Delete and validate
  _dccexProtocol.deleteCSConsist(5);
  EXPECT_EQ(_dccexProtocol.csConsists->getFirst(), csConsist);
}

/**
 * @brief Test deleting a consist by the object
 */
TEST_F(CSConsistTests, TestDeleteConsistByObject) {
  // Create it
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(3, false);
  ASSERT_EQ(_dccexProtocol.csConsists->getFirst(), csConsist);

  // Delete and validate
  _dccexProtocol.deleteCSConsist(csConsist);
  EXPECT_EQ(_dccexProtocol.csConsists->getFirst(), nullptr);
}

/**
 * @brief Test deleting a consist by an invalid object fails sanely
 */
TEST_F(CSConsistTests, TestDeleteConsistByInvalidObject) {
  // Delete by address shouldn't crash if none exist
  _dccexProtocol.deleteCSConsist(3);
  // Delete using nullptr shouldn't crash
  _dccexProtocol.deleteCSConsist(nullptr);
}

/**
 * @brief Tests clearing all CSConsists
 */
TEST_F(CSConsistTests, TestClearCSConsists) {
  // Create a list
  CSConsist *first = new CSConsist();
  CSConsist *second = new CSConsist();
  CSConsist *third = new CSConsist();
  ASSERT_EQ(_dccexProtocol.csConsists->getFirst(), first);
  ASSERT_EQ(_dccexProtocol.csConsists->getFirst()->getNext(), second);
  ASSERT_EQ(_dccexProtocol.csConsists->getFirst()->getNext()->getNext(), third);
  ASSERT_EQ(_dccexProtocol.csConsists->getFirst()->getNext()->getNext()->getNext(), nullptr);

  // Clear all and validate
  _dccexProtocol.clearCSConsists();
  EXPECT_EQ(_dccexProtocol.csConsists->getFirst(), nullptr);
}

/**
 * @brief Test calling setThrottle() with a valid consist creates and adds lead loco only to the queue
 */
TEST_F(CSConsistTests, TestSetThrottleAddsToQueue) {
  // Create the consist
  CSConsist *csConsist = _dccexProtocol.createCSConsist(3, false);
  csConsist->addMember(5, true);
  _dccexProtocol.setThrottle(csConsist, 10, Forward);

  // There should now be a new local loco with change pending
  Loco *loco = Loco::getByAddress(3);
  ASSERT_NE(loco, nullptr);
  EXPECT_EQ(loco->getUserSpeed(), 10);
  EXPECT_EQ(loco->getUserDirection(), Forward);
  EXPECT_TRUE(loco->getUserChangePending());
  // There should be no member loco
  Loco *member = Loco::getByAddress(5);
  ASSERT_EQ(member, nullptr);
}

/**
 * @brief Test calling setThrottle() with a roster loco uses it only
 */
TEST_F(CSConsistTests, TestSetThrottleRosterLoco) {
  // Create a local loco for the lead loco
  Loco *loco = new Loco(3, LocoSource::LocoSourceRoster);
  EXPECT_EQ(loco->getUserSpeed(), 0);
  EXPECT_EQ(loco->getUserDirection(), Forward);
  EXPECT_FALSE(loco->getUserChangePending());
  // Create the consist
  CSConsist *csConsist = _dccexProtocol.createCSConsist(3, false);
  csConsist->addMember(5, true);
  _dccexProtocol.setThrottle(csConsist, 10, Reverse);

  // The existing local loco should have change pending
  Loco *checkLoco = Loco::getByAddress(3);
  ASSERT_EQ(loco, checkLoco);
  EXPECT_EQ(loco->getUserSpeed(), 10);
  EXPECT_EQ(loco->getUserDirection(), Reverse);
  EXPECT_TRUE(loco->getUserChangePending());
  // There should be no member loco
  Loco *member = Loco::getByAddress(5);
  ASSERT_EQ(member, nullptr);
}

/**
 * @brief Test calling setThrottle() with a local loco uses it only
 */
TEST_F(CSConsistTests, TestSetThrottleLocalLoco) {
  // Create a local loco for the lead loco
  Loco *loco = new Loco(3, LocoSource::LocoSourceEntry);
  EXPECT_EQ(loco->getUserSpeed(), 0);
  EXPECT_EQ(loco->getUserDirection(), Forward);
  EXPECT_FALSE(loco->getUserChangePending());
  // Create the consist
  CSConsist *csConsist = _dccexProtocol.createCSConsist(3, false);
  csConsist->addMember(5, true);
  _dccexProtocol.setThrottle(csConsist, 10, Reverse);

  // The existing local loco should have change pending
  Loco *checkLoco = Loco::getByAddress(3);
  ASSERT_EQ(loco, checkLoco);
  EXPECT_EQ(loco->getUserSpeed(), 10);
  EXPECT_EQ(loco->getUserDirection(), Reverse);
  EXPECT_TRUE(loco->getUserChangePending());
  // There should be no member loco
  Loco *member = Loco::getByAddress(5);
  ASSERT_EQ(member, nullptr);
}

/**
 * @brief Test calling setThrottle() with an invalid consist does nothing
 */
TEST_F(CSConsistTests, TestSetThrottleInvalidConsist) {
  // Create a loco
  Loco *loco = new Loco(3, LocoSource::LocoSourceEntry);
  // Create a consist
  CSConsist *csConsist = _dccexProtocol.createCSConsist(loco->getAddress(), false);
  _dccexProtocol.setThrottle(csConsist, 20, Reverse);

  // Validate no change
  EXPECT_FALSE(loco->getUserChangePending());
  EXPECT_EQ(loco->getUserSpeed(), 0);
  EXPECT_EQ(loco->getUserDirection(), Forward);
}
