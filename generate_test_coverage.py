"""
Generate lcov.info to enable Coverage Gutters VSCode extension to display code test coverage.
"""
import os
import argparse

# check if env is specified as argument
parser = argparse.ArgumentParser()
parser.add_argument("-e", default="native_test", help="Environment to generate coverage for")
args = parser.parse_args()

current_env = "native_test"     # default environment
if (args.e):
    current_env = args.e

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

if os.path.exists(build_dir):
    cmd = (
        "gcovr -r . "
        f"{build_dir} "
        "--exclude test/ "
        "--lcov lcov.info "
        f"--html-details -o {html_file}"
    )
    print(f"Running command: {cmd}")
    os.system(cmd)

else:
    print(f"No build dir {build_dir}")
