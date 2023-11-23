#Internal Software Interface Control Document
# Software Interface Requirements
For communication and control the ECP utilizes the CANOpen protocol stack. Within the context of the Open Systems Interconnection (OSI) model, the serial RS-485, RS-422, and CAN interfaces cover the physical and data link layers, whereas CANOpen implements the other above-lying layers.

##  Message Formats
Messages shall be sent to and from the electronic propulsion system (EPS) using the message structure described below.  The message frame can carry up to 8 bytes of data. The header has an 11-bit communication object identifier (COB-ID) used to address a message service. The lowest COB-ID has the highest transmission priority.

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

### Object Dictionary (OD)
An object dictionary is used for configuration of realtime and non-realtime communication with the ECP. It is essentially a grouping of objects accessible via the bus. Each object within the object dictionary is addressed using a 16-bit index and an 8-bit Sub-Index. Therefore, an object can contain 256 parameters which are addressed by the Sub-Index.

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

###	Keeper Control
Keeper Control parameters are shown below

__Keeper Control (0x2100) Table__

| Index  | Sub-Index | Parameter Name          | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | ----------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x2100 | 0x0       | Highest Sub-Index       | UINT16    | R      | -           | UINT16      | 0x7     |
| 0x2100 | 0x1       | Keeper Voltage          | FLOAT     | RW     | -           | FLOAT       | 0       |
| 0x2100 | 0x2       | Keeper Factored Voltage | UINT16    | R      | -           | UINT16      | 0       |
| 0x2100 | 0x3       | Keeper Current          | FLOAT     | RW     | -           | FLOAT       | 0       |
| 0x2100 | 0x4       | Keeper Current Factored | UINT16    | R      | -           | UINT16      | 0       |
| 0x2100 | 0x5       | Keeper Power Supply     | UINT8     | RW     | -           | UINT8       | 0       |
| 0x2100 | 0x6       | Keeper State            | UINT8     | R      | -           | UINT8       | 0       |
| 0x2100 | 0x7       | Keeper State Stat       | UINT8     | R      | -           | UINT8       | 0       |


###	Anode Control
Anode Control parameters are shown below

__Anode Control (0x2200) Table__

| Index  | Sub-Index | Parameter Name         | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | ---------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x2200 | 0x0       | Highest Sub-Index      | UINT16    | R      | -           | UINT16      | 0x9     |
| 0x2200 | 0x1       | Anode Voltage          | FLOAT     | RW     | -           | FLOAT       | 0       |
| 0x2200 | 0x2       | Anode Factored Voltage | UINT16    | R      | -           | UINT16      | 0       |
| 0x2200 | 0x3       | Anode Current          | FLOAT     | RW     | -           | FLOAT       | 0       |
| 0x2200 | 0x4       | Anode Current Factored | UINT16    | R      | -           | UINT16      | 0       |
| 0x2200 | 0x5       | Anode Power Supply     | UINT8     | RW     | -           | UINT8       | 0       |
| 0x2200 | 0x6       | Anode State            | UINT8     | R      | -           | UINT8       | 0       |
| 0x2200 | 0x7       | Anode State Stat       | UINT8     | R      | -           | UINT8       | 0       |
| 0x2200 | 0x8       | Anode Spark timeout    | UINT16    | RW     | -           | UINT16      | 0       |
| 0x2200 | 0x9       | Anode Run Count        | UINT16    | RW     | -           | UINT16      | 0       |


###	Magnet Control
Magnet Control parameters are shown below

__Magnet Control (0x2300) Table__

