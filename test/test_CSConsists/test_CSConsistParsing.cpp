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
 * @brief Test receiving a consist list creates objects
 */
TEST_F(CSConsistTests, TestReceivingConsistList) {
  // Simulate receiving a consist
  _stream << "<^ 42 -24 3>";
  _dccexProtocol.check();

  // There now should be a first CSConsist
  ASSERT_NE(CSConsist::getFirst(), nullptr);

  // Check members and attributes
  CSConsist *csConsist = CSConsist::getFirst();
  ASSERT_NE(csConsist->getLeadLoco(), nullptr);
  EXPECT_EQ(csConsist->getLeadLoco()->getAddress(), 42);
  ASSERT_NE(csConsist->getFirstMember(), nullptr);
  CSConsistMember *first = csConsist->getFirstMember();
  EXPECT_EQ(first->getLoco()->getAddress(), 42);
  EXPECT_FALSE(first->isReversed());
  CSConsistMember *second = first->getNext();
  ASSERT_NE(second, nullptr);
  EXPECT_EQ(second->getLoco()->getAddress(), 24);
  EXPECT_TRUE(second->isReversed());
  ASSERT_NE(second->getNext(), nullptr);
  EXPECT_EQ(second->getNext()->getLoco()->getAddress(), 3);
  EXPECT_FALSE(second->getNext()->isReversed());
}

/**
 * @brief Test receiving a single loco consist doesn't create one
 */
TEST_F(CSConsistTests, TestReceivingInvalidConsist) {
  // Simulate receiving a single loco consist
  _stream << "<^ 42>";
  _dccexProtocol.check();

  // There should be no CSConsist created
  ASSERT_EQ(CSConsist::getFirst(), nullptr);
}

/**
 * @brief Test a two loco consist
 */
TEST_F(CSConsistTests, TestTwoLocoConsist) {
  // Simulate receiving a two loco consist
  _stream << "<^ 42 -24>";
  _dccexProtocol.check();

  // There now should be a first CSConsist
  ASSERT_NE(CSConsist::getFirst(), nullptr);

  // Check members and attributes
  CSConsist *csConsist = CSConsist::getFirst();
  ASSERT_NE(csConsist->getLeadLoco(), nullptr);
  EXPECT_EQ(csConsist->getLeadLoco()->getAddress(), 42);
  ASSERT_NE(csConsist->getFirstMember(), nullptr);
  CSConsistMember *first = csConsist->getFirstMember();
  EXPECT_EQ(first->getLoco()->getAddress(), 42);
  EXPECT_FALSE(first->isReversed());
  CSConsistMember *second = first->getNext();
  ASSERT_NE(second, nullptr);
  EXPECT_EQ(second->getLoco()->getAddress(), 24);
  EXPECT_TRUE(second->isReversed());
}

/**
 * @brief Test consist is built correctly with reversed lead loco
 */
TEST_F(CSConsistTests, TestReversedLeadLoco) {
  // Simulate receiving a two loco consist
  _stream << "<^ -42 24>";
  _dccexProtocol.check();

  // There now should be a first CSConsist
  ASSERT_NE(CSConsist::getFirst(), nullptr);

  // Check members and attributes
  CSConsist *csConsist = CSConsist::getFirst();
  ASSERT_NE(csConsist->getLeadLoco(), nullptr);
  EXPECT_EQ(csConsist->getLeadLoco()->getAddress(), 42);
  ASSERT_NE(csConsist->getFirstMember(), nullptr);
  CSConsistMember *first = csConsist->getFirstMember();
  EXPECT_EQ(first->getLoco()->getAddress(), 42);
  EXPECT_TRUE(first->isReversed());
  CSConsistMember *second = first->getNext();
  ASSERT_NE(second, nullptr);
  EXPECT_EQ(second->getLoco()->getAddress(), 24);
  EXPECT_FALSE(second->isReversed());
}

/**
 * @brief Test receiving multiple consists builds them correctly
 */
TEST_F(CSConsistTests, TestMultipleConsists) {
  // Simulate receiving multiple consists
  _stream << "<^ -42 24><^ 3 -33 99><^ 21 22>";
  _dccexProtocol.check();

  // Validate the CSConsists are created correctly
  ASSERT_NE(CSConsist::getFirst(), nullptr);
  CSConsist *first = CSConsist::getFirst();
  EXPECT_EQ(first->getLeadLoco()->getAddress(), 42);
  EXPECT_EQ(first->getNext()->getLeadLoco()->getAddress(), 3);
  EXPECT_EQ(first->getNext()->getNext()->getLeadLoco()->getAddress(), 21);
}

