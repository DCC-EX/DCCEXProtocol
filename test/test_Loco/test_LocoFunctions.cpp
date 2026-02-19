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

#include "../setup/LocoTests.h"

/**
 * @brief Test turning a function on sends the right command
 */
TEST_F(LocoTests, TestFunctionOn) {
  // Create a loco and validate function on
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceEntry);
  _dccexProtocol.functionOn(loco42, 0);
  EXPECT_EQ(_stream.getOutput(), "<F 42 0 1>");
}

/**
 * @brief Test turning a function off sends the right command
 */
TEST_F(LocoTests, TestFunctionOff) {
  // Create a loco and validate function off
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceEntry);
  _dccexProtocol.functionOff(loco42, 0);
  EXPECT_EQ(_stream.getOutput(), "<F 42 0 0>");
}

/**
 * @brief Test isFunctionOn returns the correct flag
 */
TEST_F(LocoTests, TestIsFunctionOn) {
  // Create a loco
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceEntry);

  // Simulate receiving function update
  _stream << "<l 42 0 128 1>";
  _dccexProtocol.check();
  EXPECT_TRUE(_dccexProtocol.isFunctionOn(loco42, 0));
}
