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
 * @brief Validate all lists are requested sequentially by getLists()
 */
TEST_F(DCCEXProtocolTests, getListsSequentialFlow) {
  // Request all lists
  // We expect ONLY the roster to be requested first.
  _dccexProtocol.getLists(true, true, true, true, true);
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

  // Simulate second details which should call receivedRosterList()
  EXPECT_CALL(_delegate, receivedRosterList()).Times(1);
  _stream << "<jR 2 \"Loco2\" \"Func2\">";
  _dccexProtocol.check();

  // Next call to getLists() should start turnouts
  _dccexProtocol.getLists(true, true, true, true, true);
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

  // Simulate second details which should call receivedTurnoutList()
  EXPECT_CALL(_delegate, receivedTurnoutList()).Times(1);
  _stream << "<jT 2 1 \"Turnout2\">";
  _dccexProtocol.check();

  // Next call to getLists() should start routes
  _dccexProtocol.getLists(true, true, true, true, true);
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

  // Simulate second details which should call receivedRouteList()
  EXPECT_CALL(_delegate, receivedRouteList()).Times(1);
  _stream << "<jA 2 A \"Route2\">";
  _dccexProtocol.check();

  // Next call to getLists() should start turntables
  _dccexProtocol.getLists(true, true, true, true, true);
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

  // CS returns all indexes which should trigger receivedTurntableList()
  EXPECT_CALL(_delegate, receivedTurntableList()).Times(1);
  _stream << "<jP 2 0 180 \"Turntable2 Home\">";
  _dccexProtocol.check();
  _stream << "<jP 2 1 10 \"Turntable2 Index1\">";
  _dccexProtocol.check();
  _stream << "<jP 2 2 20 \"Turntable2 Index2\">";
  _dccexProtocol.check();
  _stream.clearOutput();

  // Simulate Signal List
  _dccexProtocol.getLists(true, true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "<J S>");
  _stream.clearOutput();

  EXPECT_FALSE(_dccexProtocol.receivedLists());

  // Simulate receiving the signal list and stream should now request first signal entry details
  _stream << "<jS 100 101>";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J S 100>");
  _stream.clearOutput();

  EXPECT_CALL(_delegate, receivedSignalList()).Times(1);

  _stream << "<jS 100 R  10 \"Signal 100\">";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J S 101>");
  _stream.clearOutput();

 _stream << "<jS 101 A  \"Signal 101\">";     // send without aspect
  _dccexProtocol.check();


  // Final getLists() should set received true
  _dccexProtocol.getLists(true, true, true, true, true);

  // receivedLists() should return true when all lists complete
  EXPECT_TRUE(_dccexProtocol.receivedLists());
  EXPECT_TRUE(_dccexProtocol.receivedRoster());
  EXPECT_TRUE(_dccexProtocol.receivedTurnoutList());
  EXPECT_TRUE(_dccexProtocol.receivedRouteList());
  EXPECT_TRUE(_dccexProtocol.receivedTurntableList());
  EXPECT_TRUE(_dccexProtocol.receivedSignalList());
}

/**
 * @brief Test requesting roster only
 */
TEST_F(DCCEXProtocolTests, getRosterList) {
  // Request all lists
  // We expect ONLY the roster to be requested first.
  _dccexProtocol.getLists(true, false, false, false, false);
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

  // Simulate second details which should call receivedRosterList()
  EXPECT_CALL(_delegate, receivedRosterList()).Times(1);
  _stream << "<jR 2 \"Loco2\" \"Func2\">";
  _dccexProtocol.check();

  // Final getLists() should set received true
  _dccexProtocol.getLists(true, false, false, false, false);

  // receivedLists() should return true when all lists complete
  EXPECT_TRUE(_dccexProtocol.receivedLists());
  EXPECT_TRUE(_dccexProtocol.receivedRoster());
  EXPECT_FALSE(_dccexProtocol.receivedTurnoutList());
  EXPECT_FALSE(_dccexProtocol.receivedRouteList());
  EXPECT_FALSE(_dccexProtocol.receivedTurntableList());
  EXPECT_FALSE(_dccexProtocol.receivedSignalList());
}

/**
 * @brief Test requesting turnouts only
 */
