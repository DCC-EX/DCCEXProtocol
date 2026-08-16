#include "../setup/DCCEXProtocolTests.h"

namespace {
static_assert(PowerOff == 0 && PowerOn == 1 && PowerUnknown == 2);
static_assert(MAIN == 0 && PROG == 1 && DC == 2 && DCX == 3 && NONE == 4);
}

TEST_F(DCCEXProtocolTests, mixedInboundFramesRemainIndependentlyDispatchable) {
  Turnout turnout(7, false);

  EXPECT_CALL(_delegate, receivedTrackPower(PowerOn));
  EXPECT_CALL(_delegate, receivedTurnoutAction(7, true));
  EXPECT_CALL(_delegate, receivedJMRISensorBroadcast(9, Activated));

  _stream << "<p1><H 7 1><Q 9>";
  _dccexProtocol.check();
}

TEST_F(DCCEXProtocolTests, outboundFramesStayDistinctAcrossFeatureFamilies) {
  _dccexProtocol.powerOn();
  _dccexProtocol.throwTurnout(7);
  _dccexProtocol.readCV(29);

  EXPECT_EQ(_stream.getOutput(), "<1><T 7 1><R 29>");
}
