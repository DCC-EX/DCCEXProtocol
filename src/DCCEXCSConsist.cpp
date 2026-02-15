/* -*- c++ -*-
 *
 * Copyright © 2026 Peter Cole
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

#include "DCCEXCSConsist.h"

// CSConsistMember public methods

CSConsistMember::CSConsistMember(Loco *loco, bool reversed) : _loco(loco), _reversed(reversed), _next(nullptr) {}

Loco *CSConsistMember::getLoco() { return _loco; }

bool CSConsistMember::isReversed() { return _reversed; }

void CSConsistMember::setNext(CSConsistMember *next) { _next = next; }

CSConsistMember *CSConsistMember::getNext() { return _next; }

CSConsistMember::~CSConsistMember() {}

// CSConsist public methods

CSConsist *CSConsist::_first = nullptr;

CSConsist::CSConsist(Loco *leadLoco) : _leadLoco(leadLoco), _firstMember(nullptr), _next(nullptr) {
  _addToConsistList(this);
}

CSConsist::CSConsist(int leadLocoAddress) : _leadLoco(nullptr), _firstMember(nullptr), _next(nullptr) {
  Loco *loco = new Loco(leadLocoAddress, LocoSource::LocoSourceEntry);
  _leadLoco = loco;
  _addToConsistList(this);
}

CSConsist *CSConsist::getFirst() { return _first; }

CSConsist *CSConsist::getNext() { return _next; }

void CSConsist::addMember(Loco *loco, bool reversed) {
  if (!loco)
    return;

  if (isInConsist(loco))
    return;

  CSConsistMember *member = new CSConsistMember(loco, reversed);
  _addToMemberList(member);
}

void CSConsist::addMember(int address, bool reversed) {
  if (isInConsist(address))
    return;

  Loco *loco = Loco::getByAddress(address);
  if (!loco)
    loco = new Loco(address, LocoSource::LocoSourceEntry);

  if (!loco)
    return;

  CSConsistMember *member = new CSConsistMember(loco, reversed);
  _addToMemberList(member);
}

void CSConsist::removeMember(Loco *loco) {
  if (!loco)
    return;

  _removeFromMemberList(loco);
}

void CSConsist::removeMember(int address) {
  CSConsistMember *member = getMember(address);
  if (member)
    _removeFromMemberList(member->getLoco());
}

Loco *CSConsist::getLeadLoco() { return _leadLoco; }

CSConsistMember *CSConsist::getFirstMember() { return _firstMember; }

CSConsistMember *CSConsist::getMember(Loco *loco) {
  if (!loco)
    return nullptr;

  for (CSConsistMember *member = _firstMember; member; member = member->getNext()) {
    if (member->getLoco() == loco) {
      return member;
    }
  }

  return nullptr;
}

CSConsistMember *CSConsist::getMember(int address) {
  for (CSConsistMember *member = _firstMember; member; member = member->getNext()) {
    if (member->getLoco()->getAddress() == address) {
      return member;
    }
  }

  return nullptr;
}

bool CSConsist::isInConsist(Loco *loco) {
  if (!loco)
    return false;

  if (_leadLoco == loco)
    return true;

  for (CSConsistMember *member = _firstMember; member; member = member->getNext()) {
    if (member->getLoco() == loco) {
      return true;
    }
  }

  return false;
}

bool CSConsist::isInConsist(int address) {
  if (_leadLoco->getAddress() == address)
    return true;

  for (CSConsistMember *member = _firstMember; member; member = member->getNext()) {
    if (member->getLoco()->getAddress() == address) {
      return true;
    }
  }

  return false;
}

bool CSConsist::isReversed(Loco *loco) {
  if (!loco)
    return false;

  for (CSConsistMember *member = _firstMember; member; member = member->getNext()) {
    if (member->getLoco() == loco) {
      return member->isReversed();
    }
  }

  return false;
}

bool CSConsist::isReversed(int address) {
  for (CSConsistMember *member = _firstMember; member; member = member->getNext()) {
    if (member->getLoco()->getAddress() == address) {
      return member->isReversed();
    }
  }

  return false;
}

CSConsist::~CSConsist() {
  // Clean up the member list first
  CSConsistMember *current = _firstMember;
  while (current != nullptr) {
    CSConsistMember *next = current->getNext();
    delete current;
    current = next;
  }
  _firstMember = nullptr;

  // If there's no CSConsist list, no need to clean up
  if (!_first)
    return;

  // Clean up the CSConsist linked list
  if (_first == this) {
    _first = this->_next;
  } else {
    CSConsist *current = _first;
    while (current->getNext() != this) {
      current = current->_next;
    }
    if (current->_next) {
      current->_next = this->_next;
    }
  }
}

// CSConsist private methods

void CSConsist::_addToConsistList(CSConsist *csConsist) {
  if (!csConsist)
    return;

  if (!_first) {
    _first = csConsist;
  } else {
    CSConsist *current = _first;
    while (current->_next != nullptr) {
      current = current->_next;
    }
    current->_next = csConsist;
  }
}

void CSConsist::_addToMemberList(CSConsistMember *member) {
  if (!member)
    return;

  if (_firstMember == nullptr) {
    _firstMember = member;
  } else {
    CSConsistMember *current = _firstMember;
    while (current->getNext() != nullptr) {
      current = current->getNext();
    }
    current->setNext(member);
  }
}

void CSConsist::_removeFromMemberList(Loco *loco) {
  if (!loco)
    return;

  CSConsistMember *previous = nullptr;
  CSConsistMember *current = _firstMember;
  while (current) {
    if (current->getLoco() == loco) {
      CSConsistMember *next = current->getNext();
      if (previous) {
        previous->setNext(next);
      } else {
        _firstMember = next;
      }
      delete current;
      current = next;
    } else {
      previous = current;
      current = current->getNext();
    }
  }

  if (!_firstMember)
    _firstMember = nullptr;
}
