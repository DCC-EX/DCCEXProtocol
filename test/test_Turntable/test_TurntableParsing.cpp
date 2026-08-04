/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
 *
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

#include "../setup/TurntableTests.h"

TEST_F(TurntableTests, parseEmptyTurntableList) {
  // Received flag should be false to start
  EXPECT_FALSE(_dccexProtocol.receivedTurntableList());
  _dccexProtocol.getLists(false, false, false, true);
  EXPECT_EQ(_stream.getOutput(), "<J O>");
  _stream.clearOutput();

  // Empty turntable list response
  _stream << "<jO>";
  _dccexProtocol.check();

  // Should be true given turntable list is empty
  EXPECT_TRUE(_dccexProtocol.receivedTurntableList());
}

TEST_F(TurntableTests, parseTwoTurntables) {
  // Received flag should be false to start
  EXPECT_FALSE(_dccexProtocol.receivedTurntableList());
  _dccexProtocol.getLists(false, false, false, true);
  EXPECT_EQ(_stream.getOutput(), "<J O>");
  _stream.clearOutput();

  // Two turntables in response
  _stream << "<jO 1 2>";
  _dccexProtocol.check();

  // Received should still be false while waiting for details
  EXPECT_FALSE(_dccexProtocol.receivedTurntableList());

  // First turntable response - EX-Turntable at ID 1 with 5 indexes, currently at home position
  _stream << R"(<jO 1 1 0 5 "EX-Turntable">)";

  // Second turntable response - DCC Turntable at ID 1 with 6 indexes, currently at position 3
  _stream << R"(<jO 2 0 3 6 "DCC Turntable">)";

  // ID 1 Position responses
  _stream << R"(<jP 1 0 900 "Home">)";
  _dccexProtocol.check();
  _stream << R"(<jP 1 1 450 "Position 1">)";
  _dccexProtocol.check();
  _stream << R"(<jP 1 2 1800 "Position 2">)";
  _dccexProtocol.check();
  _stream << R"(<jP 1 3 2700 "Position 3">)";
  _dccexProtocol.check();
  _stream << R"(<jP 1 4 3000 "Position 4">)";
  _dccexProtocol.check();

  // ID 2 Position responses
  _stream << R"(<jP 2 0 0 "Home">)";
  _dccexProtocol.check();
  _stream << R"(<jP 2 1 450 "Position 1">)";
  _dccexProtocol.check();
  _stream << R"(<jP 2 2 1800 "Position 2">)";
  _dccexProtocol.check();
  _stream << R"(<jP 2 3 2700 "Position 3">)";
  _dccexProtocol.check();
  _stream << R"(<jP 2 4 3000 "Position 4">)";
  _dccexProtocol.check();
  _stream << R"(<jP 2 5 3300 "Position 5">)";

  // Delegate should call back once here
  EXPECT_CALL(_delegate, receivedTurntableList()).Times(Exactly(1));
  _dccexProtocol.check();

  // Now the flag should be true
  EXPECT_TRUE(_dccexProtocol.receivedTurntableList());

  // Validate count
  EXPECT_EQ(_dccexProtocol.getTurntableCount(), 2);
}

/**
 * @brief Test turntable entries received out of order are accepted without requesting missing details
 */
TEST_F(TurntableTests, parseTurntableEntriesOutOfOrder) {
  // Received flag should be false to start
  EXPECT_FALSE(_dccexProtocol.receivedTurntableList());
  _dccexProtocol.getLists(false, false, false, true);
  _stream.clearOutput();

  // Two turntables in response
  _stream << "<jO 1 2>";
  _dccexProtocol.check();

  // Entries received out of order, starting with the last in the list
  _stream << R"(<jO 2 0 3 6 "DCC Turntable">)";
  _dccexProtocol.check();

  _stream << R"(<jO 1 1 0 5 "EX-Turntable">)";
  _dccexProtocol.check();

  // Count must remain two, no extra turntables created
  EXPECT_EQ(_dccexProtocol.getTurntableCount(), 2);
}

/**
 * @brief Test that a turntable broadcast notifies the delegate and updates the turntable
 */
TEST_F(TurntableTests, turntableBroadcastDelegateCalled) {
  // Set up a dummy turntable
  Turntable *tt = new Turntable(1);
  tt->setType(TurntableType::TurntableTypeDCC);
  tt->addIndex(new TurntableIndex(1, 0, 0, "Home"));
  tt->addIndex(new TurntableIndex(1, 1, 0, "Index1"));

  // Simulate receiving a broadcast to move to index 1, not moving
  EXPECT_CALL(_delegate, receivedTurntableAction(1, 1, false)).Times(Exactly(1));
  _stream << "<I 1 1 0>";
  _dccexProtocol.check();

  // The turntable should reflect the new index and moving state
  EXPECT_EQ(tt->getIndex(), 1);
  EXPECT_FALSE(tt->isMoving());
}

/**
 * @brief Test that a turntable broadcast with the moving flag set is forwarded to the delegate
 */
TEST_F(TurntableTests, turntableBroadcastMovingFlag) {
  // Set up a dummy turntable
  Turntable *tt = new Turntable(1);
  tt->setType(TurntableType::TurntableTypeDCC);
  tt->addIndex(new TurntableIndex(1, 0, 0, "Home"));
  tt->addIndex(new TurntableIndex(1, 1, 0, "Index1"));

  // Simulate receiving a broadcast to move to index 0, moving
  EXPECT_CALL(_delegate, receivedTurntableAction(1, 0, true)).Times(Exactly(1));
  _stream << "<I 1 0 1>";
  _dccexProtocol.check();

  // The turntable should reflect the new index and moving state
  EXPECT_EQ(tt->getIndex(), 0);
  EXPECT_TRUE(tt->isMoving());
}

/**
 * @brief Test that malformed/orphaned index entries don't leak memory
 */
TEST_F(TurntableTests, orphanedIndexEntryLeak) {
  // Create one valid turntable so getFirst() isn't null
  new Turntable(1);
  
  // Simulate an index entry for ID 99 (which DOES NOT EXIST)
  _stream << R"(<jP 99 0 0 "Orphaned Index">)";
  _dccexProtocol.check();
  
  // Simulate an entry with wrong parameter count
  // This triggers the first 'if (DCCEXInbound::getParameterCount() != 5) return;'
  _stream << R"(<jP 1 0 0>)"; // Missing the name parameter
  _dccexProtocol.check();
  
  // Cleanup
  Turntable::clearTurntableList();
}
