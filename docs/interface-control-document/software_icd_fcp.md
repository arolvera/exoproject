# Software Interface Requirements
For communication and control the FCP utilizes the CANOpen protocol stack. Within the context of the Open Systems Interconnection (OSI) model, the CAN interface covers the physical and data link layers, whereas CANOpen implements the other above-lying layers.

##  Message Formats
Messages shall be sent to and from the Xiphos Flight Computerusing the message structure described below.  The message frame can carry up to 8 bytes of data. The header has an 11-bit communication object identifier (COB-ID) used to address a message service. The lowest COB-ID has the highest transmission priority.

__Command/Response Message Structure__

|       | SOF   | COB-ID  |  CTRL | DATA  |  CRC   |
| ----- | ----- | ------  | ----- | ----- | ------ |
| BITS  | 00:04 |  05:15  | 16:23 | 24:87 | 88-103 |


### Message Protocols
Message protocols are classified as follows:
* The real-time data are transferred with the process data object (PDO) protocol.
* The configuration parameters are transferred with the service data object (SDO) protocol.
* Special protocols for network synchronization (SYNC) protocol and emergency message (EMCY) protocol.
* The network management (NMT) protocol provides services for initialization, state control, error and network status.

__COB-ID Field__

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


__HALO/PPU Address Field__

| HALO / PPU SLOT          | Node ID |
| ------------------------ | ------- |
| Engine Control Processor | 0x22    |


### Object Dictionary (OD)
An object dictionary is used for configuration of realtime and non-realtime communication with the FCP. It is essentially a grouping of objects accessible via the bus. Each object within the object dictionary is addressed using a 16-bit index and an 8-bit Sub-Index. Therefore, an object can contain 256 parameters which are addressed by the Sub-Index.

An entry in the object dictionary is defined by:
*	**Index**, the 16-bit address of the object in the dictionary
*	**Sub-Index**, the 8-bit Sub-Index
*	**Parameter Name**, a string describing the entry
*	**Data Type**, gives the datatype of the variable
*	**Access**, gives information on the access rights for this entry, this can be read/write, read-only or write-only
*	**Value Range**, gives a valid value range for the parameter
*	**Default**, gives the default value for the parameter

__Object Example__

| Index  | Sub-Index | Parameter Name | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | -------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x3000 | 0x1       | ADC 0          | UINT16    | RO     | Y           | UINT16      | 0       |

###	Synchronization (SYNC) protocol (COB-ID: 0x080)
The SYNC protocol is a system trigger. The SYNC protocol has a very high priority and has no data. The SYNC protocol is sent to the FCP and can trigger PDO transmissions from the FCP when activated in the corresponding PDO transmission types.

__Sync Message Structure__

| COB-ID |      | DATA |
| ------ | ---- | ---- |
| 0x080  | CTRL | -    |

###	Emergency (EMCY) protocol (COB-ID: 0x080 + NodeID)
The emergency protocol is a high priority message triggered by an error event in the device. The error codes sent with the emergency message are described in the Fault Handling Section.

### Process data object (PDO) protocol (COB-ID + NodeID)
The process data object (PDO) communication allows cyclic sending and receiving of parameters in real time. Different transmission modes are available: synchronous, event or timer driven transmission. Four transmit PDOs (TxPDOs) and four receive PDOs (RxPDOs) are implemented:
* Receive process data object (RxPDO) COB-ID: 0x200, 0x300, 0x400, 0x500
* Transmit process data object (TxPDO) COB-ID: 0x180, 0x280, 0x380, 0x480

###	Service data object (SDO) protocol (COB-ID: 0x580 + NodeID, 0x600 + NodeID)
Service data objects (SDOs) enable access to all entries of the object dictionary The service data objects are used to configure the settings for the communication and to set or read application parameters. They are transmitted non-real-time with low priority.  Message structure is shown in the table below

There are two types of transfers used with the EPS, first is the normal expedited transfers.  The second is segmented transfers, which allow for larger transfers of data.

__SDO Message Structure__

