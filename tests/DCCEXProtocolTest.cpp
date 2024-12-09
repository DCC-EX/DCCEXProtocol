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
  // Count Locos in roster
  int locoCount = 0;
  Loco *currentLoco = _dccexProtocol.roster->getFirst();
  while (currentLoco != nullptr) {
    locoCount++;
    currentLoco = currentLoco->getNext();
  }

  // Store Loco pointers in an array for clean up
  Loco **deleteLocos = new Loco *[locoCount];
  currentLoco = _dccexProtocol.roster->getFirst();
  for (int i = 0; i < locoCount; i++) {
    deleteLocos[i] = currentLoco;
    currentLoco = currentLoco->getNext();
  }

  // Delete each Loco
  for (int i = 0; i < locoCount; i++) {
    delete deleteLocos[i];
  }

  // Clean up the array of pointers
  delete[] deleteLocos;

  // Reset static pointer
  Loco::setFirst(nullptr);

  // Reset roster to nullptr
  _dccexProtocol.roster = nullptr;
}
