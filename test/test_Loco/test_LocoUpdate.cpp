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

#include "../setup/LocoTests.h"

/// @brief Create a small roster and check updates are received
TEST_F(LocoTests, receiveRosterLocoUpdate) {
  // Setup a small roster, make sure it's created correctly
  ASSERT_EQ(_dccexProtocol.roster->getFirst(), nullptr);

  // Add two locos
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  loco42->setName("Loco42");
  Loco *loco120 = new Loco(120, LocoSource::LocoSourceRoster);
  loco120->setName("Loco120");

  // Now verify the roster, fatal error if first is nullptr
  Loco *firstLoco = _dccexProtocol.roster->getFirst();
  ASSERT_NE(firstLoco, nullptr);

  // Set a loco update for 42 in the stream:
  // - Speed byte = forward, speed 21
  // - Function 0 on
  _stream << "<l 42 0 150 1>";

  // Expect to receive the delegate call at the next check()
  EXPECT_CALL(_delegate, receivedLocoUpdate(loco42)).Times(Exactly(1));
  // We should also expect an update with the details
  EXPECT_CALL(_delegate, receivedLocoBroadcast(42, 21, Direction::Forward, 1)).Times(Exactly(1));
  _dccexProtocol.check();

  // Validate expected result
  EXPECT_EQ(loco42->getSpeed(), 21);
  EXPECT_EQ(loco42->getDirection(), Direction::Forward);
  EXPECT_EQ(loco42->getFunctionStates(), 1);

  // Set a loco update for 120 in the stream:
  // - Speed byte = reverse, speed 11
  // - Functions 0 and 1 on
  _stream << "<l 120 0 12 2>";

  // Expect to receive the delegate call at the next check()
  EXPECT_CALL(_delegate, receivedLocoUpdate(loco120)).Times(Exactly(1));
  // We should also expect an update with the details
  EXPECT_CALL(_delegate, receivedLocoBroadcast(120, 11, Direction::Reverse, 2)).Times(Exactly(1));
  _dccexProtocol.check();

  // Validate expected result
  EXPECT_EQ(loco120->getSpeed(), 11);
  EXPECT_EQ(loco120->getDirection(), Direction::Reverse);
  EXPECT_EQ(loco120->getFunctionStates(), 2);
}

/// @brief Check updates are received for non-roster locos
TEST_F(LocoTests, receiveNonRosterLocoUpdate) {
  // Set a loco update for an unknown loco in the stream:
  // - Address 355
  // - Speed byte = forward, speed 31
  // - Functions off
  _stream << "<l 355 0 160 0>";

  // Expect to receive the delegate call at the next check()
  EXPECT_CALL(_delegate, receivedLocoBroadcast(355, 31, Direction::Forward, 0)).Times(Exactly(1));
  // We should not receive a Loco object delegate call
  EXPECT_CALL(_delegate, receivedLocoUpdate(_)).Times(0);
  _dccexProtocol.check();

  // Set a loco update for an unknown loco in the stream:
  // - Address 42
  // - Speed byte = reverse, speed 11
  // - Functions 0 and 1 on
  _stream << "<l 42 0 12 2>";

  // Expect to receive the delegate call at the next check()
  EXPECT_CALL(_delegate, receivedLocoBroadcast(42, 11, Direction::Reverse, 2)).Times(Exactly(1));
  // We should not receive a Loco object delegate call
  EXPECT_CALL(_delegate, receivedLocoUpdate(::testing::_)).Times(0);
  _dccexProtocol.check();
}

/**
 * @brief Test receiving an update for Loco address 0 does not update
 */
TEST_F(LocoTests, TestReceiveUpdateLoco0DoesNotUpdate) {
  // Set a loco update for loco address 0 in the stream
  // address 0, speed byte forward 31, functions off
  _stream << "<l 0 0 160 0>";

  // Expect no calls should be made
  EXPECT_CALL(_delegate, receivedLocoUpdate(_)).Times(0);
  EXPECT_CALL(_delegate, receivedLocoBroadcast(_, _, _, _)).Times(0);

  // Check
  _dccexProtocol.check();
}

/**
 * @brief Test various speedbyte values calculate correctly
 */