TEST_F(DCCEXProtocolTests, getTurnoutList) {
  // Request all lists
  // We expect ONLY the turnouts to be requested first.
  _dccexProtocol.getLists(false, true, false, false, false);
  EXPECT_EQ(_stream.getOutput(), "<J T>");
  _stream.clearOutput();

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

  // Simulate second details which should call receivedTurnoutList()
  EXPECT_CALL(_delegate, receivedTurnoutList()).Times(1);
  _stream << "<jT 2 1 \"Turnout2\">";
  _dccexProtocol.check();

  // Final getLists() should set received true
  _dccexProtocol.getLists(false, true, false, false, false);

  // receivedLists() should return true when all lists complete
  EXPECT_TRUE(_dccexProtocol.receivedLists());
  EXPECT_FALSE(_dccexProtocol.receivedRoster());
  EXPECT_TRUE(_dccexProtocol.receivedTurnoutList());
  EXPECT_FALSE(_dccexProtocol.receivedRouteList());
  EXPECT_FALSE(_dccexProtocol.receivedTurntableList());
  EXPECT_FALSE(_dccexProtocol.receivedSignalList());
}

/**
 * @brief Test requesting routes only
 */
TEST_F(DCCEXProtocolTests, getRouteList) {
  // Request all lists
  // We expect ONLY the route list to be requested first.
  _dccexProtocol.getLists(false, false, true, false, false);
  EXPECT_EQ(_stream.getOutput(), "<J A>");
  _stream.clearOutput();

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

  // Simulate second details which should call receivedRouteList()
  EXPECT_CALL(_delegate, receivedRouteList()).Times(1);
  _stream << "<jA 2 A \"Route2\">";
  _dccexProtocol.check();

  // Final getLists() should set received true
  _dccexProtocol.getLists(false, false, true, false, false);

  // receivedLists() should return true when all lists complete
  EXPECT_TRUE(_dccexProtocol.receivedLists());
  EXPECT_FALSE(_dccexProtocol.receivedRoster());
  EXPECT_FALSE(_dccexProtocol.receivedTurnoutList());
  EXPECT_TRUE(_dccexProtocol.receivedRouteList());
  EXPECT_FALSE(_dccexProtocol.receivedTurntableList());
  EXPECT_FALSE(_dccexProtocol.receivedSignalList());
}

/**
 * @brief Test getting turntable list only
 */
TEST_F(DCCEXProtocolTests, getTurntableList) {
  // Request all lists
  // We expect ONLY the turntable list to be requested first.
  _dccexProtocol.getLists(false, false, false, true, false);
  EXPECT_EQ(_stream.getOutput(), "<J O>");
  _stream.clearOutput();

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

  // CS returns all indexes which should trigger receivedTurntableList()
  EXPECT_CALL(_delegate, receivedTurntableList()).Times(1);
  _stream << "<jP 2 0 180 \"Turntable2 Home\">";
  _dccexProtocol.check();
  _stream << "<jP 2 1 10 \"Turntable2 Index1\">";
  _dccexProtocol.check();
  _stream << "<jP 2 2 20 \"Turntable2 Index2\">";
  _dccexProtocol.check();

  // Final getLists() should set received true
  _dccexProtocol.getLists(false, false, false, true, false);

  // receivedLists() should return true when all lists complete
  EXPECT_TRUE(_dccexProtocol.receivedLists());
  EXPECT_FALSE(_dccexProtocol.receivedRoster());
  EXPECT_FALSE(_dccexProtocol.receivedTurnoutList());
  EXPECT_FALSE(_dccexProtocol.receivedRouteList());
  EXPECT_TRUE(_dccexProtocol.receivedTurntableList());
  EXPECT_FALSE(_dccexProtocol.receivedSignalList());
}

/**
 * @brief Test just turnouts and turntables
 */
