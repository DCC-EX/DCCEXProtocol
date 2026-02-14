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

#include "../setup/LocoTests.h"

/**
 * @brief Test calling setThrottle() for a Loco with a speed change flags pending
 */
TEST_F(LocoTests, TestLocoSetThrottleAddsSpeedToQueue) {
  // Create new loco and ensure initial states as expected
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Call setThrottle() to update speed
  _dccexProtocol.setThrottle(loco42, 10, Forward);

  // Should have a pending change now with correct user speed/direction
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // There also should be nothing in the outbound buffer
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test calling setThrottle() for a Loco with a direction change flags pending
 */
TEST_F(LocoTests, TestLocoSetThrottleAddsDirectionToQueue) {
  // Create new loco and ensure initial states as expected
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Call setThrottle() to update speed
  _dccexProtocol.setThrottle(loco42, 0, Reverse);

  // Should have a pending change now with correct user speed/direction
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // There also should be nothing in the outbound buffer
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test calling setThrottle() for a Consist with a speed change flags pending
 */
TEST_F(LocoTests, TestConsistSetThrottleAddsSpeedToQueue) {
  // Create new locos, consist, and ensure initial states as expected
  Loco *loco24 = new Loco(24, LocoSource::LocoSourceRoster);
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  Consist *consist = new Consist();
  consist->addLoco(loco24, Facing::FacingForward);
  consist->addLoco(loco42, Facing::FacingReversed);
  ASSERT_FALSE(loco24->getUserChangePending());
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco24->getUserSpeed(), 0);
  EXPECT_EQ(loco24->getUserDirection(), Forward);
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Call setThrottle() to update speed
  _dccexProtocol.setThrottle(consist, 10, Forward);

  // Should have a pending change now with correct user speed/direction
  ASSERT_TRUE(loco24->getUserChangePending());
  EXPECT_EQ(loco24->getUserSpeed(), 10);
  EXPECT_EQ(loco24->getUserDirection(), Forward);
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // There also should be nothing in the outbound buffer
  EXPECT_EQ(_stream.getOutput(), "");

  // Clean up
  delete consist;
}

/**
 * @brief Test calling setThrottle() for a Consist with a direction change flags pending
 */
TEST_F(LocoTests, TestConsistSetThrottleAddsDirectionToQueue) {
  // Create new locos, consist, and ensure initial states as expected
  Loco *loco24 = new Loco(24, LocoSource::LocoSourceRoster);
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  Consist *consist = new Consist();
  consist->addLoco(loco24, Facing::FacingForward);
  consist->addLoco(loco42, Facing::FacingReversed);
  ASSERT_FALSE(loco24->getUserChangePending());
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco24->getUserSpeed(), 0);
  EXPECT_EQ(loco24->getUserDirection(), Forward);
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Call setThrottle() to update speed
  _dccexProtocol.setThrottle(consist, 0, Reverse);

  // Should have a pending change now with correct user speed/direction
  ASSERT_TRUE(loco24->getUserChangePending());
  EXPECT_EQ(loco24->getUserSpeed(), 0);
  EXPECT_EQ(loco24->getUserDirection(), Reverse);
  // Loco42 direction doesn't need to change
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // There also should be nothing in the outbound buffer
  EXPECT_EQ(_stream.getOutput(), "");

  // Clean up
  delete consist;
}

/**
 * @brief Test calling check() before queue timer expiry does not send <t ...>
 */
TEST_F(LocoTests, TestQueuedSetThrottleUnexpiredTimer) {
  // Create a new loco and validate starting state
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Adjust speed/direction and validate change is pending
  loco42->setUserSpeed(10);
  loco42->setUserDirection(Reverse);
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // Call check()
  _dccexProtocol.check();

  // Validate the state is unchanged and nothing in the buffer
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test calling check() after queue timer expiry sends <t ...> and resets pending
 */
TEST_F(LocoTests, TestQueuedSetThrottleExpiredTimer) {
  // Create a new loco and validate starting state
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Adjust speed/direction and validate change is pending
  loco42->setUserSpeed(10);
  loco42->setUserDirection(Reverse);
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // Advance time beyond default 100ms
  advanceMillis(101);

  // Call check()
  _dccexProtocol.check();

  // Validate the state is changed and buffer contains <t 42 10 1>
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);
  EXPECT_THAT(_stream.getOutput(), StartsWith("<t 42 10 0>"));
}

/**
 * @brief Test calling check() with multiple speed changes only sends latest after timer expiry
 */
