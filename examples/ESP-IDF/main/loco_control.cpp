#include "loco_control.h"

#include "../../../src/DCCEXProtocol.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <freertos/event_groups.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>
#include <nvs_flash.h>

#if __has_include("config.h")
#include "config.h"
#else
#error config.h not found. Copy from config.example.h and update with your settings.
#endif

namespace {

constexpr const char *TAG = "DCCEXBasic";

class EspLogStream : public Stream {
public:
  int available() override { return 0; }
  int read() override { return -1; }
  void flush() override {}

  size_t write(uint8_t c) override {
    putchar(static_cast<char>(c));
    return 1;
  }

  size_t write(const uint8_t *buffer, size_t size) override {
    if (buffer == nullptr || size == 0) {
      return 0;
    }
    fwrite(buffer, 1, size, stdout);
    return size;
  }
};

// ---------------------------------------------------------------------------
// TcpStream — wraps a connected lwIP socket as a DCCEXProtocol Stream
// ---------------------------------------------------------------------------
class TcpStream : public Stream {
public:
  explicit TcpStream(int fd) : _fd(fd) {}

  int available() override {
    int count = 0;
    if (ioctl(_fd, FIONREAD, &count) < 0) {
      return 0;
    }
    return count;
  }

  int read() override {
    uint8_t byte = 0;
    const int n = recv(_fd, &byte, 1, 0);
    return (n == 1) ? static_cast<int>(byte) : -1;
  }

  size_t write(uint8_t c) override {
    return write(&c, 1);
  }

  size_t write(const uint8_t *buffer, size_t size) override {
    if (buffer == nullptr || size == 0) {
      return 0;
    }
    const ssize_t n = send(_fd, buffer, size, 0);
    return (n < 0) ? 0 : static_cast<size_t>(n);
  }

  void flush() override {}

private:
  int _fd;
};

// for random speed changes
int speed = 0;
int up = 1;
unsigned long lastTime = 0;

// define our loco object
Loco *loco = nullptr;

// Global objects
EspLogStream log_stream;
DCCEXProtocol dccexProtocol;
bool started = false;
int dcc_sock = -1;
TcpStream *dcc_stream = nullptr;

constexpr int WIFI_CONNECTED_BIT = BIT0;
constexpr int WIFI_FAIL_BIT      = BIT1;
constexpr int WIFI_MAX_RETRY     = 5;

EventGroupHandle_t s_wifi_event_group;
int s_retry_count = 0;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data) {
  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    esp_wifi_connect();
  } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    if (s_retry_count < WIFI_MAX_RETRY) {
      esp_wifi_connect();
      ++s_retry_count;
      printf("[%s] Retrying WiFi connection (%d/%d)\n", TAG, s_retry_count, WIFI_MAX_RETRY);
    } else {
      xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
      printf("[%s] WiFi connection failed\n", TAG);
    }
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    ip_event_got_ip_t *event = static_cast<ip_event_got_ip_t *>(event_data);
    printf("[%s] WiFi connected, IP: " IPSTR "\n", TAG, IP2STR(&event->ip_info.ip));
    s_retry_count = 0;
    xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
  }
}
class MyDelegate : public DCCEXProtocolDelegate {

public:
  void receivedServerVersion(int major, int minor, int patch) override {
    printf("[%s] Received version: %d.%d.%d\n", TAG, major, minor, patch);
  }

    void receivedTrackPower(TrackPower state) override {
    printf("[%s] Received Track Power: %d\n", TAG, static_cast<int>(state));
  }

  // Use for roster Locos (LocoSource::LocoSourceRoster)
  void receivedLocoUpdate(Loco *loco) override {
    printf("[%s] Received Loco update for DCC address: %d\n", TAG, loco->getAddress());
  }

  // Use for locally created Locos (LocoSource::LocoSourceEntry)
  void receivedLocoBroadcast(int address, int speed, Direction direction, int functionMap) override {
    printf("[%s] Received Loco broadcast: address|speed|direction|functionMap: %d|%d|%s|%d\n", TAG, address, speed, (direction == Direction::Forward ? "Fwd" : "Rev"), functionMap);
  }
};


} // namespace

