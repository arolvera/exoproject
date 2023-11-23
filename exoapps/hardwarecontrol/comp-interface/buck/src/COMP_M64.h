/* 
 * File:   COMP_M64.h
 * Author: fnorwood
 *
 * Created on March 11, 2022, 11:21 AM
 */

#ifndef COMP_M64_H
#define	COMP_M64_H

void COMP_init(void);

#define OVER_I_COMP_ENABLE                      \
{                                               \
    /* Clear the interrupt flag */              \
    ACSR |= (1 << AC3IF);                       \
    /* Enable interrupt */                      \
    AC3CON |= (1 << AC3IE);                     \
}

#define OVER_I_COMP_DISABLE (AC3CON &= ~(1 << AC3IE)) // Disable interrupt

#endif	/* COMP_M64_H */

