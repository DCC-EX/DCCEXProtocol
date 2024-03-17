.. include:: /include/include.rst

Contributions
=============

The |DCC-EX| team welcomes contributions to the DCCEXProtocol library.

The best way to get involved is to reach out to the |DCC-EX| team via our `Discord server <https://discord.gg/9DDjjpxzHB>`_.

You can also try the other methods outlined on our `Contact Us page <https://dcc-ex.com/support/contact-us.html>`_.

Library Maintenance
-------------------

As this library is designed to be available via the Arduino Library Manager, there are certain requirements that must be adhered to when maintaining and updating the library.

You can see DCCEXProtocol in the `Arduino Library Reference <https://www.arduino.cc/reference/en/libraries/dccexprotocol/>`_.

For details of the specific requirements of maintaining Arduino libraries, you will need to familiarise yourself with these by reviewing the `Arduino Library Manager FAQ <https://github.com/arduino/library-registry/blob/main/FAQ.md>`_.

Specifically, these are some items of note when making changes to the library:

- Ensure all public classes, methods, and attributes are documented in the code
- When adding new methods or attributes, use human-friendly names that indicate the desired purpose
- When adding a new version, ensure version notes are added to DCCEXProtocol.h

To ensure the Arduino Library Manager flags that a version update has been made, these activities must be performed:

- Update the "library.properties" file with the new version number
- When pushing updates to the "main" branch, ensure that the GitHub workflow "arduino-lint.yml" completes without error
- Add a GitHub tag to the repo with the version in the format "v0.0.1-Devel" or "v0.0.1-Prod"

Once an update has been made that changes the version, this will trigger the Arduino Library Manager to scan the DCCEXProtocol for changes and, providing no issues are encountered, will publish the new version for users to download or update.

The Arduino Library Manager will output logs for each library maintained which we can review for errors or issues. The `DCCEXProtocol library logs <https://downloads.arduino.cc/libraries/logs/github.com/DCC-EX/DCCEXProtocol/>`_ are available here.

Documentation
-------------

Library documentation is created automatically when pushes and pull requests are merged to the "main" branch via the GitHub "docs.yml" workflow.

The DCCEXProtocol class documentation is generated automatically via Doxygen and the Sphinx Breathe extension to convert code documentation to ReStructuredText, which Sphinx then generates the HTML content from.

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
