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
 * @brief Basic CSConsistMember creation and deletion with roster loco
 */
TEST_F(CSConsistTests, TestCreateConsistMemberRosterLoco) {
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  CSConsistMember *member = new CSConsistMember(loco42, true);

  ASSERT_NE(member->getLoco(), nullptr);
  EXPECT_TRUE(member->isReversed());

  // Clean up
  delete member;
}

/**
 * @brief Basic CSConsistMember creation and deletion with local loco
 */
TEST_F(CSConsistTests, TestCreateConsistMemberLocalLoco) {
  Loco *loco3 = new Loco(3, LocoSource::LocoSourceEntry);
  CSConsistMember *member = new CSConsistMember(loco3, true);

  ASSERT_NE(member->getLoco(), nullptr);
  EXPECT_TRUE(member->isReversed());

  // Clean up
  delete member;
}

/**
 * @brief Basic CSConsistMember creation and deletion that is not reversed
 */
TEST_F(CSConsistTests, TestCreateConsistMemberNotReversed) {
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  CSConsistMember *member = new CSConsistMember(loco42, false);

  ASSERT_NE(member->getLoco(), nullptr);
  EXPECT_FALSE(member->isReversed());

  // Clean up
  delete member;
}

/**
 * @brief Basic CSConsist creation and deletion using a roster Loco
 */
TEST_F(CSConsistTests, TestCreateConsistRosterLoco) {
  // Create a loco and the consist
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(loco42, false);

  // First CS consist should now be set
  ASSERT_NE(CSConsist::getFirst(), nullptr);

  // Should be the only one in the list
  EXPECT_EQ(csConsist->getNext(), nullptr);

  // Lead loco should be correct
  EXPECT_EQ(csConsist->getLeadLoco(), loco42);

  // Created in CS, pending deletion, and valid flags should be false
  EXPECT_FALSE(csConsist->isCreatedInCS());
  EXPECT_FALSE(csConsist->isDeleteCSPending());
  EXPECT_FALSE(csConsist->isValid());

  // Clean up
  delete csConsist;
}

/**
 * @brief Basic CSConsist creation and deletion using a local Loco
 */
TEST_F(CSConsistTests, TestCreateConsistLocalLoco) {
  // Create a loco and the consist
  Loco *loco3 = new Loco(3, LocoSource::LocoSourceEntry);
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(loco3, false);

  // First CS consist should now be set
  ASSERT_NE(CSConsist::getFirst(), nullptr);

  // Should be the only one in the list
  EXPECT_EQ(csConsist->getNext(), nullptr);

  // Lead loco should be correct
  EXPECT_EQ(csConsist->getLeadLoco(), loco3);

  // Created in CS, pending deletion, and valid flags should be false
  EXPECT_FALSE(csConsist->isCreatedInCS());
  EXPECT_FALSE(csConsist->isDeleteCSPending());
  EXPECT_FALSE(csConsist->isValid());

  // Clean up
  delete csConsist;
}

/**
 * @brief Basic CSConsist creation and deletion using an address
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
  EXPECT_NE(csConsist->getLeadLoco(), nullptr);
  Loco *loco = csConsist->getLeadLoco();
  EXPECT_EQ(loco->getAddress(), 3);

  // Created in CS, pending deletion, and valid flags should be false
  EXPECT_FALSE(csConsist->isCreatedInCS());
  EXPECT_FALSE(csConsist->isDeleteCSPending());
  EXPECT_FALSE(csConsist->isValid());

  // Clean up
  delete csConsist;
}

/**
 * @brief Test building a consist with loco objects
 */
