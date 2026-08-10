"""
Generate lcov.info to enable Coverage Gutters VSCode extension to display code test coverage.
"""
import os
import sys
import argparse

# check if env is specified as argument
parser = argparse.ArgumentParser()
parser.add_argument("-e", help="Environment to generate coverage for")
args = parser.parse_args()

if (args.e):
    current_env = args.e
elif (sys.platform == "darwin"):
    current_env = "native_test_macos"
else:
    current_env = "native_test"

build_dir = os.path.join(".pio", "build", current_env, "src")
html_dir = os.path.join("test_coverage")
html_file = os.path.join(html_dir, "coverage.html")

if not os.path.exists(html_dir):
    try:
        os.makedirs(html_dir, exist_ok=True)
        print(f"Created coverage directory {html_dir}")
    except Exception as error:
        print(f"Could not create coverage directory: {str(error)}")
        exit()

if not os.path.exists(build_dir):
    print(f"No build dir {build_dir}")

elif not any(file.endswith(".gcda") for file in os.listdir(build_dir)):
    print(f"No coverage data found in {build_dir} - run the tests first (e.g. pio test -e {current_env})")

else:
    cmd = (
        "gcovr -r . "
        f"{build_dir} "
        "--exclude test/ "
        "--lcov lcov.info "
        f"--html-details -o {html_file}"
    )
    print(f"Running command: {cmd}")
    os.system(cmd)
