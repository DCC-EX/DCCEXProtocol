/* -*- c++ -*-
 *
 * Copyright © 2026 Peter Cole
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
  const char *functionList = "Lights/*Horn";
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

  // Clean up
  delete consist;
}

/**
 * @brief Test calling functionOn() with a populated Consist sends a command per member
 */
TEST_F(LocoTests, TestFunctionOnPopulatedConsist) {
  // Create a consist with two locos
  Consist *consist = new Consist();
  Loco *loco10 = new Loco(10, LocoSourceRoster);
  Loco *loco20 = new Loco(20, LocoSourceRoster);
  consist->addLoco(loco10, Facing::FacingForward);
  consist->addLoco(loco20, Facing::FacingForward);

  // Turning on a function must send a command for each member
  _dccexProtocol.functionOn(consist, 3);
  EXPECT_EQ(_stream.getOutput(), "<F 10 3 1><F 20 3 1>");

  // Clean up
  delete consist;
}

/**
 * @brief Test calling functionOff() with a populated Consist sends a command per member
 */
TEST_F(LocoTests, TestFunctionOffPopulatedConsist) {
  // Create a consist with two locos
  Consist *consist = new Consist();
  Loco *loco10 = new Loco(10, LocoSourceRoster);
  Loco *loco20 = new Loco(20, LocoSourceRoster);
  consist->addLoco(loco10, Facing::FacingForward);
  consist->addLoco(loco20, Facing::FacingForward);

  // Turning off a function must send a command for each member
  _dccexProtocol.functionOff(consist, 3);
  EXPECT_EQ(_stream.getOutput(), "<F 10 3 0><F 20 3 0>");

  // Clean up
  delete consist;
}

/**
 * @brief Test isFunctionOn() with a populated Consist reflects the first member's state
 */
TEST_F(LocoTests, TestIsFunctionOnPopulatedConsist) {
  // Create a consist with two locos
  Consist *consist = new Consist();
  Loco *loco10 = new Loco(10, LocoSourceRoster);
  Loco *loco20 = new Loco(20, LocoSourceRoster);
  consist->addLoco(loco10, Facing::FacingForward);
  consist->addLoco(loco20, Facing::FacingForward);

  // The state of the first member determines the consist state
  EXPECT_FALSE(_dccexProtocol.isFunctionOn(consist, 3));
  loco10->setFunctionStates(1 << 3);
  EXPECT_TRUE(_dccexProtocol.isFunctionOn(consist, 3));
  EXPECT_FALSE(_dccexProtocol.isFunctionOn(consist, 4));

  // Clean up
  delete consist;
}

/**
 * @brief Test calling setThrottle() with a null Consist is a no-op and does not crash
 */
TEST_F(LocoTests, TestSetThrottleNullConsist) {
  // A null consist must not crash and should not queue any change
  EXPECT_NO_FATAL_FAILURE(_dccexProtocol.setThrottle(static_cast<Consist *>(nullptr), 10, Forward));
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test calling functionOn() with a null Consist is a no-op and does not crash
 */
TEST_F(LocoTests, TestFunctionOnNullConsist) {
  // A null consist must not crash and should not send a command
  EXPECT_NO_FATAL_FAILURE(_dccexProtocol.functionOn(static_cast<Consist *>(nullptr), 0));
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test calling functionOff() with a null Consist is a no-op and does not crash
 */
TEST_F(LocoTests, TestFunctionOffNullConsist) {
  // A null consist must not crash and should not send a command
  EXPECT_NO_FATAL_FAILURE(_dccexProtocol.functionOff(static_cast<Consist *>(nullptr), 0));
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test calling isFunctionOn() with a null Consist returns false and does not crash
 */
TEST_F(LocoTests, TestIsFunctionOnNullConsist) {
  // A null consist must not crash and should return false
  EXPECT_NO_FATAL_FAILURE(_dccexProtocol.isFunctionOn(static_cast<Consist *>(nullptr), 0));
  EXPECT_FALSE(_dccexProtocol.isFunctionOn(static_cast<Consist *>(nullptr), 0));
}

/**
 * @brief Test calling isFunctionOn() with an empty Consist returns false and does not crash
 */
TEST_F(LocoTests, TestIsFunctionOnEmptyConsist) {
  // Create an empty consist
  Consist *emptyConsist = new Consist();

  // An empty consist must not crash and should return false
  EXPECT_NO_FATAL_FAILURE(_dccexProtocol.isFunctionOn(emptyConsist, 0));
  EXPECT_FALSE(_dccexProtocol.isFunctionOn(emptyConsist, 0));

  // Clean up
  delete emptyConsist;
}

/**
 * @brief Test adding a null Loco to an empty Consist is a no-op and does not crash
 */
TEST_F(LocoTests, TestAddNullLocoToEmptyConsist) {
  // Create an empty consist
  Consist *consist = new Consist();

  // Adding a null loco must not crash and must not add anything
  EXPECT_NO_FATAL_FAILURE(consist->addLoco(static_cast<Loco *>(nullptr), FacingForward));
  EXPECT_EQ(consist->getLocoCount(), 0);

  // Clean up
  delete consist;
}

/**
 * @brief Test adding a null Loco to a populated Consist is a no-op and does not crash
 */
TEST_F(LocoTests, TestAddNullLocoToPopulatedConsist) {
  // Create a consist with a valid loco
  Consist *consist = new Consist();
  Loco *loco10 = new Loco(10, LocoSourceRoster);
  consist->addLoco(loco10, FacingForward);
  EXPECT_EQ(consist->getLocoCount(), 1);

  // Adding a null loco must not crash and must not poison the consist
  EXPECT_NO_FATAL_FAILURE(consist->addLoco(static_cast<Loco *>(nullptr), FacingForward));
  EXPECT_EQ(consist->getLocoCount(), 1);
  EXPECT_TRUE(consist->inConsist(10));

  // Clean up
  delete consist;
}

/**
 * @brief Test creating a consist using local locos
 */
TEST_F(LocoTests, CreateConsistWithLocalLocos) {
  // Create three local locos for the consist
  Loco *loco10 = new Loco(10, LocoSourceEntry);
  Loco *loco2 = new Loco(2, LocoSourceEntry);
  Loco *loco10000 = new Loco(10000, LocoSourceEntry);

  // Add locos to the consist, with 2 reversed
  Consist *consist = new Consist();
  consist->addLoco(loco10, Facing::FacingForward);
  consist->addLoco(loco2, Facing::FacingReversed);
  consist->addLoco(loco10000, Facing::FacingForward);

  // Validate consist makeup by object and address
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
