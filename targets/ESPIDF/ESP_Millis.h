#ifndef _ESP_MILLIS_H
#define _ESP_MILLIS_H

#include <DCCMillis.h>
#include <esp_timer.h>

inline uint64_t millis() { return esp_timer_get_time() / 1000ULL; }

class ESPDCCMillis : public DCCExController::DCCMillis {
public:
  unsigned long millis() const override { return ::millis(); }
};

#endif