| Index  | Sub-Index | Parameter Name                | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | ----------------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x2300 | 0x0       | Highest Sub-Index             | UINT16    | R      | -           | UINT16      | 0x5     |
| 0x2300 | 0x1       | Magnet Inner Current          | FLOAT     | RW     | -           | FLOAT       | 0       |
| 0x2300 | 0x2       | Magnet Inner Current Factored | UINT16    | R      | -           | UINT16      | 0       |
| 0x2300 | 0x3       | Magnet Ratio                  | FLOAT     | RW     | -           | FLOAT       | 0       |
| 0x2300 | 0x4       | Magnet State                  | UINT8     | RW     | -           | UINT8       | 0       |
| 0x2300 | 0x5       | Magnet Sync                   | UINT8     | W      | -           | UINT8       | 0       |

###	Valves Control
Valves Control parameters are shown below

__Valves Control (0x2500) Table__

| Index  | Sub-Index | Parameter Name            | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | ------------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x2500 | 0x0       | Highest Sub-Index         | UINT16    | R      | -           | UINT16      | 0x7     |
| 0x2500 | 0x1       | Valves High Flow          | FLOAT     | RW     | -           | FLOAT       | 0       |
| 0x2500 | 0x2       | Valves High Flow Factored | UINT16    | R      | -           | UINT16      | 0       |
| 0x2500 | 0x3       | Valves Low Flow           | FLOAT     | RW     | -           | FLOAT       | 0       |
| 0x2500 | 0x4       | Valves Low Flow Factored  | UINT16    | R      | -           | UINT16      | 0       |
| 0x2500 | 0x5       | Anode Flow                | FLOAT     | RW     | -           | FLOAT       | 0       |
| 0x2500 | 0x6       | Anode Flow Factored       | UINT16    | R      | -           | UINT16      | 0       |
| 0x2500 | 0x7       | Latch Valve               | UINT8     | RW     | -           | UINT8       | 0       |

###	Throttle Control
Throttle settings are shown below

__Throttle Control (0x4002) Table__

| Index  | Sub-Index | Parameter Name    | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | ----------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x4002 | 0x0       | Highest Sub-Index | UINT8     | R      | -           | UINT8       | 0xD     |
| 0x4002 | 0x1       | Table Selector    | UINT32     | R      | -           | UINT8       | 0       |
| 0x4002 | 0x2       | Row Selector      | UINT32    | RW     | -           | UINT32      | 0       |
| 0x4002 | 0x3       | Cathode           | FLOAT    | RW     | -           | UINT32      | 0       |
| 0x4002 | 0x4       | Anode             | FLOAT    | RW     | -           | UINT32      | 0       |
| 0x4002 | 0x5       | Voltage           | FLOAT    | RW     | -           | UINT32      | 0       |
| 0x4002 | 0x6       | Current           | FLOAT    | RW     | -           | UINT32      | 0       |
| 0x4002 | 0x7       | Inner             | FLOAT    | RW     | -           | UINT32      | 0       |
| 0x4002 | 0x8       | Inner Outer       | FLOAT    | RW     | -           | UINT32      | 0       |
| 0x4002 | 0x9       | Thrust            | FLOAT    | RW     | -           | UINT32      | 0       |
| 0x4002 | 0xA       | Power             | FLOAT    | RW     | -           | UINT32      | 0       |
| 0x4002 | 0xB       | Start Method      | UINT32    | RW     | -           | UINT32      | 0       |
| 0x4002 | 0xC       | Timeout           | UINT32    | RW     | -           | UINT32      | 0       |
| 0x4002 | 0xD       | Table Length      | UINT32    | RW     | -           | UINT32      | 0       |

###	Limits Control
Limit settings are shown below

__Limits Control (0x4003) Table__

