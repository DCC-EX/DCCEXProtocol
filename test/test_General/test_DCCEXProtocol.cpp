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

#include "../setup/DCCEXProtocolTests.h"

/**
 * @brief Ensure the buffer is cleared when full
 */
TEST_F(DCCEXProtocolTests, clearBufferWhenFull) {
  // Fill buffer with garbage
  for (size_t i{0}; i < 500; ++i)
    _stream.write(static_cast<uint8_t>('A' + (random() % 26)));

  // Proceed with normal message
  _stream << R"(<m "Hello World">)";
  EXPECT_CALL(_delegate, receivedMessage(StrEq("Hello World"))).Times(Exactly(1));
  _dccexProtocol.check();
}

/**
 * @brief Validate all lists are requested sequentially by getLists()
 */
TEST_F(DCCEXProtocolTests, getListsSequentialFlow) {
  // Request all lists
  // We expect ONLY the roster to be requested first.
  _dccexProtocol.getLists(true, true, true, true);
  EXPECT_EQ(_stream.getBuffer(), "<J R>\r\n");
  _stream.clear();

  // Simulate the return of the roster list
  _stream << "<jR 1 3>";  // List of Locos in roster
  _dccexProtocol.check(); // Parser sets _receivedRoster = true

  _dccexProtocol.getLists(true, true, true, true);
  EXPECT_EQ(_stream.getBuffer(), "<J T>\r\n");
  _stream.clear();

  // 3. Simulate Turnouts Received
  _stream << "<jT 1 2>";
  _dccexProtocol.check();

  _dccexProtocol.getLists(true, true, true, true);
  EXPECT_EQ(_stream.getBuffer(), "<J A>\r\n"); // Request Routes
  _stream.clear();

  // 4. Simulate Routes Received
  _stream << "<jL 1 2>";
  _dccexProtocol.check();

  _dccexProtocol.getLists(true, true, true, true);
  EXPECT_EQ(_stream.getBuffer(), "<J O>\r\n"); // Request Turntables
  _stream.clear();

  // 5. Final State: All received
  _stream << "<jP 1 2>";
  _dccexProtocol.check();

  _dccexProtocol.getLists(true, true, true, true);
  // No more commands should be sent to _stream
  EXPECT_EQ(_stream.getBuffer(), "");
}
