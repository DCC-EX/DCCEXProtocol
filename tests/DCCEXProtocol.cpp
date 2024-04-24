#include "DCCEXProtocolTest.hpp"

TEST_F(DCCEXProtocolTest, clearBufferWhenFull) {
  // Fill buffer with garbage
  for (auto i{0uz}; i < 500uz; ++i)
    _stream.write(static_cast<uint8_t>('A' + (random() % 26)));

  // Proceed with normal message
  _stream << R"(<m "Hello World">)";
  EXPECT_CALL(_delegate, receivedMessage(StrEq("Hello World"))).Times(Exactly(1));
  _dccexProtocol.check();
}
