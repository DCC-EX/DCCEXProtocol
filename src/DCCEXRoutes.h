/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
 *
 * Copyright © 2023 Peter Akers
 * Copyright © 2023 Peter Cole
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

#ifndef DCCEXROUTES_H
#define DCCEXROUTES_H

#include <Arduino.h>

enum RouteType {
  RouteTypeRoute = 'R',
  RouteTypeAutomation = 'A',
};

/// @brief Class to contain and maintain the various Route attributes and methods
class Route {
public:
  /// @brief Constructor
  /// @param id Route ID
  Route(int id);

  /// @brief Get route ID
  /// @return ID of the route
  int getId();

  /// @brief Set route name
  /// @param name Name to set for the route
  void setName(const char *name);

  /// @brief Get route name
  /// @return Current name of the route
  const char *getName();

  /// @brief Set route type (A automation, R route)
  /// @param type RouteType - RouteTypeAutomation|RouteTypeRoute
  void setType(RouteType type);

  /// @brief Get route type (A automation, R route)
  /// @return RouteTypeAutomation|RouteTypeRoute
  RouteType getType();

  /// @brief Get first Route object
  /// @return Pointer to the first Route object
  static Route *getFirst();

  /// @brief Set the next route in the list
  /// @param route Pointer to the next route
  void setNext(Route *route);

  /// @brief Get next Route object
  /// @return Pointer to the next Route object
  Route *getNext();

  /// @brief Get route object by its ID
  /// @return Pointer to the Route, or nullptr if not found
  static Route *getById(int id);

  /// @brief Clear the list of routes
  static void clearRouteList();

  /// @brief Destructor for a route
  ~Route();

private:
  int _id;
  char *_name;
  char _type;
  static Route *_first;
  Route *_next;

  /// @brief Remove the route from the list
  /// @param route Pointer to the route to remove
  void _removeFromList(Route *route);
};

#endif