TEST_F(CSConsistTests, TestBuildConsistWithLocos) {
  // Create some locos
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  Loco *loco24 = new Loco(24, LocoSource::LocoSourceRoster);
  Loco *loco3 = new Loco(3, LocoSource::LocoSourceEntry);

  // Now create the consist and validate
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(loco42, false);
  ASSERT_NE(CSConsist::getFirst(), nullptr);
  ASSERT_EQ(csConsist->getLeadLoco(), loco42);

  // Now add the other locos with 24 reversed
  csConsist->addMember(loco24, true);
  csConsist->addMember(loco3, false);

  // Now validate the expected state
  EXPECT_TRUE(csConsist->isInConsist(loco42));
  EXPECT_FALSE(csConsist->isReversed(loco42));
  EXPECT_TRUE(csConsist->isInConsist(loco24));
  EXPECT_TRUE(csConsist->isInConsist(loco3));
  EXPECT_TRUE(csConsist->isReversed(loco24));
  EXPECT_FALSE(csConsist->isReversed(loco3));

  // Make sure member is accessible and list is as expected
  CSConsistMember *member24 = csConsist->getMember(loco24);
  CSConsistMember *member3 = csConsist->getMember(loco3);
  ASSERT_NE(member24, nullptr);
  ASSERT_NE(member3, nullptr);
  EXPECT_EQ(member24->getLoco(), loco24);
  EXPECT_EQ(member3->getLoco(), loco3);
  EXPECT_EQ(member24->getNext(), member3);
  EXPECT_EQ(member3->getNext(), nullptr);

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
  ASSERT_EQ(csConsist->getLeadLoco()->getAddress(), 42);

  // Now add the other locos with 24 reversed
  csConsist->addMember(24, true);
  csConsist->addMember(3, false);

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
  EXPECT_EQ(member24->getLoco()->getAddress(), 24);
  EXPECT_EQ(member3->getLoco()->getAddress(), 3);
  EXPECT_EQ(member24->getNext(), member3);
  EXPECT_EQ(member3->getNext(), nullptr);

  // Clean up
  delete csConsist;
}

/**
 * @brief Test building a mixed consist
 */
TEST_F(CSConsistTests, TestBuildMixedConsist) {
  // Create some locos
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  Loco *loco3 = new Loco(3, LocoSource::LocoSourceEntry);

  // Now create the consist and validate
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(loco42, false);
  ASSERT_NE(CSConsist::getFirst(), nullptr);
  ASSERT_EQ(csConsist->getLeadLoco(), loco42);

  // Now add the other locos with 24 reversed
  csConsist->addMember(24, true);
  csConsist->addMember(loco3, false);

  // Now validate the expected state
  EXPECT_TRUE(csConsist->isInConsist(loco42));
  EXPECT_FALSE(csConsist->isReversed(loco42));
  EXPECT_TRUE(csConsist->isInConsist(24));
  EXPECT_TRUE(csConsist->isInConsist(loco3));
  EXPECT_TRUE(csConsist->isReversed(24));
  EXPECT_FALSE(csConsist->isReversed(loco3));

  // Make sure member is accessible and list is as expected
  CSConsistMember *member24 = csConsist->getMember(24);
  CSConsistMember *member3 = csConsist->getMember(3);
  ASSERT_NE(member24, nullptr);
  ASSERT_NE(member3, nullptr);
  EXPECT_EQ(member24->getLoco()->getAddress(), 24);
  EXPECT_EQ(member3->getLoco(), loco3);
  EXPECT_EQ(member24->getNext(), member3);
  EXPECT_EQ(member3->getNext(), nullptr);

  // Clean up
  delete csConsist;
}

/**
 * @brief Test accessing invalid members returns sensibly
 */
TEST_F(CSConsistTests, TestAccessingInvalidMembers) {
  // Create some locos and a consist
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceEntry);
  Loco *loco999 = new Loco(999, LocoSource::LocoSourceEntry);
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(loco42, false);

  // Test membership with 999 and addresses returns sane answers
  EXPECT_FALSE(csConsist->isInConsist(loco999));
  EXPECT_FALSE(csConsist->isInConsist(22));
  EXPECT_FALSE(csConsist->isInConsist(nullptr));
  EXPECT_EQ(csConsist->getMember(loco999), nullptr);
  EXPECT_EQ(csConsist->getMember(22), nullptr);
  EXPECT_EQ(csConsist->getMember(nullptr), nullptr);
  EXPECT_FALSE(csConsist->isReversed(loco999));
  EXPECT_FALSE(csConsist->isReversed(22));
  EXPECT_FALSE(csConsist->isReversed(nullptr));
}

/**
 * @brief Test calling methods with invalid parameters doesn't crash
 */
