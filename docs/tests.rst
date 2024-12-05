.. include:: /include/include.rst

Library Tests
=============

Courtesy of Vincent Hamp, the |EX-NCL| has unit tests that can be run locally as well automatically being run as a GitHub action when pushing to the main branch (see tests.yml in the repository).

These tests are written using GoogleTest which allows for mocking as well as testing.

To run these locally, you will need cmake, doxygen, and C++ build tools. If generating graphs with Doxygen, graphviz is also required. Including clang-format is recommended to help code formatting according to the included formatting also.

It is recommended these be run on Linux or macOS, as you will need to use MSYS2/MinGW on Windows, and various functions are unsupported, in addition to the case insensitivity of Windows causing issues with some Arduino/C++ headers (eg. string.h in Windows is the same as String.h). To be clear, running these tests is **not recommended on Windows**.

If you are using a Windows PC for the tests, it is best to utilise Windows Subsystem for Linux with Ubuntu, or your preference of distribution. Note also, that while your existing cloned repository is likely available to WSL in the `/mnt/c` directory, this will likely still cause issues as it is a case insensitive file system that has been mounted, and you should clone the repository within your WSL instance instead.

To use your existing installation of VSCode with WSL, you can simply change to the repository containing the cloned repository and run ``code .``, which will launch VSCode connected to your WSL instance.

For Ubuntu, install the required packages with:

.. code-block::

  sudo apt install build-essential doxygen cmake graphviz clang-format

Once these packages are installed, running the tests is pretty simple:

.. code-block::

  cd <directory>/DCCEXProtocol
  cmake -Bbuild
  cmake --build build
  ./build/tests/DCCEXProtocolTests --gtest_shuffle

If you have issues compiling the tests, you may need to delete the build directory if it already existed before you started. It is not included in the repository and is used locally only.
