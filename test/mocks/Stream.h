/*
 *  © 2026 Peter Cole
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

/**
 * @brief Mock Stream class to simulate Arduino Stream objects eg. Serial.
 * @details Utilises a separate input and output buffer to cater for bi-directional comms.
 */
class Stream : public Print {
public:
  /**
   * @brief Determines if there are more characters in the buffer
   * @return int Length of the buffer
   */
  int available() const { return _inputBuffer.length(); }

  /**
   * @brief Read a char from the buffer
   * @return int Char
   */
  int read() {
    if (_inputBuffer.empty())
      return -1;
    char c = _inputBuffer[0];
    _inputBuffer.erase(0, 1);
    return c;
  }

  /**
   * @brief Write to the output buffer
   * @param c Char to write
   * @return size_t
   */
  virtual size_t write(uint8_t c) override {
    _outputBuffer += (char)c;
    return 1;
  }

  /**
   * @brief Helper to write data to the buffer using <<
   * @tparam T
   * @param data
   * @return Stream&
   */
  template <typename T> Stream &operator<<(const T &data) {
    // We bypass write() and put this straight into input
    _inputBuffer += data;
    return *this;
  }

  /**
   * @brief Helper to view the output buffer contents
   * @return std::string
   */
  std::string getOutput() { return _outputBuffer; }

  /**
   * @brief Clear the output buffer
   */
  void clearOutput() { _outputBuffer.clear(); }

  /**
   * @brief Clear the input buffer
   */
  void clearInput() { _inputBuffer.clear(); }

private:
  std::string _inputBuffer;  // Data for read()
  std::string _outputBuffer; // Data from write()/print()
};

#endif
