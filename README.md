
# Credits

The delegate and connection code in this library is taken directly from the WiThrottle library by **Copyright © 2018-2019 Blue Knobby Systems Inc.**
The rest of the code has been developed by Peter Akers (Flash62au), Peter Cole (PeteGSX) and Chris Harlow (UKBloke)

----
----
----

# DCC-EX Native network protocol library

This library implements the DCC-EX Native protocol (as used in EX-CommandStation ONLY), allowing an device to connect to the server and act as a client (such as a hardware based throttle).

The implementation of this library is tested on ESP32 based devices running the Arduino framework.   There's nothing in here that's specific to the ESP32, and little of Arduino that couldn't be replaced as needed.


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

## Throttles

To simplify the handling of Consists/Multiple Unit Trains the library is implement to behave in a similar manor to the WiThrottle(TM) protocol, in that it *requires* that locos are attached to a 'throttles'.

The protocol provides for the throttle app to specify the number of throttles required```DCCEXProtocol(int maxThrottles=6, bool server=false);```

To acquire a loco on throttle 0 (zero) (the first throttle), must specify a DDC address or a loco from the roster.

from DCC Address:

```dccexProtocol.throttle[0].addFromEntry(11, FacingForward);``` will add a loco with a DCC address of 11 on throttle 0, facing forward.

from Roster Entry:

To add the loco to the throttle use ```dccexProtocol.throttle[1].addFromRoster(dccexProtocol.getRosterEntryNo(1), FacingForward);``` to add a loco to Throttle 0, facing forward. 

Control the speed and direction of all the locos on Throttle 0 with ```dccexProtocol.sendThrottleAction(0, speed, Forward);```

## Rosters

The Roster is stored as a Linked List.

Retrieve the list with ```dccexProtocol.getRoster();```

Or with ```dccexProtocol.getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired);```

The roster has been fully received when ```isRosterFullyReceived()``` is true.

Retrieve the size of the list (number of locos) with ```dccexProtocol.getRosterCount()```

Retrieve a ```Loco*``` object from the list with ```dccexProtocol.getRosterEntryNo(listEntryNumber)``` 


## Turnouts/Points

The List of defined Turnouts/Points is stored as a Linked List.

Retrieve the list with ```dccexProtocol.getTurnouts();```

Or with ```dccexProtocol.getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired);```

The list has been fully received when ```isTurnoutListFullyReceived()``` is true.

Retrieve the size of the list with ```dccexProtocol.getTurnoutsCount()```

Retrieve a ```Turnout*``` object from the list with ```dccexProtocol.getTurnoutsEntryNo(listEntryNumber)``` 

## Routes/Automations

The List of defined Routes/Automations is stored as a Linked List.

Retrieve the list with ```dccexProtocol.getRoutes();```

Or with ```dccexProtocol.getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired);```

The list has been fully received when ```isRouteListFullyReceived()``` is true.

Retrieve the size of the list with ```dccexProtocol.getRoutesCount()```

Retrieve a ```Route*``` object from the list with ```dccexProtocol.getRoutesEntryNo(listEntryNumber)``` 

## Turntables 

The List of defined Turntables is stored as a Linked List.

Retrieve the list with ```dccexProtocol.getTurntables();```

Or with ```dccexProtocol.getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired);```

The list has been fully received when ```isTurntableListFullyReceived()``` is true.

Retrieve the size of the list with ```dccexProtocol.getTurntablesCount()```

Retrieve a ```Turntable*``` object from the list with ```dccexProtocol.getTurntablesEntryNo(listEntryNumber)``` 

----
----
----

# Structure

## Classes/Objects

### DCCEXProtocolDelegate

#### Public methods

    virtual void receivedServerDescription(char* version) {}
  
    virtual void receivedRosterList(int rosterSize) {}
    virtual void receivedTurnoutList(int turnoutListSize) {}    
    virtual void receivedRouteList(int routeListSize) {}
    virtual void receivedTurntableList(int turntablesListSize) {}    

    virtual void receivedSpeed(int throttleNo, int speed) { }
    virtual void receivedDirection(int throttleNo, Direction dir) { }
    virtual void receivedFunction(int throttleNo, int func, FunctionState state) { }

    virtual void receivedTrackPower(TrackPower state) { }

    virtual void receivedTurnoutAction(int turnoutId, bool thrown) { }
    virtual void receivedTurntableAction(int turntableId, int position, bool moving) { }

---

### DCCEXProtocol

