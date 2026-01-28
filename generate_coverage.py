"""
Generate lcov.info to enable Coverage Gutters VSCode extension to display code test coverage.
"""

import os

build_dir = os.path.join(".pio", "build", "native_test", "src")
if os.path.exists(build_dir):
    cmd = "gcovr -r . .pio/build/native_test/src/ --lcov lcov.info --html-details -o coverage.html"
    os.system(cmd)

else:
    print(f"No build dir {build_dir}")
