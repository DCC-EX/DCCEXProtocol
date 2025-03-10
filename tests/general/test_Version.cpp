/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
 *
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

TEST_F(DCCEXProtocolTests, request) {
  _dccexProtocol.requestServerVersion();
  EXPECT_EQ(_stream.getBuffer(), "<s>\r\n");
  _stream.clearBuffer();
}

TEST_F(DCCEXProtocolTests, versionJustZeros) {
  EXPECT_FALSE(_dccexProtocol.receivedVersion());
  _stream << "<iDCCEX V-0.0.0 / MEGA / STANDARD_MOTOR_SHIELD / 7>";
  EXPECT_CALL(_delegate, receivedServerVersion(0, 0, 0)).Times(Exactly(1));
  _dccexProtocol.check();
  EXPECT_TRUE(_dccexProtocol.receivedVersion());
  EXPECT_EQ(_dccexProtocol.getMajorVersion(), 0);
  EXPECT_EQ(_dccexProtocol.getMinorVersion(), 0);
  EXPECT_EQ(_dccexProtocol.getPatchVersion(), 0);
}

TEST_F(DCCEXProtocolTests, versionSingleDigits) {
  EXPECT_FALSE(_dccexProtocol.receivedVersion());
  _stream << "<iDCCEX V-1.2.3 / MEGA / STANDARD_MOTOR_SHIELD / 7>";
  EXPECT_CALL(_delegate, receivedServerVersion(1, 2, 3)).Times(Exactly(1));
  _dccexProtocol.check();
  EXPECT_TRUE(_dccexProtocol.receivedVersion());
  EXPECT_EQ(_dccexProtocol.getMajorVersion(), 1);
  EXPECT_EQ(_dccexProtocol.getMinorVersion(), 2);
  EXPECT_EQ(_dccexProtocol.getPatchVersion(), 3);
}

TEST_F(DCCEXProtocolTests, versionMultipleDigits) {
  EXPECT_FALSE(_dccexProtocol.receivedVersion());
  _stream << "<iDCCEX V-92.210.10 / MEGA / STANDARD_MOTOR_SHIELD / 7>";
  EXPECT_CALL(_delegate, receivedServerVersion(92, 210, 10)).Times(Exactly(1));
  _dccexProtocol.check();
  EXPECT_TRUE(_dccexProtocol.receivedVersion());
  EXPECT_EQ(_dccexProtocol.getMajorVersion(), 92);
  EXPECT_EQ(_dccexProtocol.getMinorVersion(), 210);
  EXPECT_EQ(_dccexProtocol.getPatchVersion(), 10);
}

TEST_F(DCCEXProtocolTests, versionIgnoreLabels) {
  EXPECT_FALSE(_dccexProtocol.receivedVersion());
  _stream << "<iDCCEX V-1.2.3-smartass / MEGA / STANDARD_MOTOR_SHIELD / 7>";
  EXPECT_CALL(_delegate, receivedServerVersion(1, 2, 3)).Times(Exactly(1));
  _dccexProtocol.check();
  EXPECT_TRUE(_dccexProtocol.receivedVersion());
  EXPECT_EQ(_dccexProtocol.getMajorVersion(), 1);
  EXPECT_EQ(_dccexProtocol.getMinorVersion(), 2);
  EXPECT_EQ(_dccexProtocol.getPatchVersion(), 3);
}