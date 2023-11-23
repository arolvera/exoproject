# Prop Module Built In Test Procedure

## Introuction

## Scopes
This document provides instructions for ExoTerra Propulsion Module customers to do a BIT check of the system.

## Success Criteria
| Objective                                         | Compliance Criteria                           |
| ------------------------------------------------- | --------------------------------------------- |
| Verify functionality of the ExoTerra Prop Module. | Prop Module passes all functional BIT checks. |

## Test Item Description
The test article is the collection of flight hardware delivered to the customer.

## Acronyms
The following table lists acronyms used throughout this procedure.

| Acronym | Definition                  |
| ------- | --------------------------- |
| ACK     | Anode, Cathode, Keeper      |
| HKM     | Housekeeping and Magnet     |
| PPU     | Power Processing Unit       |
| VCU     | Valve Control Unit          |
| GSE     | Ground Support Equipment    |
| PCVs    | Proportional Control Valves |
| GUI     | Graphic User Interface      |
| DMM     | Digital Multimeter          |
| BIT     | Built In Test               |

## Safety

## Work Area Safety
- Do not leave the unit under test or support equipment in an unstable position or location, secure it to the work surface whenever it is not being moved or transported.
- Ensure that cables, tubing, and other elements of the test setup do not become tripping hazards, by organizing the testing area to place them out of the way if possible or by taping them down securely if not.

## Personnel Safety
- Wear personal protective equipment appropriate to the task and conditions at all times.
- When working with or handling hardware under test:
  - Always wear latex or nitrile gloves (flight hardware only).
  - Always wear appropriate clothing (e.g. long pants) and footwear (e.g. heavy work shoes or boots).
- Do not contact hardware or support equipment while it is energized or pressurized, except in locations intended for human interaction (knobs, dials, switches, hand valves, etc.).
- Personnel who are not participating in the test are to remain outside of the marked-off testing area whenever the hardware under test or test support equipment is energized or pressurized.
-	Practice multi-person lifting and handling with heavy or awkward hardware or equipment.

## Electrical Safety
-	Use appropriate electrical/electronic protection practices (wrist straps, grounding, ESD mats, etc.) to protect the hardware under test and test support equipment from accidental electrical damage.
-	Use of isolated test equipment with overvoltage and overcurrent protection is highly recommended.
-	All personnel involved with the testing that may come in contact with sensitive electronics shall wear ESD clothing, ESD wrist strap and nitrile or latex gloves.
-	Certain tests will generate high voltage on the unit, use caution when running BIT.

## Notes, Cautions, Warnings
Notes, cautions, and warnings shall be inserted into the test procedure where applicable to notify the test team of potential pitfalls. Notes shall be used where the operator is inconvenienced by missing the material. Cautions shall be used where there is potential for damage to hardware or test equipment. Warnings shall be used where there is potential for personnel injury or death. Notes shall be black, Cautions shall be orange, and Warnings shall be red.

## Test Equipment
The table below list all recommended test equipment to perform this procedure.

| Component                |
| ------------------------ |
| Propulsion Module        |
| Digital Multimeter (DMM) |
| Power Supply             |
| Test Laptop              |

## Unit Under Test
Caution: The unit is ESD sensitive, take appropriate EDS precautions.
Caution: Inspect all pins prior to mating or demating.

## Electrical Connections
Caution: The following operation is installing connectors and harnesses.  Connector must be installed flat by alternating turning the screws slowly walking the connector down.  Turning one screw all the way down before the other WILL damage the connector and bend pins.

1. Inspect Module Connectors P030, P032, P003 for damage, bent pins, and FOD.
2. Inspect connectors on test cable for damage, bent pins, and FOD prior to mate.
3. Verify isolation between 28V input power and pwr return, referencing the below pinout (note all HCx_ARM signals are tied together and all HCx_RTN signals are tied together in the module), the impedance should be greater than 10kOhm
4. Connect Power Connector, P030, to a lab power supply following the pinout shown below

| Pin Number         | Signal Name | Signal Characteristics                |
| ------------------ | ----------- | ------------------------------------- |
| 1,2,17,18          | HC1_ARM     | Propulsion Power ARM; 28V Input Power |
| 4,5,20,21          | HC1_RTN     | Propulsion Power Return               |
| 9,10,25,26         | HC2_ARM     | Propulsion Power ARM; 28V Input Power |
| 6,7,22,23          | HC2_RTN     | Propulsion Power Return               |
| 11,12,27,28        | HC3_ARM     | Propulsion Power ARM; 28V Input Power |
| 14,15,30,31        | HC3_RTN     | Propulsion Power Return               |
| 3,8,13,16,19,24,29 | NC          | NOT CONNECTED                         |

5. 5.	Connect the Communication Connector, P032, to an RS-422 external interface following the pinout shown below

