#include "DCCEXProtocolTest.hpp"

/// @brief Create a consist with three Loco objects
TEST_F(DCCEXProtocolTest, createLegacyConsistByLoco) {
  // Create three locos for the consist
  char *functionList = "Lights/*Horn";
  Loco *loco10 = new Loco(10, LocoSourceRoster);
  loco10->setName("Loco 10");
  loco10->setupFunctions(functionList);
  Loco *loco2 = new Loco(2, LocoSourceRoster);
  loco2->setName("Loco 2");
  loco2->setupFunctions(functionList);
  Loco *loco10000 = new Loco(10000, LocoSourceRoster);
  loco10000->setName("Loco 10000");
  loco10000->setupFunctions(functionList);

  // Add locos to the consist, with 2 reversed
  Consist *legacyConsist = new Consist();
  legacyConsist->setName("Test Legacy Consist");
  legacyConsist->addLoco(loco10, Facing::FacingForward);
  legacyConsist->addLoco(loco2, Facing::FacingReversed);
  legacyConsist->addLoco(loco10000, Facing::FacingForward);

  // Validate consist makeup by object and address
  EXPECT_STREQ(legacyConsist->getName(), "Test Legacy Consist");
  EXPECT_EQ(legacyConsist->getLocoCount(), 3);
  EXPECT_TRUE(legacyConsist->inConsist(loco10));
  EXPECT_TRUE(legacyConsist->inConsist(loco2));
  EXPECT_TRUE(legacyConsist->inConsist(loco10000));
  EXPECT_TRUE(legacyConsist->inConsist(10));
  EXPECT_TRUE(legacyConsist->inConsist(2));
  EXPECT_TRUE(legacyConsist->inConsist(10000));

  // Validate the first loco is 10
  EXPECT_EQ(legacyConsist->getFirst()->getLoco(), loco10);

  // Validate the consist speed and direction comes from the first loco
  EXPECT_EQ(legacyConsist->getSpeed(), 0);
  EXPECT_EQ(legacyConsist->getDirection(), Direction::Forward);
  loco2->setSpeed(35);
  loco10000->setDirection(Direction::Reverse);
  EXPECT_EQ(legacyConsist->getSpeed(), 0);
  EXPECT_EQ(legacyConsist->getDirection(), Direction::Forward);
  loco10->setSpeed(21);
  loco10->setDirection(Direction::Reverse);
  EXPECT_EQ(legacyConsist->getSpeed(), 21);
  EXPECT_EQ(legacyConsist->getDirection(), Direction::Reverse);

  // Validate removal of middle loco is as expected
  legacyConsist->removeLoco(loco2);
  EXPECT_EQ(legacyConsist->getLocoCount(), 2);
  EXPECT_EQ(legacyConsist->getFirst()->getLoco(), loco10);
  EXPECT_EQ(legacyConsist->getSpeed(), 21);
  EXPECT_EQ(legacyConsist->getDirection(), Direction::Reverse);

  // Validate removal of first loco is as expected
  legacyConsist->removeLoco(loco10);
  EXPECT_EQ(legacyConsist->getLocoCount(), 1);
  EXPECT_EQ(legacyConsist->getFirst()->getLoco(), loco10000);
  EXPECT_EQ(legacyConsist->getSpeed(), 0);
  EXPECT_EQ(legacyConsist->getDirection(), Direction::Reverse);

  // Validate removal of all locos
  legacyConsist->removeAllLocos();
  EXPECT_EQ(legacyConsist->getLocoCount(), 0);
  EXPECT_EQ(legacyConsist->getFirst(), nullptr);
  EXPECT_EQ(legacyConsist->getSpeed(), 0);
  EXPECT_EQ(legacyConsist->getDirection(), Direction::Forward);

  delete loco10;
  delete loco2;
  delete loco10000;
}

/// @brief Create a consist with three Locos by address
TEST_F(DCCEXProtocolTest, createLegacyConsistByAddress) {
  // Add locos to the consist, with 2 reversed
  Consist *legacyConsist = new Consist();
  legacyConsist->addLoco(10, Facing::FacingForward);
  legacyConsist->addLoco(2, Facing::FacingReversed);
  legacyConsist->addLoco(10000, Facing::FacingForward);

  // Validate consist makeup by object and address
  EXPECT_STREQ(legacyConsist->getName(), "10"); // name should be address of first loco
  EXPECT_EQ(legacyConsist->getLocoCount(), 3);
  EXPECT_TRUE(legacyConsist->inConsist(10));
  EXPECT_TRUE(legacyConsist->inConsist(2));
  EXPECT_TRUE(legacyConsist->inConsist(10000));

  // Get loco objects for the next tests
  Loco *loco10 = Loco::getByAddress(10);
  Loco *loco2 = Loco::getByAddress(2);
  Loco *loco10000 = Loco::getByAddress(10000);

  // Validate the first loco address is 10
  EXPECT_EQ(legacyConsist->getFirst()->getLoco()->getAddress(), 10);

  // Validate the consist speed and direction comes from the first loco
  EXPECT_EQ(legacyConsist->getSpeed(), 0);
  EXPECT_EQ(legacyConsist->getDirection(), Direction::Forward);
  loco2->setSpeed(35);
  loco10000->setDirection(Direction::Reverse);
  EXPECT_EQ(legacyConsist->getSpeed(), 0);
  EXPECT_EQ(legacyConsist->getDirection(), Direction::Forward);
  loco10->setSpeed(21);
  loco10->setDirection(Direction::Reverse);
  EXPECT_EQ(legacyConsist->getSpeed(), 21);
  EXPECT_EQ(legacyConsist->getDirection(), Direction::Reverse);

  // Validate removal of middle loco is as expected
  legacyConsist->removeLoco(loco2);
  EXPECT_EQ(legacyConsist->getLocoCount(), 2);
  EXPECT_EQ(legacyConsist->getFirst()->getLoco(), loco10);
  EXPECT_EQ(legacyConsist->getSpeed(), 21);
  EXPECT_EQ(legacyConsist->getDirection(), Direction::Reverse);

  // Validate removal of first loco is as expected
  legacyConsist->removeLoco(loco10);
  EXPECT_EQ(legacyConsist->getLocoCount(), 1);
  EXPECT_EQ(legacyConsist->getFirst()->getLoco(), loco10000);
  EXPECT_EQ(legacyConsist->getSpeed(), 0);
  EXPECT_EQ(legacyConsist->getDirection(), Direction::Reverse);

  // Validate removal of all locos
  legacyConsist->removeAllLocos();
  EXPECT_EQ(legacyConsist->getLocoCount(), 0);
  EXPECT_EQ(legacyConsist->getFirst(), nullptr);
  EXPECT_EQ(legacyConsist->getSpeed(), 0);
  EXPECT_EQ(legacyConsist->getDirection(), Direction::Forward);

  delete loco10;
  delete loco2;
  delete loco10000;
}