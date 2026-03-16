#ifndef MILLIS_H
#define MILLIS_H

#include <stdint.h>

inline unsigned long _currentMillis = 0;

inline void advanceMillis(unsigned long ms) { _currentMillis += ms; }
inline void resetMillis() { _currentMillis = 0; }
inline unsigned long getMillis() { return _currentMillis; }

#endif