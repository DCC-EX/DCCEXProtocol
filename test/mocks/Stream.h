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

#include <cstdarg>
#include <cstdio>
#include <DCCStream.h>
#include <string>

using namespace DCCExController;

/**
 * @brief Mock Stream class to simulate Arduino Stream objects eg. Serial.
 * @details Utilises a separate input and output buffer to cater for bi-directional comms.
 */
class Stream : public DCCStream {
public:
  /**
   * @brief Determines if there are more characters in the buffer
   * @return int Length of the buffer
   */
  int available() const override { return _inputBuffer.length(); } 

  /**
   * @brief Read a char from the buffer
   * @return int Char
   */
  int read() override {
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
  virtual size_t write(uint8_t c) {
    _outputBuffer += (char)c;
    return 1;
  }


  virtual size_t write(const uint8_t *buffer, size_t size) override {
    _outputBuffer += std::string(reinterpret_cast<const char *>(buffer), size);
    return size;
  };

  /**
   * @brief Helper to write data to the buffer using <<
   * @tparam T
   * @param data
   * @return Stream&
   */
  template <typename T> Stream &operator<<(const T &data) {
    // We bypass write() and put this straight into output
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
  void clearOutput() { _outputBuffer = ""; }


  virtual void flush() override {
    _outputBuffer = "";
  }


  virtual void println(const char* format, ...) override {
    va_list args;
    va_start(args, format);
    _appendFormat(format, args);
    va_end(args);
    _outputBuffer += "\r\n";
  }

  virtual void print(const char* format, ...) override {
    va_list args;
    va_start(args, format);
    _appendFormat(format, args);
    va_end(args);
  }

private:
  void _appendFormat(const char *format, va_list args) {
    va_list argsCopy;
    va_copy(argsCopy, args);
    int needed = vsnprintf(nullptr, 0, format, argsCopy);
    va_end(argsCopy);
    if (needed <= 0) {
      return;
    }
    std::string formatted(needed + 1, '\0');
    vsnprintf(&formatted[0], formatted.size(), format, args);
    _outputBuffer += formatted.c_str();
  }

  std::string _inputBuffer;  // Data for read()
  std::string _outputBuffer; // Data from write()/print()
};

#endif
