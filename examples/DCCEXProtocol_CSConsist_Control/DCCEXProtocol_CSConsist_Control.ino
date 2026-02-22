// DCCEXProtocol library: CSConsist control example
//
// Shows how to create and control a command station consist
// Tested with ESP32-WROOM board
//
// Peter Cole (PeteGSX), 2026

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
  void receivedLocoBroadcast(int address, int speed, Direction direction, int functionMap) override {
    Serial.print("\n\nReceived Loco broadcast: address|speed|direction|functionMap: ");
    Serial.print(address);
    Serial.print("|");
    Serial.print(speed);
    Serial.print("|");
    if (direction == Direction::Forward) {
      Serial.print("Fwd");
    } else {
      Serial.print("Rev");
    }
    Serial.print("|");
    Serial.println(functionMap);
  }
};

// for random speed changes
int speed = 0;
int up = 1;
unsigned long lastTime = 0;

// define our CSConsist object
CSConsist *csConsist = nullptr;

// Global objects
WiFiClient client;
DCCEXProtocol dccexProtocol;
MyDelegate myDelegate;

void setup() {

  Serial.begin(115200);
  Serial.println("DCCEXProtocol CSConsist Control Demo");
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

  // Logging on Serial
  dccexProtocol.setLogStream(&Serial);

  // Pass the delegate instance to wiThrottleProtocol
  dccexProtocol.setDelegate(&myDelegate);

  dccexProtocol.enableHeartbeat();

  // Pass the communication to wiThrottleProtocol
  dccexProtocol.connect(&client);
  Serial.println("DCC-EX connected");

  // Turn track power on for locos to move
  dccexProtocol.powerOn();

  lastTime = millis();
}

void loop() {
  // parse incoming messages
  dccexProtocol.check();

  if (!consist) {
    // Create a new CSConsist for loco address 11 in the normal direction of travel, and replicate functions across the
    // consist.
    // By default, functions will only affect the lead loco
    csConsist = dccexProtocol.createCSConsist(11, false, true);

    // Add loco 12 to the consist, reverse to the normal direction of travel
    dccexProtocol.addCSConsistMember(12, true);

    // turn track power on or the loco won't move
    dccexProtocol.powerOn();
  }

  if (csConsist) {
    // every 10 seconds change speed and set a random function on or off
    if ((millis() - lastTime) >= 10000) {
      if (speed >= 100)
        up = -1;
      if (speed <= 0)
        up = 1;
      speed = speed + up;
      dccexProtocol.setThrottle(csConsist, speed, Direction::Forward);

      int fn = random(0, 27);
      int fns = random(0, 100);
      bool fnState = (fns < 50) ? false : true;

      if (fnState) {
        dccexProtocol.functionOn(csConsist, fn);
      } else {
        dccexProtocol.functionOff(csConsist, fn);
      }

      lastTime = millis();
    }
  }
}
