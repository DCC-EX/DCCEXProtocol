/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
 *
 * Copyright © 2024 Peter Cole
 *
 * This work is licensed under the Creative Commons Attribution-ShareAlike
 * 4.0 International License. To view a copy of this license, visit
 * http://creativecommons.org/licenses/by-sa/4.0/ or send a letter to
 * Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
 *
 * Attribution — You must give appropriate credit, provide a link to the
 * license, and indicate if changes were made. You may do so in any
 * reasonable manner, but not in any way that suggests the licensor
 * endorses you or your use.
 *
 * ShareAlike — If you remix, transform, or build upon the material, you
 * must distribute your contributions under the same license as the
 * original.
 *
 * All other rights reserved.
 *
 */

#include "../setup/TurntableTests.h"

/// @brief Test creating a single turntable index
TEST_F(TurntableTests, createTurntableIndex) {
  TurntableIndex *index = new TurntableIndex(1, 0, 900, "Home");

  // Fatal error if it wasn't created
  ASSERT_NE(index, nullptr);

  // Validate details
  EXPECT_EQ(index->getTTId(), 1);
  EXPECT_EQ(index->getId(), 0);
  EXPECT_EQ(index->getAngle(), 900);
  EXPECT_STREQ(index->getName(), "Home");
  EXPECT_EQ(index->getNextIndex(), nullptr);

  // Clean up
  delete index;
}

/// @brief Test creating a complete EX-turntable
TEST_F(TurntableTests, createEXTurntable) {
  // Create an EX-Turntable with:
  // - ID 1
  // - Currently at the home position
  // - Has 5 positions including home
  // - Name "Test EX-Turntable"
  Turntable *turntable1 = new Turntable(1);
  // Fatal fail if the turntable is not created
  ASSERT_NE(turntable1, nullptr);

  // Set and check details
  turntable1->setType(TurntableType::TurntableTypeEXTT);
  turntable1->setIndex(0);
  turntable1->setNumberOfIndexes(5);
  turntable1->setName("Test EX-Turntable");
  EXPECT_EQ(turntable1->getType(), TurntableType::TurntableTypeEXTT);
  EXPECT_EQ(turntable1->getIndex(), 0);
  EXPECT_EQ(turntable1->getNumberOfIndexes(), 5);
  EXPECT_STREQ(turntable1->getName(), "Test EX-Turntable");
  EXPECT_EQ(turntable1->getNext(), nullptr);

  // Create 5 positions and add to list
  TurntableIndex *index0 = new TurntableIndex(1, 0, 900, "Home");
  turntable1->addIndex(index0);
  TurntableIndex *index1 = new TurntableIndex(1, 1, 450, "EX-Turntable Index 1");
  turntable1->addIndex(index1);
  TurntableIndex *index2 = new TurntableIndex(1, 2, 1800, "EX-Turntable Index 2");
  turntable1->addIndex(index2);
  TurntableIndex *index3 = new TurntableIndex(1, 3, 2700, "EX-Turntable Index 3");
  turntable1->addIndex(index3);
  TurntableIndex *index4 = new TurntableIndex(1, 4, 3000, "EX-Turntable Index 4");
  turntable1->addIndex(index4);

  // Validate we have added all 5 positions
  EXPECT_EQ(turntable1->getIndexCount(), 5);

  // Validate the first index is available and correct
  EXPECT_EQ(turntable1->getFirstIndex(), index0);

  // Validate various attributes
  EXPECT_EQ(index0->getAngle(), 900);
  EXPECT_EQ(index1->getId(), 1);
  EXPECT_EQ(index2->getTTId(), 1);
  EXPECT_STREQ(index3->getName(), "EX-Turntable Index 3");
  EXPECT_EQ(index4->getNextIndex(), nullptr);
}

