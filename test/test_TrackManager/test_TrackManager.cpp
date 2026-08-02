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
  const char *expected = "<= A MAIN>";

  // Call power on
  _dccexProtocol.setTrackType('A', TrackManagerMode::MAIN, 0);

  // Ensure the buffer has what's expected
  ASSERT_EQ(_stream.getOutput(), expected);
}

/**
 * @brief Test setting track B to MAIN
 */
TEST_F(TrackManagerTests, setTrackTypeProg) {
  const char *expected = "<= B PROG>";

  // Call power off
  _dccexProtocol.setTrackType('B', TrackManagerMode::PROG, 0);

  // Ensure the buffer has what's expected
  ASSERT_EQ(_stream.getOutput(), expected);
}

/**
 * @brief Test setting track C to DC address 1234
 */
TEST_F(TrackManagerTests, setTrackTypeDC) {
  const char *expected = "<= C DC 1234>";

  // Call power off
  _dccexProtocol.setTrackType('C', TrackManagerMode::DC, 1234);

  // Ensure the buffer has what's expected
  ASSERT_EQ(_stream.getOutput(), expected);
}

/**
 * @brief Test setting track D to DCX address 2345
 */
TEST_F(TrackManagerTests, setTrackTypeDCX) {
  const char *expected = "<= D DCX 2345>";

  // Call power off
  _dccexProtocol.setTrackType('D', TrackManagerMode::DCX, 2345);

  // Ensure the buffer has what's expected
  ASSERT_EQ(_stream.getOutput(), expected);
}

/**
 * @brief Test setting track E to NONE
 */
TEST_F(TrackManagerTests, setTrackTypeNone) {
  const char *expected = "<= E NONE>";

  // Call power off
  _dccexProtocol.setTrackType('E', TrackManagerMode::NONE, 0);

  // Ensure the buffer has what's expected
  ASSERT_EQ(_stream.getOutput(), expected);
}

/**
 * @brief Test receiving a MAIN track type broadcast
 */
TEST_F(TrackManagerTests, TestTrackTypeBroadcastMain) {
  // Simulate receiving a track type broadcast
  _stream << "<= A MAIN>";
  EXPECT_CALL(_delegate, receivedTrackType('A', TrackManagerMode::MAIN, 0)).Times(Exactly(1));
  _dccexProtocol.check();
}

/**
 * @brief Test receiving a PROG track type broadcast
 */
TEST_F(TrackManagerTests, TestTrackTypeBroadcastProg) {
  // Simulate receiving a track type broadcast
  _stream << "<= B PROG>";
  EXPECT_CALL(_delegate, receivedTrackType('B', TrackManagerMode::PROG, 0)).Times(Exactly(1));
  _dccexProtocol.check();
}

/**
 * @brief Test receiving a DC track type broadcast with an address
 */
TEST_F(TrackManagerTests, TestTrackTypeBroadcastDC) {
  // Simulate receiving a track type broadcast
  _stream << "<= C DC 1234>";
  EXPECT_CALL(_delegate, receivedTrackType('C', TrackManagerMode::DC, 1234)).Times(Exactly(1));
  _dccexProtocol.check();
}

/**
 * @brief Test receiving a DCX track type broadcast with an address
 */
TEST_F(TrackManagerTests, TestTrackTypeBroadcastDCX) {
  // Simulate receiving a track type broadcast
  _stream << "<= D DCX 2345>";
  EXPECT_CALL(_delegate, receivedTrackType('D', TrackManagerMode::DCX, 2345)).Times(Exactly(1));
  _dccexProtocol.check();
}

/**
 * @brief Test receiving a NONE track type broadcast
 */
TEST_F(TrackManagerTests, TestTrackTypeBroadcastNone) {
  // Simulate receiving a track type broadcast
  _stream << "<= E NONE>";
  EXPECT_CALL(_delegate, receivedTrackType('E', TrackManagerMode::NONE, 0)).Times(Exactly(1));
  _dccexProtocol.check();
}

/**
 * @brief Test receiving an unknown track type broadcast does not invoke the delegate
 */
TEST_F(TrackManagerTests, TestTrackTypeBroadcastUnknownType) {
  // Simulate receiving a broadcast with an unknown track type
  _stream << "<= A 42>";
  EXPECT_CALL(_delegate, receivedTrackType(_, _, _)).Times(Exactly(0));
  _dccexProtocol.check();
}

/**
 * @brief Test receiving a track type broadcast with too few parameters does not invoke the delegate
 */
TEST_F(TrackManagerTests, TestTrackTypeTooFewParams) {
  // Simulate receiving a broadcast with only the track parameter
  _stream << "<= A>";
  EXPECT_CALL(_delegate, receivedTrackType(_, _, _)).Times(Exactly(0));
  _dccexProtocol.check();
}