bool connect_dcc() {
  char port_str[8];
  snprintf(port_str, sizeof(port_str), "%d", serverPort);

  addrinfo hints = {};
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  addrinfo *res = nullptr;
  if (getaddrinfo(serverAddress, port_str, &hints, &res) != 0 || res == nullptr) {
    printf("[DCCEXBasic] DNS lookup failed for %s\n", serverAddress);
    return false;
  }

  dcc_sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (dcc_sock < 0) {
    freeaddrinfo(res);
    printf("[DCCEXBasic] Failed to create socket\n");
    return false;
  }

  if (::connect(dcc_sock, res->ai_addr, res->ai_addrlen) != 0) {
    freeaddrinfo(res);
    close(dcc_sock);
    dcc_sock = -1;
    printf("[DCCEXBasic] TCP connect to %s:%d failed\n", serverAddress, serverPort);
    return false;
  }

  freeaddrinfo(res);

  dcc_stream = new TcpStream(dcc_sock);
  dccexProtocol.connect(dcc_stream);
  printf("[DCCEXBasic] Connected to DCC-EX at %s:%d\n", serverAddress, serverPort);
  return true;
}
bool connect_wifi() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  s_wifi_event_group = xEventGroupCreate();

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;
  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                      &wifi_event_handler, nullptr,
                                                      &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                      &wifi_event_handler, nullptr,
                                                      &instance_got_ip));

  wifi_config_t wifi_config = {};
  strncpy(reinterpret_cast<char *>(wifi_config.sta.ssid), ssid,
          sizeof(wifi_config.sta.ssid) - 1);
  strncpy(reinterpret_cast<char *>(wifi_config.sta.password), password,
          sizeof(wifi_config.sta.password) - 1);

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                         WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                         pdFALSE, pdFALSE, portMAX_DELAY);

  esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip);
  esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id);
  vEventGroupDelete(s_wifi_event_group);

  return (bits & WIFI_CONNECTED_BIT) != 0;
}
void dccex_basic_example_start() {
  dccexProtocol.setLogStream(&log_stream);
  dccexProtocol.enableHeartbeat();
  dccexProtocol.setDelegate(new MyDelegate());
  dccexProtocol.setDebug(true);

  if (!connect_wifi()) {
    printf("[%s] WiFi failed — cannot continue\n", TAG);
    return;
  }

  if (!connect_dcc()) {
    printf("[%s] DCC-EX connect failed — cannot continue\n", TAG);
    return;
  }

  printf("[%s] DCC-EX ready\n", TAG);
  started = true;

  dccexProtocol.requestServerVersion();
  // protocol.getLists();

  lastTime = esp_timer_get_time() / 1000; // current time in ms
}

void dccex_basic_example_tick() {
  if (!started) {
    return;
  }

  dccexProtocol.check();

   if (!loco) {
    // add a loco with DCC address 11 - LocoSourceEntry means it's not from the roster
    loco = new Loco(11, LocoSource::LocoSourceEntry);
    printf("[%s] Added loco: %d\n", TAG, loco->getAddress());

    // turn track power on or the loco won't move
    dccexProtocol.powerOn();
  }

  if (loco) {
    // every 10 seconds change speed and set a random function on or off
    if ((esp_timer_get_time() / 1000 - lastTime) >= 10000) {
      if (speed >= 100)
        up = -1;
      if (speed <= 0)
        up = 1;
      speed = speed + up;
      dccexProtocol.setThrottle(loco, speed, Direction::Forward);

      int fn = static_cast<int>(esp_random() % 27);
      int fns = static_cast<int>(esp_random() % 100);
      bool fnState = (fns < 50) ? false : true;

      if (fnState) {
        dccexProtocol.functionOn(loco, fn);
      } else {
        dccexProtocol.functionOff(loco, fn);
      }

      lastTime = esp_timer_get_time() / 1000; // current time in ms
    }
  }
}
