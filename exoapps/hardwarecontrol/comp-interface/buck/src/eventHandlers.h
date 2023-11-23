/* 
 * File:   eventHandlers.h
 * Author: fnorwood
 *
 * Created on March 10, 2022, 4:10 PM
 */

#ifndef EVENTHANDLERS_H
#define	EVENTHANDLERS_H

// Event handlers
buck_state_t onCommandHandler(void);
buck_state_t powerGoodHandler(void);
buck_state_t offCommandHandler(void);
buck_state_t errorHandler(void);
buck_state_t errorClearedHandler(void);

#endif	/* EVENTHANDLERS_H */

