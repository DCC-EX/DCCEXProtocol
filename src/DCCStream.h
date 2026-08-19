#ifndef _DCCSTREAM_H
#define _DCCSTREAM_H

#ifndef ARDUINO

#include <cstddef>
#include <cstdint>
#include <string>

typedef const char *__FlashStringHelper;

class Print {
public:
  virtual ~Print() {}
  virtual size_t write(uint8_t c) = 0;
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
  void print(const char *s) { if (s) while (*s) write(*s++); }
  void print(const std::string &s) { print(s.c_str()); }
  void print(__FlashStringHelper *s) { print((const char *)s); }
  void print(char c) { write(c); }
  void print(int n) { print(std::to_string(n).c_str()); }
  void print(long n) { print(std::to_string(n).c_str()); }
  void println() { write('\r'); write('\n'); }
  void println(const char *s) { print(s); println(); }
  void println(const std::string &s) { print(s); println(); }
  void println(__FlashStringHelper *s) { print(s); println(); }
  void println(int n) { print(n); println(); }
};

class Stream : public Print {
public:
  virtual ~Stream() {}
  virtual int available() { return static_cast<int>(_inputBuffer.length()); }
  virtual int read() {
    if (_inputBuffer.empty()) return -1;
    char c = _inputBuffer[0];
    _inputBuffer.erase(0, 1);
    return c;
  }
  virtual void flush() {}
  virtual size_t write(uint8_t c) override { _outputBuffer += (char)c; return 1; }
  template <typename T> Stream &operator<<(const T &data) { _inputBuffer += data; return *this; }
  std::string getOutput() { return _outputBuffer; }
  void clearOutput() { _outputBuffer.clear(); }
  void clearInput() { _inputBuffer.clear(); }
private:
  std::string _inputBuffer;
  std::string _outputBuffer;
};

#endif // ARDUINO
#endif // _DCCSTREAM_H
