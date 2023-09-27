// WiThrottleProtocol library: Roster example
//
// Shows how to create a delegate class to handle a consist with multiple locos
// Tested with ESP32-WROOM board
//
// Peter Akers, 2023
// Luca Dentella, 2020


#include <WiFi.h>
#include <DCCEXProtocol.h>

void printRoster();
void printTurnouts();
void printRoutes();
void printTurntables();

// Delegate class
class MyDelegate : public DCCEXProtocolDelegate {
  
  public:
    void receivedServerDescription(String microprocessor, String version) {     
        Serial.print("Received version: "); Serial.println(version);  
    }

    void receivedTrackPower(TrackPower state) { 
      Serial.print("Received Track Power: "); Serial.println(state);  
    }

    void receivedRosterList(int rosterSize) {
      Serial.print("Received Roster: "); Serial.println(rosterSize);  
    }
    void receivedTurnoutList(int turnoutListSize) {
      Serial.print("Received Turnout List: "); Serial.println(turnoutListSize); 
    }    
    void receivedRouteList(int routeListSize) {
        Serial.print("Received Route List: "); Serial.println(routeListSize); 
    }
    void receivedTurntablesList(int turntablesListSize) {
        Serial.print("Received Turnout List: "); Serial.println(turntablesListSize); 
    }  


    void receivedSpeed(int throttleNo, int speed) { 
        Serial.print("Received Throttle Speed: "); Serial.print(throttleNo); Serial.print(" Speed: "); Serial.println(speed); 
    }
    void receivedDirection(int throttleNo, Direction dir) { 
        Serial.print("Received Throttle Direction: "); Serial.print(throttleNo); Serial.print(" Direction: "); Serial.println(dir); 
    }
    void receivedFunction(int throttleNo, int func, bool state) { 
        Serial.print("Received Throttle Function change: "); Serial.print(throttleNo); Serial.print(" function: "); Serial.print(func); Serial.print(" state: "); Serial.println(state);
    }

};

// WiFi and server configuration
// const char* ssid = "MySSID";
// const char* password =  "MyPWD";
const char* ssid = "DCCEX_44182a";
const char* password =  "PASS_44182a";
IPAddress serverAddress(192,168,4,1);
int serverPort = 2560;
unsigned long lastTime = 0;

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

  if ((millis()) >= 20000  && !done)   { // need to wait till the roster laods
    done = true;

    // add a loco to throttle 0 from DCC address 11
    Loco loco(11, "dummy loco", LocoSourceEntry);
    dccexProtocol.throttleConsists[0].consistAddLoco(loco, FacingForward);
    Serial.print("\n\nLocos in Consist: 0 "); Serial.println(dccexProtocol.throttleConsists[0].consistGetNumberOfLocos());

    // add a loco to throttle 0 from DCC address 12
    Loco loco2 = Loco(12, "dummy loco 2", LocoSourceEntry);
    dccexProtocol.throttleConsists[0].consistAddLoco(loco2, FacingForward);
    Serial.print("\n\nLocos in Consist: 0 "); Serial.println(dccexProtocol.throttleConsists[0].consistGetNumberOfLocos());

    // add a loco to throttle 0 from DCC address 13
    Loco loco3 = Loco(13, "dummy loco 3", LocoSourceEntry);
    dccexProtocol.throttleConsists[0].consistAddLoco(loco3, FacingReversed);
    Serial.print("\n\nLocos in Consist: 0 "); Serial.println(dccexProtocol.throttleConsists[0].consistGetNumberOfLocos());

  }

  if ((millis() - lastTime) >= 20000) {
    if (speed>=100) up = -1;
    if (speed<=0) up = 1;
    speed = speed + up;
    dccexProtocol.sendThrottleAction(0, speed, Forward);

    int ttl = random(0, 1);
    int fn = random(0,28);
    int fnState = random(0,2);

    dccexProtocol.sendFunction(ttl, fn, fnState);

    lastTime = millis();
  }

}
