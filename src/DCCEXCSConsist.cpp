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

// CSConsist public methods

CSConsist *CSConsist::_first = nullptr;

CSConsist::CSConsist()
    : _firstMember(nullptr), _next(nullptr), _createdInCS(false), _deleteCSPending(false), _memberCount(0) {
  if (!_first) {
    _first = this;
  } else {
    CSConsist *current = _first;
    while (current->_next != nullptr) {
      current = current->_next;
    }
    current->_next = this;
  }
}

CSConsist *CSConsist::getFirst() { return _first; }

CSConsist *CSConsist::getNext() { return _next; }

void CSConsist::addMember(int address, bool reversed) {
  if (address < 0 || address > 10239)
    return;

  if (isInConsist(address))
    return;

  CSConsistMember *member = new CSConsistMember((uint16_t)address, (uint16_t)reversed);

  if (_firstMember == nullptr) {
    _firstMember = member;
  } else {
    CSConsistMember *current = _firstMember;
    while (current->next != nullptr) {
      current = current->next;
    }
    current->next = member;
  }
  _memberCount++;
}

void CSConsist::removeMember(int address) {
  CSConsistMember *previous = nullptr;
  CSConsistMember *current = _firstMember;
  while (current) {
    if (current->address == (uint16_t)address) {
      CSConsistMember *next = current->next;
      if (previous) {
        previous->next = next;
      } else {
        _firstMember = next;
      }
      delete current;
      current = next;
    } else {
      previous = current;
      current = current->next;
    }
  }

  if (!_firstMember)
    _firstMember = nullptr;

  _memberCount--;
  if (_memberCount < 0)
    _memberCount = 0;
}

void CSConsist::removeAllMembers() {
  CSConsistMember *current = _firstMember;
  while (current != nullptr) {
    CSConsistMember *next = current->next;
    delete current;
    current = next;
  }
  _firstMember = nullptr;
  _memberCount = 0;
}

CSConsistMember *CSConsist::getFirstMember() { return _firstMember; }

CSConsistMember *CSConsist::getMember(int address) {
  for (CSConsistMember *member = _firstMember; member; member = member->next) {
    if (member->address == (uint16_t)address) {
      return member;
    }
  }

  return nullptr;
}

bool CSConsist::isInConsist(int address) {
  for (CSConsistMember *member = _firstMember; member; member = member->next) {
    if (member->address == (uint16_t)address) {
      return true;
    }
  }

  return false;
}

bool CSConsist::isReversed(int address) {
  for (CSConsistMember *member = _firstMember; member; member = member->next) {
    if (member->address == (uint16_t)address) {
      return member->reversed;
    }
  }

  return false;
}

void CSConsist::setCreatedInCS(bool created) { _createdInCS = created; }

bool CSConsist::isCreatedInCS() { return _createdInCS; }

void CSConsist::setDeleteCSPending(bool pending) { _deleteCSPending = pending; }

bool CSConsist::isDeleteCSPending() { return _deleteCSPending; }

bool CSConsist::isValid() { return (_memberCount > 1); }

void CSConsist::clearCSConsists() {
  if (!_first)
    return;

  while (_first != nullptr)
    delete _first;
}

CSConsist *CSConsist::getLeadLocoCSConsist(int address) {
  for (CSConsist *csConsist = _first; csConsist; csConsist = csConsist->getNext()) {
    CSConsistMember *first = csConsist->getFirstMember();
    if (first && first->address == (uint16_t)address) {
      return csConsist;
    }
  }
  return nullptr;
}

CSConsist *CSConsist::getMemberCSConsist(int address) {
  for (CSConsist *csConsist = _first; csConsist; csConsist = csConsist->getNext()) {
    for (CSConsistMember *member = csConsist->getFirstMember(); member; member = member->next) {
      if (member->address == (uint16_t)address) {
        return csConsist;
      }
    }
  }
  return nullptr;
}

CSConsist::~CSConsist() {
  // Clean up the member list first
  removeAllMembers();

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
