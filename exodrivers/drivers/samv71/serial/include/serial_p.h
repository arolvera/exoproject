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
/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.h

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

#ifndef _SERIAL_PRIVATE_H    /* Guard against multiple inclusion */
#define _SERIAL_PRIVATE_H


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */

/* TODO:  Include other files here if needed. */


/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif


/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Constants                                                         */
/* ************************************************************************** */
/* ************************************************************************** */

/*  Serial Communications Constants.  */

/**
 * @Summary The size of the serial frame 
 * @Description 13 byte frame best aligns with CAN frame
 */
#define SERIAL_FRAME_SIZE 13
/**
 * @Summary The size of data portion in a serial frame
 * @Description Each serial frame contains a data section, this is the size
 *  of that section
 */
#define SERIAL_DATA_SIZE 8

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************

#pragma pack(push, 1)
struct serial_frame_data {
    uint16_t cob_id;
    uint8_t control;
    uint8_t data[SERIAL_DATA_SIZE];
    uint16_t crc;
    uint8_t unused[19]; // Round out to 32 bytes for Cache Coherency purposes
};

typedef union serial_frame {
    /**************************************************************************/
    /* 32 Bytes aligned for Cache Coherency purposes                          */
    /* data buffer  in the union makes debugging easier                       */
    uint8_t data[32];
    /**************************************************************************/
    struct serial_frame_data frame;
} Serial_Frame_t;
#pragma pack(pop)

typedef struct serial_dma {
    void *rxBuffer;
    void *txBuffer;
} RS485_Dma_t;


/* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _SERIAL_PRIVATE_H */

/* *****************************************************************************
 End of File
 */
