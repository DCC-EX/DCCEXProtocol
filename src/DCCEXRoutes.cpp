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

#include "DCCEXRoutes.h"
#include <Arduino.h>

// Public methods

Route *Route::_first = nullptr;

Route::Route(int id) {
  _id = id;
  _name = nullptr;
  _next = nullptr;
  if (!_first) {
    _first = this;
  } else {
    Route *current = _first;
    while (current->_next != nullptr) {
      current = current->_next;
    }
    current->_next = this;
  }
}

int Route::getId() { return _id; }

void Route::setName(const char *name) {
  if (_name) {
    delete[] _name;
    _name = nullptr;
  }
  int nameLength = strlen(name);
  _name = new char[nameLength + 1];
  strcpy(_name, name);
}

const char *Route::getName() { return _name; }

void Route::setType(RouteType type) { _type = type; }

RouteType Route::getType() { return (RouteType)_type; }

Route *Route::getFirst() { return _first; }

void Route::setNext(Route *route) { _next = route; }

Route *Route::getNext() { return _next; }

Route *Route::getById(int id) {
  for (Route *r = Route::getFirst(); r; r = r->getNext()) {
    if (r->getId() == id) {
      return r;
    }
  }
  return nullptr;
}

void Route::clearRouteList() {
  // Count Routes in list
  int routeCount = 0;
  Route *currentRoute = Route::getFirst();
  while (currentRoute != nullptr) {
    routeCount++;
    currentRoute = currentRoute->getNext();
  }

  // Store Route pointers in an array for clean up
  Route **deleteRoutes = new Route *[routeCount];
  currentRoute = Route::getFirst();
  for (int i = 0; i < routeCount; i++) {
    deleteRoutes[i] = currentRoute;
    currentRoute = currentRoute->getNext();
  }

  // Delete each Route
  for (int i = 0; i < routeCount; i++) {
    delete deleteRoutes[i];
  }

  // Clean up the array of pointers
  delete[] deleteRoutes;

  // Reset first pointer
  Route::_first = nullptr;
}

Route::~Route() {
  _removeFromList(this);

  if (_name) {
    delete[] _name;
    _name = nullptr;
  }

  _next = nullptr;
}

// Private methods

void Route::_removeFromList(Route *route) {
  if (!route) {
    return;
  }

  if (getFirst() == route) {
    _first = route->getNext();
  } else {
    Route *currentRoute = _first;
    while (currentRoute && currentRoute->getNext() != route) {
      currentRoute = currentRoute->getNext();
    }
    if (currentRoute) {
      currentRoute->setNext(route->getNext());
    }
  }
}
