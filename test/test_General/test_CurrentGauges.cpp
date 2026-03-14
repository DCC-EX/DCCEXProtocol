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
 * @brief Test requesting gauge limits for one track
 */
TEST_F(DCCEXProtocolTests, TestRequestGaugeLimitsOneTrack) {
  // Send <J G>
  _dccexProtocol.requestTrackCurrentGauges();
  EXPECT_EQ(_stream.getOutput(), "<J G>");

  // Simulate response <jG 1499> - A
  EXPECT_CALL(_delegate, receivedTrackCurrentGauge('A', 1499)).Times(1);
  _stream << "<jG 1499>";
  _dccexProtocol.check();
}

/**
 * @brief Test requesting gauge limits for default two tracks
 */
TEST_F(DCCEXProtocolTests, TestRequestGaugeLimitsTwoTracks) {
  // Send <J G>
  _dccexProtocol.requestTrackCurrentGauges();
  EXPECT_EQ(_stream.getOutput(), "<J G>");

  // Simulate response <jG 1499 1499> - A B
  EXPECT_CALL(_delegate, receivedTrackCurrentGauge('A', 1499)).Times(1);
  EXPECT_CALL(_delegate, receivedTrackCurrentGauge('B', 1499)).Times(1);
  _stream << "<jG 1499 1499>";
  _dccexProtocol.check();
}

/**
 * @brief Test requesting gauge limits for max 8 tracks
 */
TEST_F(DCCEXProtocolTests, TestRequestGaugeLimitsEightTracks) {
  // Send <J G>
  _dccexProtocol.requestTrackCurrentGauges();
  EXPECT_EQ(_stream.getOutput(), "<J G>");

  // Simulate response <jG 1499 1499 1499 1499 1499 1499 1499 1499> - A B C D E F G H
  EXPECT_CALL(_delegate, receivedTrackCurrentGauge('A', 1499)).Times(1);
  EXPECT_CALL(_delegate, receivedTrackCurrentGauge('B', 1499)).Times(1);
  EXPECT_CALL(_delegate, receivedTrackCurrentGauge('C', 1499)).Times(1);
  EXPECT_CALL(_delegate, receivedTrackCurrentGauge('D', 1499)).Times(1);
  EXPECT_CALL(_delegate, receivedTrackCurrentGauge('E', 1499)).Times(1);
  EXPECT_CALL(_delegate, receivedTrackCurrentGauge('F', 1499)).Times(1);
  EXPECT_CALL(_delegate, receivedTrackCurrentGauge('G', 1499)).Times(1);
  EXPECT_CALL(_delegate, receivedTrackCurrentGauge('H', 1499)).Times(1);
  _stream << "<jG 1499 1499 1499 1499 1499 1499 1499 1499>";
  _dccexProtocol.check();
}

/**
 * @brief Test requesting current for one track
 */
TEST_F(DCCEXProtocolTests, TestRequestCurrentOneTrack) {
  // Send <J I>
  _dccexProtocol.requestTrackCurrents();
  EXPECT_EQ(_stream.getOutput(), "<J I>");

  // Simulate response <jI 500> - A
  EXPECT_CALL(_delegate, receivedTrackCurrent('A', 500)).Times(1);
  _stream << "<jI 500>";
  _dccexProtocol.check();
}

/**
 * @brief Test requesting current for default two tracks
 */
TEST_F(DCCEXProtocolTests, TestRequestCurrentTwoTracks) {
  // Send <J I>
  _dccexProtocol.requestTrackCurrents();
  EXPECT_EQ(_stream.getOutput(), "<J I>");

  // Simulate response <jI 500 0> - A B
  EXPECT_CALL(_delegate, receivedTrackCurrent('A', 500)).Times(1);
  EXPECT_CALL(_delegate, receivedTrackCurrent('B', 0)).Times(1);
  _stream << "<jI 500 0>";
  _dccexProtocol.check();
}

/**
 * @brief Test requesting current for max 8 tracks
 */
TEST_F(DCCEXProtocolTests, TestRequestCurrentEightTracks) {
  // Send <J I>
  _dccexProtocol.requestTrackCurrents();
  EXPECT_EQ(_stream.getOutput(), "<J I>");

  // Simulate response <jI 1200 895 124 50 0 0 0 1300> - A B C D E F G H
  EXPECT_CALL(_delegate, receivedTrackCurrent('A', 1200)).Times(1);
  EXPECT_CALL(_delegate, receivedTrackCurrent('B', 895)).Times(1);
  EXPECT_CALL(_delegate, receivedTrackCurrent('C', 124)).Times(1);
  EXPECT_CALL(_delegate, receivedTrackCurrent('D', 50)).Times(1);
  EXPECT_CALL(_delegate, receivedTrackCurrent('E', 0)).Times(1);
  EXPECT_CALL(_delegate, receivedTrackCurrent('F', 0)).Times(1);
  EXPECT_CALL(_delegate, receivedTrackCurrent('G', 0)).Times(1);
  EXPECT_CALL(_delegate, receivedTrackCurrent('H', 1300)).Times(1);
  _stream << "<jI 1200 895 124 50 0 0 0 1300>";
  _dccexProtocol.check();
}
