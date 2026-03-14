/* -*- c++ -*-
 *
 * Copyright © 2026 Peter Cole
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

#include "../setup/TestHarnessNoDelegate.h"

/**
 * @brief Test sending a throttle command with no delegate actually sends
 */
TEST_F(TestHarnessNoDelegate, TestSendThrottleCommand) {
  // Create a loco and send a throttle command
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  _dccexProtocol.setThrottle(loco42, 10, Forward);

  // Advance past queue timer and check, should produce the command
  advanceMillis(101);
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<t 42 10 1>");
}

/**
 * @brief Test receiving the server version sets version, will segfault if delegate guard fails
 */
TEST_F(TestHarnessNoDelegate, TestReceiveServerVersion) {
  // Simulate receiving the server version
  EXPECT_FALSE(_dccexProtocol.receivedVersion());
  _stream << "<iDCCEX V-1.2.3-smartass / MEGA / STANDARD_MOTOR_SHIELD / 7>";
  _dccexProtocol.check();
  EXPECT_TRUE(_dccexProtocol.receivedVersion());
  EXPECT_EQ(_dccexProtocol.getMajorVersion(), 1);
  EXPECT_EQ(_dccexProtocol.getMinorVersion(), 2);
  EXPECT_EQ(_dccexProtocol.getPatchVersion(), 3);
}

/**
 * @brief Test request server version not blocked by no delegate
 */
TEST_F(TestHarnessNoDelegate, TestRequestServerVersion) {
  // Request server version should be in the buffer
  _dccexProtocol.requestServerVersion();
  EXPECT_EQ(_stream.getOutput(), "<s>");
}

/**
 * @brief Test checking a function on is not blocked by no delegate
 */
TEST_F(TestHarnessNoDelegate, TestIsFunctionOn) {
  // Create a loco, turn F0 on, test should be true
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  loco42->setFunctionStates(1);
  EXPECT_TRUE(_dccexProtocol.isFunctionOn(loco42, 0));
}

/**
 * @brief Test processing a track power update will seg fault this test if not guarded against no delegate
 */
TEST_F(TestHarnessNoDelegate, TestTrackPowerUpdate) {
  // Simulate receiving a track power update
  _stream << "<p1>";
  _dccexProtocol.check();
}

/**
 * @brief Test receiving a message doesn't seg fault when no delegate
 */
TEST_F(TestHarnessNoDelegate, TestReceiveMessage) {
  // Simulate receiving a message
  _stream << "<m \"Test message \">";
  _dccexProtocol.check();
}

/**
 * @brief Test getLists() doesn't cause a seg fault with no delegate set
 */
