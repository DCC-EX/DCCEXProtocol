/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
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

#ifndef TESTHARNESSCOMMON_H
#define TESTHARNESSCOMMON_H

#include "../mocks/Arduino.h"
#include <DCCEXProtocol.h>

using namespace testing;

/**
 * @brief Common elements of the test harness to be inherited by other harnesses
 */
class TestHarnessCommon : public Test {
protected:
  /**
   * @brief Mock version string for the EX-CommandStation server
   */
  const char *_mockServerVersion = "0.0.0";

  /**
   * @brief Set the Mock Server Version char array string
   * @param version Version string (eg. "4.0.0", "5.9.0")
   */
  void setMockServerVersion(const char *version) {
    if (!version)
      return;

    _mockServerVersion = version;
  }

  /**
   * @brief Get the Mock Server Version object
   * @details Must be called only after the library has sent a version request `<s>` via `requestServerVersion()` or `getLists()`.
   */
  void streamMockServerVersion() {
    if (!_mockServerVersion)
      FAIL() << "Must call setMockServerVersion() before streamMockServerVersion()";

    // Library must request version before we respond, fail here without injection to prevent state corruption
    ASSERT_EQ(_stream.getOutput(), "<s>") << "Server version not requested, call requestServerVersion() or getLists() first";
    _stream.clearOutput();
    _stream << "<iDCC-EX V-" << _mockServerVersion << ">";
    _dccexProtocol.check();
    _stream.clearOutput();

    // Assert here if this did not correctly trigger the version has been received
    ASSERT_TRUE(_dccexProtocol.receivedVersion()) << "Mock server version did not parse: '" << _mockServerVersion << "'";
  }

  /**
   * @brief Override default setup to set common objects (millis, log, and connection streams)
   */
  void SetUp() override {
    millis();
    _dccexProtocol.setLogStream(&_console);
    _dccexProtocol.connect(&_stream);
    _dccexProtocol.clearRoster();
    onSetUp();
  }

  /**
   * @brief Override default tear down to clear/reset common objects
   */
  void TearDown() override {
    resetMillis();
    _stream.clearInput();
    _stream.clearOutput();
    _dccexProtocol.clearAllLists();
    CSConsist::clearCSConsists();
    CSConsist::setAlwaysReplicateFunctions(false);
    onTearDown();
  }

  /**
   * @brief Override for derived class setup
   */
  virtual void onSetUp() {}

  /**
   * @brief Override for derived class tear down
   */
  virtual void onTearDown() {}

  /**
   * @brief Common attributes/objects
   */
  DCCEXProtocol _dccexProtocol;
  Stream _stream;
  Stream _console;
};

#endif // TESTHARNESSCOMMON_H