/// @brief Test creating a complete DCC turntable
TEST_F(TurntableTests, createDCCTurntable) {
  // Create a DCC Turntable with:
  // - ID 2
  // - Currently at position 3
  // - Has 6 positions including home
  // - Name "Test DCC Turntable"
  Turntable *turntable2 = new Turntable(2);
  // Fatal fail if the turntable is not created
  ASSERT_NE(turntable2, nullptr);

  // Set and check details
  turntable2->setType(TurntableType::TurntableTypeDCC);
  turntable2->setIndex(3);
  turntable2->setNumberOfIndexes(6);
  turntable2->setName("Test DCC Turntable");
  EXPECT_EQ(turntable2->getType(), TurntableType::TurntableTypeDCC);
  EXPECT_EQ(turntable2->getIndex(), 3);
  EXPECT_EQ(turntable2->getNumberOfIndexes(), 6);
  EXPECT_STREQ(turntable2->getName(), "Test DCC Turntable");
  EXPECT_EQ(turntable2->getNext(), nullptr);

  // Create 5 positions and add to list
  TurntableIndex *index0 = new TurntableIndex(2, 0, 0, "Home");
  turntable2->addIndex(index0);
  TurntableIndex *index1 = new TurntableIndex(2, 1, 450, "DCC Turntable Index 1");
  turntable2->addIndex(index1);
  TurntableIndex *index2 = new TurntableIndex(2, 2, 1800, "DCC Turntable Index 2");
  turntable2->addIndex(index2);
  TurntableIndex *index3 = new TurntableIndex(2, 3, 2700, "DCC Turntable Index 3");
  turntable2->addIndex(index3);
  TurntableIndex *index4 = new TurntableIndex(2, 4, 3000, "DCC Turntable Index 4");
  turntable2->addIndex(index4);
  TurntableIndex *index5 = new TurntableIndex(2, 4, 3300, "DCC Turntable Index 5");
  turntable2->addIndex(index5);

  // Validate we have added all 5 positions
  EXPECT_EQ(turntable2->getIndexCount(), 6);

  // Validate the first index is available and correct
  EXPECT_EQ(turntable2->getFirstIndex(), index0);

  // Validate various attributes
  EXPECT_EQ(index0->getAngle(), 0);
  EXPECT_EQ(index1->getId(), 1);
  EXPECT_EQ(index2->getTTId(), 2);
  EXPECT_STREQ(index3->getName(), "DCC Turntable Index 3");
  EXPECT_STREQ(index4->getName(), "DCC Turntable Index 4");
  EXPECT_EQ(index5->getNextIndex(), nullptr);
}

/// @brief Test creating multiple turntables
TEST_F(TurntableTests, createTurntableList) {
  // Create three turntables, ignore indexes for this
  Turntable *turntable1 = new Turntable(1);
  turntable1->setType(TurntableType::TurntableTypeEXTT);
  turntable1->setIndex(0);
  turntable1->setName("Test EX-Turntable");
  Turntable *turntable2 = new Turntable(2);
  turntable2->setType(TurntableType::TurntableTypeDCC);
  turntable2->setIndex(3);
  turntable2->setName("Test DCC Turntable");
  Turntable *turntable3 = new Turntable(3);
  turntable3->setType(TurntableType::TurntableTypeEXTT);
  turntable3->setIndex(0);
  turntable3->setName("Test EX-Turntable");

  // Validate all three created and the list makeup
  ASSERT_NE(turntable1, nullptr);
  ASSERT_NE(turntable2, nullptr);
  ASSERT_NE(turntable3, nullptr);
  EXPECT_EQ(Turntable::getFirst(), turntable1);
  EXPECT_EQ(turntable1->getNext(), turntable2);
  EXPECT_EQ(turntable3->getNext(), nullptr);
}

/// @brief Test operating an EX-Turntable
TEST_F(TurntableTests, operateTurntable) {
  // Create an EX-Turntable
  Turntable *turntable1 = new Turntable(1);

  // Set details
  turntable1->setType(TurntableType::TurntableTypeEXTT);
  turntable1->setIndex(0);
  turntable1->setNumberOfIndexes(5);
  turntable1->setName("Test EX-Turntable");

  // Create 5 positions and add to list
  TurntableIndex *index0 = new TurntableIndex(1, 0, 900, "Home");
  turntable1->addIndex(index0);
  TurntableIndex *index1 = new TurntableIndex(1, 1, 450, "EX-Turntable Index 1");
  turntable1->addIndex(index1);
  TurntableIndex *index2 = new TurntableIndex(1, 2, 1800, "EX-Turntable Index 2");
  turntable1->addIndex(index2);
  TurntableIndex *index3 = new TurntableIndex(1, 3, 2700, "EX-Turntable Index 3");
  turntable1->addIndex(index3);
  TurntableIndex *index4 = new TurntableIndex(1, 4, 3000, "EX-Turntable Index 4");
  turntable1->addIndex(index4);

  // Validate current position and state
  EXPECT_EQ(turntable1->getIndex(), 0);
  EXPECT_FALSE(turntable1->isMoving());
  EXPECT_STREQ(turntable1->getIndexById(turntable1->getIndex())->getName(), "Home");

  // Set moving to position 3 and validate
  turntable1->setIndex(3);
  turntable1->setMoving(true);
  EXPECT_EQ(turntable1->getIndex(), 3);
  EXPECT_TRUE(turntable1->isMoving());
  EXPECT_STREQ(turntable1->getIndexById(turntable1->getIndex())->getName(), "EX-Turntable Index 3");

  // Move finished
  turntable1->setMoving(false);
  EXPECT_EQ(turntable1->getIndex(), 3);
  EXPECT_FALSE(turntable1->isMoving());
  EXPECT_STREQ(turntable1->getIndexById(turntable1->getIndex())->getName(), "EX-Turntable Index 3");
}