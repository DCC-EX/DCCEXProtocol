/* -*- c++ -*- */

#ifndef DCC_MILLIS_WRAPPERS_H
#define DCC_MILLIS_WRAPPERS_H

/*
 * Optional platform wrappers for time-in-milliseconds.
 *
 * This header defines DCCEX_DEFAULT_MILLIS() when it can detect a supported
 * platform. If your platform is not detected, define DCCEX_MILLIS() in your
 * build flags or before including DCCEXProtocol.h.
 */

#if !defined(DCCEX_DEFAULT_MILLIS)

  #if defined(ARDUINO)
    #include <Arduino.h>
    #define DCCEX_DEFAULT_MILLIS() millis()
  #elif defined(ESP_PLATFORM)
    #include <esp_timer.h>
    static inline unsigned long dccex_esp_idf_millis() {
      return (unsigned long)(esp_timer_get_time() / 1000ULL);
    }
    #define DCCEX_DEFAULT_MILLIS() dccex_esp_idf_millis()
  #elif defined(PICO_SDK_VERSION_MAJOR) || defined(PICO_RP2040) || defined(PICO_ON_DEVICE)
    #include <pico/time.h>
    static inline unsigned long dccex_pico_millis() {
      return (unsigned long)to_ms_since_boot(get_absolute_time());
    }
    #define DCCEX_DEFAULT_MILLIS() dccex_pico_millis()
  #elif defined(__linux__) && !defined(ARDUINO)
    #include <chrono>
    static inline unsigned long dccex_linux_millis() {
      const auto now = std::chrono::steady_clock::now();
      static const auto start = now;
      return (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
    }
    #define DCCEX_DEFAULT_MILLIS() dccex_linux_millis()
  #endif

#endif

#endif
