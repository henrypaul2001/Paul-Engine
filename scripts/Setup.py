import os
import subprocess
import platform
import shutil
import sys

from SetupPython import PythonConfiguration as PythonRequirements

# Make sure everything we need for the setup is installed
PythonRequirements.Validate()

from SetupPremake import PremakeConfiguration as PremakeRequirements
from SetupVulkan import VulkanConfiguration as VulkanRequirements

os.chdir('./../')  # Change from devtools/scripts directory to root

premakeInstalled = PremakeRequirements.Validate()
VulkanRequirements.Validate()

print("\nUpdating submodules...")
subprocess.call(["git", "submodule", "update", "--init", "--recursive"])

# Build Assimp via CMake
def check_cmake_installed():
    cmake_path = shutil.which("cmake")
    if cmake_path is None:
        print("Error: CMake is not installed or not in PATH. Please install CMake to continue.")
        sys.exit(1)

def print_cmake_version():
    try:
        version_output = subprocess.check_output(["cmake", "--version"], text=True)
        print(f"Using CMake:\n{version_output.splitlines()[0]}")
    except Exception:
        print("Warning: Unable to verify CMake version.")

def build_assimp():
    print("\nBuilding Assimp with CMake...")

    assimp_dir = os.path.join(os.getcwd(), "PaulEngine/vendor/assimp/assimp")
    build_dir = os.path.join(assimp_dir, "build")

    if not os.path.exists(build_dir):
        os.makedirs(build_dir)

    # Common CMake configuration flags
    cmake_configure_cmd = [
        "cmake",
        "-S", assimp_dir,
        "-B", build_dir,
        "-DCMAKE_BUILD_TYPE=RelWithDebInfo",  # For multi-config generators like Visual Studio, this is fine
        "-DBUILD_SHARED_LIBS=OFF",
        "-DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=ON",
        "-DASSIMP_BUILD_TESTS=OFF",
        "-DASSIMP_BUILD_SAMPLES=OFF",
        "-DASSIMP_BUILD_ZLIB=ON",
        "-DASSIMP_NO_EXPORT=OFF"
    ]

    print("\nRunning CMake configure:")
    print(" ".join(cmake_configure_cmd))
    subprocess.check_call(cmake_configure_cmd)

    for config in ["Debug", "Release"]:
        cmake_build_cmd = [
            "cmake",
            "--build", build_dir,
            "--config", config,
            "--target", "assimp"
        ]

        print(f"\nBuilding Assimp library [{config}]...")
        subprocess.check_call(cmake_build_cmd)

if premakeInstalled:
    check_cmake_installed()
    print_cmake_version()

    try:
        build_assimp()
    except subprocess.CalledProcessError as e:
        print(f"Error building Assimp: {e}")
        sys.exit(1)

    if platform.system() == "Windows":
        print("\nRunning premake...")
        subprocess.call([os.path.relpath("./scripts/Win-GenerateProjects.bat"), "nopause"])
    print("\nSetup completed!")
else:
    print("Paul Engine requires Premake to generate project files.")