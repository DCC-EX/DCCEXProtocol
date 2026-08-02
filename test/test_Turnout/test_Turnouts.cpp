/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
 *
 * Copyright © 2024 Peter Cole
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

#include "../setup/TurnoutTests.h"

TEST_F(TurnoutTests, createSingleTurnout) {
  // Create a turnout 100
  Turnout *turnout100 = new Turnout(100, false);
  turnout100->setName("Turnout 100");

  // Validate turnout details
  EXPECT_EQ(turnout100->getId(), 100);
  EXPECT_STREQ(turnout100->getName(), "Turnout 100");
  EXPECT_FALSE(turnout100->getThrown());

  // Validate it's in the list by ID
  EXPECT_EQ(_dccexProtocol.turnouts->getById(100), turnout100);
}

TEST_F(TurnoutTests, createTurnoutList) {
  // Create three turnouts
  Turnout *turnout100 = new Turnout(100, false);
  turnout100->setName("Turnout 100");
  Turnout *turnout101 = new Turnout(101, true);
  turnout101->setName("Turnout 101");
  Turnout *turnout102 = new Turnout(102, false);
  turnout102->setName("");

  // Validate turnouts are in the list
  EXPECT_EQ(_dccexProtocol.turnouts->getById(100), turnout100);
  EXPECT_EQ(_dccexProtocol.turnouts->getById(101), turnout101);
  EXPECT_EQ(_dccexProtocol.turnouts->getById(102), turnout102);

  // Validate turnout details
  EXPECT_EQ(turnout100->getId(), 100);
  EXPECT_STREQ(turnout100->getName(), "Turnout 100");
  EXPECT_FALSE(turnout100->getThrown());

  // Validate turnout details
  EXPECT_EQ(turnout101->getId(), 101);
  EXPECT_STREQ(turnout101->getName(), "Turnout 101");
  EXPECT_TRUE(turnout101->getThrown());

  // Validate turnout details
  EXPECT_EQ(turnout102->getId(), 102);
  EXPECT_STREQ(turnout102->getName(), "");
  EXPECT_FALSE(turnout102->getThrown());
}

TEST_F(TurnoutTests, operateTurnout) {
  // Create a turnout 100
  Turnout *turnout100 = new Turnout(100, false);
  turnout100->setName("Turnout 100");

  // Close it and validate
  turnout100->setThrown(false);
  EXPECT_FALSE(turnout100->getThrown());

  // Throw it and validate
  turnout100->setThrown(true);
  EXPECT_TRUE(turnout100->getThrown());

  // Close it and validate
  turnout100->setThrown(false);
  EXPECT_FALSE(turnout100->getThrown());
}

/**
 * @brief Test closing a turnout updates objects and CS
 */
TEST_F(TurnoutTests, TestCloseTurnout) {
  // Create a turnout to update
  Turnout *turnout100 = new Turnout(100, false);
  ASSERT_FALSE(turnout100->getThrown());

  // Test close command
  _dccexProtocol.throwTurnout(100);
  EXPECT_EQ(_stream.getOutput(), "<T 100 1>");

  // Simulate receiving a throw broadcast and validate
  EXPECT_CALL(_delegate, receivedTurnoutAction(100, 1));
  _stream << "<H 100 1>";
  _dccexProtocol.check();
  EXPECT_TRUE(turnout100->getThrown());
}

/**
 * @brief Test throwing a turnout updates objects and CS
 */
TEST_F(TurnoutTests, TestThrowTurnout) {
  // Create a turnout to update
  Turnout *turnout100 = new Turnout(100, true);
  ASSERT_TRUE(turnout100->getThrown());

  // Test throw command
  _dccexProtocol.closeTurnout(100);
  EXPECT_EQ(_stream.getOutput(), "<T 100 0>");

  // Simulate receiving a close broadcast and validate
  EXPECT_CALL(_delegate, receivedTurnoutAction(100, 0));
  _stream << "<H 100 0>";
  _dccexProtocol.check();
  EXPECT_FALSE(turnout100->getThrown());
}

/**
 * @brief Test toggling a turnout updates objects and CS
 */
