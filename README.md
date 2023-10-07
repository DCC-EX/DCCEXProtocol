
# Credits

This delegate and connection code in this library is taken directly from the WiThrottle library by **Copyright © 2018-2019 Blue Knobby Systems Inc.**

----
----
----

# DCC-EX Native network protocol library

This library implements the DCC-EX Native protocol (as used in EX-CommandStation), allowing an device to connect to the server and act as a client (such as a hardware based throttle).

The implementation of this library is tested on ESP32 based devices running the Arduino framework.   There's nothing in here that's specific to the ESP32, and little of Arduino that couldn't be replaced as needed.

This library has been tested with version 1.3.3 of the LinkedList library by Ivan Siedel.  This library is required.

## Basic Design Principles

First of all, this library implements the DCC-EX Native protocol in a non-blocking fashion.  After creating a DCCEXProtocol object, you set up various necessities (like the network connection and a debug console) (see [Dependency Injection][depinj]).   Then call the ```check()``` method as often as you can (ideally, once per invocation of the ```loop()``` method) and the library will manage the I/O stream, reading in command and calling methods on the [delegate] as information is available.

These patterns (Dependency Injection and Delegation) allow you to keep the different parts of your sketch from becoming too intertwined with each other.  Nothing in the code that manages the pushbuttons or speed knobs needs to have any detailed knowledge of the WiThrottle network protocol.

## Included examples

### DCCEXProtocol_Basic

Basic example to implement a DCCEXProtocol client and connect to a server (with static IP).

Change the WiFi settings and enter the IP address of the Arduino running EX-CommandStation:
```
const char* ssid = "MySSID";
const char* password =  "MyPWD";
IPAddress serverAddress(192,168,1,1);
```
Compile and run, you should see the client connect in the Serial monitor.

### DCCEX_Delegate

Example to show how to implement a delegate class to handle callbacks.

Compile and run, you should see in the Serial monitor the server version printed.

### DCCEXProtocol_Roster_etc

Example to show how to retrieve the Roster, Turnouts/Point list, Routes/Automations List, and Turntable List for the Server.

Compile and run, you should see in the Serial monitor the lists printed.

### DCCEXProtocol_Loco_Control

Example to show how to acquire and control locos.  The Example assumes that you have a Roster on the EX-CommandStation with at least two entries.

Compile and run, you should see in the Serial monitor, after 20 second delays, two locos on two throttles change speed, and have functions randomly change.

----
----
----

# Usage

## Required Library

```<LinkedList.h>```  // https://github.com/ivanseidel/LinkedList

This library can be retrieved via the Arduino IDE Library Manager.  Search for "LinkedList" by Ivan Seidal.  The library has been tested with version ```1.3.3```


## Throttles

To simplify the handling of Consists/Multiple Unit Trains the library is implement to behave in a similar manor to the WiThrottle(TM) protocol, in that it *requires* that locos are attached to one of up to six 'throttles'.

The protocol provides ```Consist throttleConsists[MAX_THROTTLES]```

To acquire a loco on throttle 0 (zero) (the first throttle), you must first create a ```Loco``` object.  Details for the Loco can either be a direct dcc address or an entry from the Roster.  e.g. 

from DCC Address:

```Loco loco(11, "dummy loco", LocoSourceEntry);``` will create a loco with a DCC address of 11  and name of "dummy loco"

from Roster Entry:

```loco(dccexProtocol.roster.get(1)->getLocoAddress(), dccexProtocol.roster.get(1)->getLocoName(), dccexProtocol.roster.get(1)->getLocoSource());``` will create a loco from the second entry (1) in the Roster

To then add the loco to the throttle use ```dccexProtocol.throttleConsists[0].consistAddLoco(loco, FacingForward);``` to add the loco to Throttle 0. 

Control the speed and direction of all the locos on Throttle 0 with ```dccexProtocol.sendThrottleAction(0, speed, Forward);```

## Rosters

The Roster is stored as a Linked List.

Retrieve the list with ```dccexProtocol.getRoster();```

Or with ```dccexProtocol.getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired);```

Retrieve the size of the list (number of locos) with ```dccexProtocol.roster.size()```

Retrieve a ```ConsistLoco``` object from the list with ```dccexProtocol.roster.get(listEntryNumber)``` 


## Turnouts/Points

The List of defined Turnouts/Points is stored as a Linked List.

Retrieve the list with ```dccexProtocol.getTurnouts();```

Or with ```dccexProtocol.getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired);```

Retrieve the size of the list with ```dccexProtocol.turnouts.size()```

Retrieve a ```Turnout``` object from the list with ```dccexProtocol.turnouts.get(listEntryNumber)``` 

## Routes/Automations

The List of defined Routes/Automations is stored as a Linked List.

Retrieve the list with ```dccexProtocol.getRoutes();```

Or with ```dccexProtocol.getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired);```

Retrieve the size of the list with ```dccexProtocol.routes.size()```

Retrieve a ```Route``` object from the list with ```dccexProtocol.routes.get(listEntryNumber)``` 

## Turntables 

The List of defined Turntables is stored as a Linked List.

