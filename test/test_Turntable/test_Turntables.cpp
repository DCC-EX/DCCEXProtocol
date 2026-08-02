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

/**
 * @brief Stress test for turntable clean up and destructors
 */
TEST_F(TurntableTests, listCleanupStressTest) {
  for (int i = 0; i < 5; i++) {
    // Create turntable
    Turntable *tt = new Turntable(i);
    tt->setName("Temporary TT");
    tt->setNumberOfIndexes(2);

    // Add indices
    tt->addIndex(new TurntableIndex(i, 0, 0, "Home"));
    tt->addIndex(new TurntableIndex(i, 1, 1800, "Opposite"));

    // Immediately clear the global list
    // This tests if clearTurntableList() and ~Turntable() work 
    // together without double-freeing or leaving dangling pointers.
    Turntable::clearTurntableList();
    
    // Validate list is empty
    EXPECT_EQ(Turntable::getFirst(), nullptr);
  }
}

/**
 * @brief Test setting a null name is a no-op and does not crash
 */
TEST_F(TurntableTests, setNameNullIsNoOp) {
  // Create a turntable with a name, then clear it with a null name
  Turntable *turntable = new Turntable(1);
  turntable->setName("Test EX-Turntable");

  // Calling setName(nullptr) must not crash and should clear the name
  EXPECT_NO_FATAL_FAILURE(turntable->setName(nullptr));
  EXPECT_STREQ(turntable->getName(), "Test EX-Turntable");
}

/**
 * @brief Test rotating a DCC turntable sends the two parameter command without activity
 */
TEST_F(TurntableTests, TestRotateTurntableDCC) {
  // Create a DCC turntable
  Turntable *turntable = new Turntable(2);
  turntable->setType(TurntableType::TurntableTypeDCC);
  turntable->setIndex(0);
  turntable->setName("Test DCC Turntable");

  // Rotate to position 2 with an activity value, which must be ignored for DCC turntables
  _dccexProtocol.rotateTurntable(2, 2, 9);
  EXPECT_EQ(_stream.getOutput(), "<I 2 2>");
}

/**
 * @brief Test rotating an EX-Turntable sends the three parameter command with the activity
 */
TEST_F(TurntableTests, TestRotateTurntableEXTT) {
  // Create an EX-Turntable
  Turntable *turntable = new Turntable(1);
  turntable->setType(TurntableType::TurntableTypeEXTT);
  turntable->setIndex(0);
  turntable->setName("Test EX-Turntable");

  // Rotate to position 3 with activity 1, which must be sent for EX-Turntables
  _dccexProtocol.rotateTurntable(1, 3, 1);
  EXPECT_EQ(_stream.getOutput(), "<I 1 3 1>");
}

/**
 * @brief Test rotating an EX-Turntable with the default activity sends activity 0
 */
TEST_F(TurntableTests, TestRotateTurntableEXTTDefaultActivity) {
  // Create an EX-Turntable
  Turntable *turntable = new Turntable(1);
  turntable->setType(TurntableType::TurntableTypeEXTT);
  turntable->setIndex(0);
  turntable->setName("Test EX-Turntable");

  // Rotate to position 3 using the default activity of 0
  _dccexProtocol.rotateTurntable(1, 3);
  EXPECT_EQ(_stream.getOutput(), "<I 1 3 0>");
}

/**
 * @brief Test rotating an EX-Turntable to position 0 forces activity 2
 */
TEST_F(TurntableTests, TestRotateTurntableEXTTZeroPosition) {
  // Create an EX-Turntable
  Turntable *turntable = new Turntable(1);
  turntable->setType(TurntableType::TurntableTypeEXTT);
  turntable->setIndex(0);
  turntable->setName("Test EX-Turntable");

  // Rotate to position 0, the activity must be forced to 2 regardless of the passed value
  _dccexProtocol.rotateTurntable(1, 0, 1);
  EXPECT_EQ(_stream.getOutput(), "<I 1 0 2>");
}

/**
 * @brief Test rotating a turntable that doesn't exist does not crash and sends nothing
 */
