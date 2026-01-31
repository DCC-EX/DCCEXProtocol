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
  _dccexProtocol.getLists(true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "<J R>\r\n");
  _stream.clearOutput();

  // Simulate receiving the roster list and stream should now request first roster entry details
  _stream << "<jR 1 2>";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J R 1>\r\n");
  _stream.clearOutput();

  // Simulate receiving first roster details which should trigger retrieving second entry
  _stream << "<jR 1 \"Loco1\" \"Func1\">";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J R 2>\r\n");
  _stream.clearOutput();

  // Simulate second details which should call receivedRosterList()
  EXPECT_CALL(_delegate, receivedRosterList()).Times(1);
  _stream << "<jR 2 \"Loco2\" \"Func2\">";
  _dccexProtocol.check();

  // Next call to getLists() should start turnouts
  _dccexProtocol.getLists(true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "<J T>\r\n");
  _stream.clearOutput();

  // Simulate receiving the turnout list and stream should now request first turnout details
  _stream << "<jT 1 2>";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J T 1>\r\n");
  _stream.clearOutput();

  // Simulate receiving first turnout details which should trigger retrieving second entry
  _stream << "<jT 1 0 \"Turnout1\">";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J T 2>\r\n");
  _stream.clearOutput();

  // Simulate second details which should call receivedTurnoutList()
  EXPECT_CALL(_delegate, receivedTurnoutList()).Times(1);
  _stream << "<jT 2 1 \"Turnout2\">";
  _dccexProtocol.check();

  // Next call to getLists() should start routes
  _dccexProtocol.getLists(true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "<J A>\r\n");
  _stream.clearOutput();

  // Simulate receiving the route list and stream should now request first route details
  _stream << "<jA 1 2>";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J A 1>\r\n");
  _stream.clearOutput();

  // Simulate receiving first route details which should trigger retrieving second entry
  _stream << "<jA 1 R \"Route1\">";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J A 2>\r\n");
  _stream.clearOutput();

  // Simulate second details which should call receivedRouteList()
  EXPECT_CALL(_delegate, receivedRouteList()).Times(1);
  _stream << "<jA 2 A \"Route2\">";
  _dccexProtocol.check();

  // Next call to getLists() should start turntables
  _dccexProtocol.getLists(true, true, true, true);
  EXPECT_EQ(_stream.getOutput(), "<J O>\r\n");
  _stream.clearOutput();

  // Simulate receiving the turntable list and stream should now request first turntable details
  _stream << "<jO 1 2>";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J O 1>\r\n");
  _stream.clearOutput();

  // Simulate receiving first turntable details
  _stream << "<jO 1 0 1 3 \"Turntable1\">";
  _dccexProtocol.check();
  // This requests both the this turntable's indexes and the next turntable
  EXPECT_EQ(_stream.getOutput(), "<J P 1>\r\n<J O 2>\r\n");
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
  EXPECT_EQ(_stream.getOutput(), "<J P 2>\r\n");

  // CS returns all indexes which should trigger receivedTurntableList()
  EXPECT_CALL(_delegate, receivedTurntableList()).Times(1);
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
 * @brief Test requesting roster only
 */
TEST_F(DCCEXProtocolTests, getRosterList) {
  // Request all lists
  // We expect ONLY the roster to be requested first.
  _dccexProtocol.getLists(true, false, false, false);
  EXPECT_EQ(_stream.getOutput(), "<J R>\r\n");
  _stream.clearOutput();

  // Simulate receiving the roster list and stream should now request first roster entry details
  _stream << "<jR 1 2>";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J R 1>\r\n");
  _stream.clearOutput();

  // Simulate receiving first roster details which should trigger retrieving second entry
  _stream << "<jR 1 \"Loco1\" \"Func1\">";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J R 2>\r\n");
  _stream.clearOutput();

  // Simulate second details which should call receivedRosterList()
  EXPECT_CALL(_delegate, receivedRosterList()).Times(1);
  _stream << "<jR 2 \"Loco2\" \"Func2\">";
  _dccexProtocol.check();

  // Final getLists() should set received true
  _dccexProtocol.getLists(true, false, false, false);

  // receivedLists() should return true when all lists complete
  EXPECT_TRUE(_dccexProtocol.receivedLists());
}

/**
 * @brief Test requesting turnouts only
 */
TEST_F(DCCEXProtocolTests, getTurnoutList) {
  // Request all lists
  // We expect ONLY the turnouts to be requested first.
  _dccexProtocol.getLists(false, true, false, false);
  EXPECT_EQ(_stream.getOutput(), "<J T>\r\n");
  _stream.clearOutput();

  // Simulate receiving the turnout list and stream should now request first turnout details
  _stream << "<jT 1 2>";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J T 1>\r\n");
  _stream.clearOutput();

  // Simulate receiving first turnout details which should trigger retrieving second entry
  _stream << "<jT 1 0 \"Turnout1\">";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J T 2>\r\n");
  _stream.clearOutput();

  // Simulate second details which should call receivedTurnoutList()
  EXPECT_CALL(_delegate, receivedTurnoutList()).Times(1);
  _stream << "<jT 2 1 \"Turnout2\">";
  _dccexProtocol.check();

   // Final getLists() should set received true
  _dccexProtocol.getLists(false, true, false, false);

  // receivedLists() should return true when all lists complete
  EXPECT_TRUE(_dccexProtocol.receivedLists());
}

