/*
 *  © 2025 Peter Cole
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this code.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef STREAM_H
#define STREAM_H

#include "Print.h"

class Stream : public Print {
public:
  // This allows EXPECT_EQ(_stream.getBuffer(), ...) to work
  // by using the method we just added to Print

  int available() const { return buffer.length(); }

  int read() {
    if (buffer.empty())
      return -1;
    char c = buffer[0];
    buffer.erase(0, 1);
    return static_cast<int>(c);
  }

  // Add this to handle the << syntax
  template <typename T> Stream &operator<<(const T &data) {
    this->print(data);
    return *this;
  }

  // If your tests call clearBuffer(), add an alias:
  void clearBuffer() { clear(); }
};

#endif
