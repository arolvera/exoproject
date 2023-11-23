/* 
 * File:   HardwarePort.h
 * Author: fnorwood
 *
 * Created on March 2, 2022, 1:38 PM
 */

#ifndef HARDWAREPORT_H
#define	HARDWAREPORT_H

#define LED_ON (PORTD |= 0x01)
#define LED_OFF (PORTD &= ~0x01)

#define BUCK_ENABLE (PORTB &= ~0x80)
#define BUCK_DISABLE (PORTB |= 0x80)

void HardwarePortInit(void);            //Setup hardware port pins

#endif	/* HARDWAREPORT_H */

