#include "DCCEXProtocolTest.hpp"

/// @brief Create a single Loco using the legacy constructor
TEST_F(DCCEXProtocolTest, createLoco) {
  // Create an individual loco
  Loco *loco1 = new Loco(1, LocoSource::LocoSourceEntry);
  loco1->setName("Loco 1");

  // Check address is 1, name is correct, and LocoSource correct
  EXPECT_EQ(loco1->getAddress(), 1);
  EXPECT_STREQ(loco1->getName(), "Loco 1");
  EXPECT_EQ(loco1->getSource(), LocoSource::LocoSourceEntry);

  // Ensure next is nullptr as this is the only loco
  ASSERT_EQ(loco1->getNext(), nullptr);
}

/// @brief Create a roster of Locos using the legacy constructor
TEST_F(DCCEXProtocolTest, legacyRosterCreation) {
  // Roster should start empty, don't continue if it isn't
  ASSERT_EQ(_dccexProtocol.roster->getFirst(), nullptr);

  // Add three locos
  Loco *loco42 = new Loco(42, LocoSource::LocoSourceRoster);
  loco42->setName("Loco42");
  Loco *loco9 = new Loco(9, LocoSource::LocoSourceRoster);
  loco9->setName("Loco9");
  Loco *loco120 = new Loco(120, LocoSource::LocoSourceRoster);
  loco120->setName("Loco120");

  // Now verify the roster, fatal error if first is nullptr
  Loco *firstLoco = _dccexProtocol.roster->getFirst();
  ASSERT_NE(firstLoco, nullptr);

  // Check first loco details
  EXPECT_EQ(firstLoco->getAddress(), 42);
  EXPECT_STREQ(firstLoco->getName(), "Loco42");
  EXPECT_EQ(firstLoco->getSource(), LocoSource::LocoSourceRoster);

  // Verify second loco details and fail fatally if next is nullptr
  Loco *secondLoco = firstLoco->getNext();
  ASSERT_NE(secondLoco, nullptr);
  EXPECT_EQ(secondLoco->getAddress(), 9);
  EXPECT_STREQ(secondLoco->getName(), "Loco9");
  EXPECT_EQ(secondLoco->getSource(), LocoSource::LocoSourceRoster);

  // Verify third loco details and fail fatally if next is nullptr
  Loco *thirdLoco = secondLoco->getNext();
  ASSERT_NE(thirdLoco, nullptr);
  EXPECT_EQ(thirdLoco->getAddress(), 120);
  EXPECT_STREQ(thirdLoco->getName(), "Loco120");
  EXPECT_EQ(thirdLoco->getSource(), LocoSource::LocoSourceRoster);

  // Verify end of linked list and fail fatally if next is not nullptr
  ASSERT_EQ(thirdLoco->getNext(), nullptr) << "Unexpected fourth Loco at address: " << thirdLoco->getNext()->getAddress();
}
