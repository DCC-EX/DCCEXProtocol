
# Credits

This library is taken, in part, from the WiThrottle library by 
**Copyright © 2018-2019 Blue Knobby Systems Inc.**


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
Compile and run, you should see the client connect:


### DCCEX_Delegate

Example to show how to implement a delegate class to handle callbacks.

Compile and run, you should see in Serial monitor the server version, printed by ```void receivedVersion(String version)``` method:

### DCCEX_Delegate

Example to show how to implement a delegate class to handle callbacks.

Compile and run, you should see in Serial monitor the server version printed.

### DCCEXProtocol_Roster_etc

Example to show how to retrieve the Roster, Turnouts/Point list, Routes/Automations List, and Turntable List for the Server.

Compile and run, you should see in Serial monitor the lists printed.

### DCCEXProtocol_Loc_Control

Example to show how to acquire and control locos.  The Example assumes that you have a Roster on the EX-CommandStation with at least two entries.

Compile and run, you should see in Serial monitor, after 20 second delays, two locos on two throttles change speed, and have functions randomly change.

# Structure

## public Attributes

```
Consist throttleConsists[MAX_THROTTLES];
```

```
LinkedList<Loco*> roster = LinkedList<Loco*>();
```

```
LinkedList<Turnout*> turnouts = LinkedList<Turnout*>();
```

```
LinkedList<Route*> routes = LinkedList<Route*>();
```

```
LinkedList<Turntable*> turntables = LinkedList<Turntable*>();
```


## Public Methods

```
DCCEXProtocol(bool server = false);
```

```
void setDelegate(DCCEXProtocolDelegate *delegate);
```

```
void setLogStream(Stream *console);
```

```
void connect(Stream *stream);
```

```
void disconnect();
```

```
bool check();
```

```
void sendCommand(String cmd);
```

```
bool sendThrottleAction(int throttle, int speed, Direction direction);
```

```
bool sendLocoAction(int address, int speed, Direction direction);
```

```
bool sendFunction(int throttle, int funcNum, bool pressed);
```

```
bool sendFunction(int throttle, String address, int funcNum, bool pressed);
```

```
bool sendLocoUpdateRequest(int address);
```

```
bool sendServerDetailsRequest();
```

```
bool getRoster();
```

```
bool getTurnouts();
```

```
bool getRoutes();
```

```
bool getTurntables();
```

```
long getLastServerResponseTime();
```

```
void sendEmergencyStop();
```

```
Consist getThrottleConsist(int throttleNo);
```

```
bool sendTrackPower(TrackPower state);
```

```
bool sendTrackPower(TrackPower state, char track);
```

```
bool sendTurnoutAction(int turnoutId, TurnoutAction action);
```

```
bool sendRouteAction(int routeId);
```

```
bool sendPauseRoutes();

```
bool sendResumeRoutes();

```
bool sendTurntableAction(int turntableId, int position, int activity);
```

```
bool sendAccessoryAction(int accessoryAddress, int activate);
```

```
bool sendAccessoryAction(int accessoryAddress, int accessorySubAddr, int activate);
```

# Usage

## Required Library

```<LinkedList.h>```  // https://github.com/ivanseidel/LinkedList

This library can be retrieve via the Arduino IDE Library Manager.  Search for "LinkedList" by Ivan Seidal.  The lirbary has been tested with veriosn ```1.3.3```


## Throttles

To simplify the handling of Consists/Multiple Unit Trains the libray is implement to behave in a simlar manor to the WiThrottle(TM) protocol, in that it *requires* that locos are attached to one of up to six 'throttles'.

The protocol provides ```Consist throttleConsists[MAX_THROTTLES]```

To acquire a loco on throttle 0 (zero) (the first throttle), you must first create a ```Loco``` object.  Details for the Loco can either be a direct dcc adddress or an entry from the Roster.  e.g. 

from DCC Address:

```Loco loco(11, "dummy loco", LocoSourceEntry);```

from Roster Entry:

```Loco loco2 = Loco(dccexProtocol.roster.get(1)->getLocoAddress(), dccexProtocol.roster.get(1)->getLocoName(), dccexProtocol.roster.get(1)->getLocoSource());```


## Rosters

The Roster is stored as a Linked List.

Retrieve the list with ```dccexProtocol.getRoster();```

Retrieve the size of the list (number of locos) with ```dccexProtocol.roster.size()```

Retrieve a ```ConLoco``` object from the list with ```dccexProtocol.roster.get(listEntryNumber)``` 



# License
----

Creative Commons [CC-BY-SA 4.0][CCBYSA]   ![CCBYSA](https://i.creativecommons.org/l/by-sa/4.0/88x31.png)


**Free Software, Oh Yeah!**

[//]: # (These are reference links used in the body of this note and get stripped out when the markdown processor does its job. There is no need to format nicely because it shouldn't be seen. Thanks SO - http://stackoverflow.com/questions/4823468/store-comments-in-markdown-syntax)

   [depinj]: <https://en.wikipedia.org/wiki/Dependency_injection>
   [delegate]: <https://en.wikipedia.org/wiki/Delegation_(object-oriented_programming)>
   [CCBYSA]: <http://creativecommons.org/licenses/by-sa/4.0/>