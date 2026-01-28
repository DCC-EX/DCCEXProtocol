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

#include "../setup/TrackManagerTests.h"

/**
 * @brief Test setting track A to MAIN
 */
TEST_F(TrackManagerTests, setTrackTypeMain) {
  const char *expected = "<= A MAIN>\r\n";

  // Call power on
  _dccexProtocol.setTrackType('A', TrackManagerMode::MAIN, 0);

  // Ensure the buffer has what's expected
  ASSERT_EQ(_stream.getBuffer(), expected);
}

/**
 * @brief Test setting track B to MAIN
 */
TEST_F(TrackManagerTests, setTrackTypeProg) {
  const char *expected = "<= B PROG>\r\n";

  // Call power off
  _dccexProtocol.setTrackType('B', TrackManagerMode::PROG, 0);

  // Ensure the buffer has what's expected
  ASSERT_EQ(_stream.getBuffer(), expected);
}

/**
 * @brief Test setting track C to DC address 1234
 */
TEST_F(TrackManagerTests, setTrackTypeDC) {
  const char *expected = "<= C DC 1234>\r\n";

  // Call power off
  _dccexProtocol.setTrackType('C', TrackManagerMode::DC, 1234);

  // Ensure the buffer has what's expected
  ASSERT_EQ(_stream.getBuffer(), expected);
}

/**
 * @brief Test setting track D to DCX address 2345
 */
TEST_F(TrackManagerTests, setTrackTypeDCX) {
  const char *expected = "<= D DCX 2345>\r\n";

  // Call power off
  _dccexProtocol.setTrackType('D', TrackManagerMode::DCX, 2345);

  // Ensure the buffer has what's expected
  ASSERT_EQ(_stream.getBuffer(), expected);
}

/**
 * @brief Test setting track E to NONE
 */
TEST_F(TrackManagerTests, setTrackTypeNone) {
  const char *expected = "<= E NONE>\r\n";

  // Call power off
  _dccexProtocol.setTrackType('E', TrackManagerMode::NONE, 0);

  // Ensure the buffer has what's expected
  ASSERT_EQ(_stream.getBuffer(), expected);
}