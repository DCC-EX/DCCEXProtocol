// WiThrottleProtocol library: mDNS example
//
// Shows how to retrieve the list of discovered servers
// Tested with ESP32-WROOM board
//
// Peter Akers (Flash62au), Peter Cole (PeteGSX) and Chris Harlow (UKBloke), 2023

#include <ESPmDNS.h>
#include <WiFi.h>

// If we haven't got a custom config.h, use the example
#if __has_include("config.h")
#include "config.h"
#else
#warning config.h not found. Using defaults from config.example.h
#include "config.example.h"
#endif

void printMdnsServers();

#define MAX_SERVERS 20

IPAddress foundWitServersIPs[MAX_SERVERS];
int foundWitServersPorts[MAX_SERVERS];
String foundWitServersNames[MAX_SERVERS];
int foundWitServersCount;

unsigned long lastTime = 0;

bool mdnsListenerStarted = false;

// Global objects
WiFiClient client;

bool setupMdnsListner() {
  // setup the bonjour listener

  if (!MDNS.begin("mDNSTest")) {
    Serial.println("Error setting up MDNS responder!");
    return false;
  } else {
    Serial.println("MDNS responder started");
    return true;
  }
}

void printMdnsServers() {
  Serial.println("");

  double startTime = millis();
  double nowTime = startTime;

  const char *service = "withrottle";
  const char *proto = "tcp";

  Serial.print("Browsing for service ");
  Serial.print(service);
  Serial.print(".");
  Serial.print(proto);
  Serial.print(".local. on ");
  Serial.print(ssid);
  Serial.println(" ... ");

  int noOfWitServices = 0;
  while ((noOfWitServices == 0) && ((nowTime - startTime) <= 5000)) { // try for 5 seconds
    noOfWitServices = MDNS.queryService(service, proto);
    if (noOfWitServices == 0) {
      delay(500);
      Serial.print(".");
    }
    nowTime = millis();
  }
  Serial.println("");

  if (noOfWitServices > 0) {
    for (int i = 0; ((i < noOfWitServices) && (i < MAX_SERVERS)); ++i) {
      foundWitServersNames[i] = MDNS.hostname(i);
      foundWitServersIPs[i] = MDNS.IP(i);
      foundWitServersPorts[i] = MDNS.port(i);
      if (MDNS.hasTxt(i, "jmri")) {
        String node = MDNS.txt(i, "node");
        node.toLowerCase();
        if (foundWitServersNames[i].equals(node)) {
          foundWitServersNames[i] = "JMRI  (v" + MDNS.txt(i, "jmri") + ")";
        }
      }
    }
  }
  foundWitServersCount = noOfWitServices;

  // EX-CommnadStations in Access Point mode cannot advertise via mDNS,
  // so we have to guess it based on the SSID name
  String ssidString = String(ssid);
  if (ssidString == "DCCEX_") {
    foundWitServersIPs[foundWitServersCount].fromString("192.168.4.1");
    foundWitServersPorts[foundWitServersCount] = 2560;
    foundWitServersNames[foundWitServersCount] = "'Guessed' EX-CS WiT server";
    foundWitServersCount++;
  }

  for (int i = 0; ((i < foundWitServersCount) && (i < MAX_SERVERS)); ++i) {
    Serial.print("Name: ");
    Serial.print(foundWitServersNames[i]);
    Serial.print(" IP: ");
    Serial.print(foundWitServersIPs[i]);
    Serial.print(" : ");
    Serial.print(foundWitServersPorts[i]);
    Serial.println();
  }
}

void setup() {

  Serial.begin(115200);
  Serial.println("DCCEXProtocol mDNS example");
  Serial.println();

  // Connect to WiFi network
  Serial.println("Connecting to WiFi..");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
    delay(1000);
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // setup the mDNS listner
  mdnsListenerStarted = setupMdnsListner();

  // browse for services
  if (mdnsListenerStarted) {
    printMdnsServers();
  }
}

void loop() {

  delay(20000);
  // Redo every 20 seconds - For demonstration purposes only!
  // Normally this will only be required once, immediately after you connect to the ssid.
  if (mdnsListenerStarted) {
    printMdnsServers();
  }
}