TEST_F(DCCEXProtocolTests, getTurnoutAndTurntableList) {
  // Request all lists
  // We expect ONLY the turnout list to be requested first.
  _dccexProtocol.getLists(false, true, false, true, false);
  EXPECT_EQ(_stream.getOutput(), "<J T>");
  _stream.clearOutput();

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

  // Simulate second details which should call receivedTurnoutList()
  EXPECT_CALL(_delegate, receivedTurnoutList()).Times(1);
  _stream << "<jT 2 1 \"Turnout2\">";
  _dccexProtocol.check();

  // Next call to getLists() should start turntables
  _dccexProtocol.getLists(false, true, false, true, false);
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

  // CS returns all indexes which should trigger receivedTurntableList()
  EXPECT_CALL(_delegate, receivedTurntableList()).Times(1);
  _stream << "<jP 2 0 180 \"Turntable2 Home\">";
  _dccexProtocol.check();
  _stream << "<jP 2 1 10 \"Turntable2 Index1\">";
  _dccexProtocol.check();
  _stream << "<jP 2 2 20 \"Turntable2 Index2\">";
  _dccexProtocol.check();

  // Final getLists() should set received true
  _dccexProtocol.getLists(false, true, false, true, false);

  // receivedLists() should return true when all lists complete
  EXPECT_TRUE(_dccexProtocol.receivedLists());
  EXPECT_FALSE(_dccexProtocol.receivedRoster());
  EXPECT_TRUE(_dccexProtocol.receivedTurnoutList());
  EXPECT_FALSE(_dccexProtocol.receivedRouteList());
  EXPECT_TRUE(_dccexProtocol.receivedTurntableList());
  EXPECT_FALSE(_dccexProtocol.receivedSignalList());
}

/**
 * @brief Test requesting signal list 
 */
TEST_F(DCCEXProtocolTests, getSignalList) {
  _dccexProtocol.getLists(false, false, false, false, true);
  EXPECT_EQ(_stream.getOutput(), "<J S>");
  _stream.clearOutput();

   // Simulate receiving the signal list and stream should now request first signal details
  _stream << "<jS 200 201>";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J S 200>");
  _stream.clearOutput();

  EXPECT_CALL(_delegate, receivedSignalList()).Times(1);

  _stream << "<jS 200 G 6 \"Signal200\">";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J S 201>");
  _stream.clearOutput();

  _stream << "<jS 201 A 20 \"Signal201\">";
  _dccexProtocol.check();

   // Final getLists() should set received true
  _dccexProtocol.getLists(false, false, false, false, true);

  // receivedLists() should return true when all lists complete
  EXPECT_TRUE(_dccexProtocol.receivedLists());
  EXPECT_FALSE(_dccexProtocol.receivedRoster());
  EXPECT_FALSE(_dccexProtocol.receivedTurnoutList());
  EXPECT_FALSE(_dccexProtocol.receivedRouteList());
  EXPECT_FALSE(_dccexProtocol.receivedTurntableList());
  EXPECT_TRUE(_dccexProtocol.receivedSignalList());
}


/**
 * @brief Test requesting no lists should set receivedLists true
 */
TEST_F(DCCEXProtocolTests, testRequestNoLists) {
  // Calling getLists() with all false should immediately set receivedLists() true
  _dccexProtocol.getLists(false, false, false, false, false);
  EXPECT_EQ(_stream.getOutput(), "");
  EXPECT_TRUE(_dccexProtocol.receivedLists());
}

/**
 * @brief Test calling getLists() repeatedly while a request is in flight does not duplicate requests
 */
