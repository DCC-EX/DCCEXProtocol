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
