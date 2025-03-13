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

#include "../setup/CVTests.h"

/// @brief Validate writeLocoAddress(address) generates the correct command
TEST_F(CVTests, writeLocoAddress) {
  // Expect write address to generate <W 1234>
  const char *expected = "<W 1234>\r\n";

  // Call writeLocoAddress()
  _dccexProtocol.writeLocoAddress(1234);

  // Check the buffer
  ASSERT_EQ(_stream.getBuffer(), expected);
}

/// @brief Validate writeCV(cv, value) generates the correct command
TEST_F(CVTests, writeCV) {
  // Expect write address to generate <W 1 3>
  const char *expected = "<W 1 3>\r\n";

  // Call writeCV()
  _dccexProtocol.writeCV(1, 3);

  // Check the buffer
  ASSERT_EQ(_stream.getBuffer(), expected);
}

/// @brief Validate writeCVBit(cv, bit, value) generates the correct command
TEST_F(CVTests, writeCVBit) {
  // Expect write address to generate <B 19 4 1>
  const char *expected = "<B 19 4 1>\r\n";

  // Call writeCV()
  _dccexProtocol.writeCVBit(19, 4, 1);

  // Check the buffer
  ASSERT_EQ(_stream.getBuffer(), expected);
}

/// @brief Validate writeCVOnMain(address, cv, value) generates the correct command
TEST_F(CVTests, writeCVOnMain) {
  // Expect write CV on main to generate <W 3 8 4>
  const char *expected = "<w 3 8 4>\r\n";

  // Call writeCVOnMain()
  _dccexProtocol.writeCVOnMain(3, 8, 4);

  // Check the buffer
  ASSERT_EQ(_stream.getBuffer(), expected);
}

/// @brief Validate writeCVBitOnMain(address, cv, bit, value) generates the correct command
TEST_F(CVTests, writeCVBitOnMain) {
  // Expect write CV bit on main to generate <W 3 19 4 1>
  const char *expected = "<b 3 19 4 1>\r\n";

  // Call writeCVBitOnMain()
  _dccexProtocol.writeCVBitOnMain(3, 19, 4, 1);

  // Check the buffer
  ASSERT_EQ(_stream.getBuffer(), expected);
}

/// @brief Validate a resonse to writing a loco address calls the correct delegate method
TEST_F(CVTests, writeLocoAddressResponse) {
  _stream << "<w 1234>";
  EXPECT_CALL(_delegate, receivedWriteLoco(1234)).Times(Exactly(1));
  _dccexProtocol.check();
}

/// @brief Validate a resonse to writing a CV calls the correct delegate method
TEST_F(CVTests, writeCVResponse) {
  _stream << "<r 1 3>";
  EXPECT_CALL(_delegate, receivedWriteCV(1, 3)).Times(Exactly(1));
  _dccexProtocol.check();
}
