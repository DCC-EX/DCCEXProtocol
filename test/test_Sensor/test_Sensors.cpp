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

#include "../setup/SensorTests.h"

TEST_F(SensorTests, createSingleSensor) {
  // Create a sensor 100
  Sensor *sensor100 = new Sensor(100, false);
  sensor100->setName("Sensor 100");

  // Validate sensor details
  EXPECT_EQ(sensor100->getId(), 100);
  EXPECT_STREQ(sensor100->getName(), "Sensor 100");
  EXPECT_FALSE(sensor100->getActive());

  // Validate it's in the list by ID
  EXPECT_EQ(_dccexProtocol.sensors->getById(100), sensor100);
}

TEST_F(SensorTests, createSensorList) {
  // Create three Sensors
  Sensor *Sensor100 = new Sensor(100, false);
  Sensor100->setName("Sensor 100");
  Sensor *Sensor101 = new Sensor(101, true);
  Sensor101->setName("Sensor 101");
  Sensor *Sensor102 = new Sensor(102, false);
  Sensor102->setName("");

  // Validate Sensors are in the list
  EXPECT_EQ(_dccexProtocol.sensors->getById(100), Sensor100);
  EXPECT_EQ(_dccexProtocol.sensors->getById(101), Sensor101);
  EXPECT_EQ(_dccexProtocol.sensors->getById(102), Sensor102);

  // Validate Sensor details
  EXPECT_EQ(Sensor100->getId(), 100);
  EXPECT_STREQ(Sensor100->getName(), "Sensor 100");
  EXPECT_FALSE(Sensor100->getActive());

  // Validate Sensor details
  EXPECT_EQ(Sensor101->getId(), 101);
  EXPECT_STREQ(Sensor101->getName(), "Sensor 101");
  EXPECT_TRUE(Sensor101->getActive());

  // Validate Sensor details
  EXPECT_EQ(Sensor102->getId(), 102);
  EXPECT_STREQ(Sensor102->getName(), "");
  EXPECT_FALSE(Sensor102->getActive());

  delete Sensor101;
  delete Sensor100;
  delete Sensor102;
}


TEST_F(SensorTests, operateSensor) {
  // Create a Sensor 100
  Sensor *Sensor100 = new Sensor(100, false);
  Sensor100->setName("Sensor 100");

  // Close it and validate
  Sensor100->setActive(false);
  EXPECT_FALSE(Sensor100->getActive());

  // Throw it and validate
  Sensor100->setActive(true);
  EXPECT_TRUE(Sensor100->getActive());

  // Close it and validate
  Sensor100->setActive(false);
  EXPECT_FALSE(Sensor100->getActive());
}

/**
 * @brief Test setting a null name is a no-op and does not crash
 */
TEST_F(SensorTests, setNameNullIsNoOp) {
  // Create a sensor with a name, then clear it with a null name
  Sensor *sensor = new Sensor(100, false);
  sensor->setName("Sensor 100");

  // Calling setName(nullptr) must not crash and should clear the name
  EXPECT_NO_FATAL_FAILURE(sensor->setName(nullptr));
  EXPECT_STREQ(sensor->getName(), "Sensor 100");
}

/**
 * @brief Test deleting a middle sensor in the list preserves the remaining list
 */
TEST_F(SensorTests, TestDeleteMiddleSensor) {
  // Create three sensors
  Sensor *sensor100 = new Sensor(100, false);
  sensor100->setName("Sensor 100");
  Sensor *sensor101 = new Sensor(101, true);
  sensor101->setName("Sensor 101");
  Sensor *sensor102 = new Sensor(102, false);
  sensor102->setName("Sensor 102");

  // Validate the initial list
  ASSERT_EQ(Sensor::getFirst(), sensor100);
  EXPECT_EQ(sensor100->getNext(), sensor101);
  EXPECT_EQ(sensor101->getNext(), sensor102);
  EXPECT_EQ(sensor102->getNext(), nullptr);

  // Delete the middle of the list
  delete sensor101;

  // The remaining list must be linked directly and intact
  ASSERT_EQ(Sensor::getFirst(), sensor100);
  EXPECT_EQ(sensor100->getNext(), sensor102);
  EXPECT_EQ(sensor102->getNext(), nullptr);
  EXPECT_EQ(sensor100->getId(), 100);
  EXPECT_EQ(sensor102->getId(), 102);

  // The deleted sensor must no longer be reachable from the list head
  Sensor *current = Sensor::getFirst();
  while (current) {
    EXPECT_NE(current, sensor101);
    current = current->getNext();
  }
}

