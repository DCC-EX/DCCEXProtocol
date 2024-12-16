/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
 *
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

#include "TestHarnessBase.hpp"

TestHarnessBase::TestHarnessBase() {}

TestHarnessBase::~TestHarnessBase() {}

void TestHarnessBase::SetUp() {
  _dccexProtocol.setDelegate(&_delegate);
  _dccexProtocol.setLogStream(&_console);
  _dccexProtocol.connect(&_stream);
  _dccexProtocol.clearRoster();
}

void TestHarnessBase::TearDown() {
  _dccexProtocol.clearAllLists();
}
