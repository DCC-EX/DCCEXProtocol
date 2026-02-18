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
 * @brief Test setting replicate function flags
 */
TEST_F(CSConsistTests, TestReplicateFunctionFlags) {
  // Create  CSConsist before setting global flag and check
  CSConsist *csConsist1 = new CSConsist();
  EXPECT_FALSE(csConsist1->getReplicateFunctions());

  // Set the global flag
  CSConsist::setAlwaysReplicateFunctions(true);

  // Create a CSConsist and validate
  CSConsist *csConsist2 = new CSConsist();
  EXPECT_TRUE(csConsist2->getReplicateFunctions());

  // Explicitly set false and validate it still sets true
  CSConsist *csConsist3 = new CSConsist(false);
  EXPECT_TRUE(csConsist3->getReplicateFunctions());

  // Disable global flag
  CSConsist::setAlwaysReplicateFunctions(false);

  // Creating another should be false again
  CSConsist *csConsist4 = new CSConsist();
  EXPECT_FALSE(csConsist4->getReplicateFunctions());

  // Setting true should still take effect
  CSConsist *csConsist5 = new CSConsist(true);
  EXPECT_TRUE(csConsist5->getReplicateFunctions());

  // Test overrides
  csConsist5->setReplicateFunctions(false);
  EXPECT_FALSE(csConsist5->getReplicateFunctions());
  csConsist5->setReplicateFunctions(true);
  EXPECT_TRUE(csConsist5->getReplicateFunctions());
}

/**
 * @brief Test receiving a CSConsist list from the CS sets the flag correctly
 */
TEST_F(CSConsistTests, TestGlobalReplicationFromCSConsistList) {
  // Simulate receiving a consist without the global flag set
  _stream << "<^ 42 -24 3>";
  _dccexProtocol.check();

  // There now should be a first CSConsist, validate flag is not set
  CSConsist *first = CSConsist::getFirst();
  ASSERT_NE(first, nullptr);
  EXPECT_FALSE(first->getReplicateFunctions());

  // Set global flag and repeat for a new consist
  CSConsist::setAlwaysReplicateFunctions(true);
  _stream << "<^ 10 -5>";
  _dccexProtocol.check();

  // There now should be a first CSConsist, validate flag is not set
  CSConsist *second = first->getNext();
  ASSERT_NE(second, nullptr);
  EXPECT_TRUE(second->getReplicateFunctions());
}

/**
 * @brief Test setting functions for the lead loco replicates when enabled
 */
TEST_F(CSConsistTests, TestSetLeadLocoFunctionReplicates) {
  // Create the consist
  CSConsist *csConsist = new CSConsist(true);
  csConsist->addMember(42, false);
  csConsist->addMember(24, true);
  csConsist->addMember(3, false);

  // Locos shouldn't exist yet
  Loco *loco42 = Loco::getByAddress(42);
  ASSERT_EQ(loco42, nullptr);
  Loco *loco24 = Loco::getByAddress(24);
  ASSERT_EQ(loco24, nullptr);
  Loco *loco3 = Loco::getByAddress(3);
  ASSERT_EQ(loco3, nullptr);

  // Set function 0 on for lead loco
  
}

/**
 * @brief Test setting functions for the lead loco does not replicate when disabled
 */
TEST_F(CSConsistTests, TestSetLeadLocoFunctionDoesNotReplicate) {}
