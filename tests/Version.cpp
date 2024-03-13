#include "DCCEXProtocolTest.hpp"

TEST_F(DCCEXProtocolTest, version_just_zeros) {
  EXPECT_FALSE(_dccexProtocol.receivedVersion());
  _stream << "<iDCCEX V-0.0.0 / MEGA / STANDARD_MOTOR_SHIELD / 7>";
  EXPECT_CALL(_delegate, receivedServerVersion(0, 0, 0)).Times(Exactly(1));
  _dccexProtocol.check();
  EXPECT_TRUE(_dccexProtocol.receivedVersion());
  EXPECT_EQ(_dccexProtocol.getMajorVersion(), 0);
  EXPECT_EQ(_dccexProtocol.getMinorVersion(), 0);
  EXPECT_EQ(_dccexProtocol.getPatchVersion(), 0);
}

TEST_F(DCCEXProtocolTest, version_single_digits) {
  EXPECT_FALSE(_dccexProtocol.receivedVersion());
  _stream << "<iDCCEX V-1.2.3 / MEGA / STANDARD_MOTOR_SHIELD / 7>";
  EXPECT_CALL(_delegate, receivedServerVersion(1, 2, 3)).Times(Exactly(1));
  _dccexProtocol.check();
  EXPECT_TRUE(_dccexProtocol.receivedVersion());
  EXPECT_EQ(_dccexProtocol.getMajorVersion(), 1);
  EXPECT_EQ(_dccexProtocol.getMinorVersion(), 2);
  EXPECT_EQ(_dccexProtocol.getPatchVersion(), 3);
}

TEST_F(DCCEXProtocolTest, version_multiple_digits) {
  EXPECT_FALSE(_dccexProtocol.receivedVersion());
  _stream << "<iDCCEX V-92.210.10 / MEGA / STANDARD_MOTOR_SHIELD / 7>";
  EXPECT_CALL(_delegate, receivedServerVersion(92, 210, 10)).Times(Exactly(1));
  _dccexProtocol.check();
  EXPECT_TRUE(_dccexProtocol.receivedVersion());
  EXPECT_EQ(_dccexProtocol.getMajorVersion(), 92);
  EXPECT_EQ(_dccexProtocol.getMinorVersion(), 210);
  EXPECT_EQ(_dccexProtocol.getPatchVersion(), 10);
}

TEST_F(DCCEXProtocolTest, version_ignore_labels) {
  EXPECT_FALSE(_dccexProtocol.receivedVersion());
  _stream << "<iDCCEX V-1.2.3-smartass / MEGA / STANDARD_MOTOR_SHIELD / 7>";
  EXPECT_CALL(_delegate, receivedServerVersion(1, 2, 3)).Times(Exactly(1));
  _dccexProtocol.check();
  EXPECT_TRUE(_dccexProtocol.receivedVersion());
  EXPECT_EQ(_dccexProtocol.getMajorVersion(), 1);
  EXPECT_EQ(_dccexProtocol.getMinorVersion(), 2);
  EXPECT_EQ(_dccexProtocol.getPatchVersion(), 3);
}