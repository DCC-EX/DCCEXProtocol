#include "DCCEXProtocolTest.hpp"

TEST_F(DCCEXProtocolTest, getEmptyRoster) {
  EXPECT_FALSE(_dccexProtocol.receivedRoster());
  _dccexProtocol.getLists(true, false, false, false);
  EXPECT_EQ(_stream, "<JR>\r\n");
  _stream = {};

  // Response
  _stream << "<jR>";
  _dccexProtocol.check();

  // Returns true since roster is empty
  EXPECT_TRUE(_dccexProtocol.receivedRoster());
}

TEST_F(DCCEXProtocolTest, getRosterWithThreeIDs) {
  EXPECT_FALSE(_dccexProtocol.receivedRoster());
  _dccexProtocol.getLists(true, false, false, false);
  EXPECT_EQ(_stream, "<JR>\r\n");
  _stream = {};

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