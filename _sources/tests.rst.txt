.. include:: /include/include.rst

Library Tests
=============

Courtesy of Vincent Hamp, the |EX-NCL| has unit tests that can be run locally as well automatically being run as a GitHub action when pushing to the main branch (see tests.yml in the repository).

These tests are written using GoogleTest which allows for mocking as well as testing.

As of version 1.2.1, the tests have been migrated from cmake across to PlatformIO's native testing environment which simplifies the dependencies and process of running the tests.

Running these tests therefore requires PlatformIO, whether that be through the VSCode IDE or installed within a Linux/macOS environment directly. PlatformIO Core should be all that is required to be installed.

The recommendation to run these tests on Linux or macOS remains due to the ability to check for memory leaks and invalid pointers.

To run the tests, simply call:

.. code-block::

  pio test -e native_test

If you do run on Windows, run this command instead, note it disables the relevant sanitiser checks:

.. code-block::

  pio test -e native_test_windows

Test Coverage
-------------

To monitor test coverage, you can run the `generate_test_coverage.py` Python script which will output a HTML report in "test_coverage/coverage.html" as well as create the "lcov.info" file utilised by the VSCode extension "Coverage Gutters" to highlight test coverage directly within the VSCode IDE. When installed, simply select "Watch" from the bottom status bar to view the coverage.

Legacy cmake Tests
------------------

Prior to version 1.2.1, the tests were run using cmake. These have now been fully migrated across to PlatformIO's native test environment as described above, and the cmake build files have been removed from the repository.
