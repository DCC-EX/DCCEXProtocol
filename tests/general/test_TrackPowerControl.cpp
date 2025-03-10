/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
 *
 * Copyright © 2025 Peter Cole
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

/// @brief Validate that sending powerOn() sends <1>
TEST_F(DCCEXProtocolTests, powerAllOn) {
  const char *expected = "<1>\r\n";

  // Call power on
  _dccexProtocol.powerOn();

  // Ensure the buffer has what's expected
  ASSERT_EQ(_stream.getBuffer(), expected);
}

/// @brief Validate that sending powerOff() sends <0>
TEST_F(DCCEXProtocolTests, powerAllOff) {
  const char *expected = "<0>\r\n";

  // Call power off
  _dccexProtocol.powerOff();

  // Ensure the buffer has what's expected
  ASSERT_EQ(_stream.getBuffer(), expected);
}

/// @brief Validate that sending powerMainOn() sends <1 MAIN>
TEST_F(DCCEXProtocolTests, powerMainOn) {
  const char *expected = "<1 MAIN>\r\n";

  // Call power on
  _dccexProtocol.powerMainOn();

  // Ensure the buffer has what's expected
  ASSERT_EQ(_stream.getBuffer(), expected);
}

/// @brief Validate that sending powerMainOff() sends <0 MAIN>
TEST_F(DCCEXProtocolTests, powerMainOff) {
  const char *expected = "<0 MAIN>\r\n";

  // Call power off
  _dccexProtocol.powerMainOff();

  // Ensure the buffer has what's expected
  ASSERT_EQ(_stream.getBuffer(), expected);
}

/// @brief Validate that sending powerProgOn() sends <1 PROG>
TEST_F(DCCEXProtocolTests, powerProgOn) {
  const char *expected = "<1 PROG>\r\n";

  // Call power on
  _dccexProtocol.powerProgOn();

  // Ensure the buffer has what's expected
  ASSERT_EQ(_stream.getBuffer(), expected);
}

/// @brief Validate that sending powerProgOff() sends <0 PROG>
TEST_F(DCCEXProtocolTests, powerProgOff) {
  const char *expected = "<0 PROG>\r\n";

  // Call power off
  _dccexProtocol.powerProgOff();

  // Ensure the buffer has what's expected
  ASSERT_EQ(_stream.getBuffer(), expected);
}

/// @brief Validate that sending joinProg() sends <1 JOIN>
TEST_F(DCCEXProtocolTests, joinProg) {
  const char *expected = "<1 JOIN>\r\n";

  // Call join
  _dccexProtocol.joinProg();

  // Ensure the buffer has what's expected
  ASSERT_EQ(_stream.getBuffer(), expected);
}
