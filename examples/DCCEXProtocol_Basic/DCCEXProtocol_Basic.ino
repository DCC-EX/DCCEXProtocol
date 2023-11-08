// WiThrottleProtocol library: Basic example
//
// Shows how to create an instance of DCCEXProtocol
// and how to connect to a DCC-EX Native protocol server using static IP
// Tested with ESP32-WROOM board
//
// Peter Akers (Flash62au), Peter Cole (PeteGSX) and Chris Harlow (UKBloke), 2023
// Luca Dentella, 2020

#include <WiFi.h>
#include <DCCEXProtocol.h>

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
  
void setup() {
  
  Serial.begin(115200);
  Serial.println("DCCEXProtocol Basic Demo");
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
  //dccexProtocol.setLogStream(&Serial);

  // Pass the communication to wiThrottleProtocol
  dccexProtocol.connect(&client);
  Serial.println("DCC-EX connected");
}
  
void loop() {

  // parse incoming messages
  dccexProtocol.check();
}
