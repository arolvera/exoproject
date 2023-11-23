# Cmake variables
- EXOPROJECT_APP:STRING - defined in exoproject
- EXOMOD_SEQUENCE_FILE:STRING - a combination of EXOPROJECT_APP + EXOMOD_GAS + EXOMOD_COIL + EXOMOD_CUSTOMER 
  used to 
  choose a file at link time in the sequence module.
- TRACE_UART:BOOL - enables the #define TRACE_UART (same name). Trace util spits message through uart port when 
  TraceShowN is called. Set through cli
- HALO_SIM:BOOL - Enables simulated thruster control code. Set through cli

# #defines
- HALO_SIM - Enable for simulating halo hardware components.
- FREE_RTOS - Enables freertos code. Defined by freertos lib.
- TraceDbg - Enables trace debug messages for that file. Used at the top of file.

# Modules

---
## bootloader-server  
### Summery
bootload-server for hardware control components.

---

---
## client-control  
### Summery
- Communicates with hardware control components.
- Determines state of thruster.
### files 
#### client_booted: 
- Determines if hardware control component has booted.
#### client_control: 
- Controls state of thruster.
- Emergency shutdown.
#### client_health:
- Checks vin of keeper_telem.
- Checks set point of power in steady state.
#### client_lockout:
- thruster control locks out 
#### client_power:
- Turn on and off hardware control components.
- Header definition are chosen at link time based on EXOPROJECT_APP cmake variable
---

---
## component     
### Summery
- Command and control to communicate with hardware control.
- Header definition are chosen at link time based on EXOPROJECT_APP cmake variable
---

---
## error   
### Summery
- asdf
---

## iacm  
- Store information between bootloader -> app -> reset -> bootloader

## include     

## osal      

---
## sequence  
- Refer to folder readme
---

---
## storage  
### Summery
- asdf
---

## task-monitor  

## update

## canopen

## conditioning

## health 

## icm

## msg-handler

## setting 

## sys

## trace

## utils

