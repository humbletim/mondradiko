# Building

Mondradiko can currently be built on Windows x64 or Linux x64 (Oculus Quest support coming soon!)

## Windows Development

### Manual tool installation

Required tools (note: links are to individual downloads -- to install everything at once see below):
* [Visual Studio 2019 Community Edition](https://visualstudio.microsoft.com/vs/community/) or similar
  - if installing via GUI be sure to check the box to include the "Desktop development with C++" workflow.
* [CMake](https://cmake.org/download/) version 3.19 or later
* [Python](https://www.python.org/downloads/windows/) version 3.6 or later
* [Git](https://gitforwindows.org) version 2.2 or later

Optional tools:
* [Ninja](https://github.com/ninja-build/ninja/releases) version 1.10 or later

### Alternative "one" step tool installation:
1. Install [Chocolately](https://chocolately.org) Package Manager
2. From a Command Prompt run:
```cmd
choco install -y visualstudio2019community visualstudio2019-workload-nativedesktop cmake python3 git ninja
```

### Verify your tool versions (optional but recommended)
1. In the Start Menu search for and then launch the "x64 Native Tools Command Prompt" (Visual Studio 2019)
  - this will launch a Command Prompt preconfigured with the MSVC compiler and related tools on the PATH
  - (pro tip) you can launch directly into a Git for Windows Bash shell from here by entering `bash --login`
2. Run the following commands to check tool versions:
- `cmake --version`
- `python3 --version`
- `cl.exe`
- `ninja --version` (optional)

## Linux Development (Debian)

Use apt to install the essentials:
```bash
sudo apt install cmake ninja-build build-essential libwayland-dev libxrandr-dev
```

Currently all other dependencies should be automatically built from source or otherwise installed with the help of VCPKG.

(TODO: add system-level dependency detection and prefer compatible existing versions; eg: libvulkan-dev instead of a VCPKG in-tree build of same)

## Clone and then build using CMake (Windows and Linux)

From a bash prompt (or if Windows then a x64 Native Tools prompt) run:

```bash
git clone --recurse-submodules https://github.com/mondradiko/mondradiko
cd mondradiko
mkdir builddir
cd builddir
cmake ..
cmake --build . --config release
```

## Clone and then build using Visual Studio IDE (Windows only)

From a Windows CMD prompt:
```cmd
git clone --recurse-submodules https://github.com/mondradiko/mondradiko
cd mondradiko
mkdir builddir
cd builddir
cmake .. -G "Visual Studio 16 2019" -A x64 -Thost=x64
' you can then open the generated Mondradiko.sln from within the Visual Studio IDE
' alternatively, can build individual components at the command prompt using MSBuild:
' MSBuild.exe server/mondradiko-server.vcxproj -property:Configuration=Release -property:Platform=x64 -maxcpucount 
```

TODO: verify MSBuild is included with the one step choco commands above; _might_ require explicit installation instead:
 `choco install visualstudio2019buildtools`

## Clone and then build using Ninja (Windows and Linux)

```bash
git clone --recurse-submodules https://github.com/mondradiko/mondradiko
cd mondradiko
mkdir builddir
cd builddir
cmake .. -G Ninja
ninja
```