TEST_F(TurnoutTests, TestToggleTurnout) {
  // Create a turnout to update
  Turnout *turnout100 = new Turnout(100, false);
  ASSERT_FALSE(turnout100->getThrown());

  // Test toggle command sends throw
  _dccexProtocol.toggleTurnout(100);
  EXPECT_EQ(_stream.getOutput(), "<T 100 1>");
  _stream.clearOutput();

  // Simulate receiving a throw broadcast and validate
  EXPECT_CALL(_delegate, receivedTurnoutAction(100, 1));
  _stream << "<H 100 1>";
  _dccexProtocol.check();
  EXPECT_TRUE(turnout100->getThrown());

  // Toggle again should now close
  _dccexProtocol.toggleTurnout(100);
  EXPECT_EQ(_stream.getOutput(), "<T 100 0>");
  _stream.clearOutput();

  // Simulate receiving a close broadcast and validate
  EXPECT_CALL(_delegate, receivedTurnoutAction(100, 0));
  _stream << "<H 100 0>";
  _dccexProtocol.check();
  EXPECT_FALSE(turnout100->getThrown());
}

/**
 * @brief Test setting a null name is a no-op and does not crash
 */
TEST_F(TurnoutTests, setNameNullIsNoOp) {
  // Create a turnout with a name, then clear it with a null name
  Turnout *turnout = new Turnout(100, false);
  turnout->setName("Turnout 100");

  // Calling setName(nullptr) must not crash and should clear the name
  EXPECT_NO_FATAL_FAILURE(turnout->setName(nullptr));
  EXPECT_STREQ(turnout->getName(), "Turnout 100");
}

/**
 * @brief Test deleting a middle turnout in the list preserves the remaining list
 */
TEST_F(TurnoutTests, TestDeleteMiddleTurnout) {
  // Create three turnouts
  Turnout *turnout100 = new Turnout(100, false);
  turnout100->setName("Turnout 100");
  Turnout *turnout101 = new Turnout(101, true);
  turnout101->setName("Turnout 101");
  Turnout *turnout102 = new Turnout(102, false);
  turnout102->setName("Turnout 102");

  // Validate the initial list
  ASSERT_EQ(Turnout::getFirst(), turnout100);
  EXPECT_EQ(turnout100->getNext(), turnout101);
  EXPECT_EQ(turnout101->getNext(), turnout102);
  EXPECT_EQ(turnout102->getNext(), nullptr);

  // Delete the middle of the list
  delete turnout101;

  // The remaining list must be linked directly and intact
  ASSERT_EQ(Turnout::getFirst(), turnout100);
  EXPECT_EQ(turnout100->getNext(), turnout102);
  EXPECT_EQ(turnout102->getNext(), nullptr);
  EXPECT_EQ(turnout100->getId(), 100);
  EXPECT_EQ(turnout102->getId(), 102);

  // The deleted turnout must no longer be reachable from the list head
  Turnout *current = Turnout::getFirst();
  while (current) {
    EXPECT_NE(current, turnout101);
    current = current->getNext();
  }
}

/**
 * @brief Test deleting the last turnout in the list preserves the remaining list
 */
TEST_F(TurnoutTests, TestDeleteLastTurnout) {
  // Create three turnouts
  Turnout *turnout100 = new Turnout(100, false);
  turnout100->setName("Turnout 100");
  Turnout *turnout101 = new Turnout(101, true);
  turnout101->setName("Turnout 101");
  Turnout *turnout102 = new Turnout(102, false);
  turnout102->setName("Turnout 102");

  // Validate the initial list
  ASSERT_EQ(Turnout::getFirst(), turnout100);
  EXPECT_EQ(turnout100->getNext(), turnout101);
  EXPECT_EQ(turnout101->getNext(), turnout102);
  EXPECT_EQ(turnout102->getNext(), nullptr);

  // Delete the last in the list
  delete turnout102;

  // The remaining list must terminate correctly
  ASSERT_EQ(Turnout::getFirst(), turnout100);
  EXPECT_EQ(turnout100->getNext(), turnout101);
  EXPECT_EQ(turnout101->getNext(), nullptr);
  EXPECT_EQ(turnout100->getId(), 100);
  EXPECT_EQ(turnout101->getId(), 101);
}
