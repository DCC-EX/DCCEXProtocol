// WiThrottleProtocol library: Roster example
//
// Shows how to create a delegate class to handle a consist with multiple locos
// Tested with ESP32-WROOM board
//
// Peter Akers (Flash62au), Peter Cole (PeteGSX) and Chris (UKBloke), 2023
// Luca Dentella, 2020


#include <WiFi.h>
#include <DCCEXProtocol.h>

void printServer();
void checkFunction(int throttleNo, int func);

// Delegate class
class MyDelegate : public DCCEXProtocolDelegate {
  
  public:
    void receivedServerDescription(char* version) {   
      Serial.println("\n\nReceived Server Description: ");
      printServer();  
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
        checkFunction(throttleNo, func);
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
Direction dir = Forward;
bool _fnState;
bool done = false;

// Global objects
WiFiClient client;
DCCEXProtocol dccexProtocol(3); // three throttles
MyDelegate myDelegate;

void printServer() {
  Serial.print("  Server Version:  "); Serial.println(dccexProtocol.serverVersion);
  Serial.print("  Server MP Type:  "); Serial.println(dccexProtocol.serverMicroprocessorType);
  Serial.print("  Server MC Type:  "); Serial.println(dccexProtocol.serverMotorcontrollerType);
  Serial.print("  Server Build No: "); Serial.println(dccexProtocol.serverBuildNumber);
  Serial.println("\n\n");  
}

void checkFunction(int throttleNo, int func) {
  Serial.print("Function "); Serial.print(func); Serial.print(": ");
  if (dccexProtocol.isFunctionOn(throttleNo, func)) {
    Serial.println("is on");
  } else {
    Serial.println("is off");
  }
}

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

  // dccexProtocol.getRoster();

  lastTime = millis();
}
  
void loop() {
  // parse incoming messages
  dccexProtocol.check();

  // sequentially request and get the required lists. To avoid overloading the buffer
  //getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired)
  dccexProtocol.getLists(true, false, false, false);

  if (dccexProtocol.isRosterFullyReceived() && !done ) { // need to wait till the roster loads
    done = true;

    // add a loco to throttle 0 from DCC address 11
    dccexProtocol.throttle[0].addFromEntry(11, FacingForward);
    Serial.print("Locos in Consist: 0 "); Serial.println(dccexProtocol.throttle[0].getLocoCount());

    // add a loco to throttle 0 from DCC address 12
    dccexProtocol.throttle[0].addFromEntry(12, FacingForward);
    Serial.print("Locos in Consist: 0 "); Serial.println(dccexProtocol.throttle[0].getLocoCount());

    // add a loco to throttle 0 from DCC address 13
    dccexProtocol.throttle[0].addFromEntry(13, FacingReversed);
    Serial.print("Locos in Consist: 0 "); Serial.println(dccexProtocol.throttle[0].getLocoCount());

  }

  if ((millis() - lastTime) >= 10000) {
    if (speed>=100) {
      up = -10;
    }
    if (speed<=0) {
      up = 10;
      if (dir==Forward) {
        dir = Reverse;
      } else { 
        dir = Forward;
      }
    }
    speed = speed + up;
    dccexProtocol.sendThrottleAction(0, speed, dir);

    int _ttl = random(0, 1);
    int _fn = random(0,28);
    int _fns = random(0,100);
    _fnState = (_fns<50) ? false : true;

    dccexProtocol.sendFunction(_ttl, _fn, _fnState);

    lastTime = millis();
  }

}