TEST_F(LocoTests, TestSpeedByteCalculation) {
  // Forward normal stop 128 = 0 1
  EXPECT_CALL(_delegate, receivedLocoBroadcast(42, 0, Direction::Forward, 0)).Times(1);
  _stream << "<l 42 0 128 0>";
  _dccexProtocol.check();
  // Forward full speed 255 = 126 1
  EXPECT_CALL(_delegate, receivedLocoBroadcast(42, 126, Direction::Forward, 0)).Times(1);
  _stream << "<l 42 0 255 0>";
  _dccexProtocol.check();
  // Forward mid speed 191 = 62 1
  EXPECT_CALL(_delegate, receivedLocoBroadcast(42, 62, Direction::Forward, 0)).Times(1);
  _stream << "<l 42 0 191 0>";
  _dccexProtocol.check();
  // Forward e stop 129 = 0 1
  EXPECT_CALL(_delegate, receivedLocoBroadcast(42, 0, Direction::Forward, 0)).Times(1);
  _stream << "<l 42 0 129 0>";
  _dccexProtocol.check();
  // Reverse normal stop 0 = 0 0
  EXPECT_CALL(_delegate, receivedLocoBroadcast(42, 0, Direction::Reverse, 0)).Times(1);
  _stream << "<l 42 0 0 0>";
  _dccexProtocol.check();
  // Reverse full speed 127 = 126 0
  EXPECT_CALL(_delegate, receivedLocoBroadcast(42, 126, Direction::Reverse, 0)).Times(1);
  _stream << "<l 42 0 127 0>";
  _dccexProtocol.check();
  // Reverse mid speed 63 = 62 0
  EXPECT_CALL(_delegate, receivedLocoBroadcast(42, 62, Direction::Reverse, 0)).Times(1);
  _stream << "<l 42 0 63 0>";
  _dccexProtocol.check();
  // Reverse e stop 1 = 0 0
  EXPECT_CALL(_delegate, receivedLocoBroadcast(42, 0, Direction::Reverse, 0)).Times(1);
  _stream << "<l 42 0 1 0>";
  _dccexProtocol.check();
}

/**
 * @brief Test F28 is updated correctly
 */
TEST_F(LocoTests, TestReceiveF28) {
  // Setup the Loco
  ASSERT_EQ(_dccexProtocol.roster->getFirst(), nullptr);
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  loco42->setName("Loco42");

  // Set a loco update for 42 in the stream:
  // - Speed byte = forward, speed 21
  // - Function 28 on
  _stream << "<l 42 0 150 268435456>";

  // Expect to receive the delegate call at the next check()
  EXPECT_CALL(_delegate, receivedLocoUpdate(loco42)).Times(Exactly(1));
  // We should also expect an update with the details
  EXPECT_CALL(_delegate, receivedLocoBroadcast(42, 21, Direction::Forward, 268435456)).Times(Exactly(1));
  _dccexProtocol.check();

  // Validate expected result
  EXPECT_EQ(loco42->getSpeed(), 21);
  EXPECT_EQ(loco42->getDirection(), Direction::Forward);
  EXPECT_EQ(loco42->getFunctionStates(), 268435456);
  EXPECT_TRUE(loco42->isFunctionOn(28));
}

/**
 * @brief Test all functions on is updated correctly
 */
TEST_F(LocoTests, TestReceiveAllFunctionsOn) {
  // Setup the Loco
  ASSERT_EQ(_dccexProtocol.roster->getFirst(), nullptr);
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  loco42->setName("Loco42");

  // Set a loco update for 42 in the stream:
  // - Speed byte = forward, speed 21
  // - Function 28 on
  _stream << "<l 42 0 150 536870911>";

  // Expect to receive the delegate call at the next check()
  EXPECT_CALL(_delegate, receivedLocoUpdate(loco42)).Times(Exactly(1));
  // We should also expect an update with the details
  EXPECT_CALL(_delegate, receivedLocoBroadcast(42, 21, Direction::Forward, 536870911)).Times(Exactly(1));
  _dccexProtocol.check();

  // Validate expected result
  EXPECT_EQ(loco42->getSpeed(), 21);
  EXPECT_EQ(loco42->getDirection(), Direction::Forward);
  EXPECT_EQ(loco42->getFunctionStates(), 536870911);
  for (int function = 0; function < 29; function++) {
    EXPECT_TRUE(loco42->isFunctionOn(function));
  }
}
