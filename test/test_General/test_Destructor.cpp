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

#include "../setup/DCCEXProtocolTests.h"

TEST_F(DCCEXProtocolTests, destructorFreesAllObjectLists) {
  Stream stream;
  DCCEXProtocol *protocol = new DCCEXProtocol(500, 50, 100);
  protocol->setDelegate(&_delegate);
  protocol->setLogStream(&_console);
  protocol->connect(&stream);

  // Populate every object type managed by the protocol
  stream << "<jR 10 11>";
  protocol->check();
  stream.clearOutput();
  stream << "<jT 12 13>";
  protocol->check();
  stream.clearOutput();
  stream << "<jA 14 15>";
  protocol->check();
  stream.clearOutput();
  stream << "<jO 16 17>";
  protocol->check();
  stream.clearOutput();
  stream << "<jS 18 19>";
  protocol->check();

  // Add a local Loco and a command station consist
  new Loco(42, LocoSource::LocoSourceEntry);
  CSConsist *consist = protocol->createCSConsist(5);
  protocol->addCSConsistMember(consist, 6);

  // Confirm objects were created before the protocol is destroyed
  EXPECT_EQ(protocol->getRosterCount(), 2);
  EXPECT_EQ(protocol->getTurnoutCount(), 2);
  EXPECT_EQ(protocol->getRouteCount(), 2);
  EXPECT_EQ(protocol->getTurntableCount(), 2);
  EXPECT_EQ(protocol->getSignalCount(), 2);
  ASSERT_NE(Loco::getFirst(), nullptr);
  ASSERT_NE(Loco::getFirstLocalLoco(), nullptr);
  ASSERT_NE(Turnout::getFirst(), nullptr);
  ASSERT_NE(Route::getFirst(), nullptr);
  ASSERT_NE(Turntable::getFirst(), nullptr);
  ASSERT_NE(Signal::getFirst(), nullptr);
  ASSERT_NE(CSConsist::getFirst(), nullptr);

  // Destroying the protocol must free all object lists
  delete protocol;

  EXPECT_EQ(Loco::getFirst(), nullptr);
  EXPECT_EQ(Loco::getFirstLocalLoco(), nullptr);
  EXPECT_EQ(Turnout::getFirst(), nullptr);
  EXPECT_EQ(Route::getFirst(), nullptr);
  EXPECT_EQ(Turntable::getFirst(), nullptr);
  EXPECT_EQ(Signal::getFirst(), nullptr);
  EXPECT_EQ(CSConsist::getFirst(), nullptr);
}