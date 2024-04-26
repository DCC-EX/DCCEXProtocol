// DCCEXProtocol_Serial
//
// Shows how to use DCCEXProtocol over serial using a Mega2560
// This uses a Mega2560 (or other AVR device) that has a second hardware serial port (Serial1)
// Tested with Arduino Mega2560
//
// Peter Cole (PeteGSX) 2024

#include <DCCEXProtocol.h>

// If we haven't got a custom config.h, use the example
#if __has_include("config.h")
#include "config.h"
#else
#warning config.h not found. Using defaults from config.example.h
#include "config.example.h"
#endif

// Setup serial macros if not already defined elsewhere
#ifndef CONSOLE
#define CONSOLE Serial    // All output should go to this
#endif
#ifndef CLIENT
#define CLIENT Serial1    // All DCCEXProtocol commands/responses/broadcasts use this
#endif

// Declare functions to call from our delegate
void printRoster();
void printTurnouts();
void printRoutes();
void printTurntables();

// Setup the delegate class to process broadcasts/responses
class MyDelegate : public DCCEXProtocolDelegate {
public:
  void receivedServerVersion(int major, int minor, int patch) {
    Serial.print("\n\nReceived version: ");
    Serial.print(major);
    Serial.print(".");
    Serial.print(minor);
    Serial.print(".");
    Serial.println(patch);
  }

  void receivedTrackPower(TrackPower state) {
    Serial.print("\n\nReceived Track Power: ");
    Serial.println(state);
    Serial.println("\n\n");
  }

  void receivedRosterList() {
    Serial.println("\n\nReceived Roster");
    printRoster();
  }
  void receivedTurnoutList() {
    Serial.print("\n\nReceived Turnouts/Points list");
    printTurnouts();
    Serial.println("\n\n");
  }
  void receivedRouteList() {
    Serial.print("\n\nReceived Routes List");
    printRoutes();
    Serial.println("\n\n");
  }
  void receivedTurntableList() {
    Serial.print("\n\nReceived Turntables list");
    printTurntables();
    Serial.println("\n\n");
  }

};

// Global objects
DCCEXProtocol dccexProtocol;
MyDelegate myDelegate;

void printRoster() {
  for (Loco *loco = dccexProtocol.roster->getFirst(); loco; loco = loco->getNext()) {
    int id = loco->getAddress();
    char *name = loco->getName();
    Serial.print(id);
    Serial.print(" ~");
    Serial.print(name);
    Serial.println("~");
    for (int i = 0; i < 32; i++) {
      char *fName = loco->getFunctionName(i);
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
    char *name = turnout->getName();
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
    char *name = route->getName();
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
    char *name = turntable->getName();
    Serial.print(id);
    Serial.print(" ~");
    Serial.print(name);
    Serial.println("~");

    int j = 0;
    for (TurntableIndex *turntableIndex = turntable->getFirstIndex(); turntableIndex;
         turntableIndex = turntableIndex->getNextIndex()) {
      char *indexName = turntableIndex->getName();
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
  CONSOLE.begin(115200);
  CLIENT.begin(115200);
  CONSOLE.println(F("DCCEXProtocol Serial Connection Demo"));
  CONSOLE.println(F(""));

  // Direct logs to CONSOLE
  dccexProtocol.setLogStream(&CONSOLE);

  // Set the delegate for broadcasts/responses
  dccexProtocol.setDelegate(&myDelegate);

  // Connect to the CS via CLIENT
  dccexProtocol.connect(&CLIENT);
  CONSOLE.println(F("DCC-EX connected"));

  dccexProtocol.requestServerVersion();
}

void loop() {
  // Parse incoming messages
  dccexProtocol.check();

  dccexProtocol.getLists(true, true, true, true);
}
