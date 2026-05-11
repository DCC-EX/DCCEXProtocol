#ifndef _STREAM_H
#define _STREAM_H

#ifndef ARDUINO 

#include <stddef.h>
#include <inttypes.h>

class Stream {
  public:
    virtual ~Stream() {}

    virtual int available() = 0;
    virtual int read() = 0;
    virtual size_t write(const uint8_t *buffer, size_t size) = 0;
    virtual void flush() = 0;
    virtual void println(const char* format, ...) {};
    virtual void print(const char* format, ...) {};
};

#endif

#endif