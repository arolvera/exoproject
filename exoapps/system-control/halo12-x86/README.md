# Halo x86

Halo builds that run on a x86 machine in emulator mode. 

## Hardware Contol Emulator

The hardware control emulator builds all four components together with message handler. The CAN interface is replaced with sockets and hardware interfaces are stubbed out to produce "happy path" results such as current and voltage measurements, spark events, etc.