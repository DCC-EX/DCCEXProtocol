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