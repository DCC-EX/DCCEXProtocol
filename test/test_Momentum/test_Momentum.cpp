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

#include "../setup/MomentumTests.h"

/**
 * @brief Test setting linear momentum
 */
TEST_F(MomentumTests, TestSetLinearAlgorithm) {
  // Set linear momentum and validate
  _dccexProtocol.setMomentumAlgorithm(Linear);
  EXPECT_EQ(_stream.getOutput(), "<m LINEAR>");
}

/**
 * @brief Test setting speed based (POWER) momentum
 */
TEST_F(MomentumTests, TestSetPowerAlgorithm) {
  // Set speed momentum and validate
  _dccexProtocol.setMomentumAlgorithm(Power);
  EXPECT_EQ(_stream.getOutput(), "<m POWER>");
}

/**
 * @brief Test setting an invalid algorithm does nothing
 */
TEST_F(MomentumTests, TestSetInvalidAlgorithm) {
  // Set invalid algorithm and validate no output
  _dccexProtocol.setMomentumAlgorithm((MomentumAlgorithm)20);
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test setting the default momentum
 */
TEST_F(MomentumTests, TestSetDefaultMomentum) {
  _dccexProtocol.setDefaultMomentum(10);
  EXPECT_EQ(_stream.getOutput(), "<m 0 10>");
}

/**
 * @brief Test setting the default accelerating/braking
 */
TEST_F(MomentumTests, TestSetDefaultSeparateMomentums) {
  _dccexProtocol.setDefaultMomentum(10, 20);
  EXPECT_EQ(_stream.getOutput(), "<m 0 10 20>");
}

/**
 * @brief Test setting momentum with an address
 */
TEST_F(MomentumTests, TestSetLocoAddressMomentum) {
  // Set single momentum value for an address and validate
  _dccexProtocol.setMomentum(3, 10);
  EXPECT_EQ(_stream.getOutput(), "<m 3 10>");
}

/**
 * @brief Test setting separate momentums with an address
 */
TEST_F(MomentumTests, TestSetLocoAddressSeparateMomentums) {
  // Set momentum values for an address and validate
  _dccexProtocol.setMomentum(3, 10, 20);
  EXPECT_EQ(_stream.getOutput(), "<m 3 10 20>");
}

/**
 * @brief Test setting momentum with a Loco
 */
TEST_F(MomentumTests, TestSetLocoMomentum) {
  // Set single momentum value for a Loco and validate
  Loco *loco3 = new Loco(3, LocoSource::LocoSourceEntry);
  _dccexProtocol.setMomentum(loco3, 10);
  EXPECT_EQ(_stream.getOutput(), "<m 3 10>");
}

/**
 * @brief Test setting separate momentums with a Loco
 */
TEST_F(MomentumTests, TestSetLocoSeparateMomentums) {
  // Set momentum values for a Loco and validate
  Loco *loco3 = new Loco(3, LocoSource::LocoSourceEntry);
  _dccexProtocol.setMomentum(loco3, 10, 20);
  EXPECT_EQ(_stream.getOutput(), "<m 3 10 20>");
}

/**
 * @brief Test setting momentum for invalid addresses and Loco
 */
TEST_F(MomentumTests, TestSetInvalidAddressLocoMomentum) {
  _dccexProtocol.setMomentum(0, 10);
  _dccexProtocol.setMomentum(10240, 10);
  _dccexProtocol.setMomentum(nullptr, 10);
  _dccexProtocol.setMomentum(0, 10, 20);
  _dccexProtocol.setMomentum(10240, 10, 20);
  _dccexProtocol.setMomentum(nullptr, 10, 20);
}
