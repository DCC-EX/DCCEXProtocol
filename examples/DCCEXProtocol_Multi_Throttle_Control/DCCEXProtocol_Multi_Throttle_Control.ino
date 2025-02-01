// WiThrottleProtocol library: Multi throttle control example
//
// Shows how to control locos with multiple client throttles
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

// Example class for a throttle
// You would associate a rotary encoder or similar with this class
// for input and control of speed/direction
class Throttle {
public:
  Throttle(DCCEXProtocol *dccexProtocol) { _dccexProtocol = dccexProtocol; }

  void setLoco(Loco *loco) { _loco = loco; }

  Loco *getLoco() { return _loco; }

  void setSpeed(int speed) {
    if (!_loco)
      return;
    _dccexProtocol->setThrottle(_loco, speed, _loco->getDirection());
  }

  void setDirection(Direction direction) {
    if (!_loco)
      return;
    _dccexProtocol->setThrottle(_loco, _loco->getSpeed(), direction);
  }

  void process() {
    // Routine calls here included in the loop to read encoder or other inputs
  }

private:
  DCCEXProtocol *_dccexProtocol;
  Loco *_loco;
};

// for random speed changes
int speed = 0;
int up = 1;
unsigned long lastTime = 0;

// Global objects
WiFiClient client;
DCCEXProtocol dccexProtocol;
MyDelegate myDelegate;

// Define an array for two throttles
const int numThrottles = 2;
Throttle *throttles[numThrottles];

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

  // Pass the communication to wiThrottleProtocol
  dccexProtocol.connect(&client);
  Serial.println("DCC-EX connected");

  dccexProtocol.requestServerVersion();

  lastTime = millis();

  // Dummy loco starting address
  int address = 12;

  // Create the throttles and add a loco to each
  for (int i = 0; i < numThrottles; i++) {
    Serial.print("Create throttle|loco address: ");
    Serial.print(i);
    Serial.print("|");
    Serial.println(address + i);
    throttles[i] = new Throttle(&dccexProtocol);
    throttles[i]->setLoco(new Loco(address + i, LocoSource::LocoSourceEntry));
  }

  // Turn track power on so locos can move
  dccexProtocol.powerOn();
}

void loop() {
  // parse incoming messages
  dccexProtocol.check();

  // throttle processing example
  for (int i = 0; i < numThrottles; i++) {
    throttles[i]->process();
  }

  // every 10 seconds change speed and set a random function on or off
  if ((millis() - lastTime) >= 10000) {
    lastTime = millis();
    for (int i = 0; i < numThrottles; i++) {
      auto th = throttles[i];
      Loco *loco = th->getLoco();
      if (loco) {
        if (speed >= 100)
          up = -1;
        if (speed <= 0)
          up = 1;
        speed = speed + up;
        dccexProtocol.setThrottle(loco, speed, Direction::Forward);

        int fn = random(0, 27);
        int fns = random(0, 100);
        bool fnState = (fns < 50) ? false : true;

        if (fnState) {
          dccexProtocol.functionOn(loco, fn);
        } else {
          dccexProtocol.functionOff(loco, fn);
        }
      }
    }
  }
}
