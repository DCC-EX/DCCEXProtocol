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
 * @brief Basic CSConsistMember creation
 */
TEST_F(CSConsistTests, TestCreateConsistMember) {
  CSConsistMember *member42 = new CSConsistMember(42, false);

  EXPECT_EQ(member42->address, 42);
  EXPECT_FALSE(member42->reversed);
  EXPECT_EQ(member42->next, nullptr);

  CSConsistMember *member2 = new CSConsistMember(2, true);

  EXPECT_EQ(member2->address, 2);
  EXPECT_TRUE(member2->reversed);
  EXPECT_EQ(member2->next, nullptr);

  // Clean up
  delete member42;
  delete member2;
}

/**
 * @brief Basic CSConsist creation and deletion
 */
TEST_F(CSConsistTests, TestCreateConsistWithAddress) {
  // Create the consist
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(3, false);

  // First CS consist should now be set
  ASSERT_NE(CSConsist::getFirst(), nullptr);

  // Should be the only one in the list
  EXPECT_EQ(csConsist->getNext(), nullptr);

  // Lead loco should not be a nullptr ahd have address 3
  EXPECT_NE(csConsist->getFirstMember(), nullptr);
  EXPECT_EQ(csConsist->getFirstMember()->address, 3);
  EXPECT_FALSE(csConsist->getFirstMember()->reversed);

  // validate flags and count
  EXPECT_FALSE(csConsist->isValid());
  EXPECT_EQ(csConsist->getMemberCount(), 1);
  EXPECT_FALSE(csConsist->getReplicateFunctions());
  EXPECT_FALSE(csConsist->getAlwaysReplicateFunctions());

  // Clean up
  delete csConsist;
}

/**
 * @brief Test building a consist with addresses
 */
TEST_F(CSConsistTests, TestBuildConsistWithAddresses) {
  // Create the consist and validate
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(42, false);
  ASSERT_NE(CSConsist::getFirst(), nullptr);
  ASSERT_EQ(csConsist->getFirstMember()->address, 42);
  ASSERT_FALSE(csConsist->getFirstMember()->reversed);
  ASSERT_FALSE(csConsist->isValid());

  // Now add the other locos with 24 reversed
  csConsist->addMember(24, true);
  csConsist->addMember(3, false);
  ASSERT_TRUE(csConsist->isValid());
  EXPECT_EQ(csConsist->getMemberCount(), 3);

  // Now validate the expected state
  EXPECT_TRUE(csConsist->isInConsist(42));
  EXPECT_FALSE(csConsist->isReversed(42));
  EXPECT_TRUE(csConsist->isInConsist(24));
  EXPECT_TRUE(csConsist->isInConsist(3));
  EXPECT_TRUE(csConsist->isReversed(24));
  EXPECT_FALSE(csConsist->isReversed(3));

  // Make sure member is accessible and list is as expected
  CSConsistMember *member24 = csConsist->getMember(24);
  CSConsistMember *member3 = csConsist->getMember(3);
  ASSERT_NE(member24, nullptr);
  ASSERT_NE(member3, nullptr);
  EXPECT_EQ(member24->address, 24);
  EXPECT_EQ(member3->address, 3);
  EXPECT_EQ(member24->next, member3);
  EXPECT_EQ(member3->next, nullptr);

  // Clean up
  delete csConsist;
}

/**
 * @brief Test accessing invalid members returns sensibly
 */
TEST_F(CSConsistTests, TestAccessingInvalidMembers) {
  // Create some locos and a consist
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(42, false);

  // Test membership with 999 and addresses returns sane answers
  EXPECT_FALSE(csConsist->isInConsist(22));
  EXPECT_EQ(csConsist->getMember(22), nullptr);
  EXPECT_FALSE(csConsist->isReversed(22));
}

/**
 * @brief Test calling methods with invalid parameters doesn't crash
 */
TEST_F(CSConsistTests, TestCallingWithInvalidParams) {
  // Create consist and a loco for testing
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(42, false);

  // Test adding/removing with invalid values doesn't cause a crash
  csConsist->addMember(11000, true);
  csConsist->addMember(0, false);
  csConsist->removeMember(22);
}

/**
 * @brief Test removing a member by address
 */
TEST_F(CSConsistTests, TestRemoveMemberByAddress) {
  // Create the consist and add members
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(42, false);
  csConsist->addMember(24, true);
  csConsist->addMember(3, false);
  ASSERT_EQ(csConsist->getFirstMember()->address, 42);
  ASSERT_TRUE(csConsist->isValid());

  // Remove loco24 and validate
  csConsist->removeMember(24);
  ASSERT_FALSE(csConsist->isInConsist(24));
  EXPECT_EQ(csConsist->getMember(24), nullptr);
  EXPECT_EQ(csConsist->getFirstMember()->next->address, 3);
  EXPECT_TRUE(csConsist->isValid());
  EXPECT_EQ(csConsist->getMemberCount(), 2);
}

