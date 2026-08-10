/* -*- c++ -*-
 *
 * Copyright © 2026 Peter Cole
 * Copyright © 2024 Vincent Hamp
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

#include "../setup/DCCEXProtocolTests.h"

/**
 * @brief Ensure parse returns false for an unterminated command
 */
TEST_F(DCCEXProtocolTests, parseUnterminatedCommand) {
  char command[] = "<1 12";
  EXPECT_FALSE(DCCEXInbound::parse(command));
  EXPECT_EQ(DCCEXInbound::getOpcode(), '1');
  // The trailing parameter is never flushed without a terminating character
  EXPECT_EQ(DCCEXInbound::getParameterCount(), 0);
}

/**
 * @brief Ensure parse returns false for an empty command
 */
TEST_F(DCCEXProtocolTests, parseEmptyCommand) {
  char command[] = "";
  EXPECT_FALSE(DCCEXInbound::parse(command));
  EXPECT_EQ(DCCEXInbound::getOpcode(), 0);
  EXPECT_EQ(DCCEXInbound::getParameterCount(), 0);
}

/**
 * @brief Ensure parse succeeds for a complete command
 */
TEST_F(DCCEXProtocolTests, parseCompleteCommand) {
  char command[] = "<1 12>";
  EXPECT_TRUE(DCCEXInbound::parse(command));
  EXPECT_EQ(DCCEXInbound::getOpcode(), '1');
  EXPECT_EQ(DCCEXInbound::getNumber(0), 12);
}

/**
 * @brief Ensure getter out-of-bounds and text/number guards return defaults
 */
TEST_F(DCCEXProtocolTests, accessorBoundsHandling) {
  char command[] = "<12 \"MAIN\">";
  EXPECT_TRUE(DCCEXInbound::parse(command));

  // Out of range or negative parameter numbers return 0/false/nullptr
  EXPECT_EQ(DCCEXInbound::getNumber(-1), 0);
  EXPECT_EQ(DCCEXInbound::getNumber(5), 0);
  EXPECT_FALSE(DCCEXInbound::isTextParameter(5));

  // getNumber on a text parameter returns 0
  EXPECT_EQ(DCCEXInbound::getNumber(1), 0);

  // getTextParameter on a numeric parameter returns nullptr
  EXPECT_EQ(DCCEXInbound::getTextParameter(0), nullptr);

  // getTextParameter / copyTextParameter with an out of range parameter return nullptr
  EXPECT_EQ(DCCEXInbound::getTextParameter(5), nullptr);
  EXPECT_EQ(DCCEXInbound::copyTextParameter(5), nullptr);
}

/**
 * @brief Ensure lowercase keyword parameters are uppercased before hashing
 */
TEST_F(DCCEXProtocolTests, lowercaseKeywordHandling) {
  // Lowercase keywords (and track letters) are uppercased before hashing
  _stream << "<= a main>";
  EXPECT_CALL(_delegate, receivedTrackType('A', TrackManagerMode::MAIN, 0)).Times(Exactly(1));
  _dccexProtocol.check();
}
