/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
 *
 * Copyright © 2026 Peter Cole
 * Copyright © 2024 Vincent Hamp
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

#ifndef TESTHARNESSBASE_HPP
#define TESTHARNESSBASE_HPP

#include <gmock/gmock.h>
#include "../mocks/MockDCCEXProtocolDelegate.h"
#include "../mocks/MockDCCMillis.h"
#include "../mocks/Stream.h"
#include "millis.h"
#include <DCCEXProtocol.h>

using namespace testing;
using namespace DCCExController;

/// @brief Test fixture to setup and tear down tests
class TestHarnessBase : public Test {
public:
  TestHarnessBase() {}
  virtual ~TestHarnessBase() {}

protected:
  MockDCCMillis _millisProvider;
  DCCEXProtocol _dccexProtocol{&_millisProvider};
  MockDCCEXProtocolDelegate _delegate;
  Stream _console;
  Stream _stream;


  void SetUp() override {
    _dccexProtocol.setDelegate(&_delegate);
    _dccexProtocol.setLogStream(&_console);
    _dccexProtocol.connect(&_stream);
    _dccexProtocol.clearRoster();
    resetMillis();
  }

  void TearDown() override {
    _dccexProtocol.clearAllLists();
    CSConsist::clearCSConsists();
    CSConsist::setAlwaysReplicateFunctions(false);
  }

};

#endif // TESTHARNESSBASE_HPP