/**
 * @brief Test removing all members cleans the list up
 */
TEST_F(CSConsistTests, TestRemoveAllMembersManually) {
  // Now create the consist and add members
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(42, false);
  csConsist->addMember(24, true);
  csConsist->addMember(3, false);
  ASSERT_EQ(csConsist->getFirstMember()->address, 42);
  ASSERT_TRUE(csConsist->isValid());

  // Remove loco24 and validate
  csConsist->removeMember(24);
  ASSERT_FALSE(csConsist->isInConsist(24));
  EXPECT_EQ(csConsist->getMember(24), nullptr);
  EXPECT_EQ(csConsist->getFirstMember()->next->address, 3);
  EXPECT_TRUE(csConsist->isValid());

  // Remove loco3 and validate
  csConsist->removeMember(3);
  ASSERT_FALSE(csConsist->isInConsist(3));
  EXPECT_EQ(csConsist->getMember(3), nullptr);
  EXPECT_FALSE(csConsist->isValid());

  // Remove loco42 and validate
  csConsist->removeMember(42);
  ASSERT_FALSE(csConsist->isInConsist(42));
  EXPECT_EQ(csConsist->getFirstMember(), nullptr);
  EXPECT_FALSE(csConsist->isValid());
  EXPECT_EQ(csConsist->getMemberCount(), 0);
}

/**
 * @brief Test adding duplicate locos doesn't crash
 */
TEST_F(CSConsistTests, TestAddDuplicates) {
  // Create the consist and add members
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(42, false);
  csConsist->addMember(24, true);
  csConsist->addMember(3, false);

  // Try adding duplicates
  csConsist->addMember(24, false);
  csConsist->addMember(3, true);
  EXPECT_TRUE(csConsist->isReversed(24));
  EXPECT_FALSE(csConsist->isReversed(3));

  // Count all members, more than 3 is a fail
  EXPECT_EQ(csConsist->getMemberCount(), 3);
}

/**
 * @brief Test calling CSConsist::deleteAll() cleans up without memory leaks
 */
TEST_F(CSConsistTests, TestCleanAllCSConsists) {
  // Create some CSConsists
  CSConsist *csConsist1 = new CSConsist();
  csConsist1->addMember(3, false);
  csConsist1->addMember(5, true);
  CSConsist *csConsist2 = new CSConsist();
  csConsist2->addMember(13, false);
  csConsist2->addMember(15, true);
  CSConsist *csConsist3 = new CSConsist();
  csConsist3->addMember(23, false);
  csConsist3->addMember(25, true);

  // Validate the list
  ASSERT_EQ(CSConsist::getFirst(), csConsist1);
  EXPECT_EQ(CSConsist::getFirst()->getNext(), csConsist2);
  EXPECT_EQ(csConsist2->getNext(), csConsist3);
  EXPECT_EQ(csConsist3->getNext(), nullptr);

  // Call clear all and validate
  CSConsist::clearCSConsists();
  ASSERT_EQ(CSConsist::getFirst(), nullptr);
}

/**
 * @brief Test getting a CSConsist with the specific lead loco address
 */
TEST_F(CSConsistTests, TestGetLeadLocoCSConsist) {
  // Set a lead loco address to check for in the list of CSConsists and create consists
  int leadLoco = 13;
  CSConsist *csConsist1 = new CSConsist();
  csConsist1->addMember(3, false);
  csConsist1->addMember(5, true);
  CSConsist *csConsist2 = new CSConsist();
  csConsist2->addMember(13, false);
  csConsist2->addMember(15, true);
  CSConsist *csConsist3 = new CSConsist();
  csConsist3->addMember(23, false);
  csConsist3->addMember(25, true);

  // Calling CSConsist::getLeadLocoCSConsist() should return csConsist2
  ASSERT_EQ(CSConsist::getLeadLocoCSConsist(leadLoco), csConsist2);
}

/**
 * @brief Test getting a CSConsist with a member address returns nullptr
 */
TEST_F(CSConsistTests, TestGetLeadLocoWithMemberFails) {
  // Set a lead loco address to check for in the list of CSConsists and create consists
  int leadLoco = 15;
  CSConsist *csConsist1 = new CSConsist();
  csConsist1->addMember(3, false);
  csConsist1->addMember(5, true);
  CSConsist *csConsist2 = new CSConsist();
  csConsist2->addMember(13, false);
  csConsist2->addMember(15, true);
  CSConsist *csConsist3 = new CSConsist();
  csConsist3->addMember(23, false);
  csConsist3->addMember(25, true);

  // Calling CSConsist::getLeadLocoCSConsist() should return nullptr
  ASSERT_EQ(CSConsist::getLeadLocoCSConsist(leadLoco), nullptr);
}

