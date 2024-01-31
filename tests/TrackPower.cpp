#include "DCCEXProtocolTest.hpp"

TEST_F(DCCEXProtocolTest, allTracksOff) {
  _stream << "<p0>";
  EXPECT_CALL(_delegate, receivedTrackPower(TrackPower::PowerOff))
      .Times(Exactly(1));
  _dccexProtocol.check();
}

TEST_F(DCCEXProtocolTest, allTracksOn) {
  _stream << "<p1>";
  EXPECT_CALL(_delegate, receivedTrackPower(TrackPower::PowerOn))
      .Times(Exactly(1));
  _dccexProtocol.check();
}
