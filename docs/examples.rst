.. include:: /include/include.rst

Examples
========

.. sidebar:: 

  .. contents:: On this page
    :depth: 2
    :local:

Several examples have been included to demonstrate functionality of the library.

Note that all included examples use a WiFi connection, but the protocol is equally suited to other connections types utilising the Arduino Stream base class including Ethernet and Serial.

To configure WiFi for your settings in all examples, you will need to copy the provided "config.example.h" to "config.h" and update the parameters to suit your environment:

.. code-block:: cpp

  const char* ssid = "YOUR_SSID_HERE";          // WiFi SSID name here
  const char* password =  "YOUR_PASSWORD_HERE"; // WiFi password here
  IPAddress serverAddress(192,168,4,1);         // IP address of your EX-CommandStation
  int serverPort = 2560;                        // Network port of your EX-CommandStation

DCCEXProtocol_Basic
-------------------

This example demonstrates the basics of creating a WiFi connection to your |EX-CS| using the library, and monitoring for broadcasts and command responses.

DCCEXProtocol_Delegate
----------------------

This example builds on the basic example and, in addition, demonstrates how to implement a custom `DCCEXProtocolDelegate`` class to respond to broadcasts and command responses received from |EX-CS|.

DCCEXProtocol_Roster_etc
------------------------

This example demonstrates how to retrieve the object types from |EX-CS|, and further demonstrates how to use the delegate to display these object lists when received.

DCCEXProtocol_Loco_Control
--------------------------

This example demonstrates basic locomotive speed and function control using dummy DCC addresses, in addition to controlling track power and further use of the delegate to notify when updates to the locomotive have been received.

DCCEXProtocol_Consist_Control
-----------------------------

This example demonstrates how to setup a software based consist (similar to how this is accomplished in Engine Driver), with basic speed and function control of the configured dummy locomotives. The delegate is also used to notify when updates to the configured locomotives have been received.

DCCEXProtocol_Turnout_control
-----------------------------

This example demonstrates the basics of controlling turnouts (or points) with the library, including being notified via the delegate when turnout/point objects have been closed/thrown.

DCCEXProtocol_Multi_Throttle_Control
------------------------------------

This example demonstrates how client throttle software may be written to control multiple locomotives (or consists for that matter) concurrently.

What can't be demonstrated in this example is the control of speed and direction, which would typically be accomplished with the use of rotary encoders or similar.

Note when setting speed and direction, these should be sent to the |EX-CS| via the |EX-PL|, and any local references to these should be set based on the response received, not directly by the input method in use.

For example, when setting the speed based on the position of a rotary encoder, send that value via the protocol's `setThrottle()` method, but do not display that speed directly. Instead, utlise the delegate's `receivedLocoUpdate()` method to update the displayed speed.

This ensures that the user of the throttle sees the accurate results of what the throttle is doing, and provides validation that the EX-CommandStation is responding to the user input.

DCCEXProtocol_Track_type
------------------------

This example demonstrates how client throttle software can change the Track type of any track/channel.  (MAIN\|PROG\|DC\|DCX\|NONE)

DCCEXProtocol_Serial
--------------------

This example demonstrates how to connect to an |EX-CS| using a serial based throttle, using a dedicated serial port for the connection in addition to the standard USB serial port.

This has been tested using an Arduino Mega2560, with the default USB port as the console/output serial port, and the second serial port "Serial1" as the |EX-CS| connection.

----

Additional Examples
-------------------

The following examples are not strictly related to the |EX-PL|, but hopefully will be useful for anyone developing a throttle to use with any |EX-CS|.

DCCEXProtocol_SSID
~~~~~~~~~~~~~~~~~~

This example demonstrates how client throttle software may be written to find all the SSIDs (networks) that are available.


DCCEXProtocol_mDNS
~~~~~~~~~~~~~~~~~~

This example demonstrates how client throttle software may be written to find all the |EX-CS| and WiThrottle servers that are advertising via mDNS.

Note that |DCC-EX| |EX-CS| only advertise as ``wiThrottle`` servers, but will use and respond with either the **WiThrottle protocol** or the |EX-NCP| depending the type of command it first receives from the client (throttle).