TEST_F(CSConsistTests, TestCallingWithInvalidParams) {
  // Create consist and a loco for testing
  Loco *loco999 = new Loco(999, LocoSource::LocoSourceEntry);
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(42, false);

  // Test adding with invalid values doesn't cause a crash
  csConsist->addMember(nullptr, true);
  csConsist->removeMember(loco999);
  csConsist->removeMember(nullptr);
  csConsist->removeMember(22);
}

/**
 * @brief Test removing a member loco
 */
TEST_F(CSConsistTests, TestRemoveMemberLoco) {
  // Create some locos
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  Loco *loco24 = new Loco(24, LocoSource::LocoSourceRoster);
  Loco *loco3 = new Loco(3, LocoSource::LocoSourceEntry);

  // Now create the consist and add members
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(loco42, false);
  csConsist->addMember(loco24, true);
  csConsist->addMember(loco3, false);
  ASSERT_EQ(csConsist->getFirstMember()->getLoco(), loco42);

  // Remove loco24 and validate
  csConsist->removeMember(loco24);
  ASSERT_FALSE(csConsist->isInConsist(loco24));
  EXPECT_EQ(csConsist->getMember(loco24), nullptr);
  EXPECT_EQ(csConsist->getFirstMember()->getNext()->getLoco(), loco3);
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
  ASSERT_EQ(csConsist->getFirstMember()->getLoco()->getAddress(), 42);

  // Remove loco24 and validate
  csConsist->removeMember(24);
  ASSERT_FALSE(csConsist->isInConsist(24));
  EXPECT_EQ(csConsist->getMember(24), nullptr);
  EXPECT_EQ(csConsist->getFirstMember()->getNext()->getLoco()->getAddress(), 3);
}

/**
 * @brief Test removing all members cleans the list up
 */
TEST_F(CSConsistTests, TestRemoveAllMembers) {
  // Create some locos
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  Loco *loco24 = new Loco(24, LocoSource::LocoSourceRoster);
  Loco *loco3 = new Loco(3, LocoSource::LocoSourceEntry);

  // Now create the consist and add members
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(loco42, false);
  csConsist->addMember(loco24, true);
  csConsist->addMember(loco3, false);
  ASSERT_EQ(csConsist->getFirstMember()->getLoco(), loco42);

  // Remove loco24 and validate
  csConsist->removeMember(loco24);
  ASSERT_FALSE(csConsist->isInConsist(loco24));
  EXPECT_EQ(csConsist->getMember(loco24), nullptr);
  EXPECT_EQ(csConsist->getFirstMember()->getNext()->getLoco(), loco3);

  // Remove loco3 and validate
  csConsist->removeMember(loco3);
  ASSERT_FALSE(csConsist->isInConsist(loco3));
  EXPECT_EQ(csConsist->getMember(loco3), nullptr);

  // Remove loco42 and validate
  csConsist->removeMember(loco42);
  ASSERT_FALSE(csConsist->isInConsist(loco42));
  EXPECT_EQ(csConsist->getFirstMember(), nullptr);
}

/**
 * @brief Test adding duplicate locos doesn't crash
 */
TEST_F(CSConsistTests, TestAddDuplicates) {
  // Create some locos
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  Loco *loco24 = new Loco(24, LocoSource::LocoSourceRoster);
  Loco *loco3 = new Loco(3, LocoSource::LocoSourceEntry);

  // Now create the consist and add members
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(loco42, false);
  csConsist->addMember(loco24, true);
  csConsist->addMember(loco3, false);

  // Try adding duplicates
  csConsist->addMember(loco24, false);
  csConsist->addMember(loco3, true);
  EXPECT_TRUE(csConsist->isReversed(loco24));
  EXPECT_FALSE(csConsist->isReversed(loco3));

  // Add address duplicates
  csConsist->addMember(24, false);
  csConsist->addMember(3, true);
  EXPECT_TRUE(csConsist->isReversed(loco24));
  EXPECT_FALSE(csConsist->isReversed(loco3));

  // Count all members, more than 2 is a fail
  int memberCount = 0;
  for (CSConsistMember *member = csConsist->getFirstMember(); member; member = member->getNext()) {
    memberCount++;
  }
  EXPECT_EQ(memberCount, 3);
}
