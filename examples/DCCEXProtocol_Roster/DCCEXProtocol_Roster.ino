// WiThrottleProtocol library: Roster example
//
// Shows how to create a delegate class to handle callbacks and retrieve the Roster
// Tested with ESP32-WROOM board
//
// Peter Akers, 2023
// Luca Dentella, 2020


#include <WiFi.h>
#include <DCCEXProtocol.h>

// Delegate class
class MyDelegate : public DCCEXProtocolDelegate {
  
  public:
    void receivedServerDescription(String microprocessor, String version) {     
        Serial.print("Received version: "); Serial.println(version);  
    }

    void receivedTrackPower(TrackPower state) { 
      Serial.print("Received Track Power: "); Serial.println(state);  
    }

    virtual void receivedRosterList(int rosterSize) {
        Serial.print("Received Roster: "); Serial.println(rosterSize);  
    }
    virtual void receivedTurnoutList(int turnoutListSize) {
        Serial.print("Received Turnout List: "); Serial.println(turnoutListSize); 
    }    
    virtual void receivedRouteList(int routeListSize) {
        Serial.print("Received Route List: "); Serial.println(routeListSize); 
    }
    virtual void receivedTurntablesList(int turntablesListSize) {
        Serial.print("Received Turnout List: "); Serial.println(turntablesListSize); 
    }  
};

// WiFi and server configuration
// const char* ssid = "MySSID";
// const char* password =  "MyPWD";
const char* ssid = "DCCEX_1ec919";
const char* password =  "PASS_1ec919";
IPAddress serverAddress(192,168,4,1);
int serverPort = 2560;

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
  dccexProtocol.getTurnouts();
  dccexProtocol.getRoutes();
  dccexProtocol.getTurntables();
}
  
void loop() {
  // parse incoming messages
  dccexProtocol.check();
}
