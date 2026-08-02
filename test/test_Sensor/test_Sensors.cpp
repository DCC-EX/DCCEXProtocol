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