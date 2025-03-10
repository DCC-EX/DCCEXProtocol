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

#include "../setup/RouteTests.h"

TEST_F(RouteTests, parseEmptyRouteList) {
  // Received flag should be false to start
  EXPECT_FALSE(_dccexProtocol.receivedRouteList());
  _dccexProtocol.getLists(false, false, true, false);
  EXPECT_EQ(_stream.getBuffer(), "<JA>\r\n");
  _stream.clearBuffer();

  // Empty route list response
  _stream << "<jA>";
  _dccexProtocol.check();

  // Should be true given route list is empty
  EXPECT_TRUE(_dccexProtocol.receivedRouteList());
}

TEST_F(RouteTests, parseThreeRoutes) {
  // Received flag should be false to start
  EXPECT_FALSE(_dccexProtocol.receivedRouteList());
  _dccexProtocol.getLists(false, false, true, false);
  EXPECT_EQ(_stream.getBuffer(), "<JA>\r\n");
  _stream.clearBuffer();

  // Three route response
  _stream << "<jA 21 121 221>";
  _dccexProtocol.check();

  // Flag should still be false while awaiting details
  EXPECT_FALSE(_dccexProtocol.receivedRouteList());

  // First route response - Route with description
  _stream << R"(<jA 21 R "Route 21">)";
  _dccexProtocol.check();

  // Second route response - Automation with description
  _stream << R"(<jA 121 A "Automation 121">)";
  _dccexProtocol.check();

  // Third route response - Route with no description
  _stream << R"(<jA 221 R "">)";
  // Delegate should call one here
  EXPECT_CALL(_delegate, receivedRouteList()).Times(Exactly(1));
  _dccexProtocol.check();

  // Flag should now be true when all routes received
  EXPECT_TRUE(_dccexProtocol.receivedRouteList());
}