/**
 * @brief Test requesting routes only
 */
TEST_F(DCCEXProtocolTests, getRouteList) {
  // Request all lists
  // We expect ONLY the route list to be requested first.
  _dccexProtocol.getLists(false, false, true, false);
  EXPECT_EQ(_stream.getOutput(), "<J A>\r\n");
  _stream.clearOutput();

  // Simulate receiving the route list and stream should now request first route details
  _stream << "<jA 1 2>";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J A 1>\r\n");
  _stream.clearOutput();

  // Simulate receiving first route details which should trigger retrieving second entry
  _stream << "<jA 1 R \"Route1\">";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J A 2>\r\n");
  _stream.clearOutput();

  // Simulate second details which should call receivedRouteList()
  EXPECT_CALL(_delegate, receivedRouteList()).Times(1);
  _stream << "<jA 2 A \"Route2\">";
  _dccexProtocol.check();

  // Final getLists() should set received true
  _dccexProtocol.getLists(false, false, true, false);

  // receivedLists() should return true when all lists complete
  EXPECT_TRUE(_dccexProtocol.receivedLists());
}

/**
 * @brief Test getting turntable list only
 */
TEST_F(DCCEXProtocolTests, getTurntableList) {
  // Request all lists
  // We expect ONLY the turntable list to be requested first.
  _dccexProtocol.getLists(false, false, false, true);
  EXPECT_EQ(_stream.getOutput(), "<J O>\r\n");
  _stream.clearOutput();

  // Simulate receiving the turntable list and stream should now request first turntable details
  _stream << "<jO 1 2>";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J O 1>\r\n");
  _stream.clearOutput();

  // Simulate receiving first turntable details
  _stream << "<jO 1 0 1 3 \"Turntable1\">";
  _dccexProtocol.check();
  // This requests both the this turntable's indexes and the next turntable
  EXPECT_EQ(_stream.getOutput(), "<J P 1>\r\n<J O 2>\r\n");
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
  EXPECT_EQ(_stream.getOutput(), "<J P 2>\r\n");

  // CS returns all indexes which should trigger receivedTurntableList()
  EXPECT_CALL(_delegate, receivedTurntableList()).Times(1);
  _stream << "<jP 2 0 180 \"Turntable2 Home\">";
  _dccexProtocol.check();
  _stream << "<jP 2 1 10 \"Turntable2 Index1\">";
  _dccexProtocol.check();
  _stream << "<jP 2 2 20 \"Turntable2 Index2\">";
  _dccexProtocol.check();

  // Final getLists() should set received true
  _dccexProtocol.getLists(false, false, false, true);

  // receivedLists() should return true when all lists complete
  EXPECT_TRUE(_dccexProtocol.receivedLists());
}

/**
 * @brief Test just turnouts and turntables
 */
TEST_F(DCCEXProtocolTests, getTurnoutAndTurntableList) {
  // Request all lists
  // We expect ONLY the turnout list to be requested first.
  _dccexProtocol.getLists(false, true, false, true);
  EXPECT_EQ(_stream.getOutput(), "<J T>\r\n");
  _stream.clearOutput();

  // Simulate receiving the turnout list and stream should now request first turnout details
  _stream << "<jT 1 2>";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J T 1>\r\n");
  _stream.clearOutput();

  // Simulate receiving first turnout details which should trigger retrieving second entry
  _stream << "<jT 1 0 \"Turnout1\">";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J T 2>\r\n");
  _stream.clearOutput();

  // Simulate second details which should call receivedTurnoutList()
  EXPECT_CALL(_delegate, receivedTurnoutList()).Times(1);
  _stream << "<jT 2 1 \"Turnout2\">";
  _dccexProtocol.check();

  // Next call to getLists() should start turntables
  _dccexProtocol.getLists(false, true, false, true);
  EXPECT_EQ(_stream.getOutput(), "<J O>\r\n");
  _stream.clearOutput();

  // Simulate receiving the turntable list and stream should now request first turntable details
  _stream << "<jO 1 2>";
  _dccexProtocol.check();
  EXPECT_EQ(_stream.getOutput(), "<J O 1>\r\n");
  _stream.clearOutput();

  // Simulate receiving first turntable details
  _stream << "<jO 1 0 1 3 \"Turntable1\">";
  _dccexProtocol.check();
  // This requests both the this turntable's indexes and the next turntable
  EXPECT_EQ(_stream.getOutput(), "<J P 1>\r\n<J O 2>\r\n");
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
  EXPECT_EQ(_stream.getOutput(), "<J P 2>\r\n");

  // CS returns all indexes which should trigger receivedTurntableList()
  EXPECT_CALL(_delegate, receivedTurntableList()).Times(1);
  _stream << "<jP 2 0 180 \"Turntable2 Home\">";
  _dccexProtocol.check();
  _stream << "<jP 2 1 10 \"Turntable2 Index1\">";
  _dccexProtocol.check();
  _stream << "<jP 2 2 20 \"Turntable2 Index2\">";
  _dccexProtocol.check();

  // Final getLists() should set received true
  _dccexProtocol.getLists(false, true, false, true);

  // receivedLists() should return true when all lists complete
  EXPECT_TRUE(_dccexProtocol.receivedLists());
}