/**
 * @brief Test deleting the last sensor in the list preserves the remaining list
 */
TEST_F(SensorTests, TestDeleteLastSensor) {
  // Create three sensors
  Sensor *sensor100 = new Sensor(100, false);
  sensor100->setName("Sensor 100");
  Sensor *sensor101 = new Sensor(101, true);
  sensor101->setName("Sensor 101");
  Sensor *sensor102 = new Sensor(102, false);
  sensor102->setName("Sensor 102");

  // Validate the initial list
  ASSERT_EQ(Sensor::getFirst(), sensor100);
  EXPECT_EQ(sensor100->getNext(), sensor101);
  EXPECT_EQ(sensor101->getNext(), sensor102);
  EXPECT_EQ(sensor102->getNext(), nullptr);

  // Delete the last in the list
  delete sensor102;

  // The remaining list must terminate correctly
  ASSERT_EQ(Sensor::getFirst(), sensor100);
  EXPECT_EQ(sensor100->getNext(), sensor101);
  EXPECT_EQ(sensor101->getNext(), nullptr);
  EXPECT_EQ(sensor100->getId(), 100);
  EXPECT_EQ(sensor101->getId(), 101);
}

/**
 * @brief Test getting a sensor by ID returns the matching sensor
 */
TEST_F(SensorTests, getSensorById) {
  // Create a sensor to find
  Sensor *sensor100 = new Sensor(100, false);
  sensor100->setName("Sensor 100");

  // Get it by ID
  EXPECT_EQ(_dccexProtocol.getSensorById(100), sensor100);
}

/**
 * @brief Test getting a sensor by an unknown ID returns nullptr
 */
TEST_F(SensorTests, getSensorByIdNotFound) {
  // No sensors exist, so it must not be found
  EXPECT_EQ(_dccexProtocol.getSensorById(100), nullptr);
}

/**
 * @brief Test requestSensorStates() sends the <Q> opcode
 */
TEST_F(SensorTests, requestSensorStatesSendsQOpcode) {
  // Requesting the current state of all sensors must send the <Q> opcode
  _dccexProtocol.requestSensorStates();
  EXPECT_EQ(_stream.getOutput(), "<Q>");
}

/**
 * @brief Test clearing the sensor list removes all sensors and resets the count
 */
TEST_F(SensorTests, clearSensorListClearsAllSensors) {
  // Populate the sensor list via inbound <Q>/<q> responses
  EXPECT_CALL(_delegate, receivedSensorList()).Times(2);
  _stream << "<Q 100>";
  _dccexProtocol.check();
  _stream << "<q 101>";
  _dccexProtocol.check();
  ASSERT_EQ(_dccexProtocol.getSensorCount(), 2);
  ASSERT_NE(Sensor::getFirst(), nullptr);

  // Clearing the list must remove every sensor and reset the count
  _dccexProtocol.clearSensorList();
  EXPECT_EQ(_dccexProtocol.getSensorCount(), 0);
  EXPECT_EQ(Sensor::getFirst(), nullptr);
  EXPECT_EQ(_dccexProtocol.getSensorById(100), nullptr);
  EXPECT_EQ(_dccexProtocol.getSensorById(101), nullptr);
}

/**
 * @brief Test Sensor::clearSensorList() deletes the whole list, and is safe on an empty list
 */
TEST_F(SensorTests, sensorClearSensorListEmptiesList) {
  // Create three sensors
  Sensor *sensor100 = new Sensor(100, false);
  sensor100->setName("Sensor 100");
  Sensor *sensor101 = new Sensor(101, true);
  sensor101->setName("Sensor 101");
  Sensor *sensor102 = new Sensor(102, false);
  sensor102->setName("Sensor 102");

  // Validate the initial list
  ASSERT_EQ(Sensor::getFirst(), sensor100);
  EXPECT_EQ(sensor100->getNext(), sensor101);
  EXPECT_EQ(sensor101->getNext(), sensor102);
  EXPECT_EQ(sensor102->getNext(), nullptr);

  // Clearing the list must delete every sensor and reset the head
  Sensor::clearSensorList();
  EXPECT_EQ(Sensor::getFirst(), nullptr);

  // Clearing an already empty list must be safe and not crash
  EXPECT_NO_FATAL_FAILURE(Sensor::clearSensorList());
  EXPECT_EQ(Sensor::getFirst(), nullptr);
}

/**
 * @brief Test refreshSensorList() clears the list, resets the flags, and re-requests on getLists()
 */
