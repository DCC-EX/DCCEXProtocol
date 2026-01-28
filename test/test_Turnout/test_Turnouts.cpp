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
