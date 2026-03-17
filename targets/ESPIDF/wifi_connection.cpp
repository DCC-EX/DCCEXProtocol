#include "wifi_connection.h"

namespace utilities {
QueueHandle_t tcp_fail_queue = xQueueCreate(10, sizeof(err_t));
} // namespace utilities