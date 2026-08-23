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

TEST_F(LocoTests, parseEmptyRoster) {
  EXPECT_FALSE(_dccexProtocol.receivedRoster());
  _dccexProtocol.getLists(true, false, false, false);
  _getListsGetServerVersion();

  _dccexProtocol.getLists(true, false, false, false);
  EXPECT_EQ(_stream.getOutput(), "<J R>");
  _stream.clearOutput();

  // Response
  _stream << "<jR>";
  _dccexProtocol.check();

  // Returns true since roster is empty
  EXPECT_TRUE(_dccexProtocol.receivedRoster());
}

TEST_F(LocoTests, parseRosterWithThreeIDs) {
  EXPECT_FALSE(_dccexProtocol.receivedRoster());
  _dccexProtocol.getLists(true, false, false, false);
  _getListsGetServerVersion();

  _dccexProtocol.getLists(true, false, false, false);
  EXPECT_EQ(_stream.getOutput(), "<J R>");
  _stream.clearOutput();

  // Response
  _stream << "<jR 42 9 120>";
  _dccexProtocol.check();

  // Still false, wait for details
  EXPECT_FALSE(_dccexProtocol.receivedRoster());

  // Detailed response for 42
  _stream << R"(<jR 42 "Loco42" "Func42">)";
  _dccexProtocol.check();

  // Detailed response for 9
  _stream << R"(<jR 9 "Loco9" "Func9">)";
  _dccexProtocol.check();

  // Detailed response for 120
  _stream << R"(<jR 120 "Loco120" "Func120">)";
  EXPECT_CALL(_delegate, receivedRosterList()).Times(Exactly(1));
  _dccexProtocol.check();

  // Returns true since roster is complete
  EXPECT_TRUE(_dccexProtocol.receivedRoster());

  // Validate roster count
  EXPECT_EQ(_dccexProtocol.getRosterCount(), 3);

  // Validate locos can be found
  Loco *test = _dccexProtocol.findLocoInRoster(42);
  EXPECT_NE(test, nullptr);
  test = _dccexProtocol.findLocoInRoster(9);
  EXPECT_NE(test, nullptr);
}

/**
 * @brief Test checking for a loco not in the roster returns sanely
 */
TEST_F(LocoTests, TestLocoNotInRoster) {
  // Simulate receiving roster
  _dccexProtocol.getLists(true, false, false, false);
  _stream << "<jR 42 9 120>";
  _dccexProtocol.check();
  _stream << R"(<jR 42 "Loco42" "Func42">)";
  _dccexProtocol.check();

  // Detailed response for 9
  _stream << R"(<jR 9 "Loco9" "Func9">)";
  _dccexProtocol.check();

  // Detailed response for 120
  _stream << R"(<jR 120 "Loco120" "Func120">)";
  EXPECT_CALL(_delegate, receivedRosterList()).Times(Exactly(1));
  _dccexProtocol.check();
  ASSERT_TRUE(_dccexProtocol.receivedRoster());

  // Validate a non-existing loco returns correcly
  Loco *test = _dccexProtocol.findLocoInRoster(22);
  EXPECT_EQ(test, nullptr);

  // Clear roster
  _dccexProtocol.clearRoster();

  // Check a loco that was in the roster now is not
  test = _dccexProtocol.findLocoInRoster(42);
  EXPECT_EQ(test, nullptr);
}

/**
 * @brief Test roster entries received out of order are accepted without requesting missing details
 */
TEST_F(LocoTests, parseRosterEntriesOutOfOrder) {
  EXPECT_FALSE(_dccexProtocol.receivedRoster());
  _dccexProtocol.getLists(true, false, false, false);
  _stream.clearOutput();

  // Response
  _stream << "<jR 42 9 120>";
  _dccexProtocol.check();

  // Entries received out of order, starting with the last in the list
  _stream << R"(<jR 120 "Loco120" "Func120">)";
  EXPECT_CALL(_delegate, receivedRosterList()).Times(Exactly(1));
  _dccexProtocol.check();

  _stream << R"(<jR 9 "Loco9" "Func9">)";
  EXPECT_CALL(_delegate, receivedRosterList()).Times(Exactly(1));
  _dccexProtocol.check();

  _stream << R"(<jR 42 "Loco42" "Func42">)";
  EXPECT_CALL(_delegate, receivedRosterList()).Times(Exactly(1));
  _dccexProtocol.check();

  // Roster is complete and the count is unchanged
  EXPECT_TRUE(_dccexProtocol.receivedRoster());
  EXPECT_EQ(_dccexProtocol.getRosterCount(), 3);
}

