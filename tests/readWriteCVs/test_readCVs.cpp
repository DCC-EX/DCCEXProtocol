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

/// @brief Validate calling readLoco() generates the correct command
TEST_F(CVTests, readLocoAddress) {
  // Reading should generate <R>
  const char *expected = "<R>\r\n";

  // Call readLoco()
  _dccexProtocol.readLoco();

  // Buffer should contain what we expect
  ASSERT_EQ(_stream.getBuffer(), expected);
}

/// @brief Validate calling readCV(cv) generates the correct command
TEST_F(CVTests, readCV) {
  // Reading CV 19 should generate <R 19>
  const char *expected = "<R 19>\r\n";

  // Call readLoco()
  _dccexProtocol.readCV(19);

  // Buffer should contain what we expect
  ASSERT_EQ(_stream.getBuffer(), expected);
}

/// @brief Validate calling validateCV(cv, value) generates the correct command
TEST_F(CVTests, validateCV) {
  // Validating CV 1 with value 3 should generate <V 1 3>
  const char *expected = "<V 1 3>\r\n";

  // Call validateCV()
  _dccexProtocol.validateCV(1, 3);

  // Buffer should contain what we expect
  ASSERT_EQ(_stream.getBuffer(), expected);
}

/// @brief Validate calling validateCVBit(cv, bit, value) generates the correct command
TEST_F(CVTests, validateCVBit) {
  // Validating CV 1 bit 3 with value 1 should generate <V 1 3 1>
  const char *expected = "<V 1 3 1>\r\n";

  // Call validateCVBit()
  _dccexProtocol.validateCVBit(1, 3, 1);

  // Buffer should contain what we expect
  ASSERT_EQ(_stream.getBuffer(), expected);
}

/// @brief Validate receiving <r value> calls receivedReadLoco()
TEST_F(CVTests, readAddressCVResponse) {
  _stream << "<r 1234>";
  EXPECT_CALL(_delegate, receivedReadLoco(1234)).Times(Exactly(1));
  _dccexProtocol.check();
}

/// @brief Validate receiving <v cv value> calls receivedValidateCV()
TEST_F(CVTests, validateCVResponse) {
  _stream << "<v 1 3>";
  EXPECT_CALL(_delegate, receivedValidateCV(1, 3)).Times(Exactly(1));
  _dccexProtocol.check();
}

/// @brief Validate receiving <v cv bit value> calls receivedValidateCVBit()
TEST_F(CVTests, validateCVBitResponse) {
  _stream << "<v 1 3 1>";
  EXPECT_CALL(_delegate, receivedValidateCVBit(1, 3, 1)).Times(Exactly(1));
  _dccexProtocol.check();
}
