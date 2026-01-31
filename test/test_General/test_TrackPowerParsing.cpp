/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
 *
 * Copyright © 2025 Peter Cole
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

TEST_F(DCCEXProtocolTests, allTracksOff) {
  _stream << "<p0>";
  EXPECT_CALL(_delegate, receivedTrackPower(TrackPower::PowerOff)).Times(Exactly(1));
  _dccexProtocol.check();
}

TEST_F(DCCEXProtocolTests, allTracksOn) {
  _stream << "<p1>";
  EXPECT_CALL(_delegate, receivedTrackPower(TrackPower::PowerOn)).Times(Exactly(1));
  _dccexProtocol.check();
}

TEST_F(DCCEXProtocolTests, mainTrackOn) {
  _stream << "<p1 MAIN>";
  EXPECT_CALL(_delegate, receivedTrackPower(TrackPower::PowerOn)).Times(Exactly(1));
  _dccexProtocol.check();
}

TEST_F(DCCEXProtocolTests, mainTrackOff) {
  _stream << "<p0 MAIN>";
  EXPECT_CALL(_delegate, receivedTrackPower(TrackPower::PowerOff)).Times(Exactly(1));
  _dccexProtocol.check();
}