TEST_F(TurntableTests, TestRotateTurntableUnknownId) {
  // Create an EX-Turntable with a different id
  Turntable *turntable = new Turntable(1);
  turntable->setType(TurntableType::TurntableTypeEXTT);
  turntable->setIndex(0);
  turntable->setName("Test EX-Turntable");

  // Rotating an unknown turntable must not crash and must not send a command
  EXPECT_NO_FATAL_FAILURE(_dccexProtocol.rotateTurntable(99, 2, 1));
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test rotating a turntable when the turntable list is empty does not crash and sends nothing
 */
TEST_F(TurntableTests, TestRotateTurntableEmptyList) {
  // No turntables have been created, rotating any turntable must not crash or send a command
  EXPECT_NO_FATAL_FAILURE(_dccexProtocol.rotateTurntable(1, 2, 1));
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test getting a turntable by id returns nullptr when the id isn't in the list
 */
TEST_F(TurntableTests, TestGetTurntableByIdNotFound) {
  // Create an EX-Turntable with id 1
  Turntable *turntable = new Turntable(1);
  turntable->setType(TurntableType::TurntableTypeEXTT);
  turntable->setIndex(0);
  turntable->setName("Test EX-Turntable");

  // Querying an unknown id must return nullptr
  EXPECT_EQ(_dccexProtocol.getTurntableById(99), nullptr);

  // Querying the existing id must return the turntable
  EXPECT_EQ(_dccexProtocol.getTurntableById(1), turntable);
}

/**
 * @brief Test getting a turntable by id returns nullptr when the turntable list is empty
 */
TEST_F(TurntableTests, TestGetTurntableByIdEmptyList) {
  // No turntables have been created, querying any id must return nullptr
  EXPECT_EQ(_dccexProtocol.getTurntableById(1), nullptr);
}

/**
 * @brief Test a single rotateTurntable command produces exactly one debug console line
 */
TEST_F(TurntableTests, TestRotateTurntableDebugSingleLine) {
  // Create an EX-Turntable and a DCC turntable
  Turntable *exTT = new Turntable(1);
  exTT->setType(TurntableType::TurntableTypeEXTT);
  exTT->setIndex(0);
  exTT->setName("Test EX-Turntable");
  Turntable *dccTT = new Turntable(2);
  dccTT->setType(TurntableType::TurntableTypeDCC);
  dccTT->setIndex(0);
  dccTT->setName("Test DCC Turntable");

  // Turn debug on and rotate the EX-Turntable
  _dccexProtocol.setDebug(true);
  _dccexProtocol.rotateTurntable(1, 3, 1);

  // A single command must produce exactly one debug line; the redundant trailing send must not add an empty line
  EXPECT_EQ(_stream.getOutput(), "<I 1 3 1>");
  EXPECT_EQ(_console.getOutput(), "==> <I 1 3 1>\r\n");

  // Rotate the DCC turntable and validate the same
  _stream.clearOutput();
  _console.clearOutput();
  _dccexProtocol.rotateTurntable(2, 2, 9);
  EXPECT_EQ(_stream.getOutput(), "<I 2 2>");
  EXPECT_EQ(_console.getOutput(), "==> <I 2 2>\r\n");

  // Reset debug for subsequent tests
  _dccexProtocol.setDebug(false);
}

/**
 * @brief Test rotating an unknown turntable with debug on produces no console output
 */
TEST_F(TurntableTests, TestRotateTurntableUnknownIdNoDebugOutput) {
  // Create an EX-Turntable with a different id
  Turntable *turntable = new Turntable(1);
  turntable->setType(TurntableType::TurntableTypeEXTT);
  turntable->setIndex(0);
  turntable->setName("Test EX-Turntable");

  // With debug on, rotating an unknown turntable must not produce a command or a debug line
  _dccexProtocol.setDebug(true);
  _dccexProtocol.rotateTurntable(99, 2, 1);
  EXPECT_EQ(_stream.getOutput(), "");
  EXPECT_EQ(_console.getOutput(), "");

  // Reset debug for subsequent tests
  _dccexProtocol.setDebug(false);
}

/**
 * @brief Test deleting a middle turntable in the list preserves the remaining list
 */
TEST_F(TurntableTests, TestDeleteMiddleTurntable) {
  // Create three turntables
  Turntable *turntable1 = new Turntable(1);
  turntable1->setType(TurntableType::TurntableTypeEXTT);
  turntable1->setName("Turntable 1");
  Turntable *turntable2 = new Turntable(2);
  turntable2->setType(TurntableType::TurntableTypeDCC);
  turntable2->setName("Turntable 2");
  Turntable *turntable3 = new Turntable(3);
  turntable3->setType(TurntableType::TurntableTypeEXTT);
  turntable3->setName("Turntable 3");

  // Validate the initial list
  ASSERT_EQ(Turntable::getFirst(), turntable1);
  EXPECT_EQ(turntable1->getNext(), turntable2);
  EXPECT_EQ(turntable2->getNext(), turntable3);
  EXPECT_EQ(turntable3->getNext(), nullptr);

  // Delete the middle of the list
  delete turntable2;

  // The remaining list must be linked directly and intact
  ASSERT_EQ(Turntable::getFirst(), turntable1);
  EXPECT_EQ(turntable1->getNext(), turntable3);
  EXPECT_EQ(turntable3->getNext(), nullptr);
  EXPECT_EQ(turntable1->getId(), 1);
  EXPECT_EQ(turntable3->getId(), 3);

  // The deleted turntable must no longer be reachable from the list head
  Turntable *current = Turntable::getFirst();
  while (current) {
    EXPECT_NE(current, turntable2);
    current = current->getNext();
  }
}

/**
 * @brief Test deleting the last turntable in the list preserves the remaining list
 */
TEST_F(TurntableTests, TestDeleteLastTurntable) {
  // Create three turntables
  Turntable *turntable1 = new Turntable(1);
  turntable1->setType(TurntableType::TurntableTypeEXTT);
  turntable1->setName("Turntable 1");
  Turntable *turntable2 = new Turntable(2);
  turntable2->setType(TurntableType::TurntableTypeDCC);
  turntable2->setName("Turntable 2");
  Turntable *turntable3 = new Turntable(3);
  turntable3->setType(TurntableType::TurntableTypeEXTT);
  turntable3->setName("Turntable 3");

  // Validate the initial list
  ASSERT_EQ(Turntable::getFirst(), turntable1);
  EXPECT_EQ(turntable1->getNext(), turntable2);
  EXPECT_EQ(turntable2->getNext(), turntable3);
  EXPECT_EQ(turntable3->getNext(), nullptr);

  // Delete the last in the list
  delete turntable3;

  // The remaining list must terminate correctly
  ASSERT_EQ(Turntable::getFirst(), turntable1);
  EXPECT_EQ(turntable1->getNext(), turntable2);
  EXPECT_EQ(turntable2->getNext(), nullptr);
  EXPECT_EQ(turntable1->getId(), 1);
  EXPECT_EQ(turntable2->getId(), 2);
}
