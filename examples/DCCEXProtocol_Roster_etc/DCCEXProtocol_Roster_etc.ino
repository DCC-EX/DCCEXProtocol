// WiThrottleProtocol library: Roster and other objects example
//
// Shows how to retrieve the Roster, Turnouts/Points, Routes, Turntables
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

void printRoster();
void printTurnouts();
void printRoutes();
void printTurntables();

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

  void receivedRosterList() override {
    Serial.println("\n\nReceived Roster");
    printRoster();
  }
  void receivedTurnoutList() override {
    Serial.print("\n\nReceived Turnouts/Points list");
    printTurnouts();
    Serial.println("\n\n");
  }
  void receivedRouteList() override {
    Serial.print("\n\nReceived Routes List");
    printRoutes();
    Serial.println("\n\n");
  }
  void receivedTurntableList() override {
    Serial.print("\n\nReceived Turntables list");
    printTurntables();
    Serial.println("\n\n");
  }
};

unsigned long lastTime = 0;

bool done = false;

// Global objects
WiFiClient client;
DCCEXProtocol dccexProtocol;
MyDelegate myDelegate;

void printRoster() {
  for (Loco *loco = dccexProtocol.roster->getFirst(); loco; loco = loco->getNext()) {
    int id = loco->getAddress();
    const char *name = loco->getName();
    Serial.print(id);
    Serial.print(" ~");
    Serial.print(name);
    Serial.println("~");
    for (int i = 0; i < 32; i++) {
      const char *fName = loco->getFunctionName(i);
      if (fName != nullptr) {
        Serial.print("loadFunctionLabels() ");
        Serial.print(fName);
        if (loco->isFunctionMomentary(i)) {
          Serial.print(" - Momentary");
        }
        Serial.println();
      }
    }
  }
  Serial.println("\n");
}

void printTurnouts() {
  for (Turnout *turnout = dccexProtocol.turnouts->getFirst(); turnout; turnout = turnout->getNext()) {
    int id = turnout->getId();
    const char *name = turnout->getName();
    Serial.print(id);
    Serial.print(" ~");
    Serial.print(name);
    Serial.println("~");
  }
  Serial.println("\n");
}

void printRoutes() {
  for (Route *route = dccexProtocol.routes->getFirst(); route; route = route->getNext()) {
    int id = route->getId();
    const char *name = route->getName();
    Serial.print(id);
    Serial.print(" ~");
    Serial.print(name);
    Serial.println("~");
  }
  Serial.println("\n");
}

void printTurntables() {
  for (Turntable *turntable = dccexProtocol.turntables->getFirst(); turntable; turntable = turntable->getNext()) {
    int id = turntable->getId();
    const char *name = turntable->getName();
    Serial.print(id);
    Serial.print(" ~");
    Serial.print(name);
    Serial.println("~");

    int j = 0;
    for (TurntableIndex *turntableIndex = turntable->getFirstIndex(); turntableIndex;
         turntableIndex = turntableIndex->getNextIndex()) {
      const char *indexName = turntableIndex->getName();
      Serial.print("  index");
      Serial.print(j);
      Serial.print(" ~");
      Serial.print(indexName);
      Serial.println("~");
      j++;
    }
  }
  Serial.println("\n");
}

void setup() {

  Serial.begin(115200);
  Serial.println("DCCEXProtocol Roster and Objects Demo");
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

  // Enable logging on Serial
  dccexProtocol.setLogStream(&Serial);

  // Pass the delegate instance to wiThrottleProtocol
  dccexProtocol.setDelegate(&myDelegate);

  // Pass the communication to wiThrottleProtocol
  dccexProtocol.connect(&client);
  Serial.println("DCC-EX connected");

  dccexProtocol.requestServerVersion();
  dccexProtocol.powerOn();
}

void loop() {
  // parse incoming messages
  dccexProtocol.check();

  // sequentially request and get the required lists. To avoid overloading the buffer
  // getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired)
  dccexProtocol.getLists(true, true, true, true);
}
