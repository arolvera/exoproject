# EARLY DRAFT ----- Emulator Setup Procedure

The SOMBRARRO (Simulation of Many Bad Reaction and Response Occurances) emulator is designed to run system control in isolation on a Vorago development board while emulating hardware control on the test machine. Hardware control is compiled with stubs for the hardware interface and sockets replacing the CAN interface so it can talk to a CAN error injector that will pass through communications between system control and hardware control until a test case is encountered where the error injector will override communications to simulate the error.

This user guide covers the procedures to get set up and to use the SOMBRARO test utility.