TEST_F(TestHarnessNoDelegate, TestGetLists) {
  // Request all lists
  // We expect ONLY the roster to be requested first.
  _dccexProtocol.getLists(true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "<J R>");
  _stream.clearOutput();

  // Simulate receiving the roster list and stream should now request first roster entry details
  _stream << "<jR 1 2>";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J R 1>");
  _stream.clearOutput();

  // Simulate receiving first roster details which should trigger retrieving second entry
  _stream << "<jR 1 \"Loco1\" \"Func1\">";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J R 2>");
  _stream.clearOutput();

  // Simulate second details
  _stream << "<jR 2 \"Loco2\" \"Func2\">";
  _dccexProtocol.check();

  // Next call to getLists() should start turnouts
  _dccexProtocol.getLists(true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "<J T>");
  _stream.clearOutput();

  // receivedLists() should still be false
  EXPECT_FALSE(_dccexProtocol.receivedLists());

  // Simulate receiving the turnout list and stream should now request first turnout details
  _stream << "<jT 1 2>";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J T 1>");
  _stream.clearOutput();

  // Simulate receiving first turnout details which should trigger retrieving second entry
  _stream << "<jT 1 0 \"Turnout1\">";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J T 2>");
  _stream.clearOutput();

  // Simulate second details
  _stream << "<jT 2 1 \"Turnout2\">";
  _dccexProtocol.check();

  // Next call to getLists() should start routes
  _dccexProtocol.getLists(true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "<J A>");
  _stream.clearOutput();

  // receivedLists() should still be false
  EXPECT_FALSE(_dccexProtocol.receivedLists());

  // Simulate receiving the route list and stream should now request first route details
  _stream << "<jA 1 2>";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J A 1>");
  _stream.clearOutput();

  // Simulate receiving first route details which should trigger retrieving second entry
  _stream << "<jA 1 R \"Route1\">";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J A 2>");
  _stream.clearOutput();

  // Simulate second details
  _stream << "<jA 2 A \"Route2\">";
  _dccexProtocol.check();

  // Next call to getLists() should start turntables
  _dccexProtocol.getLists(true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "<J O>");
  _stream.clearOutput();

  // receivedLists() should still be false
  EXPECT_FALSE(_dccexProtocol.receivedLists());

  // Simulate receiving the turntable list and stream should now request first turntable details
  _stream << "<jO 1 2>";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J O 1>");
  _stream.clearOutput();

  // Simulate receiving first turntable details
  _stream << "<jO 1 0 1 3 \"Turntable1\">";
  _dccexProtocol.check();
  // This requests both the this turntable's indexes and the next turntable
  EXPECT_EQ(_stream.getOutput(), "<J P 1><J O 2>");
  _stream.clearOutput();

  // The CS will return all indexes
  _stream << "<jP 1 0 180 \"Turntable1 Home\">";
  _dccexProtocol.check();
  _stream << "<jP 1 1 10 \"Turntable1 Index1\">";
  _dccexProtocol.check();
  _stream << "<jP 1 2 20 \"Turntable1 Index2\">";
  _dccexProtocol.check();

  // Returning the second turntable should trigger requesting its indexes
  _stream << "<jO 2 1 2 3 \"Turntable2\">";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J P 2>");

  // CS returns all indexes
  _stream << "<jP 2 0 180 \"Turntable2 Home\">";
  _dccexProtocol.check();
  _stream << "<jP 2 1 10 \"Turntable2 Index1\">";
  _dccexProtocol.check();
  _stream << "<jP 2 2 20 \"Turntable2 Index2\">";
  _dccexProtocol.check();

  // Final getLists() should set received true
  _dccexProtocol.getLists(true, true, true, true);

  // receivedLists() should return true when all lists complete
  EXPECT_TRUE(_dccexProtocol.receivedLists());
}

/**
 * @brief Test turntable broadcast does not seg fault
 */
TEST_F(TestHarnessNoDelegate, TestTurntableBroadcast) {
  // Set up a dummy turntable
  Turntable *tt = new Turntable(1);
  tt->setType(TurntableType::TurntableTypeDCC);
  tt->addIndex(new TurntableIndex(1, 0, 0, "Home"));
  tt->addIndex(new TurntableIndex(1, 1, 0, "Index1"));

  // Simulate receiving a broadcast to move to index 1 and check should not seg fault
  _stream << "<I 1 1 0>";
  _dccexProtocol.check();
}

/**
 * @brief Test receiving track current gauges does not seg fault
 */
TEST_F(TestHarnessNoDelegate, TestReceivingCurrentGauge) {
  // Simulate receiving track current gauge does not trigger seg fault
  _stream << "<jG 1499 1499>";
  _dccexProtocol.check();
}

/**
 * @brief Test receiving track currents does not seg fault
 */
TEST_F(TestHarnessNoDelegate, TestReceivingCurrent) {
  // Simulate receiving track currents does not trigger seg fault
  _stream << "<jI 600 200>";
  _dccexProtocol.check();
}

/**
 * @brief Test receiving set fast clock does not seg fault
 */
TEST_F(TestHarnessNoDelegate, TestReceivingSetFastClock) {
  _stream << "<jC 60 4>";
  _dccexProtocol.check();
}

/**
 * @brief Test receiving fast clock time does not seg fault
 */
TEST_F(TestHarnessNoDelegate, TestReceivingFastClockTime) {
  _stream << "<jC 60>";
  _dccexProtocol.check();
}
