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

#include "MockSetup.h"

namespace {

std::string stream2string(StreamMock stream) {
  std::string retval;
  while (stream.available())
    retval.push_back(static_cast<char>(stream.read()));
  return retval;
}

} // namespace

bool operator==(StreamMock lhs, StreamMock rhs) { return stream2string(lhs) == stream2string(rhs); }

bool operator==(StreamMock lhs, std::string rhs) { return stream2string(lhs) == rhs; }

size_t ExtendedStreamMock::write(uint8_t ch) {
  _buffer.push_back(static_cast<char>(ch));
  return 1;
}

std::string const &ExtendedStreamMock::getBuffer() const { return _buffer; }

void ExtendedStreamMock::clearBuffer() { _buffer.clear(); }
