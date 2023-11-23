# Exoapps

The Halo application is split into two main components; System (Thruster) Control and Hardware Control. System control is the main application starting point and is responsible for the CANOpen interface to the flight control computer, state management, error handling, etc. Hardware control is the main interface to the hardware peripherals responsible for managing power supplies to components, measuring telemetrics, etc.

For Halo 12, there are three Vorago VA41630 processors:

- ECPK - Thruster control with Keeper
- ACP - Anode control 
- MVCP - Magnet and valve control

Each processor is programmed with the same firmware but the behavior of the processor is set by a profile ID determined at startup by reading I/O lines connected to a bank of resistors.