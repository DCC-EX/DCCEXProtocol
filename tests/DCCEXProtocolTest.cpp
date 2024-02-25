#include "DCCEXProtocolTest.hpp"

// Set delegate and stream
DCCEXProtocolTest::DCCEXProtocolTest() {
  _dccexProtocol.setDelegate(&_delegate);
  _dccexProtocol.connect(&_stream);
}

DCCEXProtocolTest::~DCCEXProtocolTest() {}