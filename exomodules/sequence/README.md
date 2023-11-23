# File Naming convention:
Each module has global vars that are defined for each filetype_project_gas_coil_customer.c.  Stores an array of sequences referencing function calls. You can edit some sequences through canopen commands. 
**Refer to Executionsequence.xlxs. Refer to thruster operation flow charts for halo6 and halo12 \\exoprime\D\20 - PROJECTS - EXORESOURCE\149 - HALO 12 HALL-EFFECT THRUSTER\14900 - BASELINE DESIGN\01 - DESIGN AND ANALYSIS\Halo12 PPU**

# Global variables  
## bit
Built in tests
- bit_seqs - sequence of bit in tests.
- bite_seq_size - size of bit_seqs

## sequence
Sequence functions should all be generic and know nothing about internal
workings of the control modules.  For example, this ONLY uses functions
like keeper_is_running(), and not keeper_state == KS_CURRENT_MODE.  The
details of "is running" should be abstracted in the control module.

## setpoint
Does the work of setting/gettting/throttling to thrust setpoint. Has getter and setter for each variable.
- thrust_data_t throttle_table[THRUST_TABLE_SIZE] - stores setpoints for for throttling.

## throttle
This file contains all the logic an routine to perform throttling from one setpoint to another setpoint.
- sequence_throttle_t sequence_throttle_sequences[THROTTLE_SEQ_COUNT] - store throttling sequence for each setpoint.

## thruster-start
Sequences for transfering to steady state and ready mode.
- sequence_array_t sequence_steady_state[SEQUENCE_MAX_STEPS_ANODE] - sequence for steady state mode.
- sequence_array_t sequence_ready_mode[SEQUENCE_MAX_STEPS_KEEPER] - sequence for ready mode.