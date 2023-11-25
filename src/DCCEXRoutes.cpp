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

#include <Arduino.h>
#include "DCCEXRoutes.h"

// Public methods

Route* Route::_first=nullptr;

Route::Route(int id) {
  _id=id;
  _name=nullptr;
  _next=nullptr;
  if (!_first) {
    _first=this;
  } else {
    Route* current=_first;
    while (current->_next!=nullptr) {
      current=current->_next;
    }
    current->_next=this;
  }
  _count++;
}

int Route::getId() {
  return _id;
}

void Route::setName(char* name) {
  _name=name;
}

char* Route::getName() {
  return _name;
}

void Route::setType(RouteType type) {
  _type = type;
}

RouteType Route::getType() {
  return (RouteType)_type;
}

int Route::getCount() {
  return _count;
}

Route* Route::getFirst() {
  return _first;
}

Route* Route::getNext() {
  return _next;
}

Route* Route::getById(int id) {
  for (Route* r=getFirst(); r; r=r->getNext()) {
    if (r->getId()==id) {
      return r;
    }
  }
  return nullptr;
}