TEST_F(SensorTests, refreshSensorListResetsAndReRequests) {
  // Request and receive the sensor list
  _dccexProtocol.getLists(false, false, false, false, true);
  EXPECT_EQ(_stream.getOutput(), "<S>");
  _stream.clearOutput();

  EXPECT_CALL(_delegate, receivedSensorList()).Times(Exactly(2));
  _stream << "<Q 100>";
  _dccexProtocol.check();
  ASSERT_EQ(_dccexProtocol.getSensorCount(), 1);
  EXPECT_TRUE(_dccexProtocol.receivedSensorList());

  // Refreshing must clear the list and reset the received/requested flags
  _dccexProtocol.refreshSensorList();
  EXPECT_EQ(_dccexProtocol.getSensorCount(), 0);
  EXPECT_FALSE(_dccexProtocol.receivedSensorList());
  EXPECT_FALSE(_dccexProtocol.receivedLists());

  // A fresh getLists() must request the sensor list again
  _dccexProtocol.getLists(false, false, false, false, true);
  EXPECT_EQ(_stream.getOutput(), "<S>");
  _stream.clearOutput();

  // Receiving a new list must populate a fresh sensor list
  _stream << "<q 200>";
  _dccexProtocol.check();
  EXPECT_EQ(_dccexProtocol.getSensorCount(), 1);
  EXPECT_TRUE(_dccexProtocol.receivedSensorList());
  ASSERT_TRUE(_dccexProtocol.getSensorById(200));
  EXPECT_FALSE(_dccexProtocol.getSensorById(200)->getActive());
}

/**
 * @brief Test setting a sensor name when one already exists replaces it cleanly
 */
TEST_F(SensorTests, setNameReplacesExistingName) {
  // Create a sensor and set an initial name
  Sensor *sensor = new Sensor(100, false);
  sensor->setName("Sensor 100");

  // Replacing the name must release the old name and store the new one
  sensor->setName("Sensor Renamed");
  EXPECT_STREQ(sensor->getName(), "Sensor Renamed");

  // Deleting a sensor with a replaced name must not crash
  EXPECT_NO_FATAL_FAILURE(delete sensor);
  EXPECT_EQ(Sensor::getFirst(), nullptr);
}

/**
 * @brief Test setNext() directly relinks a sensor in the list
 */
TEST_F(SensorTests, setNextLinksSensorDirectly) {
  // Create three sensors
  Sensor *sensor100 = new Sensor(100, false);
  Sensor *sensor101 = new Sensor(101, true);
  Sensor *sensor102 = new Sensor(102, false);

  // Default construction chains them in insertion order
  EXPECT_EQ(sensor100->getNext(), sensor101);
  EXPECT_EQ(sensor101->getNext(), sensor102);
  EXPECT_EQ(sensor102->getNext(), nullptr);

  // setNext() must directly relink a sensor
  sensor101->setNext(sensor100);
  EXPECT_EQ(sensor101->getNext(), sensor100);

  // Restore the list for correct teardown
  sensor101->setNext(sensor102);
  EXPECT_EQ(sensor101->getNext(), sensor102);
}

/**
 * @brief Test deleting a sensor that is not in the list is a safe no-op
 */
TEST_F(SensorTests, removeSensorNotInListLeavesListIntact) {
  // Create three sensors
  Sensor *sensor100 = new Sensor(100, false);
  sensor100->setName("Sensor 100");
  Sensor *sensor101 = new Sensor(101, true);
  sensor101->setName("Sensor 101");
  Sensor *sensor102 = new Sensor(102, false);
  sensor102->setName("Sensor 102");

  // Validate the initial list
  ASSERT_EQ(Sensor::getFirst(), sensor100);
  EXPECT_EQ(sensor100->getNext(), sensor101);
  EXPECT_EQ(sensor101->getNext(), sensor102);

  // Orphan the middle and trailing sensors by severing the link from the head
  sensor100->setNext(nullptr);

  // Deleting a sensor that is not in the list must not crash or alter the list
  EXPECT_NO_FATAL_FAILURE(delete sensor101);
  EXPECT_EQ(Sensor::getFirst(), sensor100);
  EXPECT_EQ(sensor100->getNext(), nullptr);

  // The trailing orphan is also not in the list and must be safe to delete
  EXPECT_NO_FATAL_FAILURE(delete sensor102);
  EXPECT_EQ(Sensor::getFirst(), sensor100);
  EXPECT_EQ(sensor100->getNext(), nullptr);

  // Deleting the remaining head sensor empties the list
  delete sensor100;
  EXPECT_EQ(Sensor::getFirst(), nullptr);
}