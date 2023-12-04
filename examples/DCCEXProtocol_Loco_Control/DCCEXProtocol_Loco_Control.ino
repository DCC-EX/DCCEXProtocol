// WiThrottleProtocol library: Roster example
//
// Shows how to create a delegate class to handle callbacks and retrieve the Roster
// Tested with ESP32-WROOM board
//
// Peter Akers (Flash62au), Peter Cole (PeteGSX) and Chris Harlow (UKBloke), 2023
// Luca Dentella, 2020


#include <WiFi.h>
#include <DCCEXProtocol.h>

// If we haven't got a custom config.h, use the example
#if __has_include ("config.h")
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

    void receivedLocoUpdate(Loco* loco) {
      Serial.print("Received Loco update for DCC address: ");
      Serial.print(loco->getAddress());
    }
    
};

// for random speed changes 
int speed = 0;
int up = 1;
bool done = false;

// Global objects
WiFiClient client;
DCCEXProtocol dccexProtocol;
MyDelegate myDelegate;

void setup() {
  
  Serial.begin(115200);
  Serial.println("DCCEXProtocol Delegate Demo");
  Serial.println();

  // Connect to WiFi network
  Serial.println("Connecting to WiFi.."); 
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) delay(1000);  
  Serial.print("Connected with IP: "); Serial.println(WiFi.localIP());

  // Connect to the server
  Serial.println("Connecting to the server...");
  if (!client.connect(serverAddress, serverPort)) {
    Serial.println("connection failed");
    while(1) delay(1000);
  }
  Serial.println("Connected to the server");

  // Uncomment for logging on Serial
  dccexProtocol.setLogStream(&Serial);

  // Pass the delegate instance to wiThrottleProtocol
  dccexProtocol.setDelegate(&myDelegate);

  // Pass the communication to wiThrottleProtocol
  dccexProtocol.connect(&client);
  Serial.println("DCC-EX connected");

  dccexProtocol.sendServerDetailsRequest();

  dccexProtocol.getRoster();

  lastTime = millis();
}
  
void loop() {
  // parse incoming messages
  dccexProtocol.check();

  if (dccexProtocol.isRosterFullyReceived() && !done ) { // need to wait till the roster loads
    done = true;

    // add a loco to throttle 0 from DCC address 11
    dccexProtocol.throttle[0].addFromEntry(11, FacingForward);

    // alternate method using the loco object
    // Loco loco(11, "dummy loco", LocoSourceEntry);
    // dccexProtocol.throttleConsists[0].consistAddLoco(loco, FacingForward);

    Serial.print("\n\nLocos in Consist: 0 "); Serial.println(dccexProtocol.throttle[0].getLocoCount());
    if (dccexProtocol.getRosterCount()>=2) {
      // add a loco to throttle 1 from the second entry in the roster 
      if (dccexProtocol.getRosterEntryNo(1)!=nullptr) {
        dccexProtocol.throttle[1].addFromRoster(dccexProtocol.getRosterEntryNo(1), FacingForward);
      } else {
        Serial.println("Problem retrieving Roster Entry 1");
      }
      
      Serial.print("\n\nLocos in Consist 1: "); Serial.println(dccexProtocol.throttle[1].getLocoCount());
    }
  }

  if (done) {
    if ((millis() - lastTime) >= 10000) {
      if (speed>=100) up = -1;
      if (speed<=0) up = 1;
      speed = speed + up;
      dccexProtocol.sendThrottleAction(0, speed, Forward);
      dccexProtocol.sendThrottleAction(1, speed, Forward);

      int ttl = random(0, 1);
      int fn = random(0,28);
      int fns = random(0,100);
      bool fnState = (fns<50) ? false : true;

      dccexProtocol.sendFunction(ttl, fn, fnState);

      lastTime = millis();
    }
  }

}
