// WiThrottleProtocol library: Consist control example
//
// Shows how to create and control a consist
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
  void receivedServerVersion(int major, int minor, int patch) override {
    Serial.print("\n\nReceived version: ");
    Serial.print(major);
    Serial.print(".");
    Serial.print(minor);
    Serial.print(".");
    Serial.println(patch);
  }

  void receivedTrackPower(TrackPower state) override {
    Serial.print("\n\nReceived Track Power: ");
    Serial.println(state);
    Serial.println("\n\n");
  }

  // Use for roster Locos (LocoSource::LocoSourceRoster)
  void receivedLocoUpdate(Loco *loco) override {
    Serial.print("Received Loco update for DCC address: ");
    Serial.println(loco->getAddress());
  }

  // Use for locally created Locos (LocoSource::LocoSourceEntry)
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

// define our consist object
Consist *consist = nullptr;

// Global objects
WiFiClient client;
DCCEXProtocol dccexProtocol;
MyDelegate myDelegate;

void setup() {

  Serial.begin(115200);
  Serial.println("DCCEXProtocol Loco Control Demo");
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

  dccexProtocol.requestServerVersion();

  // Turn track power on for locos to move
  dccexProtocol.powerOn();

  lastTime = millis();
}

void loop() {
  // parse incoming messages
  dccexProtocol.check();

  if (!consist) {
    consist = new Consist();

    // create a loco with DCC address 11 - LocoSourceEntry means it's not from the roster
    Loco *loco1 = new Loco(11, LocoSource::LocoSourceEntry);
    Serial.print("Created loco: ");
    Serial.println(loco1->getAddress());

    // add this loco to the consist
    consist->addLoco(loco1, Facing::FacingForward);

    // create a second loco with DCC address 12 - LocoSourceEntry means it's not from the roster
    Loco *loco2 = new Loco(12, LocoSource::LocoSourceEntry);
    Serial.print("Created loco: ");
    Serial.println(loco2->getAddress());

    // add this loco to the consist, and it will be running in reverse direction
    consist->addLoco(loco2, Facing::FacingReversed);

    // turn track power on or the loco won't move
    dccexProtocol.powerOn();
  }

  if (consist) {
    // every 10 seconds change speed and set a random function on or off
    if ((millis() - lastTime) >= 10000) {
      if (speed >= 100)
        up = -1;
      if (speed <= 0)
        up = 1;
      speed = speed + up;
      dccexProtocol.setThrottle(consist, speed, Direction::Forward);

      int fn = random(0, 27);
      int fns = random(0, 100);
      bool fnState = (fns < 50) ? false : true;

      if (fnState) {
        dccexProtocol.functionOn(consist, fn);
      } else {
        dccexProtocol.functionOff(consist, fn);
      }

      lastTime = millis();
    }
  }
}
