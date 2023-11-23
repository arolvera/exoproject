# Setup vscode to work with segger and cmake

## VSCode plugin setup
Plugins needed:
- CMake Tools
- Cortex Debug
- c/c++

You might need to instal libcurses5.so for cortex debug to work\
Use Ctrl+Shift+P to run plugin commands such as cmake build, cortex debug disassembly view, ...

## VSCode json file setup
There are example files under .vscode/ dir that can be used to get cmake to build, debug, and use intelli sense. To use the example
files environment variable XCOMP_PATH must be set pointing to the sysrooot of cross compiler.\
\
Files under .vscode:
- tasks.json - sets up build  for clean build and rebuild
- settings.json - passes cmake vars to cmake build, sets up compiler path for cortex-debug, sets up vscode visual adjustments.
- c_cpp_properties.json - sets up intellisense
