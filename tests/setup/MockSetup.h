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

#ifndef MOCKSETUP_H
#define MOCKSETUP_H

#include <StreamMock.h>
#include <string>

// Make StreamMock comparable
bool operator==(StreamMock lhs, StreamMock rhs);
bool operator==(StreamMock lhs, std::string rhs);

class ExtendedStreamMock : public StreamMock {
public:
  /// @brief Override write method for mocking and testing buffer contents
  /// @param ch Char to write
  size_t write(uint8_t ch) override;

  /// @brief Get the buffer
  /// @return Buffer contents
  const std::string &getBuffer() const;

  /// @brief Clear the buffer
  void clearBuffer();

private:
  std::string _buffer;
};

#endif // MOCKSETUP_H
