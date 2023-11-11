#include <Arduino.h>
#include "DCCEXTurnouts.h"

Turnout* Turnout::_first=nullptr;

Turnout::Turnout(int id, bool thrown) {
  _id=id;
  _thrown=thrown;
  _name=nullptr;
  _next=nullptr;
  if (!_first) {
    _first=this;
  } else {
    Turnout* current=_first;
    while (current->_next!=nullptr) {
        current=current->_next;
    }
    current->_next=this;
  }
  _count++;
}

void Turnout::setThrown(bool thrown) {
  _thrown=thrown;
}

void Turnout::setName(char *name) {
  _name=name;
}

int Turnout::getId() {
  return _id;
}

char* Turnout::getName() {
  return _name;
}

bool Turnout::getThrown() {
  return _thrown;
}

Turnout* Turnout::getFirst() {
  return _first;
}

Turnout* Turnout::getNext() {
  return _next;
}

int Turnout::getCount() {
  return _count;
}

Turnout* Turnout::getById(int id) {
  for (Turnout* t=getFirst(); t; t=t->getNext()) {
    if (t->getId()==id) {
      return t;
    }
  }
  return nullptr;
}
