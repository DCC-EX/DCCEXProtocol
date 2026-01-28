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

#ifndef PRINT_H
#define PRINT_H

#include <cstddef>
#include <cstdint>
#include <string>

// To handle F("") macros in mocks, we need a dummy type
typedef const char *__FlashStringHelper;

class Print {
public:
  std::string buffer;

  virtual ~Print() {}

  // The core method Arduino uses for raw data
  virtual size_t write(uint8_t c) {
    buffer += (char)c;
    return 1;
  }

  virtual size_t write(const uint8_t *buffer, size_t size) {
    size_t n = 0;
    while (size--) {
      if (write(*buffer++))
        n++;
      else
        break;
    }
    return n;
  }

  // The logic your code is looking for
  void print(const std::string &s) { buffer += s; }
  void print(const char *s) {
    if (s)
      buffer += s;
  }
  void print(__FlashStringHelper *s) {
    if (s)
      buffer += (const char *)s;
  }
  void print(int n) { buffer += std::to_string(n); }
  void print(long n) { buffer += std::to_string(n); }
  void print(char c) { buffer += c; }

  void println() { buffer += "\r\n"; }
  void println(const std::string &s) {
    print(s);
    println();
  }
  void println(const char *s) {
    print(s);
    println();
  }
  void println(__FlashStringHelper *s) {
    print(s);
    println();
  }
  void println(int n) {
    print(n);
    println();
  }

  // helper for your tests
  void clear() { buffer.clear(); }
  void clearBuffer() { buffer.clear(); }
  const std::string &getBuffer() const { return buffer; }
};

#endif // PRINT_H
