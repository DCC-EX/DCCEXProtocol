// WiThrottleProtocol library: SSID example
//
// Shows how to retrieve the list of discovered SSID (Networks)
// Tested with ESP32-WROOM board
//
// Peter Akers (Flash62au), Peter Cole (PeteGSX) and Chris Harlow (UKBloke), 2023

#include <ESPmDNS.h>
#include <WiFi.h>

void printSsids();

#define MAX_SSIDS 20

String foundSsids[MAX_SSIDS];
long foundSsidRssis[MAX_SSIDS];
boolean foundSsidsOpen[MAX_SSIDS];
int foundSsidsCount;

unsigned long lastTime = 0;

bool mdnsListenerStarted = false;

// Global objects
WiFiClient client;

void printSsids() {
  Serial.println("");
  Serial.println("Browsing for SSIDs ");

  double startTime = millis();
  double nowTime = startTime;

  int numSsids = WiFi.scanNetworks();
  while ((numSsids == -1) && ((nowTime - startTime) <= 10000)) { // try for 10 seconds
    delay(250);
    Serial.print(".");
    nowTime = millis();
  }

  int foundSsidsCount = 0;
  if (numSsids == -1) {
    Serial.println("Couldn't get a wifi connection");

  } else {
    for (int thisSsid = 0; thisSsid < numSsids; thisSsid++) {
      /// remove duplicates (repeaters and mesh networks)
      boolean found = false;
      for (int i = 0; i < foundSsidsCount && i < MAX_SSIDS; i++) {
        if (WiFi.SSID(thisSsid) == foundSsids[i]) {
          found = true;
          break;
        }
      }
      if (!found) {
        foundSsids[foundSsidsCount] = WiFi.SSID(thisSsid);
        foundSsidRssis[foundSsidsCount] = WiFi.RSSI(thisSsid);
        foundSsidsOpen[foundSsidsCount] = (WiFi.encryptionType(thisSsid) == 7) ? true : false;
        foundSsidsCount++;
      }
    }
    for (int i = 0; i < foundSsidsCount; i++) {
      Serial.println(foundSsids[i]);
    }
  }
}

void setup() {

  Serial.begin(115200);
  Serial.println("DCCEXProtocol SSID example");
  Serial.println();

  // browse for SSIDs
  printSsids();
}

void loop() {

  delay(20000);
  // Redo every 20 seconds - For demonstration purposes only!
  // Normally this will only be required once.
  printSsids();
}
