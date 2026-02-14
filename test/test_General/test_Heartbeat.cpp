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
 * @brief Test heartbeat # is sent at the required intervals only
 */
TEST_F(DCCEXProtocolTests, TestHeartbeat) {
  // Enable heartbeat with default time of 60 seconds
  _dccexProtocol.enableHeartbeat();
  _dccexProtocol.check();
  // Should not have heartbeat in first call
  EXPECT_NE(_stream.getOutput(), "<#>");
  _stream.clearOutput();

  // Advance timer 30s and still should not have heartbeat
  advanceMillis(30000);
  _dccexProtocol.check();
  EXPECT_NE(_stream.getOutput(), "<#>");
  _stream.clearOutput();

  // Now go past 60s and should have heartbeat
  advanceMillis(30001);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<#>");
  _stream.clearOutput();
}

/**
 * @brief Test heartbeat # is sent at custom intervals only
 */
TEST_F(DCCEXProtocolTests, TestCustomHeartbeat) {
  // Enable heartbeat with custom time of 30 seconds
  _dccexProtocol.enableHeartbeat(30000);
  _dccexProtocol.check();
  // Should not have heartbeat in first call
  EXPECT_NE(_stream.getOutput(), "<#>");
  _stream.clearOutput();

  // Advance timer 20s and still should not have heartbeat
  advanceMillis(20000);
  _dccexProtocol.check();
  EXPECT_NE(_stream.getOutput(), "<#>");
  _stream.clearOutput();

  // Now go past 30s and should have heartbeat
  advanceMillis(30001);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<#>");
  _stream.clearOutput();

  // Past 60s should also generate another heartbeat
  advanceMillis(30001);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<#>");
  _stream.clearOutput();
}
