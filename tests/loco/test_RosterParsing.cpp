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

TEST_F(LocoTests, parseEmptyRoster) {
  EXPECT_FALSE(_dccexProtocol.receivedRoster());
  _dccexProtocol.getLists(true, false, false, false);
  EXPECT_EQ(_stream.getBuffer(), "<JR>\r\n");
  _stream.clearBuffer();

  // Response
  _stream << "<jR>";
  _dccexProtocol.check();

  // Returns true since roster is empty
  EXPECT_TRUE(_dccexProtocol.receivedRoster());
}

TEST_F(LocoTests, parseRosterWithThreeIDs) {
  EXPECT_FALSE(_dccexProtocol.receivedRoster());
  _dccexProtocol.getLists(true, false, false, false);
  
  EXPECT_EQ(_stream.getBuffer(), "<JR>\r\n");
  _stream.clearBuffer();

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

  // Returns true since roster ist complete
  EXPECT_TRUE(_dccexProtocol.receivedRoster());
}
