#include "DCCEXProtocolTest.hpp"

namespace {

std::string stream2string(StreamMock stream) {
  std::string retval;
  while (stream.available())
    retval.push_back(static_cast<char>(stream.read()));
  return retval;
}

} // namespace

bool operator==(StreamMock lhs, StreamMock rhs) { return stream2string(lhs) == stream2string(rhs); }

bool operator==(StreamMock lhs, std::string rhs) { return stream2string(lhs) == rhs; }

// Set delegate and stream
DCCEXProtocolTest::DCCEXProtocolTest() {
  _dccexProtocol.setDelegate(&_delegate);
  _dccexProtocol.setLogStream(&_console);
  _dccexProtocol.connect(&_stream);
}

DCCEXProtocolTest::~DCCEXProtocolTest() {}