Retrieve the list with ```dccexProtocol.getTurntables();```

Or with ```dccexProtocol.getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired);```

Retrieve the size of the list with ```dccexProtocol.turntables.size()```

Retrieve a ```Turntable``` object from the list with ```dccexProtocol.turntables.get(listEntryNumber)``` 

----
----
----

# Structure

## Classes/Objects

### class ConsistLoco : public Loco 

Adds ```Facing``` to the Loco class.

Used by ```Consist```

#### Public Attributes

    none

#### Public methods

    ConsistLoco() {};
    ConsistLoco(int address, String name, LocoSource source, Facing facing);
    bool setConsistLocoFacing(Facing facing);
    Facing getConsistLocoFacing();

---

### class Loco

Used by ```Roster``` directly.  Used by ```ConsistLoco``` indirectly.

#### Public Attributes

    Functions locoFunctions;

#### Public methods

    Loco() {}
    Loco(int address, String name, LocoSource source);

    bool setLocoSpeed(int speed);
    bool setLocoDirection(Direction direction);
    
    int getLocoAddress();
    bool setLocoName(String name);
    String getLocoName();
    bool setLocoSource(LocoSource source);
    LocoSource getLocoSource();
    int  getLocoSpeed();
    Direction getLocoDirection();
    void setIsFromRosterAndReceivedDetails();
    bool getIsFromRosterAndReceivedDetails();

---

#### class Functions

Used by ```Loco```

#### Public Attributes

    none

#### Public methods

    Loco(int address, char* name, LocoSource source);
    Functions locoFunctions;
    bool isFunctionOn(int functionNumber);
    bool setLocoSpeed(int speed);
    bool setLocoDirection(Direction direction);
    int getLocoAddress();
    bool setLocoName(char* name);
    char* getLocoName();
    bool setLocoSource(LocoSource source);
    LocoSource getLocoSource();
    int  getLocoSpeed();
    Direction getLocoDirection();
    void setIsFromRosterAndReceivedDetails();
    bool getIsFromRosterAndReceivedDetails();
    bool clearLocoNameAndFunctions();

---

### class ConsistLoco : public Loco

#### Public Attributes

    none

#### Public methods

    ConsistLoco(int address, char* name, LocoSource source, Facing facing);
    bool setConsistLocoFacing(Facing facing);
    Facing getConsistLocoFacing();

---

### class Consist

Used by ```ConsistThrottle[]```

#### Public Attributes

    LinkedList<ConsistLoco*> consistLocos = LinkedList<ConsistLoco*>();

#### Public methods

    Consist(char* name);
    bool consistAddLoco(Loco loco, Facing facing);
    bool consistReleaseAllLocos();
    bool consistReleaseLoco(int locoAddress);
    int consistGetNumberOfLocos();
    ConsistLoco* consistGetLocoAtPosition(int position);
    int consistGetLocoPosition(int locoAddress);
    bool consistSetLocoPosition(int locoAddress, int position);
    bool actionConsistExternalChange(int speed, Direction direction, FunctionState fnStates[]);
    bool consistSetSpeed(int speed);
    int consistGetSpeed();
    bool consistSetDirection(Direction direction);
    Direction consistGetDirection();
    bool consistSetFunction(int functionNo, FunctionState state);
    bool consistSetFunction(int address, int functionNo, FunctionState state);
    bool isFunctionOn(int functionNumber);
    bool setConsistName(char* name);
    char* getConsistName();

---

### class Turnout

Used by ```Turnouts[]```

#### Public Attributes

    none

#### Public methods

    Turnout(int id, char* name, TurnoutState state);
    bool setTurnoutState(TurnoutAction action);
    TurnoutState getTurnoutState();
    bool throwTurnout();
    bool closeTurnout();
    bool toggleTurnout();
    bool setTurnoutId(int id);
    int getTurnoutId();
    bool setTurnoutName(char* name);
    char* getTurnoutName();
    void setHasReceivedDetails();
    bool getHasReceivedDetails();

---

### class Route

Used by ```Routes[]```

#### Public Attributes

    none

#### Public methods

    Route(int id, char* name);
    int getRouteId();
    bool setRouteName(char* name);
    char* getRouteName();
    bool setRouteType(RouteType type);
    RouteType getRouteType();
    void setHasReceivedDetails();
    bool getHasReceivedDetails();

---

### class Turntable

Used by ```Turntables[]```

#### Public Attributes

    LinkedList<TurntableIndex*> turntableIndexes = LinkedList<TurntableIndex*>();

#### Public methods

    Turntable(int id, char* name, TurntableType type, int position, int indexCount);
    bool addTurntableIndex(int index, char* indexName, int indexAngle);
    bool setTurntableIndexCount(int indexCount); // what was listed in the original definition
    int getTurntableIndexCount(); // what was listed in the original definition

    int getTurntableId();
    bool setTurntableName(char* name);
    char* getTurntableName();
    bool setTurntableCurrentPosition(int index);
    bool setTurntableType(TurntableType type);
    TurntableType getTurntableType();
    int getTurntableCurrentPosition();
    int getTurntableNumberOfIndexes();
    TurntableIndex* getTurntableIndexAt(int positionInLinkedList);
    TurntableIndex* getTurntableIndex(int indexId);
    TurntableState getTurntableState();
    bool actionTurntableExternalChange(int index, TurntableState state);
    void setHasReceivedDetails();
    bool getHasReceivedDetails();