TEST_F(LocoTests, TestMultipleSpeedChanges) {
  // Create a new loco and validate starting state
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Adjust speed/direction and validate change is pending
  loco42->setUserSpeed(10);
  loco42->setUserDirection(Reverse);
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // Advance less that timer expiry and validate no command sent
  advanceMillis(50);
  _dccexProtocol.check();
  ASSERT_TRUE(loco42->getUserChangePending());
  ASSERT_EQ(_stream.getOutput(), "");

  // Advance time beyond default 100ms and validate command is sent and pending reset
  advanceMillis(101);
  _dccexProtocol.check();
  ASSERT_FALSE(loco42->getUserChangePending());
  ASSERT_THAT(_stream.getOutput(), StartsWith("<t 42 10 0>"));
  _stream.clearOutput();

  // Make several changes at intervals
  advanceMillis(20);
  loco42->setUserSpeed(20);
  _dccexProtocol.check();
  ASSERT_TRUE(loco42->getUserChangePending());
  ASSERT_EQ(_stream.getOutput(), "");

  advanceMillis(20);
  loco42->setUserSpeed(30);
  _dccexProtocol.check();
  ASSERT_TRUE(loco42->getUserChangePending());
  ASSERT_EQ(_stream.getOutput(), "");

  advanceMillis(20);
  loco42->setUserSpeed(40);
  _dccexProtocol.check();
  ASSERT_TRUE(loco42->getUserChangePending());
  ASSERT_EQ(_stream.getOutput(), "");

  advanceMillis(20);
  loco42->setUserSpeed(50);
  _dccexProtocol.check();
  ASSERT_TRUE(loco42->getUserChangePending());
  ASSERT_EQ(_stream.getOutput(), "");

  // Now advance beyond timer expiry and validate last speed only sent
  advanceMillis(40);
  _dccexProtocol.check();
  ASSERT_FALSE(loco42->getUserChangePending());
  ASSERT_THAT(_stream.getOutput(), StartsWith("<t 42 50 0>"));
  _stream.clearOutput();
}

/**
 * @brief Test receiving an emergency stop in forward direction resets pending flag
 */
