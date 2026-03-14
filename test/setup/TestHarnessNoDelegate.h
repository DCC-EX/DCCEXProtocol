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

#ifndef TESTHARNESSNODELEGATE_H
#define TESTHARNESSNODELEGATE_H

#include "../mocks/Arduino.h"
#include <DCCEXProtocol.h>

using namespace testing;

/// @brief Test fixture to setup and tear down tests
class TestHarnessNoDelegate : public Test {
public:
  TestHarnessNoDelegate() {}
  virtual ~TestHarnessNoDelegate() {}

protected:
  void SetUp() override {
    millis();
    _dccexProtocol.setLogStream(&_console);
    _dccexProtocol.connect(&_stream);
  }

  void TearDown() override {
    resetMillis();
    _stream.clearInput();
    _stream.clearOutput();
    _dccexProtocol.clearAllLists();
  }

  DCCEXProtocol _dccexProtocol;
  Stream _console;
  Stream _stream;
};

#endif // TESTHARNESSNODELEGATE_H
