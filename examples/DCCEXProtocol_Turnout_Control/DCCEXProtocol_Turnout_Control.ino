// WiThrottleProtocol library: Turnout/Point control example
//
// Shows how to retrieve the Turnouts/Points list and control them
// Tested with ESP32-WROOM board
//
// NOTE: You must have at least two turnouts defined in your EX-CommandStation
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

void printTurnouts();

// Delegate class
class MyDelegate : public DCCEXProtocolDelegate {

public:
  void receivedServerVersion(int major, int minor, int patch) override {
    Serial.print("Received version: ");
    Serial.print(major);
    Serial.print(".");
    Serial.print(minor);
    Serial.print(".");
    Serial.println(patch);
  }

  void receivedTurnoutList() override {
    Serial.print("Received turnout list:");
    printTurnouts();
  }

  void receivedTurnoutAction(int turnoutId, bool thrown) override {
    Serial.print("Received turnout action ID|thrown: ");
    Serial.print(turnoutId);
    Serial.print("|");
    Serial.println(thrown);
  }
};

unsigned long lastTime = 0;

bool doneTurnouts = false;
bool doneRoutes = false;
Turnout *turnout1 = nullptr;
Turnout *turnout2 = nullptr;

// Global objects
WiFiClient client;
DCCEXProtocol dccexProtocol;
MyDelegate myDelegate;

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

void setup() {

  Serial.begin(115200);
  Serial.println("DCCEXProtocol Turnout/Point Demo");
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
}

void loop() {
  // parse incoming messages
  dccexProtocol.check();

  // sequentially request and get the required lists. To avoid overloading the buffer
  dccexProtocol.getLists(false, true, false, false);

  if (dccexProtocol.receivedLists() && !doneTurnouts) {
    if (dccexProtocol.getTurnoutCount() >= 2) {
      turnout1 = dccexProtocol.turnouts->getFirst();
      Serial.print("Turnout 1 id: ");
      Serial.println(turnout1->getId());
      turnout2 = turnout1->getNext();
      Serial.print("Turnout 2 id: ");
      Serial.println(turnout2->getId());
    }
    doneTurnouts = true;
  }

  if ((millis() - lastTime) >= 10000) {
    if (doneTurnouts) {
      int action = random(0, 100);
      bool throwTurnout = (action > 50) ? 1 : 0;
      if (throwTurnout) {
        dccexProtocol.throwTurnout(turnout1->getId());
      } else {
        dccexProtocol.closeTurnout(turnout1->getId());
      }
      action = random(0, 100);
      throwTurnout = (action > 50) ? 1 : 0;
      if (throwTurnout) {
        dccexProtocol.throwTurnout(turnout2->getId());
      } else {
        dccexProtocol.closeTurnout(turnout2->getId());
      }
    }

    lastTime = millis();
  }
}
