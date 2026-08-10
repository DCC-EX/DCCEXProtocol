/* -*- c++ -*-
 *
 * DCCEXProtocol
 *
 * Shared utility helpers.
 *
 */

#ifndef DCCEXUTILS_H
#define DCCEXUTILS_H

/**
 * @brief Fast integer to ASCII conversion using integer math only.
 * @param n Integer value to convert.
 * @param bufferEnd Pointer to the last writable byte in destination buffer.
 * @return Pointer to the first character of the converted null-terminated string.
 */
static inline char *fastitoa(int n, char *bufferEnd) {
  char *writePtr = bufferEnd;
  *writePtr = '\0';

  bool isNegative = n < 0;
  unsigned int magnitude = static_cast<unsigned int>(n);
  if (isNegative) {
    magnitude = 0u - magnitude;
  }

  do {
    *--writePtr = static_cast<char>('0' + (magnitude % 10u));
    magnitude /= 10u;
  } while (magnitude != 0u);

  if (isNegative) {
    *--writePtr = '-';
  }

  return writePtr;
}

#endif
