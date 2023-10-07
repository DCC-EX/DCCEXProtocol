// WiThrottleProtocol library: Roster example
//
// Shows how to create a delegate class to handle a consist with multiple locos
// Tested with ESP32-WROOM board
//
// Peter Akers, 2023
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
    void receivedFunction(int throttleNo, int func, FunctionState state) { 
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
FunctionState _fnState;
bool done = false;

// Global objects
WiFiClient client;
DCCEXProtocol dccexProtocol;
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
  // dccexProtocol.setLogStream(&Serial);

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
    char loco1name[] = "dummy loco 1";
    Loco loco(11, loco1name, LocoSourceEntry);
    dccexProtocol.throttleConsists[0].consistAddLoco(loco, FacingForward);
    Serial.print("Locos in Consist: 0 "); Serial.println(dccexProtocol.throttleConsists[0].consistGetNumberOfLocos());

    // add a loco to throttle 0 from DCC address 12
    char loco2name[] = "dummy loco 2";
    Loco loco2 = Loco(12, loco2name, LocoSourceEntry);
    dccexProtocol.throttleConsists[0].consistAddLoco(loco2, FacingForward);
    Serial.print("Locos in Consist: 0 "); Serial.println(dccexProtocol.throttleConsists[0].consistGetNumberOfLocos());

    // add a loco to throttle 0 from DCC address 13
    char loco3name[] = "dummy loco 3";
    Loco loco3 = Loco(13, loco3name, LocoSourceEntry);
    dccexProtocol.throttleConsists[0].consistAddLoco(loco3, FacingReversed);
    Serial.print("Locos in Consist: 0 "); Serial.println(dccexProtocol.throttleConsists[0].consistGetNumberOfLocos());

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

    int ttl = random(0, 1);
    int fn = random(0,28);
    int _fns = random(0,100);
    if (_fns>=50) {_fnState=FunctionStateOn;}  else {_fnState=FunctionStateOff; }

    dccexProtocol.sendFunction(ttl, fn, _fnState);

    lastTime = millis();
  }

}