/**
 * @brief Test clearLocalLocos() deletes all local (non-roster) locos
 */
TEST_F(LocoTests, clearLocalLocosClearsList) {
  // Create two local locos (LocoSourceEntry locos live in the local loco list)
  Loco *loco1 = new Loco(42, LocoSource::LocoSourceEntry);
  loco1->setName("Loco 42");
  Loco *loco2 = new Loco(43, LocoSource::LocoSourceEntry);
  loco2->setName("Loco 43");

  // Validate they are in the local loco list
  ASSERT_EQ(Loco::getFirstLocalLoco(), loco1);
  EXPECT_EQ(loco1->getNext(), loco2);
  EXPECT_EQ(loco2->getNext(), nullptr);

  // Clearing local locos must delete them and reset the head
  _dccexProtocol.clearLocalLocos();
  EXPECT_EQ(Loco::getFirstLocalLoco(), nullptr);

  // The roster list must remain untouched
  EXPECT_EQ(Loco::getFirst(), nullptr);
}

/**
 * @brief Test refreshRoster() clears the roster, resets the flags, and re-requests on getLists()
 */
TEST_F(LocoTests, refreshRosterResetsAndReRequests) {
  // Request and receive the roster
  _dccexProtocol.getLists(true, false, false, false);
  _getListsGetServerVersion();

  _dccexProtocol.getLists(true, false, false, false);
  EXPECT_EQ(_stream.getOutput(), "<J R>");
  _stream.clearOutput();

  _stream << "<jR 42 9 120>";
  _dccexProtocol.check();
  _stream << R"(<jR 42 "Loco42" "Func42">)";
  _stream << R"(<jR 9 "Loco9" "Func9">)";
  _stream << R"(<jR 120 "Loco120" "Func120">)";
  EXPECT_CALL(_delegate, receivedRosterList()).Times(Exactly(1));
  _dccexProtocol.check();
  ASSERT_EQ(_dccexProtocol.getRosterCount(), 3);
  ASSERT_TRUE(_dccexProtocol.receivedRoster());

  // Clear the entry detail requests accumulated while receiving the roster
  _stream.clearOutput();

  // Refreshing must clear the roster and reset the received/requested flags
  _dccexProtocol.refreshRoster();
  EXPECT_EQ(_dccexProtocol.getRosterCount(), 0);
  EXPECT_EQ(Loco::getFirst(), nullptr);
  EXPECT_FALSE(_dccexProtocol.receivedRoster());
  EXPECT_FALSE(_dccexProtocol.receivedLists());

  // A fresh getLists() must request the roster again
  _dccexProtocol.getLists(true, false, false, false);
  EXPECT_EQ(_stream.getOutput(), "<J R>");
}

/**
 * @brief Test a roster entry for an address not in the roster is accepted without requesting details
 */
TEST_F(LocoTests, parseRosterEntryUnknownAddress) {
  EXPECT_FALSE(_dccexProtocol.receivedRoster());

  // Detailed response for an unknown address
  _stream << R"(<jR 999 "Loco999" "Func999">)";
  EXPECT_CALL(_delegate, receivedRosterList()).Times(Exactly(1));
  _dccexProtocol.check();

  // The roster is received, nothing is created, and no entry details are requested
  EXPECT_TRUE(_dccexProtocol.receivedRoster());
  EXPECT_EQ(_dccexProtocol.getRosterCount(), 0);
  EXPECT_EQ(_stream.getOutput(), "");
}
