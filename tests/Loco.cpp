#include "DCCEXProtocolTest.hpp"

/// @brief Create a single Loco that is not part of a roster
TEST_F(DCCEXProtocolTest, createLoco) {
  // Create an individual loco
  Loco *loco100 = new Loco(100, false);
  loco100->setName("Loco 100");
  // loco100->setupFunctions("Lights/*Horn/Bell///Idiot on tracks");

  // Check address is 1, name is correct, LocoSource correct, and no next
  EXPECT_EQ(loco100->getAddress(), 100);
  EXPECT_STREQ(loco100->getName(), "Loco 100");
  EXPECT_EQ(loco100->getSource(), LocoSource::LocoSourceEntry);
  EXPECT_EQ(loco100->getNext(), nullptr);

  // Check our functions
  // EXPECT_FALSE(loco100->isFunctionMomentary(0));
  // EXPECT_TRUE(loco100->isFunctionMomentary(1));
  // EXPECT_STREQ(loco100->getFunctionName(2), "Bell");
  // EXPECT_STREQ(loco100->getFunctionName(5), "Idiot on tracks");

  // Clean this Loco up
  delete loco100;
  loco100 = nullptr;

  // Make sure this is now a nullptr
  ASSERT_EQ(loco100, nullptr);
}
