"""
Generate lcov.info to enable Coverage Gutters VSCode extension to display code test coverage.
"""

import os
Import("env")


def after_test(source, target, item):
    print("\n--- Generating Coverage Report for VS Code ---")
    # This command creates the lcov.info file needed by Coverage Gutters
    # We use --lcov because it's compatible with most gcovr versions
    cmd = "gcovr -r . --lcov lcov.info"

    # Run the command and capture output
    exit_code = os.system(cmd)

    if exit_code == 0:
        print("Success: lcov.info generated.")
    else:
        print("Error: gcovr failed to generate report.")


# Tell PlatformIO to run this after the 'test' target is finished
env.AddPostAction("test", after_test)
