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

#include "../setup/TurnoutTests.h"

TEST_F(TurnoutTests, parseEmptyTurnoutList) {
  // Received flag should be false to start
  EXPECT_FALSE(_dccexProtocol.receivedTurnoutList());
  _dccexProtocol.getLists(false, true, false, false);
  EXPECT_EQ(_stream.getBuffer(), "<JT>\r\n");
  _stream.clearBuffer();

  // Empty turnout list response
  _stream << "<jT>";
  _dccexProtocol.check();

  // Should be true given turnout list is empty
  EXPECT_TRUE(_dccexProtocol.receivedTurnoutList());
}

TEST_F(TurnoutTests, parseThreeTurnouts) {
  // Received flag should be false to start
  EXPECT_FALSE(_dccexProtocol.receivedTurnoutList());
  _dccexProtocol.getLists(false, true, false, false);
  EXPECT_EQ(_stream.getBuffer(), "<JT>\r\n");
  _stream.clearBuffer();

  // Empty turnout list response
  _stream << "<jT 100 101 102>";
  _dccexProtocol.check();

  // Received flag should still be false
  EXPECT_FALSE(_dccexProtocol.receivedTurnoutList());

  // First turnout response - closed and description
  _stream << R"(<jT 100 C "Turnout 100">)";
  _dccexProtocol.check();

  // Second turnout response - thrown and description
  _stream << R"(<jT 101 T "Turnout 101">)";
  _dccexProtocol.check();

  // Third turnout response - closed and no description
  _stream << R"(<jT 102 C "">)";
  // Delegate should call once here
  EXPECT_CALL(_delegate, receivedTurnoutList()).Times(Exactly(1));
  _dccexProtocol.check();

  // Should be true given turnout list is empty
  EXPECT_TRUE(_dccexProtocol.receivedTurnoutList());
}
