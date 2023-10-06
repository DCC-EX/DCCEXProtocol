// WiThrottleProtocol library: Roster, etc example
//
// Shows how to retrieve the Roster, Turnouts/Points, Routes, Turntables
// Tested with ESP32-WROOM board
//
// Peter Akers, 2023
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

// Global objects
WiFiClient client;
DCCEXProtocol dccexProtocol;
MyDelegate myDelegate;
bool done = false;

void printServer() {
  Serial.print("  Server Version:  "); Serial.println(dccexProtocol.serverVersion);
  Serial.print("  Server MP Type:  "); Serial.println(dccexProtocol.serverMicroprocessorType);
  Serial.print("  Server MC Type:  "); Serial.println(dccexProtocol.serverMotorcontrollerType);
  Serial.print("  Server Build No: "); Serial.println(dccexProtocol.serverBuildNumber);
  Serial.println("\n\n");  
}

void printRoster() {
  for (int i=0; i<dccexProtocol.roster.size(); i++) {
    Loco* loco = dccexProtocol.roster.get(i);
    int address = loco->getLocoAddress();
    char* name = loco->getLocoName();
    Serial.print(address); Serial.print(" ~"); Serial.print(name); Serial.println("~");  
    Functions fns = loco->locoFunctions;
    for (int j=0; j<MAX_FUNCTIONS; j++) {
      char* functionName = fns.getFunctionName(j);
      Serial.print("  fn"); Serial.print(j); Serial.print(" ~"); Serial.print(functionName); Serial.print("~");  
    }
    Serial.println("\n");  
  }
  Serial.println("\n");  
}

void printTurnouts() {
  for (int i=0; i<dccexProtocol.turnouts.size(); i++) {
    Turnout* turnout = dccexProtocol.turnouts.get(i);
    int id = turnout->getTurnoutId();
    char* name = turnout->getTurnoutName();
    Serial.print(id); Serial.print(" ~"); Serial.print(name); Serial.println("~");  
  }
  Serial.println("\n");  
}

void printRoutes() {
  for (int i=0; i<dccexProtocol.routes.size(); i++) {
    Route* route = dccexProtocol.routes.get(i);
    int id = route->getRouteId();
    char* name = route->getRouteName();
    Serial.print(id); Serial.print(" ~"); Serial.print(name); Serial.println("~");  
  }
  Serial.println("\n");  
}

void printTurntables() {
  for (int i=0; i<dccexProtocol.turntables.size(); i++) {
    Turntable* turntable = dccexProtocol.turntables.get(i);
    int id = turntable->getTurntableId();
    char* name = turntable->getTurntableName();
    Serial.print(id); Serial.print(" ~"); Serial.print(name); Serial.println("~"); 
    for (int j=0; j<turntable->getTurntableNumberOfIndexes(); j++) {
      TurntableIndex* turntableIndex = turntable->turntableIndexes.get(j);
      char* indexName = turntableIndex->getTurntableIndexName();
      Serial.print("  index"); Serial.print(j); Serial.print(" ~"); Serial.print(indexName); Serial.println("~");  
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
  // dccexProtocol.setLogStream(&Serial);

  // Pass the delegate instance to wiThrottleProtocol
  dccexProtocol.setDelegate(&myDelegate);

  // Pass the communication to wiThrottleProtocol
  dccexProtocol.connect(&client);
  Serial.println("DCC-EX connected");

  dccexProtocol.sendServerDetailsRequest();
  delay(1000);
  dccexProtocol.getRoster();
  delay(1000);
  dccexProtocol.getTurnouts();
  delay(1000);
  dccexProtocol.getRoutes();
  delay(1000);
  dccexProtocol.getTurntables();

  lastTime = millis();
}
  
void loop() {
  // parse incoming messages
  dccexProtocol.check();

  // if (!done && dccexProtocol.isServerDetailsReceived()) {
  //   done = true;
  // }

//   if ((millis() - lastTime) >= 10000) {
//     lastTime = millis();
//   }  
}
