#include "DCCEXProtocolTest.hpp"

TEST_F(DCCEXProtocolTest, broadcastHelloWorld) {
  _stream << R"(<m "Hello World">)";
  EXPECT_CALL(_delegate, receivedMessage(StrEq("Hello World"))).Times(Exactly(1));
  _dccexProtocol.check();
}