/**
 * @brief Test receiving a consist with a conflicting lead loco resolves to the CS version
 */
TEST_F(CSConsistTests, TestConflictingLeadLoco) {
  // Create an existing CSConsist that is valid
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(42, false);
  csConsist->addMember(24, true);
  csConsist->addMember(3, false);

  // Simulate receiving a consist that contains a conflicting lead loco address
  _stream << "<^ -42 24 -3>";
  _dccexProtocol.check();

  // The existing CSConsist should be updated to reflect the received version
  CSConsistMember *member = csConsist->getFirstMember();
  ASSERT_NE(member, nullptr);
  EXPECT_EQ(member->getLoco()->getAddress(), 42);
  EXPECT_TRUE(member->isReversed());
  member = member->getNext();
  ASSERT_NE(member, nullptr);
  EXPECT_EQ(member->getLoco()->getAddress(), 24);
  EXPECT_FALSE(member->isReversed());
  member = member->getNext();
  ASSERT_NE(member, nullptr);
  EXPECT_EQ(member->getLoco()->getAddress(), 3);
  EXPECT_TRUE(member->isReversed());
  EXPECT_EQ(member->getNext(), nullptr);
}

/**
 * @brief Test receiving a consist with a loco as a member removes from any other CSConsist
 */
TEST_F(CSConsistTests, TestConflictingMemberLoco) {
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

  // Validate the members that will be in the simulated consist are in others
  EXPECT_TRUE(csConsist1->isInConsist(5));
  EXPECT_TRUE(csConsist3->isInConsist(25));

  // Simulate receiving a consist with those members
  _stream << "<^ 42 -5 25>";
  _dccexProtocol.check();

  // Verify there is now a fourth CSConsist with 42 lead loco and correct members
  CSConsist *csConsist4 = csConsist3->getNext();
  ASSERT_NE(csConsist4, nullptr);
  CSConsistMember *member = csConsist4->getFirstMember();
  ASSERT_NE(member, nullptr);
  EXPECT_EQ(member->getLoco()->getAddress(), 42);
  EXPECT_FALSE(member->isReversed());
  member = member->getNext();
  ASSERT_NE(member, nullptr);
  EXPECT_EQ(member->getLoco()->getAddress(), 5);
  EXPECT_TRUE(member->isReversed());
  member = member->getNext();
  ASSERT_NE(member, nullptr);
  EXPECT_EQ(member->getLoco()->getAddress(), 25);
  EXPECT_FALSE(member->isReversed());
  EXPECT_EQ(member->getNext(), nullptr);

  // Verify that locos 5 and 25 aren't in the other CSConsists any more
  EXPECT_FALSE(csConsist1->isInConsist(5));
  EXPECT_FALSE(csConsist3->isInConsist(25));
}

/**
 * @brief Test receiving a consist with members of two different CSConsists resolve correctly
 */
TEST_F(CSConsistTests, TestMemberShuffles) {
  // Create some CSConsists
  CSConsist *csConsist1 = new CSConsist();
  csConsist1->addMember(10, false);
  csConsist1->addMember(20, true);
  CSConsist *csConsist2 = new CSConsist();
  csConsist2->addMember(30, false);
  csConsist2->addMember(40, true);

  // Simulate receiving a consist with those members
  _stream << "<^ 10 -20 30>";
  _dccexProtocol.check();

  // Verify there are still only two CSConsists with correct members
  ASSERT_EQ(CSConsist::getFirst(), csConsist1);
  EXPECT_EQ(CSConsist::getFirst()->getNext(), csConsist2);
  EXPECT_EQ(csConsist2->getNext(), nullptr);
  EXPECT_EQ(CSConsist::getLeadLocoCSConsist(10), csConsist1);
  EXPECT_EQ(CSConsist::getLeadLocoCSConsist(40), csConsist2);
  EXPECT_EQ(CSConsist::getMemberCSConsist(20), csConsist1);
  EXPECT_EQ(CSConsist::getMemberCSConsist(30), csConsist1);
}
