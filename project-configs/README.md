# Project Configs

Halo software has a number of build configurations defined, including, but not limited to:

- Fully functional builds targeted for production hardware
- Test builds designed to exercise a subset of features
- Builds of submodules targeted for an x86 architecture

The basic structure of a build configuration file consists of the following elements:

- Toolchain
- Application type and architecture variables
- Exomodule declarations
- Product variant variables
- Feature selections
- Project dependencies
- Resistor ID Setting
- Single Component Build Settings

## Toolchain



## Application and Architecture Variables

These variables are primarily used to construct folder and file names and to define the project tree and path to all source files used in a build.

| Variable | Description |
| -------- | ----------- |
| EXOPROJECT_APP | Defines the targeted product. This variable is always the root of a file or folder name that is specific to a product. Values include:<br>- halo6<br>- halo12<br>- fcp |
| EXOPROJECT_APP_TYPE | Halo software is divided into two architectural blocks that can be built together or independent of each other to allow low-level testing of hardware. Values include:<br>- hardwarecontrol<br>- master-image-constructor<br>- system-control |
| EXOPROJECT_APP_SUBTYPE | Halo software can also be built in test or emulator configurations intended to facilitate isolated test and debug of the software. This variable changes the project tree to point at an alternative main function that can drive isolated tests of the software.<br><br>Defined subtypes will change over time but some of the ones created during Halo 12 development include:<br>- app (only applicable to system-control - use empty string for hardware control)<br>- can-bm-tests<br>- ebi-memory-test<br>- sequence-engine-tests<br>- storage-memory-tests<br>- hardwarecontrol-emu<br>- system-control-emu<br><br>**Important** Fully functional builds targeted for production hardware must have APP_SUBTYPE set to an empty string ("") |
| EXODRIVER_PROCESSOR | Defines the target processor for the build. This variable is always the second part of a file or folder name that is specific to a product. Values include:<br>- samv71<br>- va41630<br>- x86 |
| EXOMOD_GAS | Defines the type of gas used in the thruster. This is used to build with different setpoint voltages and currents dependent on the properties of the gas. It is also used to build folder and file names containing these setpoints. Supported values are:<br>- Krypton<br>- Xenon |
| EXOMOD_COIL | Defines the material used in the magnet coil windingswhich can impact setpoint voltages and currents. It is also used to build folder and file names containing these setpoints. Supported values are:<br>- Copper<br>- Silver |
| EXOMOD_CUSTOMER | Defines the target customer for the build. Each customer can have their own setpoints for power and thrust levels and this variable determines which setpoint are built. It is also used in file and flder names. Supported values can vary over time but are initially:<br>- Boeing |

Optional variables that have limited scope:

| Variable | Description |
| -------- | ----------- |
| SINGLE_COMPONENT_BUILD | Specific to hardwarcontrol app subtype. This allows building of just one of the components; anode, keeper, magnets, and valves. |
| BUILD_SIMULATOR | Specific to the simulator project config. Enables building of hardware and system control with stubbed out hardware interfaces. |
| SIMULATOR_USE_SOCKET | Specific to the hardwarecontrol simulator. This allows the simulator to be built with CAN when interfacing to system control running on a board, or to be built with sockets when system control is simulated. |

## Exomodule Declarations

ExoModules is a submodule that is added to ExoProject to add features that may be common to other products. Each subfolder contains all of the source code for a feature and a CMakeLists file. This section of the project config declares the module directories to include in the build.

| Feature | Description |
| ------- | ----------- |
| canopen      |  |
| client-control      |  |
| component    |  |
| conditioning |  |
| error        |  |
| iacm         |  |
| osal         |  |
| health       |  |
| msg-handler  |  |
| sequence     |  |
| storage      |  |
| sys          |  |
| task-monitor |  |
| trace        |  |
| user-setting-values |  |
| utils        |  |

## External Library Selection

