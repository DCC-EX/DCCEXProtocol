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
  EXPECT_EQ(_stream, "<JO>\r\n");
  _stream = {};

  // Empty turntable list response
  _stream << "<jO>";
  _dccexProtocol.check();

  // Should be true given turntable list is empty
  EXPECT_TRUE(_dccexProtocol.receivedTurntableList());
}
