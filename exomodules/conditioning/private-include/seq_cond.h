/*
           Copyright (C) 2021 ExoTerra Corp - All Rights Reserved

 Unauthorized copying of this file, via any medium is strictly prohibited
 Proprietary and confidential.  Any unauthorized use, duplication, transmission,
 distribution, or disclosure of this software is expressly forbidden.

 This Copyright notice may not be removed or modified without prior written
 consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this software without notice.

 ExoTerra Corp
 7640 S. Alkire Pl.
 Littleton, CO 80127
 USA

 Voice:  +1 1 (720) 788-2010
 http:   www.exoterracorp.com
 email:  contact@exoterracorp.com
*/

#ifndef HARDWARECONTROL_HALO12_VA41630_EXOMODULES_CONDITIONING_SEQUENCE_CONDITION_H_
#define HARDWARECONTROL_HALO12_VA41630_EXOMODULES_CONDITIONING_SEQUENCE_CONDITION_H_
/* seq_cond_config.h is one of the seq_cond_<proj>.h.in copied to the cmake/exomod/cond/include/ */
#include "seq_cond_config.h"

/* Interval in between health checks */
#ifndef COND_MONITOR_WAIT_MS
#error Pls define COND_MONITOR_WAIT_MS in seq_cond<seqencefiles>.c file
#endif
/* Number of loops per second */
#ifndef COND_MONITOR_LOOPS_SEC
#error Pls define COND_MONITOR_LOOPS_SEC in seq_cond<seqencefiles>.c file
#endif
/* Time to let thruster settle after throttling */
#ifndef COND_THROTTLE_SETTLE_MS
#error Pls define COND_THROTTLE_SETTLE_MS in seq_cond<seqencefiles>.c file
#endif
/* Number of loops to wait for the throttle point to settle */
#ifndef COND_THROTTLE_SETTLE_CNT
#error Pls define COND_THROTTLE_SETTLE_CNT in seq_cond<seqencefiles>.c file
#endif
/* Number of monitor loops before writing status out to NVM (60 seconds) */
#ifndef COND_STATUS_SAVE_LOOPS
#error Pls define COND_STATUS_SAVE_LOOPS in seq_cond<seqencefiles>.c file
#endif
/* Delay ms after error and before auto retry */
#ifndef COND_FAIL_RETRY_DELAY_MS
#error Pls define COND_FAIL_RETRY_DELAY_MS in seq_cond<seqencefiles>.c file
#endif
/* Enable/Disable Auto Retry   */
#ifndef COND_AUTO_RETRY
#error Pls define COND_AUTO_RETRY in seq_cond<seqencefiles>.c file
#endif
/* Max number of  Auto Retries */
#ifndef COND_AUTO_RETRY_MAX
#error Pls define COND_AUTO_RETRY_MAX in seq_cond<seqencefiles>.c file
#endif
/* The second step is the keeper, past this step we depend on 'things', for
 * example, the past the first keeper step the keeper has to be on already, and
 * for the first anode, the keeper has to be on.  We do not have logic to
 * fulfill these prerequisites at the time of this writing.
 */
/* The step past which we do not Auto Retry */
#ifndef COND_AUTO_RETRY_STEP_MAX
#error Pls define COND_AUTO_RETRY_MAX in seq_cond<seqencefiles>.c file
#endif

#ifndef CONDITIONING_KEEPER_VOLTAGE_LIMIT
#error Pls define CONDITIONING_KEEPER_VOLTAGE_LIMIT in seq_cond<seqencefiles>.c file
#endif
#ifndef CONDITIONING_ANODE_VOLTAGE_LIMIT
#error Pls define CONDITIONING_ANODE_VOLTAGE_LIMIT in seq_cond<seqencefiles>.c file
#endif
#ifndef CONDITIONING_ANODE_CURRENT_LIMIT
#error Pls define CONDITIONING_ANODE_CURRENT_LIMIT in seq_cond<seqencefiles>.c file
#endif
#ifndef CONDITIONING_ANODE_POWER_LIMIT
#error Pls define CONDITIONING_ANODE_POWER_LIMIT in seq_cond<seqencefiles>.c file
#endif
/* Throttle down        */
#ifndef CONDITIONING_ANODE_POWER_HIGH_LIMIT
#error Pls define CONDITIONING_ANODE_POWER_HIGH_LIMIT in seq_cond<seqencefiles>.c file
#endif
/* Throttle up 1st seq  */
#ifndef CONDITIONING_ANODE_POWER_LOW_LIMIT1
#error Pls define CONDITIONING_ANODE_POWER_LOW_LIMIT1 in seq_cond<seqencefiles>.c file
#endif
/* Throttle up 2nd      */
#ifndef CONDITIONING_ANODE_POWER_LOW_LIMIT2
#error Pls define CONDITIONING_ANODE_POWER_LIMIT in seq_cond<seqencefiles>.c file
#endif

#ifndef CONDITIONING_INPUT_CURRENT_LIMIT
#error Pls define CONDITIONING_INPUT_CURRENT_LIMIT in seq_cond<seqencefiles>.c file
#endif
#ifndef CONDITIONING_INPUT_POWER_LIMIT
#error Pls define CONDITIONING_INPUT_POWER_LIMIT in seq_cond<seqencefiles>.c file
#endif

#define SEQUENCE_MAX_STEPS_CONDITION  SEQUENCE_MAX_STEPS

#endif//HARDWARECONTROL_HALO12_VA41630_EXOMODULES_CONDITIONING_SEQUENCE_CONDITION_H_



