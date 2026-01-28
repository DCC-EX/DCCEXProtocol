/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
 *
 * Copyright © 2024 Vincent Hamp
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

#include "../setup/LocoTests.h"

/// @brief Create a single Loco
TEST_F(LocoTests, createSingleLoco) {
  // Create an individual loco
  Loco *loco1 = new Loco(1, LocoSource::LocoSourceEntry);
  loco1->setName("Loco 1");
  loco1->setupFunctions("Lights/*Horn/Bell///Function 5");

  // Check address is 1, name is correct, and LocoSource correct
  EXPECT_EQ(loco1->getAddress(), 1);
  EXPECT_STREQ(loco1->getName(), "Loco 1");
  EXPECT_EQ(loco1->getSource(), LocoSource::LocoSourceEntry);

  // Check our functions
  EXPECT_FALSE(loco1->isFunctionMomentary(0));
  EXPECT_TRUE(loco1->isFunctionMomentary(1));
  EXPECT_STREQ(loco1->getFunctionName(2), "Bell");
  EXPECT_STREQ(loco1->getFunctionName(5), "Function 5");

  // Check speed/direction changes
  EXPECT_EQ(loco1->getSpeed(), 0);
  EXPECT_EQ(loco1->getDirection(), Direction::Forward);
  loco1->setSpeed(13);
  loco1->setDirection(Direction::Reverse);
  EXPECT_EQ(loco1->getSpeed(), 13);
  EXPECT_EQ(loco1->getDirection(), Direction::Reverse);

  // Make sure this is not in the roster
  EXPECT_EQ(Loco::getFirst(), nullptr);

  // Make sure we can't find it by address either
  EXPECT_EQ(Loco::getByAddress(1), nullptr);

  // Ensure next is nullptr as this is the only loco
  EXPECT_EQ(loco1->getNext(), nullptr);

  // Clean up
  delete loco1;
}

/// @brief Create a roster of Locos
TEST_F(LocoTests, createRoster) {
  // Roster should start empty, don't continue if it isn't
  ASSERT_EQ(_dccexProtocol.roster->getFirst(), nullptr);

  // Add three locos
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  loco42->setName("Loco42");
  Loco *loco9 = new Loco(9, LocoSource::LocoSourceRoster);
  loco9->setName("Loco9");
  Loco *loco120 = new Loco(120, LocoSource::LocoSourceRoster);
  loco120->setName("Loco120");

  // Now verify the roster, fatal error if first is nullptr
  Loco *firstLoco = _dccexProtocol.roster->getFirst();
  ASSERT_NE(firstLoco, nullptr);

  // Check first loco details
  EXPECT_EQ(firstLoco->getAddress(), 42);
  EXPECT_STREQ(firstLoco->getName(), "Loco42");
  EXPECT_EQ(firstLoco->getSource(), LocoSource::LocoSourceRoster);

  // Verify second loco details and fail fatally if next is nullptr
  Loco *secondLoco = firstLoco->getNext();
  ASSERT_NE(secondLoco, nullptr);
  EXPECT_EQ(secondLoco->getAddress(), 9);
  EXPECT_STREQ(secondLoco->getName(), "Loco9");
  EXPECT_EQ(secondLoco->getSource(), LocoSource::LocoSourceRoster);

  // Verify third loco details and fail fatally if next is nullptr
  Loco *thirdLoco = secondLoco->getNext();
  ASSERT_NE(thirdLoco, nullptr);
  EXPECT_EQ(thirdLoco->getAddress(), 120);
  EXPECT_STREQ(thirdLoco->getName(), "Loco120");
  EXPECT_EQ(thirdLoco->getSource(), LocoSource::LocoSourceRoster);

  // Verify end of linked list and fail fatally if next is not nullptr
  EXPECT_EQ(thirdLoco->getNext(), nullptr)
      << "Unexpected fourth Loco at address: " << thirdLoco->getNext()->getAddress();
}
