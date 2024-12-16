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

#include "../setup/LocoTests.h"

/// @brief Create a consist with three Loco objects
TEST_F(LocoTests, createConsistByLoco) {
  // Create three locos for the consist
  char *functionList = "Lights/*Horn";
  Loco *loco10 = new Loco(10, LocoSourceRoster);
  loco10->setName("Loco 10");
  loco10->setupFunctions(functionList);
  Loco *loco2 = new Loco(2, LocoSourceRoster);
  loco2->setName("Loco 2");
  loco2->setupFunctions(functionList);
  Loco *loco10000 = new Loco(10000, LocoSourceRoster);
  loco10000->setName("Loco 10000");
  loco10000->setupFunctions(functionList);

  // Add locos to the consist, with 2 reversed
  Consist *consist = new Consist();
  consist->setName("Test Legacy Consist");
  consist->addLoco(loco10, Facing::FacingForward);
  consist->addLoco(loco2, Facing::FacingReversed);
  consist->addLoco(loco10000, Facing::FacingForward);

  // Validate consist makeup by object and address
  EXPECT_STREQ(consist->getName(), "Test Legacy Consist");
  EXPECT_EQ(consist->getLocoCount(), 3);
  EXPECT_TRUE(consist->inConsist(loco10));
  EXPECT_TRUE(consist->inConsist(loco2));
  EXPECT_TRUE(consist->inConsist(loco10000));
  EXPECT_TRUE(consist->inConsist(10));
  EXPECT_TRUE(consist->inConsist(2));
  EXPECT_TRUE(consist->inConsist(10000));

  // Validate the first loco is 10
  EXPECT_EQ(consist->getFirst()->getLoco(), loco10);

  // Validate the consist speed and direction comes from the first loco
  EXPECT_EQ(consist->getSpeed(), 0);
  EXPECT_EQ(consist->getDirection(), Direction::Forward);
  loco2->setSpeed(35);
  loco10000->setDirection(Direction::Reverse);
  EXPECT_EQ(consist->getSpeed(), 0);
  EXPECT_EQ(consist->getDirection(), Direction::Forward);
  loco10->setSpeed(21);
  loco10->setDirection(Direction::Reverse);
  EXPECT_EQ(consist->getSpeed(), 21);
  EXPECT_EQ(consist->getDirection(), Direction::Reverse);

  // Validate removal of middle loco is as expected
  consist->removeLoco(loco2);
  EXPECT_EQ(consist->getLocoCount(), 2);
  EXPECT_EQ(consist->getFirst()->getLoco(), loco10);
  EXPECT_EQ(consist->getSpeed(), 21);
  EXPECT_EQ(consist->getDirection(), Direction::Reverse);

  // Validate removal of first loco is as expected
  consist->removeLoco(loco10);
  EXPECT_EQ(consist->getLocoCount(), 1);
  EXPECT_EQ(consist->getFirst()->getLoco(), loco10000);
  EXPECT_EQ(consist->getSpeed(), 0);
  EXPECT_EQ(consist->getDirection(), Direction::Reverse);

  // Validate removal of all locos
  consist->removeAllLocos();
  EXPECT_EQ(consist->getLocoCount(), 0);
  EXPECT_EQ(consist->getFirst(), nullptr);
  EXPECT_EQ(consist->getSpeed(), 0);
  EXPECT_EQ(consist->getDirection(), Direction::Forward);

  // Clean up
  delete consist;
}

/// @brief Create a consist with three Locos by address
TEST_F(LocoTests, createConsistByAddress) {
  // Add locos to the consist, with 2 reversed
  Consist *consist = new Consist();
  consist->addLoco(10, Facing::FacingForward);
  consist->addLoco(2, Facing::FacingReversed);
  consist->addLoco(10000, Facing::FacingForward);

  // Validate consist makeup by object and address
  EXPECT_STREQ(consist->getName(), "10"); // name should be address of first loco
  EXPECT_EQ(consist->getLocoCount(), 3);
  EXPECT_TRUE(consist->inConsist(10));
  EXPECT_TRUE(consist->inConsist(2));
  EXPECT_TRUE(consist->inConsist(10000));

  // Get loco objects for the next tests
  Loco *loco10 = consist->getByAddress(10)->getLoco();
  ASSERT_NE(loco10, nullptr);
  ASSERT_EQ(loco10->getAddress(), 10);
  Loco *loco2 = consist->getByAddress(2)->getLoco();
  ASSERT_NE(loco2, nullptr);
  ASSERT_EQ(loco2->getAddress(), 2);
  Loco *loco10000 = consist->getByAddress(10000)->getLoco();
  ASSERT_NE(loco10000, nullptr);
  ASSERT_EQ(loco10000->getAddress(), 10000);

  // Validate the first loco address is 10
  EXPECT_EQ(consist->getFirst()->getLoco()->getAddress(), 10);

  // Validate the consist speed and direction comes from the first loco
  EXPECT_EQ(consist->getSpeed(), 0);
  EXPECT_EQ(consist->getDirection(), Direction::Forward);
  loco2->setSpeed(35);
  loco10000->setDirection(Direction::Reverse);
  EXPECT_EQ(consist->getSpeed(), 0);
  EXPECT_EQ(consist->getDirection(), Direction::Forward);
  loco10->setSpeed(21);
  loco10->setDirection(Direction::Reverse);
  EXPECT_EQ(consist->getSpeed(), 21);
  EXPECT_EQ(consist->getDirection(), Direction::Reverse);

  // Validate removal of middle loco is as expected
  consist->removeLoco(loco2);
  EXPECT_EQ(consist->getLocoCount(), 2);
  EXPECT_EQ(consist->getFirst()->getLoco(), loco10);
  EXPECT_EQ(consist->getSpeed(), 21);
  EXPECT_EQ(consist->getDirection(), Direction::Reverse);

  // Validate removal of first loco is as expected
  consist->removeLoco(loco10);
  EXPECT_EQ(consist->getLocoCount(), 1);
  EXPECT_EQ(consist->getFirst()->getLoco(), loco10000);
  EXPECT_EQ(consist->getSpeed(), 0);
  EXPECT_EQ(consist->getDirection(), Direction::Reverse);

  // Validate removal of all locos
  consist->removeAllLocos();
  EXPECT_EQ(consist->getLocoCount(), 0);
  EXPECT_EQ(consist->getFirst(), nullptr);
  EXPECT_EQ(consist->getSpeed(), 0);
  EXPECT_EQ(consist->getDirection(), Direction::Forward);

  // Clean up the consist
  delete consist;
}
