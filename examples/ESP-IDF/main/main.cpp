#include "loco_control.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main(void) {
  dccex_basic_example_start();


  while (true) {
    dccex_basic_example_tick();
    vTaskDelay(10 / portTICK_PERIOD_MS); // Delay for 10 milliseconds
  }
}