| Index  | Sub-Index | Parameter Name                         | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | -------------------------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x4003 | 0x0       | Highest Sub-Index                      | UINT8     | R      | -           | UINT8       | 0       |
| 0x4003 | 0x1       | Keeper Over Voltage Limit              | UINT8     | R      | -           | UINT8       | 0       |
| 0x4003 | 0x2       | Keeper Steady State Shutdown Limit     | UINT32    | RW     | -           | UINT32      | 0       |
| 0x4003 | 0x3       | Keeper Steady State Power High Warning | UINT32    | RW     | -           | UINT32      | 0       |
| 0x4003 | 0x4       | Keeper Steady State Power High Clear   | UINT32    | RW     | -           | UINT32      | 0       |
| 0x4003 | 0x5       | Keeper Steady State Power Low Warning  | UINT32    | RW     | -           | UINT32      | 0       |
| 0x4003 | 0x6       | Keeper Steady State Power Low Clear    | UINT32    | RW     | -           | UINT32      | 0       |
| 0x4003 | 0x7       | Magnet Current Error                   | UINT32    | RW     | -           | UINT32      | 0       |
| 0x4003 | 0x8       | Input Power Low                        | UINT32    | RW     | -           | UINT32      | 0       |
| 0x4003 | 0x9       | Input Power High                       | UINT32    | RW     | -           | UINT32      | 0x9     |

###	Sequence Table Updater
Sequence Table Updater settings are shown below

__Sequence Table Updater (0x4200) Table__

| Index  | Sub-Index | Parameter Name            | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | ------------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x4200 | 0x0       | Highest Sub-Index         | UINT16    | R      | -           | UINT16      | 0x8     |
| 0x4200 | 0x1       | Step Count                | UINT16    | RW     | -           | UINT16      | 0       |
| 0x4200 | 0x2       | Execute                   | UINT32    | RW     | -           | UINT32      | 0       |
| 0x4200 | 0x3       | Condition                 | UINT32    | RW     | -           | UINT32      | 0       |
| 0x4200 | 0x4       | Table Select              | UINT32    | RW     | -           | UINT32      | 0       |
| 0x4200 | 0x5       | Step Select               | UINT32    | RW     | -           | UINT32      | 0       |
| 0x4200 | 0x6       | Step Entry Write Upper 32 | UINT32    | RW     | -           | UINT32      | 0       |
| 0x4200 | 0x7       | Step Entry Write Lower 32 | UINT32    | RW     | -           | UINT32      | 0       |
| 0x4200 | 0x8       | Sequence Base             | UINT32    | RW     | -           | UINT32      | 0       |


###	Health Control
Health settings are shown below

__Health Control (0x5002) Table__

| Index  | Sub-Index | Parameter Name            | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | ------------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x5002 | 0x0       | Keeper Over Voltage Limit | UINT8     | R      | -           | UINT8       | 0x2     |
| 0x5002 | 0x1       | Tick Set                  | UINT32    | RW     | -           | UINT32      | 0       |
| 0x5002 | 0x2       | Tick Enable               | UINT8     | RW     | -           | UINT8       | 0       |



###	Memory Corrupter
Memory Corrupter settings are shown below

__Memory Corrupter (0x5005) Table__

| Index  | Sub-Index | Parameter Name      | Data Type | Access | Persistence | Value Range | Default |
| ------ | --------- | ------------------- | --------- | ------ | ----------- | ----------- | ------- |
| 0x5005 | 0x0       | Highest Sub-Index   | UINT8     | R      | -           | UINT8       | 0x8     |
| 0x5005 | 0x1       | Command             | UINT8     | RW     | -           | UINT8       | 0       |
| 0x5005 | 0x2       | Status              | UINT32    | R      | -           | UINT32      | 0       |
| 0x5005 | 0x3       | Failed              | UINT32    | R      | -           | UINT32      | 0       |
| 0x5005 | 0x4       | Update trigger Stat | UINT32    | R      | -           | UINT32      | 0       |
| 0x5005 | 0x5       | IACM                | DOMAIN    | R      | -           | DOMAIN      | 0       |
| 0x5005 | 0x6       | Flash Unlock        | UINT32    | RW     | -           | UINT32      | 0       |
| 0x5005 | 0x7       | Repair Enable       | UINT32    | RW     | -           | UINT32      | 0       |
| 0x5005 | 0x8       | Lock Bits           | STRING    | R      | -           | STRING      | 0       |
