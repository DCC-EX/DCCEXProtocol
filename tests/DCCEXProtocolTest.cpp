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
DCCEXProtocolTest::DCCEXProtocolTest() {}

DCCEXProtocolTest::~DCCEXProtocolTest() {}

void DCCEXProtocolTest::SetUp() {
  _dccexProtocol.setDelegate(&_delegate);
  _dccexProtocol.setLogStream(&_console);
  _dccexProtocol.connect(&_stream);
}

void DCCEXProtocolTest::TearDown() {
  // Clean up the roster
  Loco *currentLoco = _dccexProtocol.roster->getFirst();
  while (currentLoco != nullptr) {
    Loco *nextLoco = currentLoco->getNext();
    delete currentLoco;
    currentLoco = nextLoco;
  }
  _dccexProtocol.roster = nullptr;
}
