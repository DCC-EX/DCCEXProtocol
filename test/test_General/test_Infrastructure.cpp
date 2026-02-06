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
 * @brief Test sending a generic command with sendCommand
 */
TEST_F(DCCEXProtocolTests, testGenericSendCommand) {
  _dccexProtocol.sendCommand("Random command");
  EXPECT_EQ(_stream.getOutput(), "<Random command>\r\n");
  _stream.clearOutput();
}
