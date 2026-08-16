#include "../setup/DCCEXProtocolTests.h"

#include "DCCEXUtils.h"
#include "DCCEXInbound.h"

#include <limits.h>

TEST_F(DCCEXProtocolTests, NativeStreamKeepsInputAndOutputIndependent) {
  Stream stream;
  stream << "<s>";

  EXPECT_EQ(stream.available(), 3);
  EXPECT_EQ(stream.read(), '<');
  EXPECT_EQ(stream.read(), 's');
  EXPECT_EQ(stream.read(), '>');
  EXPECT_EQ(stream.read(), -1);

  stream.print("reply");
  EXPECT_EQ(stream.getOutput(), "reply");
}

TEST_F(DCCEXProtocolTests, FastItoaHandlesSignedIntegerBoundaries) {
  char buffer[sizeof(int) * 3 + 2];

  EXPECT_STREQ(fastitoa(0, &buffer[sizeof(buffer) - 1]), "0");
  EXPECT_STREQ(fastitoa(-1, &buffer[sizeof(buffer) - 1]), "-1");
  EXPECT_STREQ(fastitoa(INT_MIN, &buffer[sizeof(buffer) - 1]), "-2147483648");
  EXPECT_STREQ(fastitoa(INT_MAX, &buffer[sizeof(buffer) - 1]), "2147483647");
}

TEST_F(DCCEXProtocolTests, NativeInboundDumpUsesPortablePrint) {
  char command[] = "<1 42>";
  ASSERT_TRUE(DCCEXInbound::parse(command));

  Stream output;
  DCCEXInbound::dump(&output);

  EXPECT_EQ(output.getOutput(), "\nDCCEXInbound Opcode='1'\r\ngetNumber(0)=42\r\n");
}
