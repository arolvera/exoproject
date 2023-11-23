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

#ifndef CALIB_VALS_UCV_H
#define	CALIB_VALS_UCV_H

#ifdef	__cplusplus
extern "C" {
#endif


#pragma pack(push,1)
typedef struct{
    float counts_per_psi_three_thousand;
}calib_vals_usv_t;
#pragma pack(pop)


calib_vals_usv_t *calib_vals_usv_get(int el);

float calib_vals_usv_cnt_psi_three_thousand_get(void);
void calib_vals_usv_cnt_psi_three_thousand_set(float psi);




#ifdef	__cplusplus
}
#endif

#endif	/* CALIB_VALS_UCV_H */

