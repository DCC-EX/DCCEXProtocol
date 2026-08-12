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

#include "../setup/JMRISensorTests.h"

/**
 * @brief Test requesting the JMRI sensor list method
 */
TEST_F(JMRISensorTests, requestJMRISensorList) {
  // Request should send "<Q>" only
  _dccexProtocol.requestJMRISensorList();
  EXPECT_EQ(_stream.getOutput(), "<Q>");
  _stream.clearOutput();
}

/**
 * @brief Test receiving a JMRI sensor activated broadcast calls the delegate method
 */
TEST_F(JMRISensorTests, receiveJMRISensorActivated) {
  // Receiving "<Q 100>" should call the delegate with sensor ID 100 activated
  EXPECT_CALL(_delegate, receivedJMRISensorBroadcast(100, JMRISensorState::Activated)).Times(Exactly(1));
  _stream << "<Q 100>";
  _dccexProtocol.check();
}

/**
 * @brief Test receiving a JMRI sensor deactivated broadcast calls the delegate method
 */
TEST_F(JMRISensorTests, receiveJMRISensorDeactivated) {
  // Receiving "<q 200>" should call the delegate with sensor ID 200 deactivated
  EXPECT_CALL(_delegate, receivedJMRISensorBroadcast(200, JMRISensorState::Deactivated)).Times(Exactly(1));
  _stream << "<q 200>";
  _dccexProtocol.check();
}

/**
 * @brief Test receiving a broadcast with no params is ignored
 */
TEST_F(JMRISensorTests, receiveJMRISensorBroadcastNoParams) {
  // "<Q>" is the outbound request form, must not trigger a broadcast callback
  _stream << "<Q>";
  EXPECT_CALL(_delegate, receivedJMRISensorBroadcast(_, _)).Times(Exactly(0));
  _dccexProtocol.check();
}

/**
 * @brief Test receiving too many parameters is ignored
 */
TEST_F(JMRISensorTests, receiveJMRISensorBroadcastTooManyParams) {
  _stream << "<Q 100 200>";
  EXPECT_CALL(_delegate, receivedJMRISensorBroadcast(_, _)).Times(Exactly(0));
  _dccexProtocol.check();
}

/**
 * @brief Test receiving an invalid parameter is ignored
 */
TEST_F(JMRISensorTests, receiveJMRISensorBroadcastTextParam) {
  _stream << "<Q \"abc\">";
  EXPECT_CALL(_delegate, receivedJMRISensorBroadcast(_, _)).Times(Exactly(0));
  _dccexProtocol.check();
}

/**
 * @brief Test receiving invalid sensor ID is ignored
 */
TEST_F(JMRISensorTests, receiveJMRISensorBroadcastIdZero) {
  EXPECT_CALL(_delegate, receivedJMRISensorBroadcast(_, _)).Times(Exactly(0));
  _stream << "<Q 0>";
  _dccexProtocol.check();
}
