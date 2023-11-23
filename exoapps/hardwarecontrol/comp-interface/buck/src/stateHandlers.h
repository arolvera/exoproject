/* 
 * File:   stateHandlers.h
 * Author: fnorwood
 *
 * Created on March 10, 2022, 4:04 PM
 */

#ifndef STATEHANDLERS_H
#define	STATEHANDLERS_H

// Different states for the 28V buck
typedef enum
{
    OFF_STATE_BUCK,
    SOFT_START_STATE_BUCK,
    ON_STATE_BUCK,
    ERROR_STATE_BUCK,
} buck_state_t;

// State handlers
void offStateHandler(void);
void softStartStateHandler(void);
void onStateHandler(void);
void errorStateHandler(void);

#endif	/* STATEHANDLERS_H */

