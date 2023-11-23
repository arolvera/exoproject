# Software Interface Requirements

For communication and control the ECP utilizes the CANOpen protocol stack. Within the context of the Open Systems Interconnection (OSI) model, the serial RS-485, RS-422, and CAN interfaces cover the physical and data link layers, whereas CANOpen implements the other above-lying layers.

## Terms and Definitions

| Term | Definition                      |
|------|---------------------------------|
| 0b   | Binary number prefix            |
| 0x   | Hexadecimal number prefix       |
| COB  | Communication Object            |
| ECP  | Engine Control Processor        |
| EPS  | Electronic Propulsion System    |
| LSB  | Least Significant But or Byte   |
| MSB  | Most Significant Bit or Byte    |
| NMT  | Network Management              |
| OD   | Object Dictionary               |
| PDO  | Peripheral Data Object          |
| rx   | Receive                         |
| SDO  | Service Data Object             |
| tx   | Transmit                        |

## Table of Contents

1.  [CANopen Overview](#canopen-overview)
    1.  [Message Protocols](#message-protocols)
    2.  [Message Structure](#message-structure)
    3.  [Object Dictionary](#object-dictionary)
2.  [SYNC and EMCY Protocol](#sync-and-emcy-protocol)
    1.  [Synchronization Protocol](#synchronization-protocol)
    2.  [Emergency Protocol](#emergency-protocol)
3.  [NMT Protocol](#nmt-protocol)
4.  [State Control and Initialization](#state-control-and-initialization)
5.  [Service Data Object Protocol](#service-data-object-sdo-protocol)
    1.  [SDO Message Structure](#sdo-message-structure)
    2.  [SDO Supported Command Index's](#sdo-supported-command-indexs)
6.  [CANopen Standard Object Dictionary Entries](#canopen-standard-object-dictionary-entries)
    1.  [Device Type](#device-type-0x1000)
    2.  [Manufacturer Device Name](#manufacturer-device-name-0x1008)
    3.  [Manufacturer Hardware Version](#manufacturer-hardware-version-0x1009)
    4.  [Software Version](#software-version-0x100a)
    5.  [Heartbeat](#heartbeat-0x1017)
7.  [Thruster Commands Object 0x4000](#thruster-commands-object-0x4000)
    1.  [Thruster Command Table of Sub-Index's](#thruster-command-table-of-sub-indexs)
    2.  [Sequence Table Commands](#sequence-table-commands-sub-index-1-2-6--7)
    3.  [Thrust Setting (Sub-Index 4)](#thrust-setting-sub-index-4)
    4.  [Thruster State (Sub-Index 5)](#thruster-state-sub-index-5)
    5.  [Conditioning (Sub-Index 6)](#conditioning-sub-index-6)
    6.  [Built-In Test (Sub-Index 7)](#built-in-test-sub-index-7)
8.  [Sequence Condition Status Object 0x4001](#sequence-condition-status-object-0x4001)
9. [Clear Conditioning Stats 0x5401](#clear-conditioning-stats-0x5401)
10. [Firmware Versions Object 0x5000](#firmware-versions-object-0x5000)
    1.  [Device Selection](#device-selection)
11. [Configuration Object 0x5100](#configuration-object-0x5100)
12. [Housekeeping Diagnostic Object 0x3000](#housekeeping-diagnostic-object-0x3000)
13. [Keeper Diagnostic Object 0x3001](#keeper-diagnostic-object-0x3001)
14. [Anode Diagnostic Object 0x3002](#anode-diagnostic-object-0x3002)
15. [Magnet Diagnostic Object 0x3003/0x3004](#magnet-diagnostic-object-0x30030x3004)
16. [Valve Diagnostic Object 0x3005](#valve-diagnostic-object-0x3005)
17. [Memory Diagnostic Object 0x3100](#memory-diagnostic-object-0x3100)
18. [Debug Trace Messages 0x5001](#debug-trace-messages-0x5001)
19. [Reprogramming 0x5500](#reprogramming-0x5500)
20. [Special Sequences](#special-sequences)
21. [Fault Handling](#fault-handling)
    1.  [Fault Reaction](#fault-reaction)
    2.  [Fault Table](#fault-table)
    3.  [Fault Reaction Type](#fault-reaction-type)
    4.  [Fault Reaction Value Description](#fault-reaction-value-description)
    5.  [Fault Status Table](#fault-status-table)
    6.  [Error Register Table](#error-register-table)
    7.  [8 Bit Error Types Table](#8-bit-error-types-table)
    8.  [16 Bit Error Codes Table](#16-bit-error-codes-table)
    9.  [Error History (0x1003)](#error-history-0x1003)
    10. [Module Error Log Table (0x2832)](#module-error-log-table-0x2832)
    11. [Emergency Messages](#emergency-messages)
22. [Assumptions And Constraints](#assumptions-and-constraints)
    1.  [Software Assumptions](#software-assumptions)
    2.  [Software Constraints](#software-constraints)

&nbsp;

#  CANopen Overview

Messages shall be sent to and from the EPS using a protocol based on CANopen. For more information, refer to the [CANopen Wiki](https://en.wikipedia.org/wiki/CANopen).

**Note:** For reference, transmit and receive are from the perspective of the device. This is also refered to as upload and download from the perspective of the host. To send a command to the device, the host downloads and the device receives. To request information from the device, the host uploads and the device transmits.

## Message Structure

The CANopen command/response message structure consists of a start-of-frame (SOF), COB-ID, Control Data bits, 8 bytes of data, and a 16-bit CRC as shown below:

|       | SOF   | COB-ID  |  CTRL | DATA  |  CRC   |
| ----- | ----- | ------  | ----- | ----- | ------ |
| BITS  | 00:04 |  05:15  | 16:23 | 24:87 | 88:103 |

Unlike the CANopen specification, Exoterra byte-aligns the data for serial transmition. The SOF and COB-ID form the first two bytes of the transmission and are constructed as follows:

|       | SOF   | Function Code  | Node ID    |
| ----- | ----- | -------------  | ---------- |
| BITS  | 00:04 | 05:08          | 09:15      |

- The SOF always has the value 0xA8
- The function code can take on a number of different values depending on the communication object
    - 0b0000 = NMT Node Control
    - 0b0001 = SYNC and EMCY message
    - 0b0010 = Timestamp
    - 0b0011, 0b0101, 0b0111, 0b1001 = Transmit PDO
    - 0b0100, 0b0110, 0b1000, 0b1010 = Receive PDO
    - 0b1011 = Transmit (from device) SDO
    - 0b1100 = Receive PDO
    - 0b1110 = NMT Node Monitoring
- The node ID for Halo thrusters is always 0x22 (0b0100010)
- The node ID for the Courier Flight Computer is always 0x23 (0b0100011)

**NOTE:** node ID's are presently hard-coded in the device. If multiple thrusters are connected and powered on the same bus, they will all respond to the same commands. 

To construct the first byte of the transmission, logical OR the SOF bits with the three MSB's of the Function code. To construct the second byte of the transmission, OR the LSB of the function code with the Node ID. The following table provides additional reference for the COB-ID construction:

| Communication Object | COB - ID (h)   | PPU Node         |
| -------------------- | -------------- | ---------------- |
| NMT Node Control     | 0x000          | Receive Only     |
| Sync                 | 0x080          | Receive Only     |
| Emergency            | 0x080 + NodeID | Transmit         |
| TxPDO                | 0x180 + NodeID | 1. Transmit Data |
| RxPDO                | 0x200 + NodeID | 1. Receive Data  |
| TxPDO                | 0x280 + NodeID | 2. Transmit Data |
| RxPDO                | 0x300 + NodeID | 2. Receive Data  |
| TxPDO                | 0x380 + NodeID | 3. Transmit Data |
| TxPDO                | 0x480 + NodeID | 4. Transmit Data |
| RxPDO                | 0x500 + NodeID | 4. Receive Data  |
| SDO                  | 0x580 + NodeID | Transmit         |
| SDO                  | 0x600 + NodeID | Receive          |
| RxPDO                | 0x400 + NodeID | 3. Receive Data  |
| NMT Node Monitoring  | 0x700 + NodeID | Transmit         |

The Control Byte (bits 16:23) consists of the following data fields:

|       | RTR | IDE | Don't Care | Reserved | Length |
| ----- | --- | --- | ---------- | -------- | ------ |
| BITS  | 7   | 6   | 5          | 4        | 3:0    |

- RTR - Remote transimission bit - ignored by Halo, always set to 0
- IDE - Identifier Extension bit - Halo only supports 11-bit ID's, always set to 0
- Reserved - always set to 0
- Length - identifies the number of bytes (8 max) in the Data field.

The contents of the Data field are determined by the message protocol and detailed in later sections of this document.

The CRC is calculated across all fields using the CRC16-IBM format.

## Message Protocols

There are five message protocols defined as follows:

- PDO - Real-time data transferred from peripheral devices. Halo thrusters do not support PDO's between the thruster and the host system/flight computer.
- SDO - Enables access to all objects in the CANopen Object Dictionary
- SYNC - Manufacturer-specific network synchronization protocol 
- EMCY - Emergency message protocol.
- NMT - A master/slave network management protocol providing services for initialization, state control, error and network status

## Object Dictionary

An [object dictionary](https://canopen.readthedocs.io/en/stable/od.html) is used for configuration of all communication with the ECP. It is essentially a grouping of objects accessible via the bus. Each object within the object dictionary is addressed using a 16-bit index and an 8-bit sub-Index. Therefore, an object can contain 256 parameters which are addressed by the sub-Index.

An entry in the object dictionary is defined by:

* Index - the 16-bit address of the object in the dictionary
* Sub-Index - the 8-bit Sub-Index
* Parameter Name - a string describing the entry
* Data Type - gives the datatype of the variable
* Attribute - gives information on the access rights for this entry, this can be read/write (RW), read-only (RO) or write-only (WO)
* Value Range - gives a valid value range for the parameter
* Default - gives the default value for the parameter

Object Example:

| Index  | Sub-Index | Parameter Name | Data Type | Attribute | Persistence | Value Range | Default |
| ------ | --------- | -------------- | --------- | --------- | ----------- | ----------- | ------- |
| 0x3000 | 0x01      | ADC 0          | UINT16    | RO        | Y           | UINT16      | 0       |

&nbsp;

# SYNC and EMCY Protocol

## Synchronization Protocol (SYNC)

The SYNC protocol is a system trigger. The SYNC protocol has a very high priority and has no data. The SYNC protocol is sent to the ECP and can trigger PDO transmissions from the ECP when activated in the corresponding PDO transmission types.

Sync Message Structure:

| COB-ID |      | DATA |
| ------ | ---- | ---- |
| 0x080  | CTRL | -    |

##	Emergency Protocol (EMCY)

The emergency protocol is a high priority message triggered by an error event in the device. The error codes sent with the emergency message are described in the Fault Handling Section.

&nbsp;

# NMT Protocol

The NMT state machine and protocols are used to issue state change commands and detect ECP boot ups. The COB-ID for commanding state changes to the ECP is always 0x000, meaning that it has highest priority.  The ECP will send an initialization message on boot up.  Subsequently NMT state information is available in the heartbeat messages.

&nbsp;

# State Control and Initialization

State control is done utilizing the Network Management (NMT) Protocol. The first data byte of the message indicates the state. The second byte indicates the Node ID (always0x22 for the EPS node).

State Control Data Feilds:

|         | Data Byte 0           | Data Byte 1           |
| ------- | --------------------- | --------------------- |
| Meaning | State Control Command | Addresses Node (0x22) |

State Control Command Options:

| State                                    | Command Specifier |
| ---------------------------------------- | ----------------- |
| Go to Operational Standby Mode *(STNDBY) | 0x01              |
| Go to Stopped Mode *(EGSHTDN)            | 0x02              |
| Go to Pre-Operational / Low Power Mode   | 0x80              |
| Reinitialize                             | 0x81              |
| Reset Communication                      | 0x82              |

*See [Thruster Command](#thruster-commands-object-0x4000) for thruster control and normal thruster shutdown after reaching Standby Mode.

Whenever there is a state change, either in response to one of the commands above or occuring internally, a notification is broadcast by the ECP and always uses the COB-ID of 0x700 + Node ID.

State Change Broadcast Data Fields:

| Data Byte 0   |
| ------------- |
| Current State |

State Data Field Values:

| State                   | Command Specifier |
| ----------------------- | ----------------- |
| Initialization (bootup) | 0x00              |
| Pre-Operational         | 0x7F              |
| Operational             | 0x05              |
| Stopped                 | 0x04              |

&nbsp;

# Service Data Object (SDO) Protocol

Service data objects (SDOs) enable access to all entries of the object dictionary. The service data objects are used to configure the settings for the communication and to set or read application parameters. They are transmitted non-real-time with low priority.

There are two types of transfers used with the EPS, first is the normal expedited transfers. The second is segmented transfers, which allow for larger transfers of data.

## SDO Message Structure

The data bytes for an SDO message consist of a command byte, two command index bytes, a sub-index byte, and up to 4 bytes for a parameter value as shown below:

|         | BYTE 1 |             |        |       |       | BYTE 2-3 | BYTE 4    | BYTE 5-8 |
| ------- | ------ | ----------- | ------ | ----- | ----- | -------- | --------- | -------- |
| Length  | 3 bits | 1 bit       | 2 bits | 1 bit | 1 bit | 2 bytes  | 1 byte    | 4 bytes  |
| Meaning | CSS    | Reserved(0) | n      | e     | s     | index    | Sub-Index | Data     |

BYTE 1, also referred to as the user command byte, is defined as follows:

- ccs - is the client command specifier of the SDO transfer
    - 0 for SDO segment download (0b000)
    - 1 for initiating download (0b001)
    - 2 for initiating upload (0b010)
    - 3 for SDO segment upload (0b011)
    - 4 for aborting an SDO transfer (0b100)
    - 5 for SDO block upload (0b101)
    - 6 for SDO block download (0b110)
- n - is the number of bytes in the data part of the message which do not contain data, only valid if e and s are set
- e - if set, indicates an expedited transfer, i.e., all data exchanged are contained within the message. If this bit is cleared then the message is a segmented transfer where the data does not fit into one message and multiple messages are used.
- s - if set, indicates that the data size is specified in n (if e is set) or in the data part of the message
- index - is the object dictionary index of the data to be accessed
- Sub-Index - is the Sub-Index of the object dictionary variable
- data - contains the data to be uploaded in the case of an expedited transfer (e is set), or the size of the data to be uploaded (s is set, e is not set)

The Index, Sub-Index, and Data bytes are defined in the following sections.

## SDO Supported Command Indexs

The following table defines the indexs supported by the Halo thruster.

| Command  | Index  | CMD Description (From FC)                                                        | Protocol   |
| -------- | ------ | -------------------------------------------------------------------------------- | ---------- |
| THCMD    | 0x4000 | Thruster Control (Ready Mode, Steady State, Normal Shutdown, BIT, etc.)          | SDO        |
| EGSHUTDN | -      | Emergency Graceful Shutdown of the thruster                                      | NMT / EMCY |
| INIT     | -      | Reinitialize the ECP                                                             | NMT        |
| DEVTYPE  | 0x1000 | Device Type                                                                      | SDO        |
| HS0      | 0x1001 | Check the Health and Status of all components in the EPS                         | SDO        |
| SEF      | 0x1003 | Standard Error Field                                                             | SDO        |
| NAME     | 0x1008 | Manufacturer Device Name                                                         | SDO        |
| HWVER    | 0x1009 | Hardware Version                                                                 | SDO        |
| SWVER    | 0x100A | Software Version                                                                 | SDO        |
| HBEAT    | 0x1017 | Heartbeat Time (ms)                                                              | SDO        |
| FRT      | 0x2830 | Fault Reaction Type                                                              | SDO        |
| FSTAT    | 0x2831 | Fault Status                                                                     | SDO        |
| MEL      | 0x2832 | Detailed Error Log by Module                                                     | SDO        |
| DIAG0    | 0x3000 | Return All Telemetry Collected by the ECP MCU                                    | SDO        |
| DIAG1    | 0x3001 | Return All Telemetry Collected by the Keeper MCU                                 | SDO        |
| DIAG2    | 0x3002 | Return All Telemetry Collected by the Anode MCU                                  | SDO        |
| DIAG3    | 0x3003 | Return All Telemetry Collected by the Outer Magnet MCU                           | SDO        |
| DIAG4    | 0x3004 | Return All Telemetry Collected by Inner Magnet MCU                               | SDO        |
| DIAG5    | 0x3005 | Return All Telemetry Collected by Valve                                          | SDO        |
| DIAG10   | 0x3100 | DIAG Memory Block                                                                | SDO        |
| CSTAT    | 0x4001 | Return Conditioning Stats                                                        | SDO        |
| FIRM     | 0x5000 | Firmware Versions                                                                | SDO        |
| TRCMSG   | 0x5001 | Debug Trace Messages                                                             | SDO        |
| CONFIG   | 0x5100 | Configure the thruster settings: put configuration data into non-volatile memory | SDO        |
| CSTATCLR | 0x5401 | Clears Conditioning Stats                                                        | SDO        |
| PROG     | 0x5500 | Reprogram the Unit                                                               | SDO        |
| STNDBY   | -      | Transition the Unit into a Idle / Wait State                                            | NMT        |

&nbsp;

# CANopen Standard Object Dictionary Entries

The object dictionary is split into standardized sections where some entries are mandatory and others are customizable. This section describes the standard mandatory object dictionary entries that are not customized or ECP specific.

## Device Type 0x1000

This parameter indicates the code of the underlying device profile. The default value 401 specifies the device profile for a modular I/O device.

| Index  | Sub-Index  | Parameter Name      | Data Type | Access | Persistence | Value Range | Default |
| ------ | ---------- | ------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x1000 | 0x0        | Device Type         | UINT32    | RO     | -           | UINT32      | 401     |

## Manufacturer Device Name 0x1008

This parameter indicates the name of the system.  The default value is "PPU".

| Index  | Sub-Index  | Parameter Name            | Data Type | Access | Persistence | Value Range | Default |
| ------ | ---------- | ------------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x1008 | 0x00       | Manufacturer Device Name  | STRING    | RO     | -           | None        | "PPU"   |

## Manufacturer Hardware Version 0x1009

This parameter indicates the current hardware version of the thruster control electronics.  The default value is "PPU_SLV" (Silver Coils).

| Index  | Sub-Index  | Parameter Name                | Data Type | Access | Persistence | Value Range | Default   |
| ------ | ---------- | ----------------------------- | --------- | ------ | ----------- | ----------- | --------- |
| 0x1009 | 0x00        | Manufacturer Hardware Version | STRING    | RO     | -           | None        | "PPU_SLV" |


## Software Version 0x100a

This parameter indicates the current top level software version of the ECP.

| Index  | Sub-Index  | Parameter Name                | Data Type | Access | Persistence | Value Range | Default   |
| ------ | ---------- | ----------------------------- | --------- | ------ | ----------- | ----------- | --------- |
| 0x100a | 0x00        | Manufacturer Software Version | STRING    | RO     | -           | None        | ""        |


## Heartbeat 0x1017

Heartbeat time in milliseconds.

| Index  | Sub-Index  | Parameter Name                | Data Type | Access | Persistence | Value Range | Default   |
| ------ | ---------- | ----------------------------- | --------- | ------ | ----------- | ----------- | --------- |
| 0x1017 | 0x00        | Heartbeat Time (ms)           | UINT16    | WR     | -           | UINT16      | 0         |

&nbsp;

# Thruster Commands Object 0x4000

Thruster commands are a custom object dictionary entry for controlling the EPS.

## Thruster Command Table of Sub-Index's

| Index  | Sub-Index | Parameter Name           | Access | Data Value |
| ------ | --------- | ------------------------ | ------ | ---------- |
| 0x4000 | 0x00      | Highest Sub-Index        | RO     | -          |
| 0x4000 | 0x01      | *Ready Mode (Keeper ON)  | RW     | See Below  |
| 0x4000 | 0x02      | Steady State (Anode ON)  | RW     | See Below  |
| 0x4000 | 0x03      | Shutdown                 | WO     | 1          |
| 0x4000 | 0x04      | Set Thrust               | RW     | -          |
| 0x4000 | 0x05      | Thruster Control State   | RO     | See Below  |
| 0x4000 | 0x06      | Condition                | RW     | See Below  |
| 0x4000 | 0x07      | BIT                      | RW     | See Below  |
| 0x4000 | 0x08      | Autostart (experimental) | RW     | See Below  |

*Ready mode command is only supported on the Halo 6 thruster. Halo 12 combines the ready mode and steady state conditions in the Steady State command.

## Sequence Table Commands (Sub-Index 1, 2, 6 & 7)

Commands Ready Mode, Steady State, Condition, and BIT are multi-step commands that can take anywhere from a few seconds to a couple minutes to execute. Execution of these commands is done through Sequence Tables. These tables are arrays of function pointers that execute a series of steps necessary to complete the command. 

Each sequence table command has up to 4 bytes of data for operational parameters. The data bytes are structured as follows:

| Sub-index | Byte 3 | Byte 2 | Byte 1                 | Byte 0                 |
| --------- | ------ | ------ | ---------------------- | ---------------------- |
| 0x01      | 0x00   | 0x00   | Default Setpoint index | 1 to start, 0 to abort |
| 0x02      | 0x00   | 0x00   | Default Setpoint index | 1 to start, 0 to abort |
| 0x06      | 0x00   | 0x00   | 0x00                   | 1 to start, 0 to abort |
| 0x07      | 0x00   | 0x00   | 0x00                   | 1 to start, 0 to abort |

Referring back to the [command byte of the SDO structure](#sdo-message-structure), if the ccs value is 1 (initiating download), then this will either start or abort the sequence as defined in the data byte 0 value. If the ccs value is 2 (initiating upload), the the device will respond with a message indicating which step of the sequence is being executed and its status. This is helpful in debugging the sequence commands. The format of the response is detailed below: 

Sequence Status Response:

|             | Table | Function | Error Code | status |
| ----------- | ----- | -------- | ---------- | ------ |
| Byte Number | 3     | 2        | 1          | 0      |

- Table refers to function pointer tables
    - 0 = utility functions
    - 1 = Keeper (cathode) functions
    - 2 = Anode functions
    - 3 = Magnet functions
    - 5 = Valve functions
- Function refers to the function number currently being executed
- Error code refers to any error that may have occured
- Status refers to the current execution status of the sequence

Sequence Status Values:

| Status    | Value |
| --------- | ----- |
|  Idle     | 0x00 |
|  Queued   | 0x01 |
|  Running  | 0x02 |
|  Error    | 0x03 |
|  Abort    | 0x04 |
|  Success  | 0x05 |

## Thrust Setting (Sub-Index 4)

The Halo thruster has a number of hardcoded setpoints that determine the level of thrust from low to high. There are typically 5 setpoints and they are customer/program specific. The data value determines which setpoint to use.

## Thruster State (Sub-Index 5)

The thruster state indicates the current state of the thruster.

Thruster State Table:

| State Val | State Name                  |
| --------- | --------------------------- |
| 0x01      | Init                        |
| 0x02      | Pre-Operational             |
| 0x03      | Operational                 |
| 0x04      | Stopped                     |
| 0x06      | Powered Off                 |
| 0x07      | Transition Standby          |
| 0x08      | Standby                     |
| 0x09      | Transition Ready Mode       |
| 0x0A      | Ready Mode                  |
| 0x0B      | Transition to Steady State  |
| 0x0C      | Steady State                |
| 0xAC      | Ready Mode and Steady State |
| 0x0D      | Conditioning                |
| 0x0E      | BIT Test                    |
| 0x0F      | Lockout                     |

## Conditioning (Sub-Index 6)

Conditioning is a process to remove atmospheric contaminants at the beginning of a mission and is initiated by writing the step number in the data value. Normally the conditioning process starts with step 1. However, it can be restarted at any step should a step fail or get interrupted. Writing a 0 to the data value will abort the conditioning process. Certain steps in the process have a pre-requisite state when restarting.  For example, to restart the Anode conditioning the thruster must be in Ready Mode.

Conditioning Sequence Table for Halo 6:

| Step  | Name               | Description                                 | Pre-Requisite Mode | Run-Time (hours) |
| ----- | ------------------ | ------------------------------------------- | ------------------ | ---------------- |
|   0   | Cancel sequence    |                                             |                    |                  |
|   1   | Magnet Condition   | Gas Purge/Coil Bakeout                      | STANDBY            | 1.25             |
|   2   | Keeper Condition 1 | Cathode Conditioning at setpoint 1          | STANDBY            | 1.00             |
|   3   | Keeper Condition 2 | Cathode Conditioning at setpoint 2          | READYMODE          | 0.50             |
|   4   | Keeper Condition 3 | Cathode Conditioning at setpoint 3          | READYMODE          | 0.50             |
|   5   | Keeper Condition 4 | Cathode Conditioning at setpoint 4          | READYMODE          | 1.00             |
|   6   | Anode Condition 1  | Low Power Thruster Operation at setpoint 1  | READYMODE          | 0.50             |
|   7   | Anode Condition 2  | Low Power Thruster Operation at setpoint 2  | STEADYSTATE        | 0.50             |

Conditioning Sequence Table for Halo 12:

| Step  | Name               | Description                                 | Pre-Requisite Mode | Run-Time (hours) |
| ----- | ------------------ | ------------------------------------------- | ------------------ | ---------------- |
|   0   | Cancel sequence    |                                             |                    |                  |
|   1   | TBD                | TBD                                         | TBD                | TBD              |
|   2   | TBD                | TBD                                         | TBD                | TBD              |
|   3   | TBD                | TBD                                         | TBD                | TBD              |

## Built-In Test (Sub-Index 7)

Built-In Tests are sequences that test certain aspects of the device and typically only used during development and qualification. The sequence is initiated by writing the desired BIT number to the data value. Writing a 0 to the data value will abort the sequence.

BIT Table for Halo 6:

| BIT Index | BIT Name            |
| --------- | ------------------- |
|  0        | Cancel BIT          |
|  1        | Reserved            |
|  2        | Latch Valve Open    |
|  3        | Latch Valve Close   |
|  4        | Flow Check          |
|  5        | Valve Check         |
|  6        | PCV Drain           |
|  7        | Inner Coil Test     |
|  8        | Outer Coil Test     |
|  9        | Keeper Test         |
| 10        | Anode Test          |
| 11        | LF Check Ambient    |
| 12        | Anode Check Ambient |

BIT Table for Halo 12:

| BIT Index | BIT Name            |
| --------- | ------------------- |
|  0        | TBD                 |
|  1        | TBD                 |
|  2        | TBD                 |
|  3        | TBD                 |

&nbsp;

# Sequence Condition Status Object 0x4001

Each conditioning step consists of two sub-steps, a setup sequence and a monitoring loop. During conditioning the current step, status of step setup sequence, elapsed time of the step (milliseconds), and any error during monitoring of the step are saved in non-volatile memory.  

Status read back values are described in the table below.

| Index  | Sub-Index                               | Parameter Name      | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------------------------------------- | ------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x4001 | 0x0                                     | Highest Sub-Index   | UINT8     | RO     | Y           | UINT8       | -       |
| 0x4001 | 0x1                                     | Current Step        | UINT32    | RO     | Y           | UINT32      | -       |
| 0x4001 | (0x2 * step number) + (step number - 1) | Sequence Status     | UINT32    | RO     | Y           | UINT32      | -       |
| 0x4001 | (0x3 * step number)                     | Elapsed Time        | UINT32    | RO     | Y           | UINT32      | -       |
| 0x4001 | (0x4 * step number) - (step number - 1) | Monitor Error       | UINT32    | RO     | Y           | UINT32      | -       |

Status Explanation:

-  Current Step - Current step in the conditioning process
-  Sequence Status - Status of the setup sequence associated with the conditioning sub-step. Returns the response shown in the table below:

|            | Table #  | Function | Error  | Status |
| ---------- | -------- | -------- | ------ | ------ |
| Nibble     | 7	6   | 5 4      | 3	2 1 | 0      |

- Elapsed Time - Elapsed time of the monitoring sub-step in milliseconds
- Monitor Error - Error during the monitoring sub-step

&nbsp;

# Clear Conditioning Stats 0x5401
Resets all conditioning statuses to zero. A value of 0x63637772 must be sent to Sub-Index 1 to initiate clearing of stats.

__NOTE__ Do not send while conditioning.  

| Index  | Sub-index | Parameter Name   | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | ---------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x5401 | 0x0       | Highest Sub-Index| UINT8     | RO     | -           | UINT8       |         |
| 0x5401 | 0x1       | Clear Statuses   | UINT32    | WO     | -           | 0x63637772  |         |

&nbsp;

# Firmware Versions Object 0x5000

Firmware versions are represented by a Major, Minor, and Revision number packed into a 32 bit integer and a unique build hash identifier. Each device has three redundant backup copies in memory and an executing copy. The redundant copies are stored in the EPS memory. The running version represents the version of firmware running on the device. A value of zero indicates the device has not executed any firmware since the ECP has been initialized.

| Index  | Sub-Index | Parameter Name      | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | ------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x5000 | 0x0       | Highest Sub-Index   | UINT8     | RO     | Y           | UINT8       | -       |
| 0x5000 | 0x1       | Device Select       | UINT8     | RW     | Y           | UINT8       | -       |
| 0x5000 | 0x2       | Executing Version   | UINT32    | RO     | Y           | UINT32      | -       |
| 0x5000 | 0x3       | Executing Hash      | UINT32    | RO     | Y           | UINT32      | -       |
| 0x5000 | 0x4       | Copy 1 Hash         | UINT32    | RO     | Y           | UINT32      | -       |
| 0x5000 | 0x5       | Copy 1 Version      | UINT32    | RO     | Y           | UINT32      | -       |
| 0x5000 | 0x6       | Copy 2 Hash         | UINT32    | RO     | Y           | UINT32      | -       |
| 0x5000 | 0x7       | Copy 2 Version      | UINT32    | RO     | Y           | UINT32      | -       |
| 0x5000 | 0x8       | Copy 3 Hash         | UINT32    | RO     | Y           | UINT32      | -       |
| 0x5000 | 0x9       | Copy 3 Version      | UINT32    | RO     | Y           | UINT32      | -       |

**Note** Sub-index 0x4 through 0x9 are only supported on Halo 6. 

Firmware Versions Byte Explanation:

| Byte       | 3        | 2               | 1               | 0                  |
| ---------- | -------- | --------------- | --------------- | ------------------ |
| Definition | Reserved | Version - Major | Version - Minor | Version - Revision |

Example: firmware version = 0x00010203 indicates major = 1, minor = 2, revision = 3, which equates to Firmware v1.2.3

## Device Selection

This section details how to select which device to read firmware version from.

Device Query Sequence:
1.	SDO download Device Select Value to Index 0x5000, Sub-Index 0x1, to select desired device.
2.	SDO upload device hash or version with corresponding Sub-Index from table below.

Device Selection Values for Halo 6:

| Device Select Value | Device                       |
| ------------------- | ---------------------------- |
| 0                   | Thruster Control             |
| 1                   | Keeper                       |
| 2                   | Anode                        |
| 3                   | Outer Magnet                 |
| 4                   | Inner Magnet                 |
| 5                   | Valves                       |
| 6                   | Thruster Control boot loader |

&nbsp;

# Configuration Object 0x5100

Settings that are changeable by the user.

| Index  | Sub-Index | Parameter Name       | Data Type | Access | Persistence | Value Range     | Default |
| ------ | --------- | -------------------- | --------- | ------ | ----------- | --------------- | ------- |
| 0x5100 | 0x0       | Highest Sub-Index    | UINT8     | RO     | -           | UINT8           | -       |
| 0x5100 | 0x1       | Config Select Byte 0 | UINT8     | RW     | Y           | See Table Below | 0       |

CONFIG SELECT BYTE:

| BIT | Option Name      | Description                         |
| --- | ---------------- | ----------------------------------- |
| 0:1 | Baud Rate Select | 00: 115200 bits/s  11:921600 bits/s |
| 2:7 | TBD              |                                     |

&nbsp;

# Housekeeping Diagnostic Object 0x3000

All telemetry values collected by the ECP are shown below.

House Keeping Diagnostic Values:

| Index  | Sub-Index | Parameter Name        | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | --------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x3000 | 0x0       | Highest Sub-Index     | UINT8     | RO     | -           | UINT8       | -       |
| 0x3000 | 0x1       | Housekeeping 28V (mA) | UINT16    | RO     | -           | UINT16      | -       |
| 0x3000 | 0x2       | Housekeeping 14V (mV) | UINT16    | RO     | -           | UINT16      | -       |
| 0x3000 | 0x3       | Housekeeping 14V (mA) | UINT16    | RO     | -           | UINT16      | -       |
| 0x3000 | 0x4       | Housekeeping 7V  (mV) | UINT16    | RO     | -           | UINT16      | -       |
| 0x3000 | 0x5       | Housekeeping 7V (mA)  | UINT16    | RO     | -           | UINT16      | -       |
| 0x3000 | 0x6       | MECCEMSB (See Below)  | UINT16    | RO     | -           | UINT16      | -       |
| 0x3000 | 0x7       | UECCEMSB (See Below)  | UINT16    | RO     | -           | UINT16      | -       |
| 0x3000 | 0x8       | MECCELSB (See Below)  | UINT16    | RO     | -           | UINT16      | -       |
| 0x3000 | 0x9       | UECCELSB (See Below)  | UINT16    | RO     | -           | UINT16      | -       |
| 0x3000 | 0xA       | Region Status         | UINT16    | RO     | -           | UINT16      | -       |
| 0x3000 | 0xB       | Failed Repairs        | UINT16    | RO     | -           | UINT16      | -       |
| 0x3000 | 0xC       | Repair Status         | UINT16    | RO     | -           | UINT16      | -       |

* __MECCEMSB__ - Number of Multiple ECC Errors detected on the MSB Part of the Memory Flash Data Bus.
* __UECCEMSB__ - Number of Unique ECC Errors corrected on MSB Part of the Memory Flash Data Bus.
* __MECCELSB__ - Number of Multiple ECC Error on LSB Part of the Memory Flash Data Bus.
* __UECCELSB__ - Number of Unique ECC Errors corrected on the LSB Part of the Memory Flash Data Bus.
* __Region Status__  - Bit mask representing the current status of redundant memory copies
* __Failed Repairs__ - Bit mask representing a failed attempt to recover a corrupt redundant copy
* __Repair Status__  - Bit mask representing a successful repair of a redundant copy

&nbsp;

# Keeper Diagnostic Object 0x3001

All diagnostics collected by the Keeper are shown below.

__Keeper Diagnostic Values (0x3001)__

| Index  | Sub-Index | Parameter Name     | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | ------------------ | --------- | ------ | ----------- | ----------- | ------- |
| 0x3001 | 0x0       | Highest Sub-Index  | UINT8     | RO     | -           | UINT8       | -       |
| 0x3001 | 0x1       | SEPIC Voltage (mV) | UINT32    | RO     | -           | UINT32      | -       |
| 0x3001 | 0x2       | Vin (mV)           | UINT16    | RO     | -           | UINT16      | -       |
| 0x3001 | 0x3       | Iout (mA)          | UINT16    | RO     | -           | UINT16      | -       |
| 0x3001 | 0x4       | DAC (count)        | UINT16    | RO     | -           | UINT16      | -       |
| 0x3001 | 0x5       | Last Error         | UINT16    | RO     | -           | UINT16      | -       |
| 0x3001 | 0x6       | Current Offset     | UINT16    | RO     | -           | UINT16      | -       |
| 0x3001 | 0x7       | Message Count      | UINT16    | RO     | -           | UINT16      | -       |
| 0x3001 | 0x8       | CAN Error          | UINT16    | RO     | -           | UINT16      | -       |

**Note:** sub-index 0x8 is not presently supported.

&nbsp;

# Anode Diagnostic Object 0x3002

All diagnostics collected by the Anode MCU are shown below.

Anode Diagnostic Values (0x3002):

| Index  | Sub-Index | Parameter Name      | Data type | Access | Persistence | Value Range | Default |
| ------ | --------- | ------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x3002 | 0x0       | Highest Sub-Index   | UINT8     | RO     | -           | UINT8       | -       |
| 0x3002 | 0x1       | VX (mV)             | UINT32    | RO     | -           | UINT32      | -       |
| 0x3002 | 0x2       | VY (mV)             | UINT32    | RO     | -           | UINT32      | -       |
| 0x3002 | 0x3       | Vout (mV)           | UINT32    | RO     | -           | UINT32      | -       |
| 0x3002 | 0x4       | Iout (mA)           | UINT16    | RO     | -           | UINT16      | -       |
| 0x3002 | 0x5       | DAC (count)         | UINT16    | RO     | -           | UINT16      | -       |
| 0x3002 | 0x6       | HS Temp (degrees C) | UINT16    | RO     | -           | UINT16      | -       |
| 0x3002 | 0x7       | Last Error          | UINT16    | RO     | -           | UINT16      | -       |
| 0x3002 | 0x8       | Current Offset      | UINT16    | RO     | -           | UINT16      | -       |
| 0x3002 | 0x9       | Message Count       | UINT16    | RO     | -           | UINT16      | -       |
| 0x3002 | 0xA       | CAN Error           | UINT16    | RO     | -           | UINT16      | -       |

&nbsp;

# Magnet Diagnostic Object 0x3003/0x3004

All diagnostics collected by the Magnet MCUs are shown below. Halo 6 has an inner (0x3003) and outer (0x3004) magnet. Halo 12 only has one magnet coil and only supports the 0x3003 diagnostic object.

Magnet Diagnostic Values (0x3003/0x3004):

| Index    | Sub-Index | Parameter Name    | Data type | Access | Persistence | Value Range | Default |
| -------- | --------- | ----------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x3003/4 | 0x0       | Highest Sub-Index | UINT8     | RO     | -           | UINT8       | -       |
| 0x3003/4 | 0x1       | Vout (mV)         | UINT16    | RO     | -           | UINT16      | -       |
| 0x3003/4 | 0x2       | Iout (mA)         | UINT16    | RO     | -           | UINT16      | -       |
| 0x3003/4 | 0x3       | DAC (count)       | UINT16    | RO     | -           | UINT16      | -       |
| 0x3003/4 | 0x4       | Last Error        | UINT16    | RO     | -           | UINT16      | -       |
| 0x3003/4 | 0x5       | Message Count     | UINT16    | RO     | -           | UINT16      | -       |
| 0x3003/4 | 0x6       | CAN Error         | UINT16    | RO     | -           | UINT16      | -       |

&nbsp;

# Valve Diagnostic Object 0x3005

All diagnostics collected by the Valve MCU are shown below.

Valve Diagnostic Values (0x3005):

| Index  | Sub-Index | Parameter Name            | Data type | Access | Persistence | Value Range | Default |
| ------ | --------- | ------------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x3005 | 0x0       | Highest Sub-Index         | UINT8     | RO     | -           | UINT8       | -       |
| 0x3005 | 0x1       | Anode Voltage (mV)        | UINT16    | RO     | -           | UINT16      | -       |
| 0x3005 | 0x2       | Cathode HF Voltage (mV)   | UINT16    | RO     | -           | UINT16      | -       |
| 0x3005 | 0x3       | Cathode LF Voltage (mV)   | UINT16    | RO     | -           | UINT16      | -       |
| 0x3005 | 0x4       | Temperature (degrees C)   | INT32     | RO     | -           | INT32       | -       |
| 0x3005 | 0x5       | Tank Pressure (mPSI)      | UINT32    | RO     | -           | UINT32      | -       |
| 0x3005 | 0x6       | Cathode Pressure (mPSI)   | UINT16    | RO     | -           | UINT16      | -       |
| 0x3005 | 0x7       | Anode Pressure (mPSI)     | UINT16    | RO     | -           | UINT16      | -       |
| 0x3005 | 0x8       | Regulator Pressure (mPSI) | UINT16    | RO     | -           | UINT16      | -       |
| 0x3005 | 0x9       | Message Count             | UINT16    | RO     | -           | UINT16      | -       |
| 0x3005 | 0xA       | CAN Error                 | UINT16    | RO     | -           | UINT16      | -       |

&nbsp;

# Memory Diagnostic Object 0x3100

ToDo: why have this at all? Will require a segmented transfer and we'll need to document order of data.

Contiguous Block of all DIAG values above, excluding all 'Highest Sub-Index' values.

DIAG Memory (0x3100):

| Index  | Sub-Index | Parameter Name     | Data type | Access | Persistence | Value Range | Default |
| ------ | --------- | ------------------ | --------- | ------ | ----------- | ----------- | ------- |
| 0x3100 | 0x0       | Highest Sub-Index  | UINT8     | RO     | -           | UINT8       | -       |
| 0x3100 | 0x1       | DIAG Mem           | DOMAIN    | RO     | -           | -           | -       |

&nbsp;

# Debug Trace Messages 0x5001

Circular log buffer containing info, debug, and error messages.  Messages can be downloaded from the tail (oldest) or head (newest)

Trace Object (0x5001):

| Index  | Sub-index | Parameter Name   | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | ---------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x5001 | 0x0       | Highest Sub-Index| UINT8     | RO     | -           | UINT8       |         |
| 0x5001 | 0x1       | Trace Flag       | UINT32    | RW     | -           | UINT8       |         |
| 0x5001 | 0x2       | Trace Head       | UINT32    | RW     | -           | UINT8       |         |
| 0x5001 | 0x3       | Trace Tail       | UINT32    | RW     | -           | UINT8       |         |
| 0x5001 | 0x4       | Trace Size       | UINT32    | RO     | -           | UINT8       |         |
| 0x5001 | 0x5       | Trace Msg Head   | STRING    | RO     | -           | -           |         |
| 0x5001 | 0x6       | Trace Msg Tail   | STRING    | RO     | -           | -           |         |

* __Trace_Flag__ - bitmask that controls log levels and modules
* __Trace_Head__ - position of the newest trace message
* __Trace_Tail__ - position of the oldest trace message
* __Trace_Size__ - size of the trace buffer (number of messages)
* __Trace_Msg_Head__ - Download the newest trace message
* __Trace_Msg_Tail__ - Download the oldest trace message

&nbsp;

# Reprogramming 0x5500

Reprogram Table (0x5500):

| Index  | Sub-index | Parameter Name    | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | ----------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x5500 | 0x0       | Highest Sub-Index | UINT8     | RO     | -           | UINT8       | -       |
| 0x5500 | 0x1       | Program           | UINT32    | WO     | -           | UINT32      | -       |
| 0x5500 | 0x2       | Verify            | UINT32    | RW     | -           | UINT32      | -       |
| 0x5500 | 0x3       | Install           | UINT8     | WO     | -           | UINT8       | -       |
| 0x5500 | 0x4       | Program Enable    | UINT8     | RO     | -           | UINT8       | -       |

* __Program__ - used to upload binary update file
* __Verify__ - write a 1 to trigger an integrity check of the uploaded update binary.  Perform an SDO download to readback result
* __Install__ - write a 1 to install trigger the firmware install
* __Program_Enable__ - program enable discrete state

&nbsp;

# Special Sequences

## Device Reprogramming
The reprogramming sequence is as follows:
1. User asserts “Reprogram” discrete on external connector (if applicable)
2. Perform SDO upload of binary update file to Program Sub-Index in the Reprogram table
3. An integrity check is performed by the ECP when the last data packet is received.  The transfer will fail if the integrity check fails.  The integrity check can be repeated at any time using the Verify Sub-Index in the Reprogram Table.  If the verify fails, the write to the Verify Sub-Index will fail
4. Client writes a '1' to the Install Sub-Index in the Reprogram Table to trigger the install process
5. ECP boot loader image is copied into executable memory by ECP application and the unit is reset
6. The boot loader installs the new ECP application and all redundant device images
7. The next time the system is put into Operational mode the ECP will boot load all client MCUs with the newly installed version

&nbsp;

# Fault Handling

## Fault Reaction
If a fault occurs, the EPS software throws a fault. The corresponding fault is set in the fault state \<FaultStatus\> (0x2831). If no fault reaction for this fault is defined within the parameter \<FaultReactionType\> (0x2830) no further fault reaction is done. If a fault reaction is configured, the fault code is saved in an array \<StandardErrorField\> (0x1003) which holds the last eight thrown faults. If the configured fault reaction of the actual thrown fault requests a change of the device state, the corresponding transition of the device state machine will be forced.

## Fault Table
The faults that can occur in the PPU and the actions taken by the ECP to handle the fault are shown in the table below.  The Data Word column corresponds to the \<FaultStatus\>(0x2831) Fault Status Data Word numbers.  The Bit column corresponds to the bit position in the specified Fault Status Data Word.

| Fault Code | Fault Code | Data Word |     |                                         |                                                                  | \<FaultReactionType\> (0x2830) |
| ---------- | ---------- | --------- | --- | --------------------------------------- | ---------------------------------------------------------------- | ------------------------------ |
| Dec        | Hex        | Num       | Bit | Fault Name                              | Fault Description                                                | Value                          |
| 0          | 0x0        | -         | -   | Benign Reset or No Error                | No Fault                                                         | -                              |
| 1          | 0x1        | 1         | 0   | HK/M Over Current                       | Over Current Detected on the Housekeeping/Magnet Card            | 0x03                           |
| 2          | 0x2        | 1         | 1   | HK/M Over Voltage                       | Voltage on Housekeeping/Magnet Card above specified range        | 0x02                           |
| 3          | 0x3        | 1         | 2   | HK/M Under Voltage                      | Voltage on Housekeeping/Magnet Card below specified range        | 0x02                           |
| 4          | 0x4        | 1         | 3   | HK/M Short Detected                     | A short has been detected on the Housekeeping/Magnet Card        | 0x02                           |
| 5          | 0x5        | 1         | 4   | HK/M Comm Lost                          | The Housekeeping/Magnet Card is no longer responding             | 0x02                           |
| 6          | 0x6        | 1         | 5   | Reserved                                |                                                                  |                                |
| 7          | 0x7        | 1         | 6   | Reserved                                |                                                                  |                                |
| 8          | 0x8        | 1         | 7   | Reserved                                |                                                                  |                                |
| 9          | 0x9        | 1         | 8   | Reserved                                |                                                                  |                                |
| 10         | 0xA        | 1         | 9   | Reserved                                |                                                                  |                                |
| 11         | 0xB        | 1         | 10  | Reserved                                |                                                                  |                                |
| 12         | 0xC        | 1         | 11  | ECP Comm Timing Error                   | ECP CAN driver shows timing error occurred                       | 0x02                           |
| 13         | 0xD        | 1         | 12  | ECP Comm CRC Error                      | ECP CAN driver shows CRC error occurred                          | 0x02                           |
| 14         | 0xE        | 1         | 13  | Reserved                                |                                                                  |                                |
| 15         | 0xF        | 1         | 14  | ECP Watchdog Timeout                    | ECP had watchdog timeout error                                   | 0x01                           |
| 16         | 0x10       | 1         | 15  | ECP Memory Fault                        | ECP memory check showed error                                    | 0x01                           |
| 17         | 0x11       | 1         | 16  | ECP Brownout Detected                   | ECP brownout detector tripped                                    | 0x02                           |
| 18         | 0x12       | 1         | 17  | ECP Install Error                       | Installation of ECP App failure                                  | 0x01                           |
| 19         | 0x13       | 1         | 18  | BL Install Error                        | Installation of ECP Boot loader failure                          | 0x01                           |
| 20         | 0x14       | 1         | 19  | ECP Unknown Reset                       | ECP was reset for reasons unknown                                | 0x01                           |
| 21         | 0x15       | 1         | 20  | ECP Backup Reset                        | ECP was reset from back up                                       | 0x01                           |
| 22         | 0x16       | 1         | 21  | ECP NRST                                | ECP was reset by reset signal                                    | 0x01                           |
| 23         | 0x17       | 1         | 22  | ECP EEFC ECC Error                      | ECP Flash Controller ECC Error                                   | 0x01                           |
| 24         | 0x18       | 1         | 23  | Reserved                                |                                                                  |                                |
| 25         | 0x19       | 1         | 24  | Inner Magnet Comm Unknown Command Error | The Inner Magnet MCU received an unknown command                 | 0x01                           |
| 26         | 0x1A       | 1         | 25  | Inner Magnet Comm Timing Error          | Inner Magnet MCU CAN driver shows timing error occurred          | 0x01                           |
| 27         | 0x1B       | 1         | 26  | Inner Magnet Comm CRC Error             | Inner Magnet MCU CAN driver shows CRC error occurred             | 0x01                           |
| 28         | 0x1C       | 1         | 27  | Inner Magnet Voltage                    | Inner Magnet over or under voltage                               | 0x04                           |
| 29         | 0x1D       | 1         | 28  | Inner Magnet Watchdog Timeout           | Inner Magnet MCU had watchdog timeout error                      | 0x02                           |
| 30         | 0x1E       | 1         | 29  | Inner Magnet Memory Fault               | Inner Magnet MCU memory check showed error                       | 0x02                           |
| 31         | 0x1F       | 1         | 30  | Inner Magnet Brownout Detected          | Inner Magnet MCU brownout detector tripped                       | 0x02                           |
| 32         | 0x20       | 1         | 31  | Inner Magnet Magnet Under Current       | Inner Magnet Under Current                                       | 0x03                           |
| 33         | 0x21       | 2         | 0   | Inner Magnet Magnet Over Current        | Inner Magnet Over Current                                        | 0x03                           |
| 34         | 0x22       | 2         | 1   | Reserved                                |                                                                  |                                |
| 35         | 0x23       | 2         | 2   | Reserved                                |                                                                  |                                |
| 36         | 0x24       | 2         | 3   | Reserved                                |                                                                  |                                |
| 37         | 0x25       | 2         | 4   | Reserved                                |                                                                  |                                |
| 38         | 0x26       | 2         | 5   | Reserved                                |                                                                  |                                |
| 39         | 0x27       | 2         | 6   | Outer Magnet Comm Unknown Command Error | The Outer Magnet MCU received an unknown command                 | 0x01                           |
| 40         | 0x28       | 2         | 7   | Outer Magnet Comm Timing Error          | Outer Magnet MCU CAN driver shows timing error occurred          | 0x01                           |
| 41         | 0x29       | 2         | 8   | Outer Magnet Comm CRC Error             | Outer Magnet MCU CAN driver shows CRC error occurred             | 0x01                           |
| 42         | 0x2A       | 2         | 9   | Outer Magnet  Voltage                   | Outer Magnet over or under voltage                               | 0x04                           |
| 43         | 0x2B       | 2         | 10  | Outer Magnet Watchdog Timeout           | Outer Magnet MCU had watchdog timeout error                      | 0x02                           |
| 44         | 0x2C       | 2         | 11  | Outer Magnet Memory Fault               | Outer Magnet MCU memory check showed error                       | 0x02                           |
| 45         | 0x2D       | 2         | 12  | Outer Magnet Brownout Detected          | Outer Magnet MCU brownout detector tripped                       | 0x02                           |
| 46         | 0x2E       | 2         | 13  | Outer Magnet Magnet Under Current       | Outer Magnet Under Current                                       | 0x03                           |
| 47         | 0x2F       | 2         | 14  | Outer Magnet Magnet Over Current        | Outer Magnet Over Current                                        | 0x03                           |
| 48         | 0x30       | 2         | 15  | Reserved                                |                                                                  |                                |
| 49         | 0x31       | 2         | 16  | Reserved                                |                                                                  |                                |
| 50         | 0x32       | 2         | 17  | Reserved                                |                                                                  |                                |
| 51         | 0x33       | 2         | 18  | Reserved                                |                                                                  |                                |
| 52         | 0x34       | 2         | 19  | Reserved                                |                                                                  |                                |
| 53         | 0x35       | 2         | 20  | ACK Over Current                        | Over Current Detected on the Anode Cathode Keeper Card           | 0x02                           |
| 54         | 0x36       | 2         | 21  | ACK Over Voltage                        | Voltage on Anode Cathode Keeper Card above specified range       | 0x02                           |
| 55         | 0x37       | 2         | 22  | ACK Under Voltage                       | Voltage on Anode Cathode Keeper Card below specified range       | 0x02                           |
| 56         | 0x38       | 2         | 23  | ACK Short Detected                      | A short has been detected on the Anode Cathode Keeper Card       | 0x02                           |
| 57         | 0x39       | 2         | 24  | Reserved                                |                                                                  |                                |
| 58         | 0x3A       | 2         | 25  | Reserved                                |                                                                  |                                |
| 59         | 0x3B       | 2         | 26  | Reserved                                |                                                                  |                                |
| 60         | 0x3C       | 2         | 27  | Anode General                           | The Anode MCU had a general error                                | 0x04                           |
| 61         | 0x3D       | 2         | 28  | Anode Comm Lost                         | The Anode MCU is no longer responding                            | 0x02                           |
| 62         | 0x3E       | 2         | 29  | Anode Comm Unknown Command Error        | The Anode MCU received an unknown command                        | 0x01                           |
| 63         | 0x3F       | 2         | 30  | Anode Comm Timing Error                 | Anode MCU CAN driver shows timing error occurred                 | 0x01                           |
| 64         | 0x40       | 2         | 31  | Anode Comm CRC Error                    | Anode MCU CAN driver shows CRC error occurred                    | 0x01                           |
| 65         | 0x41       | 3         | 0   | Anode Heat Sink Temp                    | Anode head sink temp is too high                                 | 0x01                           |
| 66         | 0x42       | 3         | 1   | Anode Watchdog Timeout                  | Anode MCU had watchdog timeout error                             | 0x02                           |
| 67         | 0x43       | 3         | 2   | Anode Memory Fault                      | Anode MCU memory check showed error                              | 0x02                           |
| 68         | 0x44       | 3         | 3   | Anode Brownout Detected                 | Anode MCU brownout detector tripped                              | 0x02                           |
| 69         | 0x45       | 3         | 4   | Anode Under Voltage                     | Anode under voltage                                              | 0x03                           |
| 70         | 0x46       | 3         | 5   | Anode Over Voltage                      | Anode over voltage                                               | 0x04                           |
| 71         | 0x47       | 3         | 6   | Anode X Overcurrent                     | Anode X overcurrent                                              | 0x04                           |
| 72         | 0x48       | 3         | 7   | Anode Y Overcurrent                     | Anode Y overcurrent                                              | 0x04                           |
| 73         | 0x49       | 3         | 8   | Anode Undercurrent                      | Anode undercurrent                                               | 0x04                           |
| 74         | 0x4A       | 3         | 9   | Anode No Spark                          | No spark detected during startup                                 | 0x01                           |
| 75         | 0x4B       | 3         | 10  | Anode Voltage Balance Error             | Anode X/Y balance error                                          | 0x04                           |
| 76         | 0x4C       | 3         | 11  | Reserved                                |                                                                  |                                |
| 77         | 0x4D       | 3         | 12  | Keeper Comm Lost                        | The Keeper MCU is no longer responding                           | 0x02                           |
| 78         | 0x4E       | 3         | 13  | Keeper Comm Unknown Command Error       | The Keeper MCU received an unknown command                       | 0x02                           |
| 79         | 0x4F       | 3         | 14  | Keeper Comm Timing Error                | Keeper MCU CAN driver shows timing error occurred                | 0x02                           |
| 80         | 0x50       | 3         | 15  | Keeper Comm CRC Error                   | Keeper MCU CAN driver shows CRC error occurred                   | 0x02                           |
| 81         | 0x51       | 3         | 16  | Keeper No Spark Error                   | No spark was detected when starting the Keeper                   | 0x04                           |
| 82         | 0x52       | 3         | 17  | Keeper Watchdog Timeout                 | Keeper MCU Microprocessor had watchdog timeout error             | 0x02                           |
| 83         | 0x53       | 3         | 18  | Keeper Memory Fault                     | Keeper MCU memory check showed error                             | 0x02                           |
| 84         | 0x54       | 3         | 19  | Keeper Brownout Detected                | Keeper MCU brownout detected                                     | 0x02                           |
| 85         | 0x55       | 3         | 20  | Keeper Over Voltage Voltage Control     | Keeper Voltage limit exceeded in voltage control mode            | 0x04                           |
| 86         | 0x56       | 3         | 21  | Keeper Over Voltage Current Control     | Keeper Voltage limit exceeded in current control mode            | 0x04                           |
| 87         | 0x57       | 3         | 22  | Keeper Over Voltage                     | Keeper Voltage limit exceeded                                    | 0x04                           |
| 88         | 0x58       | 3         | 23  | Keeper Current Low                      | Keeper Current low limit exceeded                                | 0x04                           |
| 89         | 0x59       | 3         | 24  | Reserved                                |                                                                  |                                |
| 90         | 0x5A       | 3         | 25  | Reserved                                |                                                                  |                                |
| 91         | 0x5B       | 3         | 26  | Reserved                                |                                                                  |                                |
| 92         | 0x5C       | 3         | 27  | Reserved                                |                                                                  |                                |
| 93         | 0x5D       | 3         | 28  | VCC Over Current                        | Over Current Detected on the Valve Controller Card               | 0x02                           |
| 94         | 0x5E       | 3         | 29  | VCC Over Voltage                        | Voltage on Valve Controller Card above specified range           | 0x02                           |
| 95         | 0x5F       | 3         | 30  | VCC Under Voltage                       | Voltage on Valve Controller Card below specified range           | 0x02                           |
| 96         | 0x60       | 3         | 31  | VCC Short Detected                      | A short has been detected on the Valve Controller Card           | 0x02                           |
| 97         | 0x61       | 4         | 0   | VCC Comm Lost                           | The Valve Controller Card is no longer responding                | 0x02                           |
| 98         | 0x62       | 4         | 1   | Reserved                                |                                                                  |                                |
| 99         | 0x63       | 4         | 2   | Reserved                                |                                                                  |                                |
| 100        | 0x64       | 4         | 3   | Reserved                                |                                                                  |                                |
| 101        | 0x65       | 4         | 4   | Reserved                                |                                                                  |                                |
| 102        | 0x66       | 4         | 5   | Reserved                                |                                                                  |                                |
| 103        | 0x67       | 4         | 6   | Valve Comm Unknown Command Error        | The Valve MCU received an unknown command                        | 0x01                           |
| 104        | 0x68       | 4         | 7   | Valve Comm Timing Error                 | Valve MCU CAN driver shows timing error occurred                 | 0x01                           |
| 105        | 0x69       | 4         | 8   | Valve Comm CRC Error                    | Valve MCU CAN driver shows CRC error occurred                    | 0x01                           |
| 106        | 0x6A       | 4         | 9   | Reserved                                |                                                                  |                                |
| 107        | 0x6B       | 4         | 10  | Valve Watchdog Timeout                  | Valve MCU Microprocessor had watchdog timeout error              | 0x02                           |
| 108        | 0x6C       | 4         | 11  | Valve Memory Fault                      | VCC memory check showed error                                    | 0x02                           |
| 109        | 0x6D       | 4         | 12  | Valve Brownout Detected                 | VCC brownout detector tripped                                    | 0x02                           |
| 110        | 0x6E       | 4         | 13  | Reserved                                |                                                                  |                                |
| 111        | 0x6F       | 4         | 14  | Reserved                                |                                                                  |                                |
| 112        | 0x70       | 4         | 15  | Reserved                                |                                                                  |                                |
| 113        | 0x71       | 4         | 16  | Reserved                                |                                                                  |                                |
| 114        | 0x72       | 4         | 17  | Reserved                                |                                                                  |                                |
| 115        | 0x73       | 4         | 18  | Reserved                                |                                                                  |                                |
| 116        | 0x74       | 4         | 19  | Reserved                                |                                                                  |                                |
| 117        | 0x75       | 4         | 20  | MCU Boot Error                          | One or more MCUs had an error booting                            | 0x02                           |
| 118        | 0x76       | 4         | 21  | MCU Thruster Fault                      | One of more MCUs are not controlling thruster operation properly | 0x02                           |
| 119        | 0x77       | 4         | 22  | Reserved                                |                                                                  |                                |
| 120        | 0x78       | 4         | 23  | Reserved                                |                                                                  |                                |
| 121        | 0x79       | 4         | 24  | Reserved                                |                                                                  |                                |
| 122        | 0x7A       | 4         | 25  | Thruster Power High Fault               | Thruster power is high enough to risk part damage                | 0x04                           |
| 123        | 0x7B       | 4         | 26  | Thruster Power High Warn                | Thruster power is above expected level                           | 0x01                           |
| 124        | 0x7C       | 4         | 27  | Thruster Power Low Warn                 | Thruster power is below expected level                           | 0x01                           |
| 125        | 0x7D       | 4         | 28  | Reserved                                |                                                                  |                                |
| 126        | 0x7E       | 4         | 29  | Reserved                                |                                                                  |                                |
| 127        | 0x7F       | 4         | 30  | Reserved                                |                                                                  |                                |
| 128        | 0x80       | 4         | 31  | Reserved                                |                                                                  |                                |
| 129        | 0x81       | 5         | 0   | Memory Scrubber Fault                   | An ICM memory mismatch was detected                              | 0x01                           |
| 130        | 0x82       | 5         | 1   | Reserved                                |                                                                  |                                |
| 131        | 0x83       | 5         | 2   | Reserved                                |                                                                  |                                |
| 132        | 0x84       | 5         | 3   | Reserved                                |                                                                  |                                |
| 133        | 0x85       | 5         | 4   | Reserved                                |                                                                  |                                |
| 134        | 0x86       | 5         | 5   | Reserved                                |                                                                  |                                |
| 135        | 0x87       | 5         | 6   | Reserved                                |                                                                  |                                |
| 136        | 0x88       | 5         | 7   | Reserved                                |                                                                  |                                |
| 137        | 0x89       | 5         | 8   | Comm Overrun Error                      | A comm buffer overflowed                                         | 0x01                           |
| 138        | 0x8A       | 5         | 9   | Comm CRC error                          | Received/calculated CRC mismatch                                 | 0x01                           |
| 139        | 0x8B       | 5         | 10  | Comm Frame Error                        | Frame format error                                               | 0x01                           |
| 140        | 0x8C       | 5         | 11  | Reserved                                |                                                                  |                                |
| 141        | 0x8D       | 5         | 12  | Reserved                                |                                                                  |                                |
| 142        | 0x8E       | 5         | 13  | Reserved                                |                                                                  |                                |
| 143        | 0x8F       | 5         | 14  | Reserved                                |                                                                  |                                |
| 144        | 0x90       | 5         | 15  | Reserved                                |                                                                  |                                |
| 145        | 0x91       | 5         | 16  | Reserved                                |                                                                  |                                |
| 146        | 0x92       | 5         | 17  | Reserved                                |                                                                  |                                |
| 147        | 0x93       | 5         | 18  | Reserved                                |                                                                  |                                |
| 148        | 0x94       | 5         | 19  | Reserved                                |                                                                  |                                |
| 149        | 0x95       | 5         | 20  | Reserved                                |                                                                  |                                |
| 150        | 0x96       | 5         | 21  | Reserved                                |                                                                  |                                |
| 151        | 0x97       | 5         | 22  | Ready Mode Fault                        | Fault transitioning to Ready Mode                                | 0x01                           |
| 152        | 0x98       | 5         | 23  | Steady State Fault                      | Fault transitioning to Steady State                              | 0x01                           |
| 153        | 0x99       | 5         | 24  | Throttling Fault                        | Fault transitioning to throttle setpoint                         | 0x04                           |
| 154        | 0x9a       | 5         | 25  | Conditioning Fault                      | Fault during a conditioning sequence                             | 0x04                           |
| 155        | 0x9b       | 5         | 26  | BIT Fault                               | Fault during a BIT                                               | 0x01                           |
| 156        | 0x9c       | 5         | 27  | Reserved                                |                                                                  |                                |
| 157        | 0x9d       | 5         | 28  | Reserved                                |                                                                  |                                |
| 158        | 0x9e       | 5         | 29  | Reserved                                |                                                                  |                                |
| 159        | 0x9f       | 5         | 30  | Reserved                                |                                                                  |                                |
| 160        | 0xa0       | 5         | 31  | Unknown Err                             | An unknown error occurred                                        | 0x01                           |

## Fault Reaction Type

The fault reaction parameter \<FaultReactionType\> (0x2830), shown below, is used to define the fault behavior for each fault code. The fault reaction for each fault event is configured with different fault reaction types.

| Index  | Sub-Index | Parameter Name       | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | -------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x2830 | 0         | Sub-Index Count      | UINT8     | RO     | Y           | UINT8       | -       |
| 0x2830 | 1         | Fault Code Select    | UINT8     | RW     | Y           | 0-160       | 0       |
| 0x2830 | 2         | Fault Reaction Value | UINT8     | RW     | Y           | 0-4         | 0       |


## Fault Reaction Value Description

| FaultReactionType  | Description |
| ------------------ | ----------- |
| 0 (NONE)           | No fault reaction, error is ignored |
| 1 (ALERT)          | Send an emergency message. If a malfunction for the monitored fault is detected, an emergency message will be sent onto the bus. The PPU continues to operate. Special care must be taken, as the malfunction may have an impact on the PPU. |
| 2 (ALERT_PREOP)    | If a malfunction for the monitored fault is detected, the device state machine enters the Pre-Operational state and an emergency message will be sent onto the bus. The power stage of the MCUs are switched off, while all ECP functions are still alive. The ECP must be transitioned to the Operational state to return to normal operation. |
| 3 (ALERT_LOCKOUT)  | A fault was detected that might result in part damage if certain state changes are allowed.  Lockout ECP until devices can safely transition states again. |
| 4 (ALERT_SHUTDOWN) | A fault was detected that requires the thruster to change state back to Standby. |

## Fault Status Table

The bit coded fault status indicates which faults are currently reported for the device. Each bit of the fault status array (x5 32-bit words, built with the sub-indexes 2-6 of the fault status) stands for a fault code. The corresponding fault code is equal to the bit position in the 160-bit field (5x32 bit). Fault status is shown below.

| Index  | Sub-Index | Parameter Name               | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | ---------------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x2831 | 0x0       | Highest Sub-Index            | UINT8     | RO     | -           | UINT32      | -       |
| 0x2831 | 0x1       | Fault Status Code (Dump All) | Domain    | RO     | -           | UINT32      | 0       |
| 0x2831 | 0x2       | Fault Status Data Word 1     | UINT32    | RO     | -           | UINT32      | 0       |
| 0x2831 | 0x3       | Fault Status Data Word 2     | UINT32    | RO     | -           | UINT32      | 0       |
| 0x2831 | 0x4       | Fault Status Data Word 3     | UINT32    | RO     | -           | UINT32      | 0       |
| 0x2831 | 0x5       | Fault Status Data Word 4     | UINT32    | RO     | -           | UINT32      | 0       |
| 0x2831 | 0x6       | Fault Status Data Word 5     | UINT32    | RO     | -           | UINT32      | 0       |

## Error Register Table

The \<ErrorRegister\> (0x1001) displays the error state of the system in bit-coded form. Bit-code 0 of the \<ErrorRegister\> (0x1001) will always be set in an error state.  Bits 1-7 are set when their corresponding error codes are set. Bit-codes are cleared when __all__ error codes with the corresponding bit-code have been cleared.

| Index  | Sub-Index | Parameter Name | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | -------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x1001 | 0x0       | Error Register | UINT8     | RO     | \-          | UINT8       | 0       |

## 8 Bit Error Types Table

| Bit-code | Description               |
| -------- | ------------------------- |
| 0        | Generic error (any error) |
| 1        | Current Error             |
| 2        | Voltage Error             |
| 3        | Temperature Error         |
| 4        | Communication Error       |
| 5        | Profile Error             |
| 6        | Reserved                  |
| 7        | Manufacturer Error        |

## 16 Bit Error Codes Table

CANopen Specific error codes are used in conjunction with 8-bit Error Types and user defined error fields to construct emergency messages (see Example Emergency Message Decoding).  The 16-bit error codes are described in the table below.  

| Error Code Value | Error Code Name                    | Description                           |
|------------------|------------------------------------|---------------------------------------|
| 0x0000           | CO_EMCY_CODE_NO_ERR                | Error reset or no error               |
| 0x1000           | CO_EMCY_CODE_GEN_ERR               | Generic error                         |
| 0x2000           | CO_EMCY_CODE_CUR_ERR               | Current                               |
| 0x2100           | CO_EMCY_CODE_CUR_INPUT_ERR         | Current, device input side            |
| 0x2200           | CO_EMCY_CODE_CUR_INTERN_ERR        | Current inside the device             |
| 0x2300           | CO_EMCY_CODE_CUR_OUTPUT_ERR        | Current, device output side           |
| 0x3000           | CO_EMCY_CODE_VOL_ERR               | Voltage                               |
| 0x3100           | CO_EMCY_CODE_VOL_INPUT_ERR         | Mains voltage                         |
| 0x3200           | CO_EMCY_CODE_VOL_INTERN_ERR        | Voltage inside the device             |
| 0x3300           | CO_EMCY_CODE_VOL_OUTPUT_ERR        | Output voltage                        |
| 0x4000           | CO_EMCY_CODE_TEMP_ERR              | Temperature                           |
| 0x4100           | CO_EMCY_CODE_TEMP_AMBIENT_ERR      | Ambient temperature                   |
| 0x4200           | CO_EMCY_CODE_TEMP_DEVICE_ERR       | Device temperature                    |
| 0x5000           | CO_EMCY_CODE_HW_ERR                | Device hardware                       |
| 0x6000           | CO_EMCY_CODE_SW_ERR                | Device software                       |
| 0x6100           | CO_EMCY_CODE_SW_INTERNAL_ERR       | Internal software                     |
| 0x6200           | CO_EMCY_CODE_SW_USER_ERR           | User software                         |
| 0x6300           | CO_EMCY_CODE_SW_DATASET_ERR        | Data set                              |
| 0x7000           | CO_EMCY_CODE_ADD_MODULES_ERR       | Additional modules                    |
| 0x8000           | CO_EMCY_CODE_MON_ERR               | Monitoring                            |
| 0x8100           | CO_EMCY_CODE_MON_COM_ERR           | Communication                         |
| 0x8110           | CO_EMCY_CODE_MON_COM_OVERRUN_ERR   | CAN overrun (objects lost)            |
| 0x8120           | CO_EMCY_CODE_MON_COM_PASSIVE_ERR   | CAN in error passive mode             |
| 0x8130           | CO_EMCY_CODE_MON_COM_HEARTBEAT_ERR | Life guard error or heartbeat error   |
| 0x8140           | CO_EMCY_CODE_MON_COM_RECOVER_ERR   | Recovered from bus off                |
| 0x8150           | CO_EMCY_CODE_MON_COM_COLLISION_ERR | Transmit COB-ID collision             |
| 0x8200           | CO_EMCY_CODE_MON_PROT_ERR          | Protocol error                        |
| 0x8210           | CO_EMCY_CODE_MON_PROT_PDO_IGN_ERR  | PDO not processed due to length error |
| 0x8220           | CO_EMCY_CODE_MON_PROT_PDO_LEN_ERR  | PDO length exceeded                   |
| 0x9000           | CO_EMCY_CODE_EXT_ERR               | External error                        |
| 0xF000           | CO_EMCY_CODE_ADD_FUNC_ERR          | Additional functions                  |
| 0xFF00           | CO_EMCY_CODE_DEV_ERR               | Device specific                       |

## Error History (0x1003)

When a fault is thrown, information about the fault is stored to the \<StandardErrorField\> (0x1003) parameter array. The \<StandardErrorField\> (0x1003) parameter array contains a list of up to 7 entries. This Fault Code provides information about the reason for the error. The parameter \<NumberOfErrors\> (0x1003) holds information about the number of errors currently recorded. Every new fault is stored in the first element of the parameter array \<FaultCode\> (0x1003), the older ones move down in the list. If the maximum number of entries is reached, and a new fault occurred, the oldest fault information will be deleted. Writing the value 0 to the object \<NumberOfErrors\> (0x1003), sub-index 0, deletes the entire error code history. Standard error field is shown in the table below.

Standard Error Field

| Index  | Sub-Index | Parameter Name       | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | -------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x1003 | 0x0       | Number of Errors     | UINT8     | RW     | \-          | UINT8       | 0       |
| 0x1003 | 0x1…7     | Standard Error Field | UINT32    | RO     | \-          | UINT32      | 0       |

Example Standard Error Field Decode:

| Data             | Description      | Decoding                                                        |
| ---------------- | ---------------- | --------------------------------------------------------------- |
| 0x __0081__ 8B00 | 16-bit CANopen error code | Communication error, refer to 16-Bit Error Codes Table         |
| 0x0081 __8B00__  | Fault Code                | Comm Frame Error, see Fault Table                              |


## Module Error Log Table (0x2832)
Additional error logs are also maintained on a per-module basis.  Each module subscribed to error handling keeps a circular buffer of its error history.

| Index  | Sub-Index | Parameter Name            | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | ------------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x2832 | 0x1       | Serial Error Log          | Domain    | RO     | -           | See Below   | 0       |
| 0x2832 | 0x2       | Client Control Error Log  | Domain    | RO     | -           | See Below   | 0       |
| 0x2832 | 0x3       | Memory Scrubber Error Log | Domain    | RO     | -           | See Below   | 0       |
| 0x2832 | 0x4       | Application Error Log     | Domain    | RO     | -           | See Below   | 0       |
| 0x2832 | 0x5       | Update Command            | Domain    | RO     | -           | See Below   | 0       |
| 0x2832 | 0x6       | Anode Control             | Domain    | RO     | -           | See Below   | 0       |

Module Error Logs are Arrays of Data Structures. Every module's error detail data structure includes a __base error detail__ and __module specific error details__

Base Error Detail (__base_error_detail_t__)

| Name        | Data Type | Description                                |
| ----------- | --------- | ------------------------------------------ |
| Error Count | UINT32    | Position of error in log array             |
| Line Number | UINT16    | Line number in code where error was logged |
| Error Type  | UINT8     | General type of error logged               |
| Error Code  | UINT8     | Fault number                               |

Serial Error Log (__serial_error_detail_t__)

| Name        | Data Type           | Description               |
| ----------- | ------------------- | ------------------------- |
| Base Detail | base_error_detail_t | Base module error details |


Client Control Error Log (__client_control_error_detail_t__)

| Name        | Data Type           | Description               |
| ----------- | ------------------- | ------------------------- |
| Base Detail | base_error_detail_t | Base module error details |
| Boot Status | UINT32              | Client MCU boot status    |
| Power Level | UINT32              | Client MCU power Level    |

Memory Scrubber Error Log (__memory_scrubber_error_detail_t__)

| Name        | Data Type           | Description                            |
| ----------- | ------------------- | -------------------------------------- |
| Base Detail | base_error_detail_t | Base module error details              |
| EEFC FSR    | UINT32              | Flash controller fault status register |
| Digest      | UINT32              | Bit mask for region digest mismatches  |

Application Error Log (__app_error_detail_t__)

| Name           | Data Type           | Description                                   |
| -------------- | ------------------- | --------------------------------------------- |
| Base Detail    | base_error_detail_t | Base module error details                     |
| Repair Status  | UINT32              | Repair status of regions monitored by the ICM |
| Install Status | UINT32              | ECP app or boot loader install status         |
| Install Error  | UINT32              | ECP app or boot loader install errors Detected|
| Reset Reason   | UINT32              | Last detected reset reason                    |

Update Command (__update_error_detail_t__)

| Name          | Data Type           | Description               |
| ------------- | ------------------- | ------------------------- |
| Base Detail   | base_error_detail_t | Base module error details |
| Update Status | UINT32              | Software update Status    |

 Anode Control (__anode_error_detail_t__)

| Name            | Data Type           | Description               |
| --------------- | ------------------- | ------------------------- |
| Base Detail     | base_error_detail_t | Base module error details |
| Anode Status    | UINT32              | Anode Broadcast Status    |
| Anode ADC Value | UINT32              | Anode Broadcast ADC value |

## Emergency Messages

When an alert level or higher fault reaction occurs, an emergency message with error register, error code and manufacturer specific data is sent to the master. An emergency message will also be sent when an error has been cleared. In this case the fault code 0x00 (Error reset or no error) will be sent.

The coding of the emergency message is as follows in the tables below and message structure in the figure below:

| BYTE        | 7 6                       | 5              | 4                       | 3          | 2 1         | 0           |
| ----------- | ------------------------- | -------------- | ----------------------- | ---------- | ----------- | ----------- |
| Description | 16 bit CANOpen error code | Error Register | Manufacturer Error code | Error Type | line number | error count |
| Type        | UINT16                    | UINT8          | UINT8                   | UINT8      | UINT16      | UINT8       |

Example Emergency Message:

| COB-ID    | CTRL | Data               |
| --------- | ---- | ------------------ |
| 0x80 + ID | -    | 0x008111048b790100 |

Example Emergency Message Decoding:

| Data                      | Description               | Decoding                                                        |
| ------------------------- | ------------------------- | --------------------------------------------------------------- |
| 0x __0081__ 11048b790100  | 16-bit CANopen error code | Communication error (refer to 16-Bit Error Codes Table)         |
| 0x 0081 __11__ 048b790100 | Error Register            | Error register bitmask (refer to Index 0x1001)                  |
| 0x 008111 __04__ 8b790100 | Error Type                | Communication Error (refer to 8-Bit Error Types (0x1001) Table) |
| 0x 00811104 __8b__ 790100 | Fault Code                | Comm Frame Error (see Fault Table)                              |
| 0x 008111048b __7901__ 00 | Line Number               | Error was detected at line 0x0179 (377) in software             |
| 0x 008111048b7901 __00__  | Error Count               | Error is at position 0 in Serial module error log               |

&nbsp;

# Assumptions And Constraints

## Software Assumptions

Unless otherwise specified, the following data representation conventions will be applicable for the entire document:
-	When bits are numbered, a higher numbered bit represents a more significant bit than a lower numbered bit. Bit 0 always represents the least significant bit in a field of bits.
-	When bytes are numbered, a higher numbered byte represents a more significant byte than a lowered numbered byte. Byte 0 always represents the least significant byte in a field of bytes.
-   When raw data is shown as part of a CANopen message, it is in little endian in accordance with the CANopen specification.

## Software Constraints

The RS-485 and RS-422 interfaces default to 115200 bits/s but can be commanded to discrete speeds of:
-	115200 bits/s
-	921600 bits/s

The message structure for RS-485/422 physical layer follows the CAN message structure.