| Pin Number | Signal Name | Signal Characteristics          |
| ---------- | ----------- | ------------------------------- |
| 1          | SC_CMD_P    | RS-422 High (Spacecraft to PPU) |
| 2          | SC_CMD_N    | RS-422 Low (Spacecraft to PPU)  |
| 3,6        | DRTN        | Signal Return                   |
| 4          | SC_TLM_P    | RS-422 High (PPU to Spacecraft) |
| 5          | SC_TLM_N    | RS-422 Low (PPU to Spacecraft)  |

## Bit Operation
Prior to running BIT, ensure the python environment and modules are installed as detailed in the BIT script user guide documentation.

## Turn on And Startup
1.	Turn on the Power Supply. Input current should be around 0.1-0.2A.
2.	Navigate to the location where the python test scripts were installed and Shift + right click on some whitespace. Click “Open PowerShell window here”. (Or use terminal prompt of choice)

3.	In the PowerShell window, type “python .\listener.py –g” (for windows systems) or “python3 listener.py -g” (for Linux systems) and hit enter. This will open up the Health and Status Information (HSI) window for real time HSI monitoring.

4.	Open a new PowerShell window and
For windows environment: type “python thruster_command.py 0x22 COM#” (Where # = the COM port number associated with the RS-422 interface in the test setup) and hit enter.
For Linux environments: type “python3 thruster_command.py /dev/ttyUSB# 0x22” #” (Where # = the COM port number associated with the RS-422 interface in the test setup) and hit enter.

The COM port is the port that is connected to the RS-422 USB cable and can be found in the computer’s hardware manager COM section, or will be listed as an available port if any number is used in place of the #. Try the different COM ports until the PowerShell window looks like this (“System Controller Connected” indicates the script connected to the PPU):

5.	Type “4” and hit enter. The power supply current should rise to 0.3A or 0.4A and the PowerShell window should indicate the it is switching states to operational (you should see something similar to the bottom few lines below):
At this point, looking at the HSI window and seeing all “MSG_CNT” data cells increment indicates that all MCU’s booted and are responding to HSI query’s.

## Valve BIT Sequences
1.	Ensure the Latch Valve is closed by running the latch valve close BIT script. Accomplish this by typing 11 into the PowerShell Window and then 3 when prompted

2.	Test the Cathode high flow valve by typing 11 into the PowerShell Window and then 6 when prompted to run the bit_pcv_drain sequence

3.	Looking at the HSI window you will see the cath_hf_v data cell drop from approx. 500 to approx. 50 (this may take a few second)

4.	This BIT sequence has a long timeout as it is used to drain the tubing between the latch valve and vacuum chamber during checkout. You can exit the sequence as soon as you see the value change in the HSI window by typing 11, then 0 when prompted (this is the method for exiting all BIT sequences if it’s desired to stop the test prior to the BIT sequence natural end)

5.	Test the cathode low flow valve by typing “11” into the PowerShell Window and then “b” when prompted

6.	Looking at the HSI window you will see the cath_lf_v data cell drop from approx. 500 to approx. 50 (this may take a few second)

7.	The cathode low flow test will timeout after 30 seconds. You can wait until the unit times out, or exit the sequence after seeing the HSI value change by typing 11 and then 0 in the PowerShell Window to exit the BIT sequence.

8.	Open the anode valve by typing 11 into the PowerShell Window and then c when prompted

9.	Looking at the HSI window you will see the anode_v data cell drop from approx. 500 to approx. 50 (this may take a few second)

10.	The anode valve test will timeout after 30 seconds. You can wait until the unit times out, or exit the sequence after seeing the HSI value change by typing 11 and then 0 in the PowerShell Window to exit the BIT sequence.

## HALO BIT Sequences
1.	Test the Inner Magnet Coil circuit by typing 11 and then 7 in the PowerShell Window when prompted. You will see the Magnet Inner current “IOUT” HSI value increase from 0 in the HSI window

11.	The test will timeout after 1 minute. You can wait until the unit times out, or exit the sequence after seeing the HSI value change by typing 11 and then 0 in the PowerShell Window to exit the BIT sequence.

2.	Test the Inner Magnet Coil circuit by typing 11 and then 8 in the PowerShell Window when prompted. You will see the Magnet Inner current “IOUT” HSI value increase from 0 in the HSI window

12.	The test will timeout after 1 minute. You can wait until the unit times out, or exit the sequence after seeing the HSI value change by typing 11 and then 0 in the PowerShell Window to exit the BIT sequence.

13.	Test the Keeper Supply by typing 11 and then 9 in the PowerShell Window. Have the HSI window available as this test times out after only 5 seconds. You will see the VSEPIC value increase to greater than 500V during this test

14.	Test the Anode Supply by typing 11 and then a in the PowerShell Window. Have the HSI window available as this test times out after only 5 seconds. You will see the VOUT value increase to greater than 500V during this test

15.	At this point all BIT tests have been completed. The “thruster_control.py” script can be exited by typing 0 in the PowerShell window or by closing the PowerShell window. The HSI window can be exited by closing the PowerShell window that was used to do the “listener.py -g” command.

16.	The unit can be turned off and disconnected from the test equipment at this point.
