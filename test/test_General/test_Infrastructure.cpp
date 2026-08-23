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
 * @brief Ensure the buffer is cleared when full
 */
TEST_F(DCCEXProtocolTests, clearBufferWhenFull) {
  // Fill the input buffer with garbage (operator<< feeds the input side)
  _stream << std::string(500, 'A');

  // Proceed with normal message
  _stream << R"(<m "Hello World">)";
  EXPECT_CALL(_delegate, receivedMessage(StrEq("Hello World"))).Times(Exactly(1));
  _dccexProtocol.check();
}

/**
 * @brief Ensure a command longer than the command buffer is dropped without crashing
 */
TEST_F(DCCEXProtocolTests, overlongCommandDropped) {
  // Construct a protocol with a small command buffer to isolate the buffer-full path
  DCCEXProtocol smallBuffer(20, 100);
  smallBuffer.setDelegate(&_delegate);
  smallBuffer.connect(&_stream);

  // Feed a command that exceeds the 20 byte buffer (22 bytes including '>')
  _stream << "<jR 1 2 3 4 5 6 7 8 9>";
  EXPECT_CALL(_delegate, receivedRosterList()).Times(0);
  smallBuffer.check();
  EXPECT_FALSE(smallBuffer.receivedRoster());
}

/**
 * @brief Ensure a command with more parameters than maxCommandParams is dropped without crashing
 */
TEST_F(DCCEXProtocolTests, commandExceedingMaxParamsDropped) {
  // Construct a protocol with a small maximum parameter count to isolate the max-params path
  DCCEXProtocol smallParams(500, 10);
  smallParams.setDelegate(&_delegate);
  smallParams.connect(&_stream);

  // Feed a command with 12 parameters (more than the max of 10)
  _stream << "<jR 1 2 3 4 5 6 7 8 9 10 11>";
  EXPECT_CALL(_delegate, receivedRosterList()).Times(0);
  smallParams.check();
  EXPECT_FALSE(smallParams.receivedRoster());
}

/**
 * @brief Test sending a generic command with sendCommand
 */
TEST_F(DCCEXProtocolTests, testGenericSendCommand) {
  _dccexProtocol.sendCommand("Random command");
  EXPECT_EQ(_stream.getOutput(), "<Random command>");
  _stream.clearOutput();
}

/**
 * @brief Test sending a null command with sendCommand is a no-op and does not crash
 */
TEST_F(DCCEXProtocolTests, TestSendNullCommand) {
  // A null command must not crash and should not send anything
  EXPECT_NO_FATAL_FAILURE(_dccexProtocol.sendCommand(nullptr));
  EXPECT_EQ(_stream.getOutput(), "<>");
}

/**
 * @brief Test the library version can be retrieved via the static method
 */
TEST_F(DCCEXProtocolTests, TestLibraryVersion) { ASSERT_STREQ(DCCEXProtocol::getLibraryVersion(), "1.5.0"); }

/**
 * @brief Test calling disconnect() is a no-op and does not crash
 */
TEST_F(DCCEXProtocolTests, disconnectIsNoOp) {
  // Disconnecting must not crash and should not send anything
  EXPECT_NO_FATAL_FAILURE(_dccexProtocol.disconnect());
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test getLastServerResponseTime() records the last server response
 */
TEST_F(DCCEXProtocolTests, getLastServerResponseTime) {
  // Before any response, the time must be the initial value
  EXPECT_EQ(_dccexProtocol.getLastServerResponseTime(), 0UL);

  // Advance the clock and process a server response
  advanceMillis(1000);
  EXPECT_CALL(_delegate, receivedTrackPower(TrackPower::PowerOn));
  _stream << "<p1>";
  _dccexProtocol.check();

  // The last response time must match the time the response was processed
  EXPECT_EQ(_dccexProtocol.getLastServerResponseTime(), 1000UL);
}

/**
 * @brief Test requestLocoUpdate() sends the correct command
 */
TEST_F(DCCEXProtocolTests, requestLocoUpdateSendsCommand) {
  _dccexProtocol.requestLocoUpdate(42);
  EXPECT_EQ(_stream.getOutput(), "<t 42>");
}

/**
 * @brief Test emergencyStop() sends the correct command
 */
TEST_F(DCCEXProtocolTests, emergencyStopSendsCommand) {
  _dccexProtocol.emergencyStop();
  EXPECT_EQ(_stream.getOutput(), "<!>");
}

/**
 * @brief Test getNumberSupportedLocos() sends the correct command
 */
TEST_F(DCCEXProtocolTests, getNumberSupportedLocosSendsCommand) {
  _dccexProtocol.getNumberSupportedLocos();
  EXPECT_EQ(_stream.getOutput(), "<#>");
}

/**
 * @brief Test activateAccessory() sends the correct command
 */
TEST_F(DCCEXProtocolTests, activateAccessorySendsCommand) {
  _dccexProtocol.activateAccessory(1, 2);
  EXPECT_EQ(_stream.getOutput(), "<a 1 2 1>");
}

/**
 * @brief Test deactivateAccessory() sends the correct command
 */
TEST_F(DCCEXProtocolTests, deactivateAccessorySendsCommand) {
  _dccexProtocol.deactivateAccessory(1, 2);
  EXPECT_EQ(_stream.getOutput(), "<a 1 2 0>");
}

/**
 * @brief Test activateLinearAccessory() sends the correct command
 */
TEST_F(DCCEXProtocolTests, activateLinearAccessorySendsCommand) {
  _dccexProtocol.activateLinearAccessory(1);
  EXPECT_EQ(_stream.getOutput(), "<a 1 1>");
}

/**
 * @brief Test deactivateLinearAccessory() sends the correct command
 */
TEST_F(DCCEXProtocolTests, deactivateLinearAccessorySendsCommand) {
  _dccexProtocol.deactivateLinearAccessory(1);
  EXPECT_EQ(_stream.getOutput(), "<a 1 0>");
}

/**
 * @brief Test startRoute() sends the correct command
 */
TEST_F(DCCEXProtocolTests, startRouteSendsCommand) {
  _dccexProtocol.startRoute(5);
  EXPECT_EQ(_stream.getOutput(), "</ START 5>");
}

/**
 * @brief Test pauseRoutes() sends the correct command
 */
TEST_F(DCCEXProtocolTests, pauseRoutesSendsCommand) {
  _dccexProtocol.pauseRoutes();
  EXPECT_EQ(_stream.getOutput(), "</ PAUSE>");
}

/**
 * @brief Test resumeRoutes() sends the correct command
 */
TEST_F(DCCEXProtocolTests, resumeRoutesSendsCommand) {
  _dccexProtocol.resumeRoutes();
  EXPECT_EQ(_stream.getOutput(), "</ RESUME>");
}

/**
 * @brief Test setTrackType() with an invalid type is a no-op and does not crash
 */
TEST_F(DCCEXProtocolTests, setTrackTypeInvalidTypeIgnored) {
  // An invalid track manager mode must not send a command
  EXPECT_NO_FATAL_FAILURE(_dccexProtocol.setTrackType('A', (TrackManagerMode)99, 0));
  EXPECT_EQ(_stream.getOutput(), "");
}
