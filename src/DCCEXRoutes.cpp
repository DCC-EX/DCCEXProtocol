#include <Arduino.h>
#include "DCCEXRoutes.h"

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