TEST_F(DCCEXProtocolTests, getListsNoDuplicateRequests) {
  // Roster stage: first call requests the roster, a second call must not
  _dccexProtocol.getLists(true, true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "<J R>");
  _stream.clearOutput();
  _dccexProtocol.getLists(true, true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "");

  // Complete the roster
  _stream << "<jR 1 2>";
  _dccexProtocol.check();
  _stream.clearOutput();
  _stream << "<jR 1 \"Loco1\" \"Func1\">";
  _dccexProtocol.check();
  _stream.clearOutput();
  EXPECT_CALL(_delegate, receivedRosterList()).Times(Exactly(1));
  _stream << "<jR 2 \"Loco2\" \"Func2\">";
  _dccexProtocol.check();
  EXPECT_TRUE(_dccexProtocol.receivedRoster());

  // Turnout stage: request, then duplicate call must not re-request
  _dccexProtocol.getLists(true, true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "<J T>");
  _stream.clearOutput();
  _dccexProtocol.getLists(true, true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "");

  // Complete the turnouts
  _stream << "<jT 1 2>";
  _dccexProtocol.check();
  _stream.clearOutput();
  _stream << "<jT 1 0 \"Turnout1\">";
  _dccexProtocol.check();
  _stream.clearOutput();
  EXPECT_CALL(_delegate, receivedTurnoutList()).Times(Exactly(1));
  _stream << "<jT 2 1 \"Turnout2\">";
  _dccexProtocol.check();
  EXPECT_TRUE(_dccexProtocol.receivedTurnoutList());

  // Route stage: request, then duplicate call must not re-request
  _dccexProtocol.getLists(true, true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "<J A>");
  _stream.clearOutput();
  _dccexProtocol.getLists(true, true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "");

  // Complete the routes
  _stream << "<jA 1 2>";
  _dccexProtocol.check();
  _stream.clearOutput();
  _stream << "<jA 1 R \"Route1\">";
  _dccexProtocol.check();
  _stream.clearOutput();
  EXPECT_CALL(_delegate, receivedRouteList()).Times(Exactly(1));
  _stream << "<jA 2 A \"Route2\">";
  _dccexProtocol.check();
  EXPECT_TRUE(_dccexProtocol.receivedRouteList());

  // Turntable stage: request, then duplicate call must not re-request
  _dccexProtocol.getLists(true, true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "<J O>");
  _stream.clearOutput();
  _dccexProtocol.getLists(true, true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "");

  // Complete a single turntable and its index
  _stream << "<jO 1>";
  _dccexProtocol.check();
  _stream.clearOutput();
  _stream << "<jO 1 0 0 1 \"Turntable1\">";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J P 1>");
  _stream.clearOutput();
  EXPECT_CALL(_delegate, receivedTurntableList()).Times(Exactly(1));
  _stream << "<jP 1 0 0 \"Home\">";
  _dccexProtocol.check();
  EXPECT_TRUE(_dccexProtocol.receivedTurntableList());

  // Signal stage: request, then duplicate call must not re-request
  _dccexProtocol.getLists(true, true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "<J S>");
  _stream.clearOutput();
  _dccexProtocol.getLists(true, true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "");

  // Complete signals
  EXPECT_CALL(_delegate, receivedSignalList()).Times(Exactly(1));

  _stream << "<jS 10>";
  _dccexProtocol.check();
  _stream.clearOutput();
  _stream << "<jS 10 R 3 \"Signal10\">";
  _dccexProtocol.check();
  _stream.clearOutput();
  EXPECT_TRUE(_dccexProtocol.receivedSignalList());


  // Final getLists() completes the sequence
  _dccexProtocol.getLists(true, true, true, true, true);
  EXPECT_TRUE(_dccexProtocol.receivedLists());

  // Once all lists are received, any further call must not send anything
  _dccexProtocol.getLists(true, true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test refreshAllLists() clears the lists and resets all the received flags
 */
TEST_F(DCCEXProtocolTests, refreshAllListsResetsListsAndFlags) {
  // Request all lists using empty list responses to set every received flag
  _dccexProtocol.getLists(true, true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "<J R>");
  _stream.clearOutput();
  _stream << "<jR>";
  _dccexProtocol.check();
  EXPECT_TRUE(_dccexProtocol.receivedRoster());

  _dccexProtocol.getLists(true, true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "<J T>");
  _stream.clearOutput();
  _stream << "<jT>";
  _dccexProtocol.check();
  EXPECT_TRUE(_dccexProtocol.receivedTurnoutList());

  _dccexProtocol.getLists(true, true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "<J A>");
  _stream.clearOutput();
  _stream << "<jA>";
  _dccexProtocol.check();
  EXPECT_TRUE(_dccexProtocol.receivedRouteList());

  _dccexProtocol.getLists(true, true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "<J O>");
  _stream.clearOutput();
  _stream << "<jO>";
  _dccexProtocol.check();
  EXPECT_TRUE(_dccexProtocol.receivedTurntableList());

  _dccexProtocol.getLists(true, true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "<J S>");
  _stream.clearOutput();
  _stream << "<jS>";
  _dccexProtocol.check();
  EXPECT_TRUE(_dccexProtocol.receivedSignalList());

  // All lists received
  _dccexProtocol.getLists(true, true, true, true, true);
  EXPECT_TRUE(_dccexProtocol.receivedLists());

  // Refreshing all lists must clear them and reset every received flag
  _dccexProtocol.refreshAllLists();
  EXPECT_FALSE(_dccexProtocol.receivedLists());
  EXPECT_FALSE(_dccexProtocol.receivedRoster());
  EXPECT_FALSE(_dccexProtocol.receivedTurnoutList());
  EXPECT_FALSE(_dccexProtocol.receivedRouteList());
  EXPECT_FALSE(_dccexProtocol.receivedTurntableList());
  EXPECT_FALSE(_dccexProtocol.receivedSignalList());

  // A fresh getLists() should request the roster again
  _dccexProtocol.getLists(true, true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "<J R>");
}
