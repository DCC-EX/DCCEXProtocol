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
  EXPECT_EQ(_stream.getBuffer(), "<JO>\r\n");
  _stream.clearBuffer();

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
  EXPECT_EQ(_stream.getBuffer(), "<JO>\r\n");
  _stream.clearBuffer();

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
}
