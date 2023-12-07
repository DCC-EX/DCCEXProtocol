.. include:: /include/include.rst

Contributions
=============

The |DCC-EX| team welcomes contributions to the DCCEXProtocol library.

The best way to get involved is to reach out to the |DCC-EX| team via our `Discord server <https://discord.gg/PuPnNMp8Qf>`_.

You can also try the other methods outlined on our `Contact Us page <https://dcc-ex.com/support/contact-us.html>`_.

Documentation
-------------

This documentation is available via the `DCC-EX website <https://dcc-ex.com/DCCEXProtocol/index.html>`_.

For contributors wishing to build local copies of the documentation while updating the library, here is the very high level process of the requirements to make this work on Windows:

- Install `MSYS2 C++ <https://code.visualstudio.com/docs/cpp/config-mingw#_prerequisites>`_ compilers
- Install `CMake <https://cmake.org/download/>`_ and ensure you select the option to add to your user path
- Install `Doxygen <https://www.doxygen.nl/download.html>`_ and once complete, add to your user path
- Install the CMake Tools extension in VSCode
- Setup a Python virtual environment with "virtualenv venv" and activate with "venv\scripts\activate"
- Install required Python modules with "pip3 install -r requirements.txt"
- Change to the docs directory and run "make html"

Credit for how to do this to the following:

- Oliver K Ernst on `Medium <https://medium.com/practical-coding/c-documentation-with-doxygen-cmake-sphinx-breathe-for-those-of-use-who-are-totally-lost-7d555386fe13>`_
- Sy Brand in her `Microsoft Blog <https://devblogs.microsoft.com/cppblog/clear-functional-c-documentation-with-sphinx-breathe-doxygen-cmake/>`_
