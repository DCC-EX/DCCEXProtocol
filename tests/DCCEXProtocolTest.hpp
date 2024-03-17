#include "DCCEXProtocolDelegateMock.hpp"
#include <DCCEXProtocol.h>
#include <StreamMock.h>
#include <string>

using namespace testing;

// Make StreamMock comparable
bool operator==(StreamMock lhs, StreamMock rhs);
bool operator==(StreamMock lhs, std::string rhs);

// DCCEXProtocol test fixture
class DCCEXProtocolTest : public Test {
public:
  DCCEXProtocolTest();
  virtual ~DCCEXProtocolTest();

protected:
  DCCEXProtocol _dccexProtocol;
  DCCEXProtocolDelegateMock _delegate;
  StreamMock _console;
  StreamMock _stream;
};