.. include:: /include/include.rst

Documentation for the DCC-EX Native command protocol library - DCCEXProtocol
============================================================================

.. toctree::
  :maxdepth: 4
  :hidden:

  DCC-EX Native command protocol library <self>
  overview
  usage
  examples
  library
  bugs-requests
  tests
  contribute
  site-index

DCC-EX Native command protocol library
--------------------------------------

This library implements the |EX-NCP| (as used in the |DCC-EX| |EX-CS| ONLY), allowing a device to connect to the server and act as a client (such as a hardware based throttle).

The implementation of this library is tested on ESP32 based devices running the Arduino framework. There's nothing in here that's specific to the ESP32, and little of Arduino that couldn't be replaced as needed.

There has also been limited testing on STM32F103C8 Bluepill with a serial connection.

Credits
-------

The delegate and connection code in this library is taken directly from the WiThrottle library by **Copyright © 2018-2019 Blue Knobby Systems Inc.**
The rest of the code has been developed by Peter Cole (peteGSX), Peter Akers (Flash62au) and Chris Harlow (UKBloke).