---

#### class TurntableIndex

Used by ```Turntable```

#### Public Attributes

    int turntableIndexId;
    int turntableIndexIndex;
    String turntableIndexName;
    int turntableIndexAngle;
    bool hasReceivedDetail;

#### Public methods

    TurntableIndex(int index, char* name, int angle);
    void setHasReceivedDetails(); //????????????????? Probably not needed
    bool getHasReceivedDetails(); //????????????????? Probably not needed
    char* getTurntableIndexName();
    int getTurntableIndexId();
    int getTurntableIndexIndex();

----
----

## public Attributes

```
Consist throttleConsists[MAX_THROTTLES];
```

Linked List of type ```Consist```

```
LinkedList<Loco*> roster = LinkedList<Loco*>();
```

Linked List of type ```Loco```

```
LinkedList<Turnout*> turnouts = LinkedList<Turnout*>();
```

Linked List of type ```Turnout```

```
LinkedList<Route*> routes = LinkedList<Route*>();
```

Linked List of type ```Route```

```
LinkedList<Turntable*> turntables = LinkedList<Turntable*>();
```

Linked List of type ```Turntable```


## Public Methods

### Connection related

```
DCCEXProtocol(bool server = false);
```

TBA

```
void setDelegate(DCCEXProtocolDelegate *delegate);
```

TBA

```
void setLogStream(Stream *console);
```

TBA

```
void connect(Stream *stream);
```

TBA

```
void disconnect();
```

TBA

```
bool check();
```

TBA

### Control Related

#### general

```
void sendCommand(String cmd);
```

***This should not be used and will likely be made private***

#### Throttle Control

```
bool sendThrottleAction(int throttle, int speed, Direction direction);
```

Send a speed and direction to all locos on a Throttle.

```
bool sendFunction(int throttle, int funcNum, bool pressed);
```

Send a function to a Throttle.  Generally only the lead loco will get the command.

```
bool sendFunction(int throttle, String address, int funcNum, bool pressed);
```

Send a function a specific loco on a Throttle.

```
bool isFunctionOn(int throttle, int functionNumber);
```

TBA

```
Consist getThrottleConsist(int throttleNo);
```

TBA

```
bool sendLocoAction(int address, int speed, Direction direction);
``` 

***This should not be used and will likely be made private***

```
bool sendLocoUpdateRequest(int address);
```

***This should not be used and will likely be made private***

#### Server Info 

```
bool sendServerDetailsRequest();
```

TBA

```
bool getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired);
```

TBA

```
bool getRoster();
```

TBA

```
bool isRosterRequested();
```

TBA

```
bool isRosterFullyReceived();
```

TBA

```
bool getTurnouts();
```

TBA

```
bool isTurnoutListRequested();
```

TBA

```
bool isTurnoutListFullyReceived();
```

TBA

```
bool getRoutes();
```

TBA

```
bool isRouteListRequested();
```

TBA

```
bool isRouteListFullyReceived();
```

TBA

```
bool getTurntables();
```

TBA

```
bool isTurntableListRequested();
```

TBA

```
bool isTurntableListFullyReceived();
```

TBA

```
long getLastServerResponseTime();
```

TBA

```
void sendEmergencyStop();
```

TBA

```
bool sendTrackPower(TrackPower state);
```

TBA

```
bool sendTrackPower(TrackPower state, char track);
```

#### Turnout/Point commands

```
bool sendTurnoutAction(int turnoutId, TurnoutAction action);
```

TBA

#### Route/Automation commands

```
bool sendRouteAction(int routeId);
```

TBA

```
bool sendPauseRoutes();
```

TBA

```
bool sendResumeRoutes();
```

TBA

#### Turntable commands

```
bool sendTurntableAction(int turntableId, int position, int activity);
```

TBA

```
bool sendAccessoryAction(int accessoryAddress, int activate);
```

TBA

```
bool sendAccessoryAction(int accessoryAddress, int accessorySubAddr, int activate);
```

TBA

----
----
----

# License
----

Creative Commons [CC-BY-SA 4.0][CCBYSA]   ![CCBYSA](https://i.creativecommons.org/l/by-sa/4.0/88x31.png)


**Free Software, Oh Yeah!**

[//]: # (These are reference links used in the body of this note and get stripped out when the markdown processor does its job. There is no need to format nicely because it shouldn't be seen. Thanks SO - http://stackoverflow.com/questions/4823468/store-comments-in-markdown-syntax)

   [depinj]: <https://en.wikipedia.org/wiki/Dependency_injection>
   [delegate]: <https://en.wikipedia.org/wiki/Delegation_(object-oriented_programming)>
   [CCBYSA]: <http://creativecommons.org/licenses/by-sa/4.0/>