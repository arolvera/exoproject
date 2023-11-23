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
/****************************************************************************
* Name:         trace_flags.h
* Function:     This file the trace flag macros for enabling and disabling trace
* messages in a give module.
* 
* Trace flags are a bit mask.  They enabled/disabled with the global trcMsg
* variable
*
*  Revisions:
* 2021-03-21, jlm: port to SAMV71
****************************************************************************/


#define TrcMsgUpdate       0x00000001  /* Trace flag for the update process */
#define TrcMsgSm           0x00000002  /* Trace flag for Storage Modules    */
#define TrcMsgMcuCtl       0x00000004  /* Trace flag for controling MCUs    */
#define TrcMsgSerial       0x00000008  /* Trace flag for Serial interfac    */
#define TrcMsgBLServ       0x00000010  /* Trace flag for Boot Server        */
#define TrcMsgHSI          0x00000020  /* Trace flag for HSI                */
#define TrcMsgSeq          0x00000040  /* Trace flag for Sequences          */
#define TrcMsgNmt          0x00000080  /* Trace flag for NMT                */
#define TrcMsgADC          0x00000100  /* Trace flag for ADC                */
#define TrcMsgMemSrb       0x00000200  /* Trace flag for Memscrubber        */
