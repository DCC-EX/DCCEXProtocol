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
  virtual ~Print() {}

  // The single "Bottleneck" method.
  // Every print call must eventually call this.
  virtual size_t write(uint8_t c) = 0;

  // Multi-byte write helper
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

  // --- Print Overloads ---

  void print(const char *s) {
    if (s)
      while (*s)
        write(*s++);
  }

  void print(const std::string &s) { print(s.c_str()); }

  void print(__FlashStringHelper *s) { print((const char *)s); }

  void print(char c) { write(c); }

  void print(int n) { print(std::to_string(n).c_str()); }

  void print(long n) { print(std::to_string(n).c_str()); }

  // --- Println Overloads ---

  void println() {
    write('\r');
    write('\n');
  }

  void println(const char *s) {
    print(s);
    println();
  }

  void println(const std::string &s) {
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
};

#endif // PRINT_H
