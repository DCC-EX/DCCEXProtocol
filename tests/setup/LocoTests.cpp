/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * This package implements a DCCEX native protocol connection,
 * allow a device to communicate with a DCC-EX EX-CommandStation.
 *
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

#include "LocoTests.h"

void LocoTests::TearDown() {
  // Count Locos in roster
  int locoCount = 0;
  Loco *currentLoco = _dccexProtocol.roster->getFirst();
  while (currentLoco != nullptr) {
    locoCount++;
    currentLoco = currentLoco->getNext();
  }

  // Store Loco pointers in an array for clean up
  Loco **deleteLocos = new Loco *[locoCount];
  currentLoco = _dccexProtocol.roster->getFirst();
  for (int i = 0; i < locoCount; i++) {
    deleteLocos[i] = currentLoco;
    currentLoco = currentLoco->getNext();
  }

  // Delete each Loco
  for (int i = 0; i < locoCount; i++) {
    delete deleteLocos[i];
  }

  // Clean up the array of pointers
  delete[] deleteLocos;

  // Reset roster to nullptr
  _dccexProtocol.roster = nullptr;
}