|         | BYTE 1 |             |        |       |       | BYTE 2-3 | BYTE 4    | BYTE 5-8 |
| ------- | ------ | ----------- | ------ | ----- | ----- | -------- | --------- | -------- |
| Length  | 3 bits | 1 bit       | 2 bits | 1 bit | 1 bit | 1 byte   | 1 byte    | 4 bytes  |
| Meaning | CSS    | Reserved(0) | n      | e     | s     | index    | Sub-Index | Data     |

*	**ccs** is the client command specifier of the SDO transfer, this is 0 for SDO segment download, 1 for initiating download, 2 for initiating upload, 3 for SDO segment upload, 4 for aborting an SDO transfer, 5 for SDO block upload and 6 for SDO block download
*	**n** is the number of bytes in the data part of the message which do not contain data, only valid if e and s are set
*	**e** if set, indicates an expedited transfer, i.e., all data exchanged are contained within the message. If this bit is cleared then the message is a segmented transfer where the data does not fit into one message and multiple messages are used.
*	**s** if set, indicates that the data size is specified in n (if e is set) or in the data part of the message
*	**index** is the object dictionary index of the data to be accessed
*	**Sub-Index** is the Sub-Index of the object dictionary variable
*	**data** contains the data to be uploaded in the case of an expedited transfer (e is set), or the size of the data to be uploaded (s is set, e is not set)

###	State Control (NMT) (COB-ID: 0x000, 0x700 + Node-ID)
The NMT protocols are used to issue state change commands and detect FCP boot ups. The COB-ID for commanding state changes to the FCP is always 0x000, meaning that it has highest priority.  The FCP will send an initialization message on boot up.  Subsequently NMT state are in available in the heartbeat messages.



__FCP User Commands Summary__

