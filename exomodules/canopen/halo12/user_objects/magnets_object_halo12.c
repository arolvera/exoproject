#include "magnets_object.h"
#include "hsi_memory.h"

typedef enum{
  MAGNET_DIAG_SUBIDX_COUNT,
  MAGNET_DIAG_SUBIDX_VOUT,
  MAGNET_DIAG_SUBIDX_IOUT,
  /* Depending on the implementation it will be DAC or PWM, but it will be at
   * the same SUB-INDEX */
  MAGNET_DIAG_SUBIDX_CURR_STATE,
  MAGNET_DIAG_SUBIDX_ERR_CODE,
  MAGNET_DIAG_SUBIDX_PWM_OUTPUT,
  MAGNET_DIAG_SUBIDX_TEMP,
  MAGNET_DIAG_SUBIDX_EOL
} OD_MAGNET_DIAG;

void magnet_obj_add_telemetry(OD_DYN *self)
{
    // ODs for magnet DIAG
    ODAdd(self, CO_KEY(OD_INDEX_MAGNETS_O_DIAG, MAGNET_DIAG_SUBIDX_COUNT, CO_UNSIGNED8  |CO_OBJ_D__R_), 0, MAGNET_DIAG_SUBIDX_EOL - 1);

    ODAdd(self, CO_KEY(OD_INDEX_MAGNETS_O_DIAG, MAGNET_DIAG_SUBIDX_VOUT, CO_UNSIGNED16 | CO_OBJ____R_),
          0, (uintptr_t)&hsi_mem.magnet_telem.vout);

    ODAdd(self, CO_KEY(OD_INDEX_MAGNETS_O_DIAG, MAGNET_DIAG_SUBIDX_IOUT, CO_UNSIGNED16 | CO_OBJ____R_),
          0, (uintptr_t)&hsi_mem.magnet_telem.iout);

    ODAdd(self, CO_KEY(OD_INDEX_MAGNETS_O_DIAG, MAGNET_DIAG_SUBIDX_CURR_STATE, CO_UNSIGNED16 | CO_OBJ____R_),
          0, (uintptr_t)&hsi_mem.magnet_telem.current_state);

    ODAdd(self, CO_KEY(OD_INDEX_MAGNETS_O_DIAG, MAGNET_DIAG_SUBIDX_ERR_CODE, CO_UNSIGNED16 | CO_OBJ____R_),
          0, (uintptr_t)&hsi_mem.magnet_telem.error_code);

    ODAdd(self, CO_KEY(OD_INDEX_MAGNETS_O_DIAG, MAGNET_DIAG_SUBIDX_PWM_OUTPUT, CO_UNSIGNED16 | CO_OBJ____R_),
          0, (uintptr_t)&hsi_mem.magnet_telem.pwm_output);

    ODAdd(self, CO_KEY(OD_INDEX_MAGNETS_O_DIAG, MAGNET_DIAG_SUBIDX_TEMP, CO_UNSIGNED16 | CO_OBJ____R_),
          0, (uintptr_t)&hsi_mem.magnet_telem.temperature);
}
