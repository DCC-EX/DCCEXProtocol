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

/// @brief Validate a newly created Route reports RouteTypeRoute before setType() is called
TEST_F(RouteTests, newRouteDefaultsToRouteType) {
  // Create a route without setting its type
  Route *route = new Route(500);

  // Validate it defaults to a route (not an automation)
  EXPECT_EQ(route->getType(), RouteType::RouteTypeRoute);
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

/**
 * @brief Test deleting the first route in the list moves the head and preserves the remaining list
 */
TEST_F(RouteTests, TestDeleteFirstRoute) {
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

  // Delete the first in the list
  delete route200;

  // The list head must move and the remaining list must stay intact
  ASSERT_EQ(Route::getFirst(), route300);
  EXPECT_EQ(route300->getNext(), route400);
  EXPECT_EQ(route400->getNext(), nullptr);
  EXPECT_EQ(route300->getId(), 300);
  EXPECT_EQ(route400->getId(), 400);

  // The deleted route must no longer be reachable from the list head
  Route *current = Route::getFirst();
  while (current) {
    EXPECT_NE(current, route200);
    current = current->getNext();
  }
}

/**
 * @brief Test Route::clearRouteList() deletes the whole list, and is safe on an empty list
 */
TEST_F(RouteTests, routeClearRouteListEmptiesList) {
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

  // Clearing the list must delete every route and reset the head
  Route::clearRouteList();
  EXPECT_EQ(Route::getFirst(), nullptr);

  // Clearing an already empty list must be safe and not crash
  EXPECT_NO_FATAL_FAILURE(Route::clearRouteList());
  EXPECT_EQ(Route::getFirst(), nullptr);
}

/**
 * @brief Test clearing the route list removes all routes and resets the count
 */
TEST_F(RouteTests, clearRouteListClearsAllRoutes) {
  // Populate the route list via inbound <jA> responses
  _dccexProtocol.getLists(false, false, true, false);
  _stream.clearOutput();
  _stream << "<jA 21 121 221>";
  _dccexProtocol.check();
  _stream << R"(<jA 21 R "Route 21">)";
  _stream << R"(<jA 121 A "Automation 121">)";
  _stream << R"(<jA 221 R "Route 221">)";
  EXPECT_CALL(_delegate, receivedRouteList()).Times(Exactly(1));
  _dccexProtocol.check();
  ASSERT_EQ(_dccexProtocol.getRouteCount(), 3);
  ASSERT_NE(Route::getFirst(), nullptr);

  // Clearing the list must remove every route and reset the count
  _dccexProtocol.clearRouteList();
  EXPECT_EQ(_dccexProtocol.getRouteCount(), 0);
  EXPECT_EQ(Route::getFirst(), nullptr);
  EXPECT_EQ(Route::getById(21), nullptr);
  EXPECT_EQ(Route::getById(121), nullptr);
  EXPECT_EQ(Route::getById(221), nullptr);
}

TEST_F(RouteTests, TestRouteState) {
  // Populate the route list via inbound <jA> responses
  _dccexProtocol.getLists(false, false, true, false);
  _stream.clearOutput();
  _stream << "<jA 21 121>";
  _dccexProtocol.check();
  _stream << R"(<jA 21 R "Route 21">)";
  _stream << R"(<jA 121 A "Automation 121">)";
  EXPECT_CALL(_delegate, receivedRouteList()).Times(Exactly(1));
  _dccexProtocol.check();
  ASSERT_EQ(_dccexProtocol.getRouteCount(), 2);
  ASSERT_NE(Route::getFirst(), nullptr);

  // Test setting and getting route state
  _stream.clearOutput();
  EXPECT_CALL(_delegate, receivedRouteState(21, RouteStateActive)).Times(Exactly(1));
  _stream << "<jB 21 1>";   // set active state for route 21
  _dccexProtocol.check();
  ASSERT_EQ(Route::getById(21)->getState(), RouteStateActive);

  EXPECT_CALL(_delegate, receivedRouteState(21, RouteStateHidden)).Times(Exactly(1));
  _stream.clearOutput();
  _stream << "<jB 21 2>";   // set hidden state for route 21
  _dccexProtocol.check();
  ASSERT_EQ(Route::getById(21)->getState(), RouteStateHidden);

   EXPECT_CALL(_delegate, receivedRouteState(121, RouteStateInactive)).Times(Exactly(1));
 _stream.clearOutput();
  _stream << "<jB 121 0>";   // set Inactive state for route 121
  _dccexProtocol.check();
  ASSERT_EQ(Route::getById(121)->getState(), RouteStateInactive);

  EXPECT_CALL(_delegate, receivedRouteState(121, RouteStateDisabled)).Times(Exactly(1));
  _stream.clearOutput();
  _stream << "<jB 121 4>";   // set disabled state for route 121
  _dccexProtocol.check();
  ASSERT_EQ(Route::getById(121)->getState(), RouteStateDisabled);

  // Clearing the list must remove every route and reset the count
  _dccexProtocol.clearRouteList();
  EXPECT_EQ(_dccexProtocol.getRouteCount(), 0);
  EXPECT_EQ(Route::getFirst(), nullptr);
  EXPECT_EQ(Route::getById(21), nullptr);
  EXPECT_EQ(Route::getById(121), nullptr);
}

TEST_F(RouteTests, TestRouteCaption) {
  std::string caption;
 
  // populate the route list via inbound <jA> responses
  _dccexProtocol.getLists(false, false, true, false);
  _stream.clearOutput();
  _stream << "<jA 21 121>";
  _dccexProtocol.check();
  _stream << R"(<jA 21 R "Route 21">)";
  _stream << R"(<jA 121 A "Automation 121">)";
  EXPECT_CALL(_delegate, receivedRouteList()).Times(Exactly(1));
  _dccexProtocol.check();
  ASSERT_EQ(_dccexProtocol.getRouteCount(), 2);
  ASSERT_NE(Route::getFirst(), nullptr);

  // Test setting and getting route state
  _stream << R"(<jB 21 "Active">)";   // set active state for route 21
   EXPECT_CALL(_delegate, receivedRouteCaption(21, testing::StrEq("Active"))).Times(Exactly(1));
  _dccexProtocol.check();
  caption = Route::getById(21)->getCaption();
  ASSERT_EQ(caption, "Active");

  _stream << R"(<jB 21 "Hidden">)";   // set hidden state for route 21
   EXPECT_CALL(_delegate, receivedRouteCaption(21, testing::StrEq("Hidden"))).Times(Exactly(1));
  _dccexProtocol.check();
  caption = Route::getById(21)->getCaption();
  ASSERT_EQ(caption, "Hidden");

  _stream << R"(<jB 121 "Inactive">)";   // set Inactive state for route 121
  EXPECT_CALL(_delegate, receivedRouteCaption(121, testing::StrEq("Inactive"))).Times(Exactly(1));
  _dccexProtocol.check();
  caption = Route::getById(121)->getCaption();
  ASSERT_EQ(caption, "Inactive");

  _stream.clearOutput();
  _stream << R"(<jB 121 "Disabled">)";   // set disabled state for route 121
   EXPECT_CALL(_delegate, receivedRouteCaption(121, testing::StrEq("Disabled"))).Times(Exactly(1));
  _dccexProtocol.check();
  caption = Route::getById(121)->getCaption();
  ASSERT_EQ(caption, "Disabled");

  // Clearing the list must remove every route and reset the count
  _dccexProtocol.clearRouteList();
  EXPECT_EQ(_dccexProtocol.getRouteCount(), 0);
  EXPECT_EQ(Route::getFirst(), nullptr);
  EXPECT_EQ(Route::getById(21), nullptr);
  EXPECT_EQ(Route::getById(121), nullptr);
}
