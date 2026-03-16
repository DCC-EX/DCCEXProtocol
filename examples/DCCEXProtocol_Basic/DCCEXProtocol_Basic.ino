// WiThrottleProtocol library: Basic example
//
// Shows how to create an instance of DCCEXProtocol
// and how to connect to a DCC-EX Native protocol server using static IP
// Tested with ESP32-WROOM board
//
// Peter Akers (Flash62au), Peter Cole (PeteGSX) and Chris Harlow (UKBloke), 2023
// Luca Dentella, 2020

#include <Arduino.h>
#include <DCCEXProtocol.h>
#include <WiFi.h>
#include <stdarg.h>
#include <stdio.h>


// If we haven't got a custom config.h, use the example
#if __has_include("config.h")
#include "config.h"
#else
#warning config.h not found. Using defaults from config.example.h
#include "config.example.h"
#endif

using namespace DCCExController;

class ArduinoDCCMillis : public DCCMillis {
public:
  unsigned long millis() const override { return ::millis(); }
};

class ArduinoDCCStream : public DCCStream {
public:
  explicit ArduinoDCCStream(Stream &stream) : _stream(stream) {}

  int available() const override {
    return const_cast<Stream &>(_stream).available();
  }

  int read() override { return _stream.read(); }

  size_t write(const uint8_t *buffer, size_t size) override {
    return _stream.write(buffer, size);
  }

  void flush() override { _stream.flush(); }

  void println(const char *format, ...) override {
    char message[160];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    _stream.println(message);
  }

  void print(const char *format, ...) override {
    char message[160];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    _stream.print(message);
  }

private:
  Stream &_stream;
};



// Global objects
WiFiClient client;
ArduinoDCCMillis dccMillis;
ArduinoDCCStream dccTransport(client);
ArduinoDCCStream dccLog(Serial);
DCCEXProtocol dccexProtocol(&dccMillis);

void setup() {

  Serial.begin(115200);
  Serial.println("DCCEXProtocol Basic Demo");
  Serial.println();

  // Connect to WiFi network
  Serial.println("Connecting to WiFi..");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
    delay(1000);
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());

  // Connect to the server
  Serial.println("Connecting to the server...");
  if (!client.connect(serverAddress, serverPort)) {
    Serial.println("connection failed");
    while (1)
      delay(1000);
  }
  Serial.println("Connected to the server");

  dccexProtocol.setLogStream(&dccLog);

  dccexProtocol.enableHeartbeat();

  // Pass the communication to wiThrottleProtocol
  dccexProtocol.connect(&dccTransport);
  Serial.println("DCC-EX connected");
}

void loop() {

  // parse incoming messages
  dccexProtocol.check();
}
