# CMSIS
## cmsis/cmsis/
cmsis/cmsis consists standard files. It is structured this way so that when linking against cmsis 
include are "cmsis/cmsis_x.h"

## devices/
- Stores processor specific sys_init.h interface file. 
- Processor specifc device.h file. device.h defines:
  - IRQ_typen enum. 
  - Peripheral register layout and addresses
  - cmsis #defines such as (__FPU_PRESENT)


