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
  void receivedServerVersion(int major, int minor, int patch) override {
    CONSOLE.print("\n\nReceived version: ");
    CONSOLE.print(major);
    CONSOLE.print(".");
    CONSOLE.print(minor);
    CONSOLE.print(".");
    CONSOLE.println(patch);
  }

  void receivedTrackPower(TrackPower state) override {
    CONSOLE.print("\n\nReceived Track Power: ");
    CONSOLE.println(state);
    CONSOLE.println("\n\n");
  }

  void receivedRosterList() override {
    CONSOLE.println("\n\nReceived Roster");
    printRoster();
  }
  void receivedTurnoutList() override {
    CONSOLE.print("\n\nReceived Turnouts/Points list");
    printTurnouts();
    CONSOLE.println("\n\n");
  }
  void receivedRouteList() override {
    CONSOLE.print("\n\nReceived Routes List");
    printRoutes();
    CONSOLE.println("\n\n");
  }
  void receivedTurntableList() override {
    CONSOLE.print("\n\nReceived Turntables list");
    printTurntables();
    CONSOLE.println("\n\n");
  }

  void receivedScreenUpdate(int screen, int row, char *message) override {
    CONSOLE.println("\n\nReceived screen|row|message");
    CONSOLE.print(screen);
    CONSOLE.print("|");
    CONSOLE.print(row);
    CONSOLE.print("|");
    CONSOLE.println(message);
  }

};

// Global objects
DCCEXProtocol dccexProtocol;
MyDelegate myDelegate;

void printRoster() {
  for (Loco *loco = dccexProtocol.roster->getFirst(); loco; loco = loco->getNext()) {
    int id = loco->getAddress();
    const char *name = loco->getName();
    CONSOLE.print(id);
    CONSOLE.print(" ~");
    CONSOLE.print(name);
    CONSOLE.println("~");
    for (int i = 0; i < 32; i++) {
      const char *fName = loco->getFunctionName(i);
      if (fName != nullptr) {
        CONSOLE.print("loadFunctionLabels() ");
        CONSOLE.print(fName);
        if (loco->isFunctionMomentary(i)) {
          CONSOLE.print(" - Momentary");
        }
        CONSOLE.println();
      }
    }
  }
  CONSOLE.println("\n");
}

void printTurnouts() {
  for (Turnout *turnout = dccexProtocol.turnouts->getFirst(); turnout; turnout = turnout->getNext()) {
    int id = turnout->getId();
    const char *name = turnout->getName();
    CONSOLE.print(id);
    CONSOLE.print(" ~");
    CONSOLE.print(name);
    CONSOLE.println("~");
  }
  CONSOLE.println("\n");
}

void printRoutes() {
  for (Route *route = dccexProtocol.routes->getFirst(); route; route = route->getNext()) {
    int id = route->getId();
    const char *name = route->getName();
    CONSOLE.print(id);
    CONSOLE.print(" ~");
    CONSOLE.print(name);
    CONSOLE.println("~");
  }
  CONSOLE.println("\n");
}

void printTurntables() {
  for (Turntable *turntable = dccexProtocol.turntables->getFirst(); turntable; turntable = turntable->getNext()) {
    int id = turntable->getId();
    const char *name = turntable->getName();
    CONSOLE.print(id);
    CONSOLE.print(" ~");
    CONSOLE.print(name);
    CONSOLE.println("~");

    int j = 0;
    for (TurntableIndex *turntableIndex = turntable->getFirstIndex(); turntableIndex;
         turntableIndex = turntableIndex->getNextIndex()) {
      const char *indexName = turntableIndex->getName();
      CONSOLE.print("  index");
      CONSOLE.print(j);
      CONSOLE.print(" ~");
      CONSOLE.print(indexName);
      CONSOLE.println("~");
      j++;
    }
  }
  CONSOLE.println("\n");
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
