"""
Generate lcov.info to enable Coverage Gutters VSCode extension to display code test coverage.
"""

import os

build_dir = os.path.join(".pio", "build", "native_test", "src")
html_dir = os.path.join("test_coverage")
html_file = os.path.join(html_dir, "coverage.html")

if not os.path.exists(html_dir):
    try:
        os.makedirs(html_dir, exist_ok=True)
        print(f"Created coverage directory {html_dir}")
    except Exception as error:
        print(f"Could not create coverage directory: {str(error)}")
        exit()

if os.path.exists(build_dir):
    cmd = (
        "gcovr -r . "
        ".pio/build/native_test/src/ "
        "--exclude test/ "
        "--lcov lcov.info "
        f"--html-details -o {html_file}"
    )
    print(f"Running command: {cmd}")
    os.system(cmd)

else:
    print(f"No build dir {build_dir}")
