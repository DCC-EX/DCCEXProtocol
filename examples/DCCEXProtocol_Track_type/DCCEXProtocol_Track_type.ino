// WiThrottleProtocol library: Track Type example
//
// Shows how to change the track types
// Tested with ESP32-WROOM board
//
// Peter Akers (Flash62au), Peter Cole (PeteGSX) and Chris Harlow (UKBloke), 2023
// Luca Dentella, 2020

#include <DCCEXProtocol.h>
#include <WiFi.h>


// If we haven't got a custom config.h, use the example
#if __has_include("config.h")
#include "config.h"
#else
#warning config.h not found. Using defaults from config.example.h
#include "config.example.h"
#endif

// Delegate class
class MyDelegate : public DCCEXProtocolDelegate {

public:
  void receivedTrackType(char track, TrackManagerMode type, int address) override {
    Serial.print("\n\nReceived TrackType: ");
    Serial.print(track);
    Serial.print(" : ");
    Serial.print(type);
    Serial.print(" : ");
    Serial.println(address);
  }
};

// Global objects
WiFiClient client;
DCCEXProtocol dccexProtocol;
MyDelegate myDelegate;

// for changes
unsigned long lastTime = 0;
int lastTrackType = 0;

void setup() {

  Serial.begin(115200);
  Serial.println("DCCEXProtocol Track Type Demo");
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

  dccexProtocol.setLogStream(&Serial);

  // Pass the delegate instance to wiThrottleProtocol
  dccexProtocol.setDelegate(&myDelegate);

  // Pass the communication to wiThrottleProtocol
  dccexProtocol.connect(&client);
  Serial.println("DCC-EX connected");
}

void loop() {

  // parse incoming messages
  dccexProtocol.check();

  // every 10 seconds change speed and set a random function on or off
  if ((millis() - lastTime) >= 10000) {

    lastTrackType++;
    if (lastTrackType > 4)
      lastTrackType = 0;

    if (lastTrackType == 0) {
      Serial.println("Set A:MAIN  B:PROG");
      dccexProtocol.setTrackType('A', MAIN, 0);
      // dccexProtocol.setTrackType('B',PROG,0);
    } else if (lastTrackType == 1) {
      Serial.println("Set A:PROG  B:DC 10");
      dccexProtocol.setTrackType('A', PROG, 0);
      dccexProtocol.setTrackType('B', DC, 10);
    } else if (lastTrackType == 2) {
      Serial.println("Set A:DC 10  B:DCX 11");
      dccexProtocol.setTrackType('A', DC, 10);
      dccexProtocol.setTrackType('B', DCX, 11);
    } else if (lastTrackType == 3) {
      Serial.println("Set A:DCX 11  B:NONE");
      dccexProtocol.setTrackType('B', NONE, 0);
      dccexProtocol.setTrackType('A', DCX, 11);
    } else if (lastTrackType == 4) {
      Serial.println("Set A:NONE  B:MAIN");
      dccexProtocol.setTrackType('A', NONE, 0);
      dccexProtocol.setTrackType('B', MAIN, 0);
    }

    lastTime = millis();
  }
}