| Command  | Index  | CMD Description (From FC)                                                        | Protocol   |
| -------- | ------ | -------------------------------------------------------------------------------- | ---------- |
| THCMD    | 0x4000 | Thruster Control (Ready Mode, Steady State, Normal Shutdown, BIT, etc.)          | SDO        |
| EGSHUTDN | -      | Emergency Graceful Shutdown of the thruster                                      | NMT / EMCY |
| INIT     | -      | Reinitialize the FCP                                                             | NMT        |
| DEVTYPE  | 0x1000 | Device Type                                                                      | SDO        |
| HS0      | 0x1001 | Check the Health and Status of all components in the EPS                         | SDO        |
| SEF      | 0x1003 | Standard Error Field                                                             | SDO        |
| NAME     | 0x1008 | Manufacturer device name                                                         | SDO        |
| HWVER    | 0x1009 | Hardware Version                                                                 | SDO        |
| SWVER    | 0x100A | Software Version                                                                 | SDO        |
| HBEAT    | 0x1017 | Heartbeat time (ms)                                                              | SDO        |
| FRT      | 0x2830 | Fault Reaction Type                                                              | SDO        |
| FSTAT    | 0x2831 | Fault Status                                                                     | SDO        |
| MEL      | 0x2832 | Detailed Error Log by Module                                                     | SDO        |
| TRD      | 0x3000 | Torque Rod Driver Control                                                        | SDO        |
| SSSC     | 0x3001 | Sun Sensor Signal Conditioning Control                                           | SDO        |
| TSCLW    | 0x3002 | Temperature Sensor Conditioning Control (Frangibolt #4, Left Wing)               | SDO        |
| TSCRW    | 0x3003 | Temperature Sensor Conditioning Control (Frangibolt #4, Right Wing)              | SDO        |
| TSCTD    | 0x3004 | Temperature Sensor Conditioning Control (Frangibolt #8, Thruster Deploy)         | SDO        |
| SMD      | 0x3005 | Stepper Motor Driver Control                                                     | SDO        |
| DMW1     | 0x3006 | Deployment Motor Control (Wing 1)                                                | SDO        |
| DMW2     | 0x3007 | Deployment Motor Control (Wing 2)                                                | SDO        |
| TH       | 0x3008 | Tank Heater Control                                                              | SDO        |
| PMH      | 0x3009 | Pressure Manifold Heater Control                                                 | SDO        |
| PCVH     | 0x300A | PCV Heater Control                                                               | SDO        |
| CH       | 0x300B | Chassis Heater Control                                                           | SDO        |
| CSTAT    | 0x4001 | Return conditioning stats                                                        | SDO        |
| FIRM     | 0x5000 | Firmware Versions                                                                | SDO        |
| TRCMSG   | 0x5001 | Debug trace messages                                                             | SDO        |
| CONFIG   | 0x5100 | Configure the thruster settings: put configuration data into non-volatile memory | SDO        |
| CSTATCLR | 0x5401 | Clears conditioning stats                                                        | SDO        |
| PROG     | 0x5500 | Reprogram the unit                                                               | SDO        |
| STNDBY   | -      | Put the unit into a idle / wait state                                            | NMT        |

###	Software Message Details
The details for the commands and dictionary objects are shown in the sections below.

###	State Control and Initialization COB-ID: 0x000, 0x700 + Node ID
State control is done utilizing the Network Management (NMT) Protocol.  The COB-ID for controlling the state is always 0x000.  The first data byte of the message indicates the state.  The second byte indicates the Node ID (always the EPS node 0x22).

State change notification are broadcast by the FCP and always use the COB-ID of 0x700 + Node ID.

###	State Control and Initialization Command
State control data fields and initialize commands are shown below.

__State Control Data Fields__

| COB-ID | Data Byte 0     | Data Byte 1           |
| ------ | --------------- | --------------------- |
| 0x000  | Requested State | Addresses Node (0x22) |

__State Control Commands__

| State                                    | Command Specifier |
| ---------------------------------------- | ----------------- |
| Go to Operational Standby Mode *(STNDBY) | 0x01              |
| Go to Stopped Mode *(EGSHTDN)            | 0x02              |
| Go to Pre-Operational / Low Power Mode   | 0x80              |
| Reinitialize                             | 0x81              |
| Reset Communication                      | 0x82              |

*See Thruster Command for thruster control and normal thruster shutdown after reaching Standby Mode.

__State Change Data Fields__

| COB-ID | Data Byte 0 |
| ------ | ----------- |
| 0x722  | State       |

__State Data Fields__

| State                   | Command Specifier |
| ----------------------- | ----------------- |
| Initialization (bootup) | 0x00              |
| Pre-Operational         | 0x7F              |
| Operational             | 0x05              |
| Stopped                 | 0x04              |

### CANopen mandatory Object Dictionary Entries

The object dictionary is split into standardized sections where some entries are mandatory and others are customizable.  This section describes the standard mandatory object dictionary entries that are not customized or FCP specific.

#### DEVTYPE Device Type 0x1000

This parameter indicates the code of the underlying device profile.  The default value 401 specifies the device profile for a modular I/O device.

| Index  | Sub-Index  | Parameter Name      | Data Type | Access | Persistence | Value Range | Default |
| ------ | ---------- | ------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x1000 | 0x0        | Device Type         | UINT32    | RO     | -           | UINT32      | 401     |

#### NAME Manufacturer device name 0x1008

This parameter indicates the name of the system.  The default value is "FCP".

| Index  | Sub-Index  | Parameter Name            | Data Type | Access | Persistence | Value Range | Default |
| ------ | ---------- | ------------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x1008 | 0x0        | Manufacturer Device Name  | STRING    | RO     | -           | None        | "PPU"   |

#### HWVER Manufacturer hardware version 0x1009

This parameter indicates the current hardware version of the thruster control electronics.  The default value is "FCP".

| Index  | Sub-Index  | Parameter Name                | Data Type | Access | Persistence | Value Range | Default   |
| ------ | ---------- | ----------------------------- | --------- | ------ | ----------- | ----------- | --------- |
| 0x1009 | 0x0        | Manufacturer Hardware Version | STRING    | RO     | -           | None        | "PPU_SLV" |


#### SWVER Version 0x100a

This parameter indicates the current top level software version of the FCP.

| Index  | Sub-Index  | Parameter Name                | Data Type | Access | Persistence | Value Range | Default   |
| ------ | ---------- | ----------------------------- | --------- | ------ | ----------- | ----------- | --------- |
| 0x100a | 0x0        | Manufacturer Software Version | STRING    | RO     | -           | None        | ""        |


#### HBEAT Heartbeat time 0x1017

Heartbeat time in milliseconds.

| Index  | Sub-Index  | Parameter Name                | Data Type | Access | Persistence | Value Range | Default   |
| ------ | ---------- | ----------------------------- | --------- | ------ | ----------- | ----------- | --------- |
| 0x1017 | 0x0        | Heartbeat Time (ms)           | UINT16    | WR     | -           | UINT16      | 0         |


###	FIRM Firmware Versions Object 0x5000
Firmware versions are represented by a Major, Minor, and Revision number packed into a 32 bit integer and a unique build hash identifier.  Each device has three redundant backup copies in memory and an executing copy.   The redundant copies are stored in the FCP memory.  The running version represents the version of firmware running on the device.  A value of zero indicates the device has not executed any firmware since the FCP has been initialized.

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

__Firmware Versions Byte Explanation__

| Byte       | 3        | 2               | 1               | 0                  |
| ---------- | -------- | --------------- | --------------- | ------------------ |
| Definition | Reserved | Version - Major | Version - Minor | Version - Revision |

#### Device Selection

Device Query Sequence:
1.	SDO download Device Select Value to Index 0x5000, Sub-Index 0x1, to select desired device.
2.	SDO upload device hash or version with corresponding Sub-Index from table below.

__Device Selection Values__

| Device Select Value | Device                       |
| ------------------- | ---------------------------- |
| 0                   | Thruster Control             |
| 1                   | Buck 28v                     |
| 2                   | Thruster Control boot loader |

### TRCMSG Debug Trace Messages 0x5001

Circular log buffer containing info, debug, and error messages.  Messages can be downloaded from the tail (oldest) or head (newest)

__Trace Object (0x5001)__

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

### PROG Reprogramming 0x5500

__Reprogram Table (0x5500)__

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



## FCP Control Objects

#### Torque Rod Driver 0x3000 

Control PWM Inputs and Read ADC Feedback From Torque Rod Driver

| Index  | Sub-Index  | Parameter Name                | Data Type | Access | Persistence | Value Range | Default   |
| ------ | ---------- | ----------------------------- | --------- | ------ | ----------- | ----------- | --------- |
| 0x3000 | 0x0        | Highest Sub-Index             | UINT8     | RO     | -           | UINT8       | -         |
| 0x3000 | 0x1        | ADC output[0] (ticks)         | UINT16    | RO     | -           | UINT16      | -         |
| 0x3000 | 0x2        | ADC output[1] (ticks)         | UINT16    | RO     | -           | UINT16      | -         |
| 0x3000 | 0x3        | ADC output[2] (ticks)         | UINT16    | RO     | -           | UINT16      | -         |
| 0x3000 | 0x4        | PWM[0] Period                 | UINT16    | RW     | -           | UINT16      | -         |
| 0x3000 | 0x5        | PWM[1] Period                 | UINT16    | RW     | -           | UINT16      | -         |
| 0x3000 | 0x6        | PWM[2] Period                 | UINT16    | RW     | -           | UINT16      | -         |
| 0x3000 | 0x7        | PWM[0] Duty Cycle             | UINT16    | RO     | -           | UINT16      | -         |
| 0x3000 | 0x8        | PWM[1] Duty Cycle             | UINT16    | RO     | -           | UINT16      | -         |
| 0x3000 | 0x9        | PWM[2] Duty Cycle             | UINT16    | RO     | -           | UINT16      | -         |
| 0x3000 | 0xA        | PWM[0] Channel Start          | UINT16    | RO     | -           | UINT16      | -         |
| 0x3000 | 0xB        | PWM[1] Channel Start          | UINT16    | RO     | -           | UINT16      | -         |
| 0x3000 | 0xC        | PWM[2] Channel Start          | UINT16    | RO     | -           | UINT16      | -         |

#### Sun Senor Signal Conditioning 0x3001 

Read ADC Feedback From Sun Sensor

| Index  | Sub-Index  | Parameter Name                | Data Type | Access | Persistence | Value Range | Default   |
| ------ | ---------- | ----------------------------- | --------- | ------ | ----------- | ----------- | --------- |
| 0x3001 | 0x0        | Highest Sub-Index             | UINT8     | RO     | -           | UINT8       | -         |
| 0x3001 | 0x1        | ADC output[0] (ticks)         | UINT16    | RO     | -           | UINT16      | -         |
| 0x3001 | 0x2        | ADC output[1] (ticks)         | UINT16    | RO     | -           | UINT16      | -         |
| 0x3001 | 0x3        | ADC output[2] (ticks)         | UINT16    | RO     | -           | UINT16      | -         |
| 0x3001 | 0x4        | ADC output[3] (ticks)         | UINT16    | RO     | -           | UINT16      | -         |


#### Temperature Sensor Conditioning (Frangibolt #4, Left Wing) 0x3002 

Enable or Disable Frangibolt Heaters and Read ADC Feedback

| Index  | Sub-Index  | Parameter Name                | Data Type | Access | Persistence | Value Range | Default   |
| ------ | ---------- | ----------------------------- | --------- | ------ | ----------- | ----------- | --------- |
| 0x3002 | 0x0        | Highest Sub-Index             | UINT8     | RO     | -           | UINT8       | -         |
| 0x3002 | 0x1        | ADC output (ticks)            | UINT16    | RO     | -           | UINT16      | -         |
| 0x3002 | 0x2        | Digital Input                 | UINT8     | RW     | -           | UINT8       | -         |


#### Temperature Sensor Conditioning (Frangibolt #4, Right Wing) 0x3003 

Enable or Disable Frangibolt Heaters and Read ADC Feedback

| Index  | Sub-Index  | Parameter Name                | Data Type | Access | Persistence | Value Range | Default   |
| ------ | ---------- | ----------------------------- | --------- | ------ | ----------- | ----------- | --------- |
| 0x3003 | 0x0        | Highest Sub-Index             | UINT8     | RO     | -           | UINT8       | -         |
| 0x3003 | 0x1        | ADC output (ticks)            | UINT16    | RO     | -           | UINT16      | -         |
| 0x3003 | 0x2        | Digital Input                 | UINT8     | RW     | -           | UINT8       | -         |

#### Temperature Sensor Conditioning (Frangibolt #8, Thruster Deploy) 0x3004 

Enable or Disable Frangibolt Heaters and Read ADC Feedback

| Index  | Sub-Index  | Parameter Name                | Data Type | Access | Persistence | Value Range | Default   |
| ------ | ---------- | ----------------------------- | --------- | ------ | ----------- | ----------- | --------- |
| 0x3004 | 0x0        | Highest Sub-Index             | UINT8     | RO     | -           | UINT8       | -         |
| 0x3004 | 0x1        | ADC output (ticks)            | UINT16    | RO     | -           | UINT16      | -         |
| 0x3004 | 0x2        | Digital Input                 | UINT8     | RW     | -           | UINT8       | -         |

#### Stepper Motor Driver 0x3005  

Control Digital Inputs to Stepper Motor

| Index  | Sub-Index  | Parameter Name                | Data Type | Access | Persistence | Value Range | Default   |
| ------ | ---------- | ----------------------------- | --------- | ------ | ----------- | ----------- | --------- |
| 0x3005 | 0x0        | Highest Sub-Index             | UINT8     | RO     | -           | UINT8       | -         |
| 0x3005 | 0x1        | Digital Input[0]              | UINT8     | RO     | -           | UINT8       | -         |
| 0x3005 | 0x2        | Digital Input[1]              | UINT8     | RO     | -           | UINT8       | -         |
| 0x3005 | 0x3        | Digital Input[2]              | UINT8     | RO     | -           | UINT8       | -         |


#### DC Motor Driver with Current Feedback (Deployment Motor, Wing 1) 0x3006  

Control DC Motor and Read ADC Feedback

| Index  | Sub-Index  | Parameter Name                | Data Type | Access | Persistence | Value Range | Default   |
| ------ | ---------- | ----------------------------- | --------- | ------ | ----------- | ----------- | --------- |
| 0x3006 | 0x0        | Highest Sub-Index             | UINT8     | RO     | -           | UINT8       | -         |
| 0x3006 | 0x1        | ADC                           | UINT16    | RO     | -           | UINT16      | -         |
| 0x3006 | 0x2        | Digital Input[0]              | UINT8     | RW     | -           | UINT8       | -         |
| 0x3006 | 0x3        | Digital Input[1]              | UINT8     | RW     | -           | UINT8       | -         |


#### DC Motor Driver with Current Feedback (Deployment Motor, Wing 2) 0x3007  

Control DC Motor and Read ADC Feedback

| Index  | Sub-Index  | Parameter Name                | Data Type | Access | Persistence | Value Range | Default   |
| ------ | ---------- | ----------------------------- | --------- | ------ | ----------- | ----------- | --------- |
| 0x3007 | 0x0        | Highest Sub-Index             | UINT8     | RO     | -           | UINT8       | -         |
| 0x3007 | 0x1        | ADC                           | UINT16    | RO     | -           | UINT16      | -         |
| 0x3007 | 0x2        | Digital Input[0]              | UINT8     | RW     | -           | UINT8       | -         |
| 0x3007 | 0x3        | Digital Input[1]              | UINT8     | RW     | -           | UINT8       | -         |


#### Heater (Tank Heater) 0x3008  

Enable the Tank Heater and Read ADC Output

| Index  | Sub-Index  | Parameter Name                | Data Type | Access | Persistence | Value Range | Default   |
| ------ | ---------- | ----------------------------- | --------- | ------ | ----------- | ----------- | --------- |
| 0x3008 | 0x0       | Highest Sub-Index              | UINT8     | RO     | -           | UINT8       | -         |
| 0x3008 | 0x1        | ADC                           | UINT16    | RO     | -           | UINT16      | -         |
| 0x3008 | 0x2        | Digital Input                 | UINT8     | RW     | -           | UINT8       | -         |

#### Heater (Pressure Manifold Heater) 0x3009  

Enable the Tank Heater and Read ADC Output

| Index  | Sub-Index  | Parameter Name                | Data Type | Access | Persistence | Value Range | Default   |
| ------ | ---------- | ----------------------------- | --------- | ------ | ----------- | ----------- | --------- |
| 0x3009 | 0x0       | Highest Sub-Index              | UINT8     | RO     | -           | UINT8       | -         |
| 0x3009 | 0x1        | ADC                           | UINT16    | RO     | -           | UINT16      | -         |
| 0x3009 | 0x2        | Digital Input                 | UINT8     | RW     | -           | UINT8       | -         |

#### Heater (PCV Heater) 0x300A  

Enable the PCV Heater and Read ADC Output

| Index  | Sub-Index  | Parameter Name                | Data Type | Access | Persistence | Value Range | Default   |
| ------ | ---------- | ----------------------------- | --------- | ------ | ----------- | ----------- | --------- |
| 0x300A | 0x0        | Highest Sub-Index             | UINT8     | RO     | -           | UINT8       | -         |
| 0x300A | 0x1        | ADC                           | UINT16    | RO     | -           | UINT16      | -         |
| 0x300A | 0x2        | Digital Input                 | UINT8     | RW     | -           | UINT8       | -         |

#### Heater (Chassis Heater) 0x300B  

Enable the Chassis Heater and Read ADC Output

| Index  | Sub-Index  | Parameter Name                | Data Type | Access | Persistence | Value Range | Default   |
| ------ | ---------- | ----------------------------- | --------- | ------ | ----------- | ----------- | --------- |
| 0x300B | 0x0        | Highest Sub-Index             | UINT8     | RO     | -           | UINT8       | -         |
| 0x300B | 0x1        | Digital Input                 | UINT8     | RW     | -           | UINT8       | -         |


## Fault Handling

### Fault Reaction
If a fault occurs, the EPS software throws a fault. The corresponding fault is set in the fault state \<FaultStatus\> (0x2831). If no fault reaction for this fault is defined within the parameter \<FaultReactionType\> (0x2830) no further fault reaction is done. If a fault reaction is configured, the fault code is saved in an array \<StandardErrorField\> (0x1003) which holds the last eight thrown faults. If the configured fault reaction of the actual thrown fault requests a change of the device state, the corresponding transition of the device state machine will be forced.

#### Fault Table
The faults that can occur in the PPU and the actions taken by the FCP to handle the fault are shown in the table below.  The Data Word column corresponds to the \<FaultStatus\>(0x2831) Fault Status Data Word numbers.  The Bit column corresponds to the bit position in the specified Fault Status Data Word.

| Fault Code | Fault Code | Data Word |     |                                         |                                                                  | \<FaultReactionType\> (0x2830) |
| ---------- | ---------- | --------- | --- | --------------------------------------- | ---------------------------------------------------------------- | ------------------------------ |
| Dec        | Hex        | Num       | Bit | Fault Name                              | Fault Description                                                | Value                          |
| 0          | 0x0        | -         | -   | Benign Reset or No Error                | No Fault                                                         | -                              |
-- To be filled in on later versions --



### Fault Reaction Type

The fault reaction parameter \<FaultReactionType\> (0x2830), shown below, is used to define the fault behavior for each fault code.
The fault reaction for each fault event is configured with different fault reaction types.

#### FRT Fault Reaction Type (0x2830)

| Index  | Sub-Index | Parameter Name       | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | -------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x2830 | 0         | Sub-Index Count      | UINT8     | RO     | Y           | UINT8       | -       |
| 0x2830 | 1         | Fault Code Select    | UINT8     | RW     | Y           | 0-160       | 0       |
| 0x2830 | 2         | Fault Reaction Value | UINT8     | RW     | Y           | 0-4         | 0       |


#### Fault Reaction Value Description

| FaultReactionType  | Description                                                                                                                                                                                                                                                                                                                                     |
| ------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0 (NONE)           | No fault reaction, error is ignored                                                                                                                                                                                                                                                                                                             |
| 1 (ALERT)          | Send an emergency message. If a malfunction for the monitored fault is detected, an emergency message will be sent onto the bus. The PPU continues to operate. Special care must be taken, as the malfunction may have an impact on the PPU.                                                                                                    |
| 2 (ALERT_PREOP)    | If a malfunction for the monitored fault is detected, the device state machine enters the Pre-Operational state and an emergency message will be sent onto the bus. The power stage of the MCUs are switched off, while all FCP functions are still alive. The FCP must be transitioned to the Operational state to return to normal operation. |
| 3 (ALERT_SHUTDOWN) | A fault was detected that requires the FCP change state back to Standby.                                                                                                                                                                                                                                                                |

### Fault Status
The bit coded fault status indicates which faults are currently reported for the device. Each bit of the fault status array (x5 32-bit words, built with the sub-indexes 2-6 of the fault status) stands for a fault code. The corresponding fault code is equal to the bit position in the 160-bit field (5x32 bit). Fault status is shown below.

#### FSTAT Fault Status (0x2831) Table

| Index  | Sub-Index | Parameter Name               | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | ---------------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x2831 | 0x0       | Highest Sub-Index            | UINT8     | RO     | -           | UINT32      | -       |
| 0x2831 | 0x1       | Fault Status Code (Dump All) | Domain    | RO     | -           | UINT32      | 0       |
| 0x2831 | 0x2       | Fault Status Data Word 1     | UINT32    | RO     | -           | UINT32      | 0       |
| 0x2831 | 0x3       | Fault Status Data Word 2     | UINT32    | RO     | -           | UINT32      | 0       |
| 0x2831 | 0x4       | Fault Status Data Word 3     | UINT32    | RO     | -           | UINT32      | 0       |
| 0x2831 | 0x5       | Fault Status Data Word 4     | UINT32    | RO     | -           | UINT32      | 0       |
| 0x2831 | 0x6       | Fault Status Data Word 5     | UINT32    | RO     | -           | UINT32      | 0       |


#### HS0 Error Register 0x1001

The \<ErrorRegister\> (0x1001) displays the error information about the last reported fault in bit-coded form. Bit 0 of the \<ErrorRegister\> (0x1001), shown in the table below, is set as soon as any error occurs on the FCP.

#### Error Register (0x1001) Table

| Index  | Sub-Index | Parameter Name | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | -------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x1001 | 0x0       | Error Register | UINT8     | RO     | \-          | UINT8       | 0       |

#### 8 Bit Error Types (0x1001) Table

| Bit | Description               |
| --- | ------------------------- |
| 0   | Generic error (any error) |
| 1   | Current Error             |
| 2   | Voltage Error             |
| 3   | Temperature Error         |
| 4   | Communication Error       |
| 5   | Profile Error             |
| 6   | Reserved                  |
| 7   | Manufacturer Error        |

#### 16 Bit Error Codes Table

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


#### Error History (0x1003)
When a fault is thrown, information about the fault is stored to the \<StandardErrorField\> (0x1003) parameter array. The \<StandardErrorField\> (0x1003) parameter array contains a list of up to 8 entries. This Fault Code provides information about the reason for the error. The parameter \<NumberOfErrors\> (0x1003) holds information about the number of errors currently recorded. Every new fault is stored in the first element of the parameter array \<FaultCode\> (0x1003), the older ones move down in the list. If the maximum number of entries is reached, and a new fault occurred, the oldest fault information will be deleted. Writing the value 0 to the object \<NumberOfErrors\> (0x1003), sub-index 0, deletes the entire error code history. Standard error field is shown in the table below.

##### SEF Standard Error Field (0x1003)

| Index  | Sub-Index | Parameter Name       | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | -------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x1003 | 0x0       | Number of Errors     | UINT8     | RW     | \-          | UINT8       | 0       |
| 0x1003 | 0x1…7     | Standard Error Field | UINT32    | RO     | \-          | UINT32      | 0       |

#### Emergency Messages
When an alert level or higher fault reaction occurs, an emergency message with error register, error code and manufacturer specific data is sent to the master. An emergency message will also be sent when an error has been cleared. In this case the fault code 0x00 (Error reset or no error) will be sent.

The coding of the emergency message is as follows in the tables below and message structure in the figure below:

#### Example Emergency Message Structure and Description

| BYTE        | 7 6                       | 5              | 4                       | 3          | 2 1         | 0           |
| ----------- | ------------------------- | -------------- | ----------------------- | ---------- | ----------- | ----------- |
| Description | 16 bit CANOpen error code | Error Register | Manufacturer Error code | Error Type | line number | error count |
| Type        | UINT16                    | UINT8          | UINT8                   | UINT8      | UINT16      | UINT8       |

#### Example Emergency Message

| COB-ID    | CTRL | Data               |
| --------- | ---- | ------------------ |
| 0x80 + ID | -    | 0x008111048b790100 |

#### Example Emergency Message Decoding

| Data                      | Description               | Decoding                                                        |
| ------------------------- | ------------------------- | --------------------------------------------------------------- |
| 0x __0081__ 11048b790100  | 16-bit CANopen error code | Communication error (refer to 16-Bit Error Codes Table)         |
| 0x 0081 __11__ 048b790100 | Error Register            | Error register bitmask (refer to Index 0x1001)                  |
| 0x 008111 __04__ 8b790100 | Error Type                | Communication Error (refer to 8-Bit Error Types (0x1001) Table) |
| 0x 00811104 __8b__ 790100 | Fault Code                | Comm Frame Error (see Fault Table)                              |
| 0x 008111048b __7901__ 00 | Line Number               | Error was detected at line 0x0179 (377) in software             |
| 0x 008111048b7901 __00__  | Error Count               | Error is at position 0 in Serial module error log               |


