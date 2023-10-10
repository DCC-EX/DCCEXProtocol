// WiThrottleProtocol library: Turnout/Point control example
//
// Shows how to retrieve the Turnouts/Points list and control them
// Tested with ESP32-WROOM board
//
// Peter Akers, 2023
// Luca Dentella, 2020


#include <WiFi.h>
#include <DCCEXProtocol.h>

void printServer();
void printTurnouts();
void printRoutes();

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
      Serial.println("\n\n");  
    }    

    void receivedTurnoutAction(int turnoutId, TurnoutState state) { 
      Serial.print("Received Turnout Action: Id: "); Serial.print(turnoutId); Serial.print(" state: ");Serial.println(state);  
      // Serial.println("\n");  
    }

    void receivedRouteAction(int routeId, RouteState state) { 
      Serial.print("Received Route Action: Id: "); Serial.print(routeId); Serial.print(" state: ");Serial.println(state);  
      // Serial.println("\n");
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

bool doneTurnouts = false;
bool doneRoutes = false;
int turnout1 = 0;
int turnout2 = 0;
int route1 = 0;
int route2 = 0;

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
  dccexProtocol.getLists(false, true, true, false);


  if (dccexProtocol.isTurnoutListFullyReceived() && !doneTurnouts) {
    if (dccexProtocol.turnouts.size()>=2) {
      turnout1 = dccexProtocol.turnouts.get(0)->getTurnoutId();
      turnout2 = dccexProtocol.turnouts.get(1)->getTurnoutId();


      Serial.print("\nTurnout 1: id: "); Serial.println(turnout1);
      Serial.print("Turnout 2: id: "); Serial.println(turnout2);
    }
    doneTurnouts = true;
  }

  if (dccexProtocol.isRouteListFullyReceived() && !doneRoutes) {
    if (dccexProtocol.routes.size()>=2) {
      route1 = dccexProtocol.routes.get(0)->getRouteId();
      route2 = dccexProtocol.routes.get(1)->getRouteId();

      Serial.print("\nRoute 1: id: "); Serial.println(route1);
      Serial.print("Route 2: id: "); Serial.println(route2);
    }
    doneRoutes = true;
  }


  if ((millis() - lastTime) >= 10000) {
    if (doneTurnouts) {
      int action = random(0, 100);
      TurnoutState tAction = (action>50) ? TurnoutThrow : TurnoutClose;
      dccexProtocol.sendTurnoutAction(turnout1, tAction);
      action = random(0, 100);
      tAction = (action>50) ? TurnoutThrow : TurnoutClose;
      dccexProtocol.sendTurnoutAction(turnout2, tAction);
    }

    if (doneRoutes) {
      int action = random(0, 100);
      if (action>50) {
        dccexProtocol.sendRouteAction(route1);
      } else {
        dccexProtocol.sendRouteAction(route2);
      }
    }

    lastTime = millis();
  }
 
}
