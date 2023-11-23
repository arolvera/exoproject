# Hardware Control

Hardware control can be built in a number of different ways to facilitate test and debugging. 

- Full build with system control
- Single component build 
- Emulator build

hwc_main.c includes a main() function and a hwc_main() function. 

hwc_main() is meant to run as a FreeRTOS task while the main() function is only intended to be run when system control is not part of the build.

There are four main settings that determine the behavior for each build:

- SINGLE_COMPONENT_BUILD - custom cmake variable set in a project config file
- BUILD_FREERTOS - custom cmake variable set in a project config file
- task-monitor - exomodule declaration that initializes each component and task and is set in a project config file 
- Resistor ID - a setting that sets the behavioral profile for each build
- BUILD_SIMULATOR - custom cmake variable set in a project config file

## Full System Build

When building a complete firmware image for Halo 12, a project config file for system control is created that includes hardware control as a dependency to build everything together.

- SINGLE_COMPONENT_BUILD = OFF
- BUILD_FREERTOS = ON
- task-monitor is included in the exomodule declarations 
- Resistor ID is read from I/O lines
- BUILD_SIMULATOR = OFF

whats the difference between thruster control project config and master image constructor?

## Single Component Builds

Single component builds just one of the thruster component modules (Keeper, Anode, Magnet, Valves). There are dependencies with system control that must be included to support CAN communication but the build is intended to test just one component in isolation. 

- SINGLE_COMPONENT_BUILD = ON
- BUILD_FREERTOS = OFF
- task-monitor is excluded in the exomodule declarations 
- Resistor ID is forced to the desired value in code prior to build
- BUILD_SIMULATOR = OFF

## Emulator Builds

Emulator builds are similar to single component builds except that all of the components are included and it is compiler for x86.

- SINGLE_COMPONENT_BUILD = OFF
- BUILD_FREERTOS = OFF
- task-monitor is excluded in the exomodule declarations 
- Resistor ID is forced to a value that includes all components
- BUILD_SIMULATOR = ON
