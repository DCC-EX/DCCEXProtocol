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

#include "../setup/RouteTests.h"

TEST_F(RouteTests, createSingleRoute) {
  // Create a route 200 as a route type
  Route *route200 = new Route(200);
  route200->setName("Route 200");
  route200->setType(RouteType::RouteTypeRoute);

  // Validate details are correct
  EXPECT_EQ(route200->getId(), 200);
  EXPECT_STREQ(route200->getName(), "Route 200");
  EXPECT_EQ(route200->getType(), RouteType::RouteTypeRoute);

  // Validate it is the first in the list with no next
  EXPECT_EQ(Route::getFirst(), route200);
  EXPECT_EQ(route200->getNext(), nullptr);
}

TEST_F(RouteTests, createThreeRoutes) {
  // Create three routes, route, automation, and route with no name
  Route *route200 = new Route(200);
  route200->setName("Route 200");
  route200->setType(RouteType::RouteTypeRoute);
  Route *route300 = new Route(300);
  route300->setName("Automation 300");
  route300->setType(RouteType::RouteTypeAutomation);
  Route *route400 = new Route(400);
  route400->setName("");
  route400->setType(RouteType::RouteTypeRoute);

  // Validate routes are in the route list
  EXPECT_EQ(_dccexProtocol.routes->getById(200), route200);
  EXPECT_EQ(_dccexProtocol.routes->getById(300), route300);
  EXPECT_EQ(_dccexProtocol.routes->getById(400), route400);

  // Validate route details
  EXPECT_EQ(route200->getId(), 200);
  EXPECT_STREQ(route200->getName(), "Route 200");
  EXPECT_EQ(route200->getType(), RouteType::RouteTypeRoute);

  // Validate route details
  EXPECT_EQ(route300->getId(), 300);
  EXPECT_STREQ(route300->getName(), "Automation 300");
  EXPECT_EQ(route300->getType(), RouteType::RouteTypeAutomation);

  // Validate route details
  EXPECT_EQ(route400->getId(), 400);
  EXPECT_STREQ(route400->getName(), "");
  EXPECT_EQ(route400->getType(), RouteType::RouteTypeRoute);
}

/// @brief Validate that sending handOffLoco(int locoAddress, int automationId) sends </ locoAddress automationId>
TEST_F(RouteTests, automationHandOff) {
  // Start automation ID 100 using loco address 1234
  const char *expected = "</ START 1234 100>";

  // An automation 100 must exist
  Route *automation100 = new Route(100);
  automation100->setType(RouteType::RouteTypeAutomation);

  // Call power on
  _dccexProtocol.handOffLoco(1234, 100);

  // Ensure the buffer has what's expected
  ASSERT_EQ(_stream.getOutput(), expected);
}

/**
 * @brief Test setting a null name is a no-op and does not crash
 */
TEST_F(RouteTests, setNameNullIsNoOp) {
  // Create a route with a name, then clear it with a null name
  Route *route = new Route(200);
  route->setName("Route 200");

  // Calling setName(nullptr) must not crash and should clear the name
  EXPECT_NO_FATAL_FAILURE(route->setName(nullptr));
  EXPECT_STREQ(route->getName(), "Route 200");
}

/**
 * @brief Test deleting a middle route in the list preserves the remaining list
 */
TEST_F(RouteTests, TestDeleteMiddleRoute) {
  // Create three routes
  Route *route200 = new Route(200);
  route200->setName("Route 200");
  Route *route300 = new Route(300);
  route300->setName("Automation 300");
  Route *route400 = new Route(400);
  route400->setName("Route 400");

  // Validate the initial list
  ASSERT_EQ(Route::getFirst(), route200);
  EXPECT_EQ(route200->getNext(), route300);
  EXPECT_EQ(route300->getNext(), route400);
  EXPECT_EQ(route400->getNext(), nullptr);

  // Delete the middle of the list
  delete route300;

  // The remaining list must be linked directly and intact
  ASSERT_EQ(Route::getFirst(), route200);
  EXPECT_EQ(route200->getNext(), route400);
  EXPECT_EQ(route400->getNext(), nullptr);
  EXPECT_EQ(route200->getId(), 200);
  EXPECT_EQ(route400->getId(), 400);

  // The deleted route must no longer be reachable from the list head
  Route *current = Route::getFirst();
  while (current) {
    EXPECT_NE(current, route300);
    current = current->getNext();
  }
}

/**
 * @brief Test deleting the last route in the list preserves the remaining list
 */
TEST_F(RouteTests, TestDeleteLastRoute) {
  // Create three routes
  Route *route200 = new Route(200);
  route200->setName("Route 200");
  Route *route300 = new Route(300);
  route300->setName("Automation 300");
  Route *route400 = new Route(400);
  route400->setName("Route 400");

  // Validate the initial list
  ASSERT_EQ(Route::getFirst(), route200);
  EXPECT_EQ(route200->getNext(), route300);
  EXPECT_EQ(route300->getNext(), route400);
  EXPECT_EQ(route400->getNext(), nullptr);

  // Delete the last in the list
  delete route400;

  // The remaining list must terminate correctly
  ASSERT_EQ(Route::getFirst(), route200);
  EXPECT_EQ(route200->getNext(), route300);
  EXPECT_EQ(route300->getNext(), nullptr);
  EXPECT_EQ(route200->getId(), 200);
  EXPECT_EQ(route300->getId(), 300);
}

/// @brief Validate that handOffLoco() with no matching automation sends nothing
TEST_F(RouteTests, automationHandOffNoAutomation) {
  // No automation 999 exists
  _dccexProtocol.handOffLoco(1234, 999);

  // Nothing should be sent
  EXPECT_EQ(_stream.getOutput(), "");
}

/// @brief Validate that handOffLoco() with a non-automation route sends nothing
TEST_F(RouteTests, automationHandOffWrongType) {
  // Route 100 exists but is not an automation
  Route *route100 = new Route(100);
  route100->setType(RouteType::RouteTypeRoute);

  // Handing off to a non-automation route must not send anything
  _dccexProtocol.handOffLoco(1234, 100);
  EXPECT_EQ(_stream.getOutput(), "");
}

/**
 * @brief Test setting a new name replaces the existing name
 */
TEST_F(RouteTests, TestSetNameReplacesExistingName) {
  // Create a route with a name, then replace it
  Route *route200 = new Route(200);
  route200->setName("Route 200");
  route200->setName("Route Renamed");

  // The new name must replace the old name
  EXPECT_STREQ(route200->getName(), "Route Renamed");

  // Clean up
  delete route200;
  EXPECT_EQ(Route::getFirst(), nullptr);
}
