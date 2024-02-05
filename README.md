# CMake Skeleton

This is a skeleton project to bootstrap with [CMake](https://cmake.org/). You can folk this project and use it as a template to create new CMake projects.

## Prerequisites
1. Make sure you have compiler installed.
    * Windows: Visual Studio/MinGW
    * macOS: XCode/LLVM/GCC
    * Ubuntu: LLVM/GCC
2. Install CMake from the [download page](https://cmake.org/download/).
3. Install the VSCode CMake extension.

## Build the project
### Option 1: In VSCode
Execute the command `CMake:Build`

### Option 2: Command line
In terminal, execute the following commands
```shell
mkdir build
cd build
cmake ..
cmake --build .
```

## Debug the project
Execute the command `CMake:Debug`