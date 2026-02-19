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
  CSConsist *csConsist = _dccexProtocol.createCSConsist(42, false, true);
  _dccexProtocol.addCSConsistMember(csConsist, 24, true);
  _dccexProtocol.addCSConsistMember(csConsist, 3);
  // Clear stream from consist creation
  _stream.clearOutput();

  // Loco shouldn't exist yet
  Loco *loco42 = Loco::getByAddress(42);
  ASSERT_EQ(loco42, nullptr);

  // Set function 0 on for the consist
  _dccexProtocol.functionOn(csConsist, 0);

  // This should cause a Loco object to be created for the lead loco
  Loco *first = Loco::getFirstLocalLoco();
  ASSERT_NE(first, nullptr);
  EXPECT_EQ(first->getAddress(), 42);
  EXPECT_EQ(first->getNext(), nullptr);

  // This should also have a function 0 call in the buffer for each loco
  EXPECT_EQ(_stream.getOutput(), "<F 42 0 1><F 24 0 1><F 3 0 1>");
}

/**
 * @brief Test setting functions for the lead loco does not replicate when disabled
 */
TEST_F(CSConsistTests, TestSetLeadLocoFunctionDoesNotReplicate) {
  // Create the consist
  CSConsist *csConsist = _dccexProtocol.createCSConsist(42);
  _dccexProtocol.addCSConsistMember(csConsist, 24, true);
  _dccexProtocol.addCSConsistMember(csConsist, 3);
  ASSERT_FALSE(csConsist->getReplicateFunctions());

  // Clear stream from consist creation
  _stream.clearOutput();

  // Loco shouldn't exist yet
  Loco *loco42 = Loco::getByAddress(42);
  ASSERT_EQ(loco42, nullptr);

  // Set function 0 on for the consist
  _dccexProtocol.functionOn(csConsist, 0);

  // This should cause a Loco object to be created for the lead loco only
  Loco *first = Loco::getFirstLocalLoco();
  ASSERT_NE(first, nullptr);
  EXPECT_EQ(first->getAddress(), 42);
  EXPECT_EQ(first->getNext(), nullptr);

  // This should only have a function 0 call for the lead loco in the buffer
  EXPECT_EQ(_stream.getOutput(), "<F 42 0 1>");
}

/**
 * @brief Test turning a function on for an invalid CSConsist fails sanely
 */
TEST_F(CSConsistTests, TestSetFunctionOnInvalidCSConsist) {
  // Create a CSConsist with only one member, invalid
  CSConsist *csConsist = _dccexProtocol.createCSConsist(3);
  _dccexProtocol.functionOn(csConsist, 0);
  EXPECT_EQ(_stream.getOutput(), "");
  EXPECT_EQ(Loco::getFirstLocalLoco(), nullptr);
}

/**
 * @brief Test setting functions off for the lead loco replicates when enabled
 */
TEST_F(CSConsistTests, TestSetLeadLocoFunctionOffReplicates) {
  // Create the consist
  CSConsist *csConsist = new CSConsist(true);
  csConsist->addMember(42, false);
  csConsist->addMember(24, true);
  csConsist->addMember(3, false);

  // Loco shouldn't exist yet
  Loco *loco42 = Loco::getByAddress(42);
  ASSERT_EQ(loco42, nullptr);

  // Set function 0 off for the consist
  _dccexProtocol.functionOff(csConsist, 0);

  // This should cause a Loco object to be created for the lead loco
  Loco *first = Loco::getFirstLocalLoco();
  ASSERT_NE(first, nullptr);
  EXPECT_EQ(first->getAddress(), 42);
  EXPECT_EQ(first->getNext(), nullptr);

  // This should also have a function 0 call in the buffer for each loco
  EXPECT_EQ(_stream.getOutput(), "<F 42 0 0><F 24 0 0><F 3 0 0>");
}

/**
 * @brief Test setting functions off for the lead loco does not replicate when disabled
 */
TEST_F(CSConsistTests, TestSetLeadLocoFunctionOffDoesNotReplicate) {
  // Create the consist
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(42, false);
  csConsist->addMember(24, true);
  csConsist->addMember(3, false);

  // Locos shouldn't exist yet
  Loco *loco42 = Loco::getByAddress(42);
  ASSERT_EQ(loco42, nullptr);

  // Set function 0 on for the consist
  _dccexProtocol.functionOff(csConsist, 0);

  // This should cause a Loco object to be created for the lead loco only
  Loco *first = Loco::getFirstLocalLoco();
  ASSERT_NE(first, nullptr);
  EXPECT_EQ(first->getAddress(), 42);
  EXPECT_EQ(first->getNext(), nullptr);

  // This should only have a function 0 call for the lead loco in the buffer
  EXPECT_EQ(_stream.getOutput(), "<F 42 0 0>");
}

/**
 * @brief Test turning a function off for an invalid CSConsist fails sanely
 */
TEST_F(CSConsistTests, TestSetFunctionOffInvalidCSConsist) {
  // Create a CSConsist with only one member, invalid
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(3, false);
  _dccexProtocol.functionOff(csConsist, 0);
  EXPECT_EQ(_stream.getOutput(), "");
  EXPECT_EQ(Loco::getFirstLocalLoco(), nullptr);
}

/**
 * @brief Test isFunctionOn checks lead loco for function states
 */
TEST_F(CSConsistTests, TestIsFunctionOn) {
  // Create the consist
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(42, false);
  csConsist->addMember(24, true);
  csConsist->addMember(3, false);

  // Locos shouldn't exist
  Loco *loco42 = Loco::getByAddress(42);
  ASSERT_EQ(loco42, nullptr);

  // Check if function 0 is on, should be false due to no Loco
  bool state = _dccexProtocol.isFunctionOn(csConsist, 0);
  EXPECT_FALSE(state);

  // Turn it on and check again
  _dccexProtocol.functionOn(csConsist, 0);
  state = _dccexProtocol.isFunctionOn(csConsist, 0);
  EXPECT_FALSE(state);

  // Simulate receiving the broadcast which should turn it on
  _stream << "<l 42 0 128 1>";
  _dccexProtocol.check();
  state = _dccexProtocol.isFunctionOn(csConsist, 0);
  EXPECT_TRUE(state);
}

/**
 * @brief Test isFunctionOn for an invalid CSConsist fails sanely
 */
TEST_F(CSConsistTests, TestSetIsFunctionOnInvalidCSConsist) {
  // Create a CSConsist with only one member, invalid
  CSConsist *csConsist = new CSConsist();
  csConsist->addMember(3, false);
  bool state = _dccexProtocol.isFunctionOn(csConsist, 0);
  EXPECT_FALSE(state);
}