TEST_F(LocoTests, TestEmergencyStopForwardResetsPendingFlag) {
  // Create a new loco and validate starting state
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Adjust speed/direction and validate change is pending
  loco42->setUserSpeed(10);
  loco42->setUserDirection(Reverse);
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // Simulate receiving a forward EStop and call check()
  _stream << "<l 42 0 129 0>";
  _dccexProtocol.check();

  // Validate pending is reset and speed is back at 0, direction should not be changed
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // This should not trigger a <t ...>
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test receiving an emergency stop in reverse direction resets pending flag
 */
TEST_F(LocoTests, TestEmergencyStopReverseResetsPendingFlag) {
  // Create a new loco and validate starting state
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Adjust speed/direction and validate change is pending
  loco42->setUserSpeed(10);
  loco42->setUserDirection(Forward);
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Simulate receiving a forward EStop and call check()
  _stream << "<l 42 0 1 0>";
  _dccexProtocol.check();

  // Validate pending is reset and speed is back at 0, direction should not be changed
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // This should not trigger a <t ...>
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test a loco broadcast to the same state as the pending state prevents <t ...> command
 */
TEST_F(LocoTests, TestLocoBroadcastMatchingPendingPreventsCommand) {
  // Create a new loco and validate starting state
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Adjust speed/direction and validate change is pending
  loco42->setUserSpeed(10);
  loco42->setUserDirection(Reverse);
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // Simulate receiving an update to the same state and call check()
  _stream << "<l 42 0 11 0>";
  _dccexProtocol.check();

  // No command should be sent
  ASSERT_EQ(_stream.getOutput(), "");

  // Validate pending is reset and speed/direction unchanged
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // Now go beyond timer expiry and ensure no command sent
  advanceMillis(101);
  _dccexProtocol.check();

  // This should not trigger a <t ...>
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test a loco broadcast to different state as the pending state still allows <t ...> command
 */
TEST_F(LocoTests, TestDifferentLocoBroadcastAllowsCommand) {
  // Create a new loco and validate starting state
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Adjust speed/direction and validate change is pending
  loco42->setUserSpeed(10);
  loco42->setUserDirection(Reverse);
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // Simulate receiving an update to a different state and call check()
  _stream << "<l 42 0 130 0>";
  _dccexProtocol.check();

  // No command should be sent
  ASSERT_EQ(_stream.getOutput(), "");

  // Validate still pending and speed/direction unchanged
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // Now go beyond timer expiry and ensure command is sent
  advanceMillis(101);
  _dccexProtocol.check();

  // This should trigger a <t ...>
  EXPECT_THAT(_stream.getOutput(), StartsWith("<t 42 10 0"));
}

/**
 * @brief Test calling setThrottle() for a local Loco with a speed change flags pending
 */
TEST_F(LocoTests, TestLocalLocoSetThrottleAddsSpeedToQueue) {
  // Create new loco and ensure initial states as expected
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceEntry);
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Call setThrottle() to update speed
  _dccexProtocol.setThrottle(loco42, 10, Forward);

  // Should have a pending change now with correct user speed/direction
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // There also should be nothing in the outbound buffer
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test calling setThrottle() for a local Loco with a direction change flags pending
 */
TEST_F(LocoTests, TestLocalLocoSetThrottleAddsDirectionToQueue) {
  // Create new loco and ensure initial states as expected
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceEntry);
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Call setThrottle() to update speed
  _dccexProtocol.setThrottle(loco42, 0, Reverse);

  // Should have a pending change now with correct user speed/direction
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // There also should be nothing in the outbound buffer
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test calling setThrottle() for a Consist with local locos with a speed change flags pending
 */
TEST_F(LocoTests, TestLocalConsistSetThrottleAddsSpeedToQueue) {
  // Create new locos, consist, and ensure initial states as expected
  Loco *loco24 = new Loco(24, LocoSource::LocoSourceEntry);
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceEntry);
  Consist *consist = new Consist();
  consist->addLoco(loco24, Facing::FacingForward);
  consist->addLoco(loco42, Facing::FacingReversed);
  ASSERT_FALSE(loco24->getUserChangePending());
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco24->getUserSpeed(), 0);
  EXPECT_EQ(loco24->getUserDirection(), Forward);
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Call setThrottle() to update speed
  _dccexProtocol.setThrottle(consist, 10, Forward);

  // Should have a pending change now with correct user speed/direction
  ASSERT_TRUE(loco24->getUserChangePending());
  EXPECT_EQ(loco24->getUserSpeed(), 10);
  EXPECT_EQ(loco24->getUserDirection(), Forward);
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // There also should be nothing in the outbound buffer
  EXPECT_EQ(_stream.getOutput(), "");

  // Clean up
  delete consist;
}

/**
 * @brief Test calling setThrottle() for a Consist with local locos with a direction change flags pending
 */
TEST_F(LocoTests, TestLocalConsistSetThrottleAddsDirectionToQueue) {
  // Create new locos, consist, and ensure initial states as expected
  Loco *loco24 = new Loco(24, LocoSource::LocoSourceEntry);
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceEntry);
  Consist *consist = new Consist();
  consist->addLoco(loco24, Facing::FacingForward);
  consist->addLoco(loco42, Facing::FacingReversed);
  ASSERT_FALSE(loco24->getUserChangePending());
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco24->getUserSpeed(), 0);
  EXPECT_EQ(loco24->getUserDirection(), Forward);
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Call setThrottle() to update speed
  _dccexProtocol.setThrottle(consist, 0, Reverse);

  // Should have a pending change now with correct user speed/direction
  ASSERT_TRUE(loco24->getUserChangePending());
  EXPECT_EQ(loco24->getUserSpeed(), 0);
  EXPECT_EQ(loco24->getUserDirection(), Reverse);
  // Loco42 direction doesn't need to change
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // There also should be nothing in the outbound buffer
  EXPECT_EQ(_stream.getOutput(), "");

  // Clean up
  delete consist;
}

/**
 * @brief Test calling check() before queue timer expiry does not send <t ...> for local locos
 */
TEST_F(LocoTests, TestLocalQueuedSetThrottleUnexpiredTimer) {
  // Create a new loco and validate starting state
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceEntry);
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Adjust speed/direction and validate change is pending
  loco42->setUserSpeed(10);
  loco42->setUserDirection(Reverse);
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // Call check()
  _dccexProtocol.check();

  // Validate the state is unchanged and nothing in the buffer
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test calling check() after queue timer expiry sends <t ...> and resets pending for local locos
 */
TEST_F(LocoTests, TestLocalQueuedSetThrottleExpiredTimer) {
  // Create a new loco and validate starting state
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceEntry);
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Adjust speed/direction and validate change is pending
  loco42->setUserSpeed(10);
  loco42->setUserDirection(Reverse);
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // Advance time beyond default 100ms
  advanceMillis(101);

  // Call check()
  _dccexProtocol.check();

  // Validate the state is changed and buffer contains <t 42 10 1>
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);
  EXPECT_THAT(_stream.getOutput(), StartsWith("<t 42 10 0>"));
}

/**
 * @brief Test calling check() with multiple speed changes for local loco only sends latest after timer expiry
 */
