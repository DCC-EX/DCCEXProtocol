// WiThrottleProtocol library: Roster, etc example
//
// Shows how to retrieve the Roster, Turnouts/Points, Routes, Turntables
// Tested with ESP32-WROOM board
//
// Peter Akers (Flash62au), Peter Cole (PeteGSX) and Chris Harlow (UKBloke), 2023
// Luca Dentella, 2020


#include <WiFi.h>
#include <DCCEXProtocol.h>

void printServer();
void printRoster();
void printTurnouts();
void printRoutes();
void printTurntables();

// Delegate class
class MyDelegate : public DCCEXProtocolDelegate {
  
  public:
    void receivedServerDescription(char* version) {   
      Serial.println("\n\nReceived Server Description: ");
      printServer();  
    }

    void receivedTrackPower(TrackPower state) { 
      Serial.print("\n\nReceived Track Power: "); Serial.println(state);  
      Serial.println("\n\n");  
    }

    void receivedRosterList(int rosterSize) {
      Serial.print("\n\nReceived Roster: "); Serial.println(rosterSize);  
      Serial.print("\n\nReceived Roster: "); Serial.println(rosterSize);  
      Serial.print("\n\nReceived Roster: "); Serial.println(rosterSize);  
      Serial.print("\n\nReceived Roster: "); Serial.println(rosterSize);  
      printRoster();
    }
    void receivedTurnoutList(int turnoutListSize) {
      Serial.print("\n\nReceived Turnouts/Points list: "); Serial.println(turnoutListSize);  
      printTurnouts();
      Serial.println("\n\n");  
    }    
    void receivedRouteList(int routeListSize) {
      Serial.print("\n\nReceived Routes List: "); Serial.println(routeListSize);  
      printRoutes();
      Serial.println("\n\n");  
    }
    void receivedTurntableList(int turntablesListSize) {
      Serial.print("\n\nReceived Turntables list: "); Serial.println(turntablesListSize);  
      printTurntables();
      Serial.println("\n\n");  
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

void printRoster() {
  for (Loco* loco=dccexProtocol.roster->getFirst(); loco; loco=loco->getNext()) {
    int id=loco->getAddress();
    char* name=loco->getName();
    Serial.print(id);
    Serial.print(" ~");
    Serial.print(name);
    Serial.println("~");
  }
  Serial.println("\n");  
}

void printTurnouts() {
  for (Turnout* turnout=dccexProtocol.turnouts->getFirst(); turnout; turnout=turnout->getNext()) {
    int id=turnout->getId();
    char* name=turnout->getName();
    Serial.print(id);
    Serial.print(" ~");
    Serial.print(name);
    Serial.println("~");
  }
  Serial.println("\n");  
}

void printRoutes() {
  for (Route* route=dccexProtocol.routes->getFirst(); route; route=route->getNext()) {
    int id=route->getId();
    char* name=route->getName();
    Serial.print(id);
    Serial.print(" ~");
    Serial.print(name);
    Serial.println("~");
  }
  Serial.println("\n");  
}

void printTurntables() {
  for (Turntable* turntable=dccexProtocol.turntables->getFirst(); turntable; turntable=turntable->getNext()) {
    int id=turntable->getId();
    char* name=turntable->getName();
    Serial.print(id);
    Serial.print(" ~");
    Serial.print(name);
    Serial.println("~");

    int j = 0;
    for (TurntableIndex* turntableIndex=turntable->getFirstIndex(); turntableIndex; turntableIndex=turntableIndex->getNext()) {
      char* indexName = turntableIndex->getName();
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
  dccexProtocol.sendTrackPower(PowerOn);

  lastTime = millis();
}
  
void loop() {
  // parse incoming messages
  dccexProtocol.check();

  // sequentially request and get the required lists. To avoid overloading the buffer
  //getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired)
  dccexProtocol.getLists(true, true, true, true);

  // if (!done && dccexProtocol.isServerDetailsReceived()) {
  //   done = true;
  // }

//   if ((millis() - lastTime) >= 10000) {
//     lastTime = millis();
//   }  
}