#### Public methods

    **Protocol and Comms Related**
    DCCEXProtocol(int maxThrottles=6, bool server=false);
    void setDelegate(DCCEXProtocolDelegate *delegate);
    void setLogStream(Stream *console);
    void connect(Stream *stream);
    void disconnect();
    bool check();

    **Server Related**
    bool sendServerDetailsRequest();
    long getLastServerResponseTime();  // seconds since Arduino start

    **Power Related**
	bool sendTrackPower(TrackPower state);
	bool sendTrackPower(TrackPower state, char track);

    **Lists Related**
    bool getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired);

    **Roster Related**
    bool getRoster();
    int getRosterCount();
    Loco* getRosterEntryNo(int entryNo);
    Loco* findLocoInRoster(int address);
    bool isRosterRequested();
    bool isRosterFullyReceived();

    **Turnout Related**
    bool getTurnouts();
    int getTurnoutsCount();
    Turnout* getTurnoutsEntryNo(int entryNo);
    Turnout* getTurnoutById(int turnoutId);
    bool isTurnoutListRequested();
    bool isTurnoutListFullyReceived();
    void closeTurnout(int turnoutId);
    void throwTurnout(int turnoutId);
    void toggleTurnout(int turnoutId);

    **Route Related**
    bool getRoutes();
    int getRoutesCount();
    Route* getRoutesEntryNo(int entryNo);
    bool isRouteListRequested();
    bool isRouteListFullyReceived();
    bool sendRouteAction(int routeId);
    bool sendPauseRoutes();
    bool sendResumeRoutes();

    **Turntable Related**
    bool getTurntables();
    int getTurntablesCount();
    bool isTurntableListRequested();
    bool isTurntableListFullyReceived();
    Turntable* getTurntableById(int turntableId);
    bool sendTurntableAction(int turntableId, int position, int activity);

    **Throttle Related**
    Consist getThrottleConsist(int throttleNo);
    bool sendThrottleAction(int throttle, int speed, Direction direction);
    bool sendFunction(int throttle, int functionNumber, bool pressed);
    bool sendFunction(int throttle, int address, int functionNumber, bool pressed);
    bool isFunctionOn(int throttle, int functionNumber);
    void sendEmergencyStop();

    bool sendLocoAction(int address, int speed, Direction direction);
    bool sendLocoUpdateRequest(int address);

    **Accessory Related**
    bool sendAccessoryAction(int accessoryAddress, int activate);
    bool sendAccessoryAction(int accessoryAddress, int accessorySubAddr, int activate);


#### Public Attributes

    Consist *throttle;
    Loco* roster;
    Turnout* turnouts;
    Route* routes;
    Turntable* turntables;



---

### class Loco

???
Used by ```Roster``` directly.  Used by ```Throttle``` indirectly.

#### Public Attributes

    Functions locoFunctions;

#### Public methods

    Loco(int address, LocoSource source);
    int getAddress();
    void setName(char* name);
    char* getName();
    void setSpeed(int speed);
    int getSpeed();
    void setDirection(Direction direction);
    Direction getDirection();
    LocoSource getSource();
    void setupFunctions(char* functionNames);
    bool isFunctionOn(int function);
    void setFunctionStates(int functionStates);
    int getFunctionStates();
    int getCount();
    static Loco* getFirst();
    Loco* getNext();

---

### class ConsistLoco : public Loco 

Adds ```Facing``` to the Loco class.

Used by ```Consist```

#### Public Attributes

    none

#### Public methods

    ConsistLoco(int address, LocoSource source, Facing facing);
    void setFacing(Facing facing);
    Facing getFacing();
    ConsistLoco* getNext();

---

### class Consist

Used by ```throttle```

#### Public Attributes

    none

#### Public methods

    Consist();
    void setName(char* name);
    char* getName();
    void addFromRoster(Loco* loco, Facing facing);
    void addFromEntry(int address, Facing facing);
    void releaseAll();
    void releaseLoco(int address);
    int getLocoCount();
    bool inConsist(int address);
    void setSpeed(int speed);
    int getSpeed();
    void setDirection(Direction direction);
    Direction getDirection();
    ConsistLoco* getFirst();

---

### class Turnout

Used by ```turnouts```

#### Public Attributes

    none

#### Public methods

    Turnout(int id, bool thrown);
    void setThrown(bool thrown);
    void setName(char* _name);
    int getId();
    char* getName();
    bool getThrown();
    static Turnout* getFirst();
    Turnout* getNext();
    int getCount();

---

### class Route

Used by ```routes```

#### Public Attributes

    none

#### Public methods

    Route(int id);
    int getId();
    void setName(char* name);
    char* getName();
    void setType(RouteType type);
    RouteType getType();
    int getCount();
    static Route* getFirst();
    Route* getNext();

---

### class Turntable

Used by ```turntables```

#### Public Attributes

    none

#### Public methods

    Turntable(int id);
    int getId();
    void setType(TurntableType type);
    TurntableType getType();
    void setIndex(int index);
    int getIndex();
    void setNumberOfIndexes(int numberOfIndexes);
    int getNumberOfIndexes();
    void setName(char* name);
    char* getName();
    void setMoving(bool moving);
    bool isMoving();
    int getCount();
    int getIndexCount();
    static Turntable* getFirst();
    Turntable* getNext();
    void addIndex(TurntableIndex* index);
    TurntableIndex* getFirstIndex();

---

#### class TurntableIndex

Used by ```Turntable```

#### Public Attributes

    none

#### Public methods

    TurntableIndex(int ttId, int id, int angle, char* name);
    int getTTId();
    int getId();
    int getAngle();
    char* getName();
    TurntableIndex* getNext();

----
----


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