| Library | Description |
| ------- | ----------- |
| BUILD_CANOPEN  | CANopen is a CAN-based communication system that comprises higher-layer protocols and profile specifications. It as been developed as a standardized embedded network with highly flexible configuration capabilities. |
| BUILD_CMSIS    | CMSIS enables consistent device support and simple software interfaces to the processor and its peripherals, simplifying software reuse, reducing the learning curve for microcontroller developers, and reducing the time to market for new devices. |
| BUILD_FREERTOS | FreeRTOS is a market-leading real-time operating system (RTOS) for microcontrollers and small microprocessors. Distributed freely under the MIT open source license, FreeRTOS includes a kernel and a growing set of IoT libraries suitable for use across all industry sectors. FreeRTOS is built with an emphasis on reliability and ease of use. |
| BUILD_LIBCRC   | LibCRC is an MIT licensed library written in C containing various checksum algorithms. These include the most common CRC implementations but also other checksums like the NMEA checksum used by marine equipment. |
| BUILD_PERCEPIO | Percepio is a provider of visual trace diagnostics and firmware monitoring tools that help embedded software developers improve software quality, product performance, development productivity and time-to-market. |

## Project dependencies



## Resistor ID Setting

Halo 12 software is designed to be deployed as one monolithic image containing the system (thruster) control application and all four hardware (Keeper, Anode, Magnets, Valves) control applications. The initial PPU design has three Vorago chips that each receive this image but have different personalities based on the population of a bank of 5 resistors connected to GPIO.

The three personalities in the first PPU design are:

- ECPK - Engine (thruster) control and keeper control are enabled
- MVCP - Magnet and valve control are enabled
- ACP - Anode control is enabled

Additional personalities exist for running a build on a development board or as an emulator where the resistor bank is not connected or accessible.

- SYS_CTRL_RESID - Engine (thruster) control is enabled, all hardware control disabled
- HRD_CTRL_RESID - Hardware control is enabled subject to SINGLE_COMPONENT_BUILD variable, sytem control is disabled
- ALL_CTRL_RESID - All components are enabled

| Variable | Definition |
| -------- | ---------- |
| EXORUN_WITH_FIXED_RESID | Boolean variable that determines if the app reads the GPIO for the resistor ID or uses a fixed value set by one of the remaining variables.<br>OFF = Read from GPIO<br>ON = set one of the following modes. |
| EXORUN_WITH_ECPK_RESID | Set the fixed resistor ID to ECPK (Engine ctrl + keeper) |
| EXORUN_WITH_MVCP_RESID | Set the fixed resistor ID to MVCP (Magnets + Valves) |
| EXORUN_WITH_ACP_RESID | Set the fixed resistor ID to ACP (Anode ctrl) |
| EXORUN_WITH_SYS_CTRL_RESID | Set the fixed resistor ID to SYS_STRL (Engine ctrl only) |
| EXORUN_WITH_HRD_CTRL_RESID | Set the fixed resistor ID to HRD_CTRL (Hardware ctrl only) |
| EXORUN_WITH_ALL_CTRL_RESID | Set the fixed resistor ID to ALL_CTRL (System ctrl + hardware ctrl) |

## Single Component Build Settings

Only applicable for project configs where APP_TYPE is "hardwarecontrol". When on, this will build juist one of the components (anode, keeper, magnets, valeves) and will build with a main function that then launches a task.

| Variable | Description |
| -------- | ----------- |
| SINGLE_COMPONENT_BUILD | Boolean variable that determines if the app builds just one hardware component |
| EXORUN_SINGLE_COMPONENT_ANODE | Set the single component to Anode |
| EXORUN_SINGLE_COMPONENT_KEEPER | Set the single component to Keeper |
| EXORUN_SINGLE_COMPONENT_MAGNET | Set the single component to Magnet |
| EXORUN_SINGLE_COMPONENT_VALVE | Set the single component to Valves |
