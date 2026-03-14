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
 * @brief Test setting the fast clock
 */
TEST_F(DCCEXProtocolTests, TestSetFastClock) {
  // Send <J C 60 4> - 1am 4, times speed
  _dccexProtocol.setFastClock(60, 4);
  EXPECT_EQ(_stream.getOutput(), "<J C 60 4>");

  // Simulate response <jC 60 4>
  EXPECT_CALL(_delegate, receivedSetFastClock(60, 4));
  // Also screen update <@ 0 6 "Time 01:00 (4)">
  EXPECT_CALL(_delegate, receivedScreenUpdate(0, 6, StrEq("Time 01:00 (4)")));

  _stream << "<jC 60 4>";
  _stream << "<@ 0 6 \"Time 01:00 (4)\">";
  _dccexProtocol.check();
}

/**
 * @brief Test requesting the fast clock time
 */
TEST_F(DCCEXProtocolTests, TestRequestFastClockTime) {
  // Send <J C>
  _dccexProtocol.requestFastClockTime();
  EXPECT_EQ(_stream.getOutput(), "<J C>");

  // Simulate response <jC 60>
  _stream << "<jC 60>";
  EXPECT_CALL(_delegate, receivedFastClockTime(60));
  _dccexProtocol.check();
}

/**
 * @brief Test setting fast clock time/speed with invalid values
 */
TEST_F(DCCEXProtocolTests, TestInvalidFastClockTimeAndSpeed) {
  _dccexProtocol.setFastClock(0, 0);
  EXPECT_EQ(_stream.getOutput(), "");
  _dccexProtocol.setFastClock(0, -1);
  EXPECT_EQ(_stream.getOutput(), "");
  _dccexProtocol.setFastClock(-1, 4);
  EXPECT_EQ(_stream.getOutput(), "");
  _dccexProtocol.setFastClock(2000, 4);
  EXPECT_EQ(_stream.getOutput(), "");
}
