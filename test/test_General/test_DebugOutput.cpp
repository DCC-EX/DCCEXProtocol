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

#include "../setup/DCCEXProtocolTests.h"

/**
 * @brief Test calling a command with default debug outputs nothing to console
 */
TEST_F(DCCEXProtocolTests, TestDefaultDebugOutput) {
  // Default should have debug off, so this should not send to console
  _dccexProtocol.setDefaultMomentum(10, 20);
  EXPECT_EQ(_stream.getOutput(), "<m 0 10 20>");
  EXPECT_EQ(_console.getOutput(), "");
}

/**
 * @brief Test calling a command with debug output off outputs nothing to console
 */
TEST_F(DCCEXProtocolTests, TestDebugOutputOff) {
  // Turn debug off, should not send to console
  _dccexProtocol.setDebug(false);
  _dccexProtocol.setDefaultMomentum(10, 20);
  EXPECT_EQ(_stream.getOutput(), "<m 0 10 20>");
  EXPECT_EQ(_console.getOutput(), "");
}

/**
 * @brief Test calling a command with debug output on outputs to console
 */
TEST_F(DCCEXProtocolTests, TestDebugOutputOn) {
  // Turn debug on, should send to console
  _dccexProtocol.setDebug(true);
  _dccexProtocol.setDefaultMomentum(10, 20);
  EXPECT_EQ(_stream.getOutput(), "<m 0 10 20>");
  EXPECT_EQ(_console.getOutput(), "==> <m 0 10 20>\r\n");
}

/**
 * @brief Test receiving a broadcast with debug output off outputs nothing to console
 */
TEST_F(DCCEXProtocolTests, TestBroadcastDebugOutputOff) {
  // Turn debug off, should send to console
  _dccexProtocol.setDebug(false);
  _stream << "<l 42 0 128 0>";
  _dccexProtocol.check();
  EXPECT_EQ(_console.getOutput(), "");
}

/**
 * @brief Test receiving a broadcast with debug output on outputs to console
 */
TEST_F(DCCEXProtocolTests, TestBroadcastDebugOutputOn) {
  // Turn debug on, should send to console
  _dccexProtocol.setDebug(true);
  _stream << "<l 42 0 128 0>";
  _dccexProtocol.check();
  EXPECT_EQ(_console.getOutput(), "<== <l 42 0 128 0>\r\n");
}
