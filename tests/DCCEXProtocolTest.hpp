#include "DCCEXProtocolDelegateMock.hpp"
#include <DCCEXProtocol.h>
#include <StreamMock.h>

using namespace testing;

// DCCEXProtocol test fixture
class DCCEXProtocolTest : public Test {
public:
  DCCEXProtocolTest();
  virtual ~DCCEXProtocolTest();

protected:
  DCCEXProtocol _dccexProtocol;
  DCCEXProtocolDelegateMock _delegate;
  StreamMock _stream;
};