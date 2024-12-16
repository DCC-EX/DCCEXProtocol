.. include:: /include/include.rst

Usage
=====

.. sidebar:: 

  .. contents:: On this page
    :depth: 2
    :local:
    

Whilst this library extrapolates the need for understanding the specific |DCC-EX| native commands from a throttle developer, it is highly recommended to familiarise yourself with the concepts outlined in the `<https://dcc-ex.com/throttles/tech-reference.html>`_.

Setup
-----

Once the `DCCEXProtocol` object is instantiated, a connection must be made to the |EX-CS| using the `connect(&stream)` method and providing a suitable Arduino Stream, such as a WiFi client or serial connection.

It is also recommended to enable logging to an Arduino Stream using the `setLogStream(&stream)` method.

For WiFi clients, long periods of no interactive commands being sent may cause the WiFi client to be disconnected, so it is recommended to enable heartbeats for these, which defaults to sending a heartbeat every 60 seconds. If commands are sent regularly, no heartbeats are sent.

An example using an ESP32 with WiFi to connect to EX-CommandStation, with logging to the serial console:

.. code-block:: cpp

  WiFiClient client;
  DCCEXProtocol dccexProtocol;

  void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while(WiFi.status() != WL_CONNECTED) delay(1000);
    if (!client.connect(serverAddress, serverPort)) {
      while(1) delay(1000);
    }
    dccexProtocol.setLogStream(&Serial);
    dccexProtocol.enableHeartbeat();
    dccexProtocol.connect(&client);
  }

  void loop() {
    dccexProtocol.check();
    // other code here
  }

An example using STM32F103C8 Bluepill with hardware serial port 1 connecting to EX-CommandStation, and logging to the USB serial console:

.. code-block:: cpp

  DCCEXProtocol dccexProtocol;
  
  void setup() {
    Serial.begin(115200);
    Serial1.begin(115200);
    dccexProtocol.setLogStream(&Serial);
    dccexProtocol.connect(&Serial1);
  }

  void loop() {
    dccexProtocol.check();
    // other code here
  }

As covered in the design principles above, you must include the `check()` method as often as possible to receive command responses and broadcasts and have these processed by the library and any event handlers defined in your custom `DCCEXProtocolDelegate` class.

Refer to the :doc:`examples` to see how this may be implemented.

Control and Inputs
------------------

It is up to the client software utilising this library to manage control and input methods to provide input into the protocol functions such as setting locomotive speed and direction, turning functions on and off, and controlling the various other objects and methods available.

For example, multiple rotary encoders may be used to simultaneously control multiple locomotives or consists.

There is, however, no need to instantiate more than one `DCCEXProtocol` or `DCCEXProtocolDelegate` object providing the client software is written appropriately, and we recommend creating a custom class that can take the `DCCEXProtocol` object as a parameter to enable this.

See the `DCCEXProtocol_Multi_Throttle_Control` example for an idea of how this may be implemented.

A further note is that controls and inputs should be passed to the protocol only, and should not update local references to object attributes (such as speed and direction), but rather that the responses to these inputs as received by the protocol and delegate events should be used to update local references.

In this manner, the user of the throttle/client software will see the true results of their inputs which will reflect what EX-CommandStation is doing in response to those inputs.

Retrieving and referring to object lists
----------------------------------------

To retrieve the various objects lists from |EX-CS|, use the `getLists(bool rosterRequired, bool turnoutListRequired, bool routeListRequired, bool turntableListRequired)` method within your `loop()` function to ensure these are retrieved successfully.

If you have a lot of defined objects in your |EX-CS| (eg. 50+ turnouts or 50+ roster entries), you will likely need to increase the maximum number of parameters allowed when defining the DCCEXProtocol instance which is now a configurable parameter as of version 1.0.0 of the library.

You can set the command buffer size and parameter count:

.. code-block:: cpp

  // dccexProtocol(maxCmdBuffer, maxCommandParams);
  DCCEXProtocol dccexProtocol; // Use default 500 byte buffer, 50 parameters
  DCCEXProtocol dccexProtocol(500, 100); // Use default 500 byte buffer, 100 parameters

All objects are contained within linked lists and can be access via for loops:

.. code-block:: cpp

  for (Loco* loco=dccexProtocol.roster->getFirst(); loco; loco=loco->getNext()) {
    // loco methods are available here
  }

  for (Turnout* turnout=dccexProtocol.turnouts->getFirst(); turnout; turnout=turnout->getNext()) {
    // turnout methods are available here
  }

  for (Route* route=dccexProtocol.route->getFirst(); route; route=route->getNext()) {
    // route methods are available here
  }

  for (Turntable* turntable=dccexProtocol.turntables->getFirst(); turntable; turntable=turntable->getNext()) {
    // turntable methods are available here
    for (TurntableIndex* ttIndex=turntable->getFirstIndex(); ttIndex; ttIndex=ttIndex->getNextIndex()) {
      // turntable index methods are available here
    }
  }

Refer to the `DCCEXProtocol_Roster_etc` example for an idea of how this may be implemented.
