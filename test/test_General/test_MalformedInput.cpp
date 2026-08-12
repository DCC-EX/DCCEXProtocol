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
 * @brief Ensure a power broadcast with too many parameters is dropped
 */
TEST_F(DCCEXProtocolTests, powerBroadcastExcessParamsDropped) {
  _stream << "<p 1 2 3>";
  EXPECT_CALL(_delegate, receivedTrackPower(_)).Times(0);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Ensure a power broadcast with a text first parameter is dropped
 */
TEST_F(DCCEXProtocolTests, powerBroadcastTextParamDropped) {
  _stream << "<p \"x\">";
  EXPECT_CALL(_delegate, receivedTrackPower(_)).Times(0);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Ensure a loco broadcast with the wrong parameter count is dropped
 */
TEST_F(DCCEXProtocolTests, locoBroadcastBadParamCountDropped) {
  _stream << "<l 1 2 3>";
  EXPECT_CALL(_delegate, receivedLocoBroadcast(_, _, _, _)).Times(0);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Ensure a loco broadcast with a text first parameter is dropped
 */
TEST_F(DCCEXProtocolTests, locoBroadcastTextParamDropped) {
  _stream << "<l \"x\" 1 2 3>";
  EXPECT_CALL(_delegate, receivedLocoBroadcast(_, _, _, _)).Times(0);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Ensure a throttle list response with a text first parameter is dropped
 */
TEST_F(DCCEXProtocolTests, throttleListTextParamDropped) {
  _stream << "<j \"x\">";
  EXPECT_CALL(_delegate, receivedRosterList()).Times(0);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Ensure a turnout broadcast with a text first parameter is dropped
 */
TEST_F(DCCEXProtocolTests, turnoutBroadcastTextParamDropped) {
  _stream << "<H \"x\">";
  EXPECT_CALL(_delegate, receivedTurnoutAction(_, _)).Times(0);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Ensure a turnout broadcast with the wrong parameter count is dropped
 */
TEST_F(DCCEXProtocolTests, turnoutBroadcastBadParamCountDropped) {
  _stream << "<H 1>";
  EXPECT_CALL(_delegate, receivedTurnoutAction(_, _)).Times(0);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Ensure a read loco response with a text first parameter is dropped
 */
TEST_F(DCCEXProtocolTests, readResponseTextParamDropped) {
  _stream << "<r \"x\">";
  EXPECT_CALL(_delegate, receivedReadLoco(_)).Times(0);
  EXPECT_CALL(_delegate, receivedWriteCV(_, _)).Times(0);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Ensure a write loco response with a text first parameter is dropped
 */
TEST_F(DCCEXProtocolTests, writeResponseTextParamDropped) {
  _stream << "<w \"x\">";
  EXPECT_CALL(_delegate, receivedWriteLoco(_)).Times(0);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Ensure a validate CV response with a text first parameter is dropped
 */
TEST_F(DCCEXProtocolTests, validateCVResponseTextParamDropped) {
  _stream << "<v \"x\">";
  EXPECT_CALL(_delegate, receivedValidateCV(_, _)).Times(0);
  EXPECT_CALL(_delegate, receivedValidateCVBit(_, _, _)).Times(0);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Ensure an unknown opcode is ignored without crashing
 */
TEST_F(DCCEXProtocolTests, unknownOpcodeIgnored) {
  EXPECT_NO_FATAL_FAILURE({
    _stream << "<X 1 2>";
    _dccexProtocol.check();
  });
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Ensure a CSConsist response with a text first parameter is dropped
 */
TEST_F(DCCEXProtocolTests, csConsistTextParamDropped) {
  _stream << "<^ \"x\">";
  EXPECT_CALL(_delegate, receivedCSConsist(_, _)).Times(0);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Ensure an individual track power broadcast for a non-MAIN track returns early
 */
TEST_F(DCCEXProtocolTests, individualTrackPowerNonMainDropped) {
  _stream << "<p1 PROG>";
  EXPECT_CALL(_delegate, receivedTrackPower(_)).Times(0);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Ensure a loco broadcast function map is masked to 29 bits
 */
TEST_F(DCCEXProtocolTests, locoBroadcastFunctionMapMasked) {
  _stream << "<l 5 0 1 536870912>";
  EXPECT_CALL(_delegate, receivedLocoBroadcast(5, 0, Reverse, 0)).Times(Exactly(1));
  _dccexProtocol.check();
}

/**
 * @brief Test screen update not called with a numeric instead of text parameter
 */
TEST_F(DCCEXProtocolTests, screenUpdateNumericMessageDropped) {
  _stream << "<@ 1 1 2>";
  EXPECT_CALL(_delegate, receivedScreenUpdate(_, _, _)).Times(0);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test screen update not called with too many parameters
 */
TEST_F(DCCEXProtocolTests, screenUpdateTooManyParamsDropped) {
  _stream << "<@ 1 1 \"x\" \"y\">";
  EXPECT_CALL(_delegate, receivedScreenUpdate(_, _, _)).Times(0);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test turntable broadcast not called with an invalid parameter count
 */
TEST_F(DCCEXProtocolTests, turntableBroadcastBadParamCountDropped) {
  _stream << "<I 1 1>";
  EXPECT_CALL(_delegate, receivedTurntableAction(_, _, _)).Times(0);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test message broadcast not called with a numeric parameter
 */
TEST_F(DCCEXProtocolTests, messageBroadcastNumericParamDropped) {
  _stream << "<m 123>";
  EXPECT_CALL(_delegate, receivedMessage(_)).Times(0);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test unknown throttle subcommand ignored
 */
TEST_F(DCCEXProtocolTests, unknownThrottleSubcommandIgnored) {
  EXPECT_NO_FATAL_FAILURE({
    _stream << "<jX 1>";
    _dccexProtocol.check();
  });
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test invalid fast clock param count is ignored
 */
TEST_F(DCCEXProtocolTests, fastClockBadParamCountIgnored) {
  _stream << "<jC>";
  EXPECT_CALL(_delegate, receivedFastClockTime(_)).Times(0);
  EXPECT_CALL(_delegate, receivedSetFastClock(_, _)).Times(0);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "");
}