TEST_F(LocoTests, TestMultipleLocalSpeedChanges) {
  // Create a new loco and validate starting state
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceEntry);
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Adjust speed/direction and validate change is pending
  loco42->setUserSpeed(10);
  loco42->setUserDirection(Reverse);
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // Advance less that timer expiry and validate no command sent
  advanceMillis(50);
  _dccexProtocol.check();
  ASSERT_TRUE(loco42->getUserChangePending());
  ASSERT_EQ(_stream.getOutput(), "");

  // Advance time beyond default 100ms and validate command is sent and pending reset
  advanceMillis(101);
  _dccexProtocol.check();
  ASSERT_FALSE(loco42->getUserChangePending());
  ASSERT_THAT(_stream.getOutput(), StartsWith("<t 42 10 0>"));
  _stream.clearOutput();

  // Make several changes at intervals
  advanceMillis(20);
  loco42->setUserSpeed(20);
  _dccexProtocol.check();
  ASSERT_TRUE(loco42->getUserChangePending());
  ASSERT_EQ(_stream.getOutput(), "");

  advanceMillis(20);
  loco42->setUserSpeed(30);
  _dccexProtocol.check();
  ASSERT_TRUE(loco42->getUserChangePending());
  ASSERT_EQ(_stream.getOutput(), "");

  advanceMillis(20);
  loco42->setUserSpeed(40);
  _dccexProtocol.check();
  ASSERT_TRUE(loco42->getUserChangePending());
  ASSERT_EQ(_stream.getOutput(), "");

  advanceMillis(20);
  loco42->setUserSpeed(50);
  _dccexProtocol.check();
  ASSERT_TRUE(loco42->getUserChangePending());
  ASSERT_EQ(_stream.getOutput(), "");

  // Now advance beyond timer expiry and validate last speed only sent
  advanceMillis(40);
  _dccexProtocol.check();
  ASSERT_FALSE(loco42->getUserChangePending());
  ASSERT_THAT(_stream.getOutput(), StartsWith("<t 42 50 0>"));
  _stream.clearOutput();
}

/**
 * @brief Test receiving an emergency stop in forward direction resets pending flag
 */
TEST_F(LocoTests, TestEmergencyStopForwardResetsPendingFlagLocal) {
  // Create a new loco and validate starting state
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceEntry);
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Adjust speed/direction and validate change is pending
  loco42->setUserSpeed(10);
  loco42->setUserDirection(Reverse);
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // Simulate receiving a forward EStop and call check()
  _stream << "<l 42 0 129 0>";
  _dccexProtocol.check();

  // Validate pending is reset and speed is back at 0, direction should not be changed
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // This should not trigger a <t ...>
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test receiving an emergency stop in reverse direction resets pending flag
 */
TEST_F(LocoTests, TestEmergencyStopReverseResetsPendingFlagLocal) {
  // Create a new loco and validate starting state
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceEntry);
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Adjust speed/direction and validate change is pending
  loco42->setUserSpeed(10);
  loco42->setUserDirection(Forward);
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Simulate receiving a forward EStop and call check()
  _stream << "<l 42 0 1 0>";
  _dccexProtocol.check();

  // Validate pending is reset and speed is back at 0, direction should not be changed
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // This should not trigger a <t ...>
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test a local loco broadcast to the same state as the pending state prevents <t ...> command
 */
TEST_F(LocoTests, TestLocalLocoBroadcastMatchingPendingPreventsCommand) {
  // Create a new loco and validate starting state
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceEntry);
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Adjust speed/direction and validate change is pending
  loco42->setUserSpeed(10);
  loco42->setUserDirection(Reverse);
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // Simulate receiving an update to the same state and call check()
  _stream << "<l 42 0 11 0>";
  _dccexProtocol.check();

  // No command should be sent
  ASSERT_EQ(_stream.getOutput(), "");

  // Validate pending is reset and speed/direction unchanged
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // Now go beyond timer expiry and ensure no command sent
  advanceMillis(101);
  _dccexProtocol.check();

  // This should not trigger a <t ...>
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test a local loco broadcast to different state as the pending state still allows <t ...> command
 */
TEST_F(LocoTests, TestDifferentLocalLocoBroadcastAllowsCommand) {
  // Create a new loco and validate starting state
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceEntry);
  ASSERT_FALSE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 0);
  EXPECT_EQ(loco42->getUserDirection(), Forward);

  // Adjust speed/direction and validate change is pending
  loco42->setUserSpeed(10);
  loco42->setUserDirection(Reverse);
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // Simulate receiving an update to a different state and call check()
  _stream << "<l 42 0 130 0>";
  _dccexProtocol.check();

  // No command should be sent
  ASSERT_EQ(_stream.getOutput(), "");

  // Validate still pending and speed/direction unchanged
  ASSERT_TRUE(loco42->getUserChangePending());
  EXPECT_EQ(loco42->getUserSpeed(), 10);
  EXPECT_EQ(loco42->getUserDirection(), Reverse);

  // Now go beyond timer expiry and ensure command is sent
  advanceMillis(101);
  _dccexProtocol.check();

  // This should trigger a <t ...>
  EXPECT_THAT(_stream.getOutput(), StartsWith("<t 42 10 0"));
}