/**
 * @brief Test getting a CSConsist with an unknown address returns nullptr
 */
TEST_F(CSConsistTests, TestGetLeadLocoWithUnknownFails) {
  // Set a lead loco address to check for in the list of CSConsists and create consists
  int leadLoco = 99;
  CSConsist *csConsist1 = new CSConsist();
  csConsist1->addMember(3, false);
  csConsist1->addMember(5, true);
  CSConsist *csConsist2 = new CSConsist();
  csConsist2->addMember(13, false);
  csConsist2->addMember(15, true);
  CSConsist *csConsist3 = new CSConsist();
  csConsist3->addMember(23, false);
  csConsist3->addMember(25, true);

  // Calling CSConsist::getLeadLocoCSConsist() should return nullptr
  ASSERT_EQ(CSConsist::getLeadLocoCSConsist(leadLoco), nullptr);
}

/**
 * @brief Test getting a CSConsist with the specific member address
 */
TEST_F(CSConsistTests, TestGetMemberCSConsist) {
  // Set a member loco address to check for in the list of CSConsists and create consists
  int memberLoco = 25;
  CSConsist *csConsist1 = new CSConsist();
  csConsist1->addMember(3, false);
  csConsist1->addMember(5, true);
  CSConsist *csConsist2 = new CSConsist();
  csConsist2->addMember(13, false);
  csConsist2->addMember(15, true);
  CSConsist *csConsist3 = new CSConsist();
  csConsist3->addMember(23, false);
  csConsist3->addMember(25, true);

  // Calling CSConsist::getLeadLocoCSConsist() should return csConsist3
  ASSERT_EQ(CSConsist::getMemberCSConsist(memberLoco), csConsist3);
}

/**
 * @brief Test getting a CSConsist with a lead loco address returns the CSConsist
 */
TEST_F(CSConsistTests, TestGetMemberWithLeadCSConsist) {
  // Set a member loco address to check for in the list of CSConsists and create consists
  int memberLoco = 13;
  CSConsist *csConsist1 = new CSConsist();
  csConsist1->addMember(3, false);
  csConsist1->addMember(5, true);
  CSConsist *csConsist2 = new CSConsist();
  csConsist2->addMember(13, false);
  csConsist2->addMember(15, true);
  CSConsist *csConsist3 = new CSConsist();
  csConsist3->addMember(23, false);
  csConsist3->addMember(25, true);

  // Calling CSConsist::getLeadLocoCSConsist() should return csConsist2
  ASSERT_EQ(CSConsist::getMemberCSConsist(memberLoco), csConsist2);
}

/**
 * @brief Test getting a CSConsist with an unknown address returns nullptr
 */
TEST_F(CSConsistTests, TestGetMemberWithUnknownFails) {
  // Set a member loco address to check for in the list of CSConsists and create consists
  int memberLoco = 99;
  CSConsist *csConsist1 = new CSConsist();
  csConsist1->addMember(3, false);
  csConsist1->addMember(5, true);
  CSConsist *csConsist2 = new CSConsist();
  csConsist2->addMember(13, false);
  csConsist2->addMember(15, true);
  CSConsist *csConsist3 = new CSConsist();
  csConsist3->addMember(23, false);
  csConsist3->addMember(25, true);

  // Calling CSConsist::getLeadLocoCSConsist() should return nullptr
  ASSERT_EQ(CSConsist::getMemberCSConsist(memberLoco), nullptr);
}

/**
 * @brief Test removing all CSConsistMember objects clears list and doesn't crash
 */
TEST_F(CSConsistTests, TestRemoveAllMembers) {
  // Create the consist and add members
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(42, false);
  csConsist->addMember(24, true);
  csConsist->addMember(3, false);

  // Ensure list order is correct
  ASSERT_NE(csConsist->getFirstMember(), nullptr);
  ASSERT_TRUE(csConsist->isValid());
  CSConsistMember *member = csConsist->getFirstMember();
  EXPECT_EQ(member->address, 42);
  member = member->next;
  ASSERT_NE(member, nullptr);
  EXPECT_EQ(member->address, 24);
  member = member->next;
  ASSERT_NE(member, nullptr);
  EXPECT_EQ(member->address, 3);
  ASSERT_EQ(member->next, nullptr);

  // Call removeAllMembers() and validate
  csConsist->removeAllMembers();
  ASSERT_FALSE(csConsist->isValid());
  ASSERT_EQ(csConsist->getFirstMember(), nullptr);
  EXPECT_EQ(csConsist->getMemberCount(), 0);
}
