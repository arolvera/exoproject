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
* Name:         trace.h
* Function:     This file contains macro, data and prototype information for
* the background Trace facility.
*
* It represents the definitions that are common to both system processor and
* and the flash module processor. It is included from the separate trace.h
* headers, which are custom to the module.
*
*  Revisions:
* 2021-03-21, jlm: port to SAMV71
****************************************************************************/
#ifndef _TRACECMN_H_
#define _TRACECMN_H_

#include <stdint.h>
#include "ext_decl_define.h"
#include "trace_flags.h"
#include "osal/osal.h"

#ifndef _TRACE_ENABLE
#define _TRACE_ENABLE 1
#endif

#if _TRACE_ENABLE

/* TraceSize is the number of trace messages (type TraceEntry) that can be stored.
 * When we increase from 0x200 to 0x400 we consume 20,480 more bytes of memory suggesting
 * 40 bytes per message. This holds up increasing from 0x400 to 0x800.
 * If we reserve 65k of memory for trace messages, then that means max number of messages
 * is 1,638.
 * So how is the message size set to 40 bytes? format_str and func are pointers. The rest
 * of the structure is 26 bytes. And why is the number of messages limited to powers of 2?
 * */
// ToDo: Size this for the number of messages that will fit in allocated memory
#define TraceSize       0x00000010  /* must be power of 2 */    // Why??????

#define TRACE_DATA_SIZE 24  /* Number of Data bytes in a trace message */

#pragma pack(push, 1)
/* Structure of one debug trace message. - keep it 32 bit aligned and 1 byte
 * pack it.  We have had some compilation units get different ideas about
 * how it should be packed, which does weird and unexpected things.
 */
typedef struct {
    const char *format_str; /* printf type format string    */
    const char *func;       /* Function String              */
    uint16_t level;         /* Log Level                    */
    uint16_t line;          /* Line number                  */
    uint32_t tag;           /* Time stamp                   */
    uint32_t data[TRACE_DATA_SIZE / 4];
} TraceEntry;
#pragma pack(pop)

/* Debug trace messages use a ring buffer with a head and tail pointer. */
EXT_DECL TraceEntry traceBuf[TraceSize]; // @TODO <-- Point this to RAM
EXT_DECL uint32_t traceHd, traceTl;

EXT_DECL OSAL_MUTEX_HANDLE_TYPE traceMtx;

#define TrcMsgErr1         0x80000000  /* Fatal errors */
#define TrcMsgErr2         0x40000000  /* Severe, recoverable errors */
#define TrcMsgErr3         0x20000000  /* Warnings */
#define TrcMsgInfo         0x10000000  /* Info messages */
#define TrcMsgDbg          0x08000000  /* Temporary Dbg messages */
#define TrcMsgAlways       0xffffffff  /* Always */

/* Default trace msg */
#define TrcMsgDefault (TrcMsgInfo  |TrcMsgErr1  | TrcMsgErr2   | TrcMsgErr3   | TrcMsgDbg | \
                      TrcMsgUpdate | TrcMsgMcuCtl | TrcMsgBLServ | TrcMsgHSI | \
                      TrcMsgSeq    | TrcMsgMemSrb)

EXT_DECL  uint32_t trcMsg
#   if InitVar
        = TrcMsgDefault
#   endif
    ;

EXT_DECL volatile uint32_t trace_timer
#   if InitVar
        = 0
#   endif
    ;

#define GET_TIME() trace_timer;

/* Move Log Level (TrcMsgErr/Info above) down to the upper byte of a 16 bit int */
#define LEVEL_LINE_LOG_MASK(__X__) ((__X__ & 0xf0000000) >> 16)
/* The TracePut and TracePrintfPut macros are where the real work of the
 * "input side" of the trace facility. They place new messages into the trace
 * buffer. Printing and/or saving of the trace message happens later.
 * 
 */
#define TracePut(log_level, fmt, d1, d2, d3, d4, d5, d6, _line) {              \
    traceHd &= TraceSize-1;                                                    \
    traceBuf[traceHd].format_str = fmt;                                        \
    traceBuf[traceHd].func    = __FUNCTION__;                                  \
    traceBuf[traceHd].level   = LEVEL_LINE_LOG_MASK(log_level);                \
    traceBuf[traceHd].line    = _line;                                         \
    traceBuf[traceHd].tag     = GET_TIME();                                    \
    traceBuf[traceHd].data[0] = (uint32_t) (d1);                               \
    traceBuf[traceHd].data[1] = (uint32_t) (d2);                               \
    traceBuf[traceHd].data[2] = (uint32_t) (d3);                               \
    traceBuf[traceHd].data[3] = (uint32_t) (d4);                               \
    traceBuf[traceHd].data[4] = (uint32_t) (d5);                               \
    traceBuf[traceHd].data[5] = (uint32_t) (d6);                               \
    traceHd = (traceHd+1) & (TraceSize-1);                                     \
    if (traceHd==traceTl) traceTl = (traceTl+1) & (TraceSize-1);               \
    OSAL_MUTEX_Unlock(&traceMtx);                                                       \
}

/* The Trace macros below handle putting messages in the trace buffer.
** Each message has a format string and a group of 6 words that can be
** converted and displayed. Only a pointer to the format string is placed
** in the entry so the string must be static. There are four trace macros
** to be used in the code corresponding macros to message levels:
**   Trace: Informational (no warning/error)
**   TraceDbg: During debug this is on by default.
**   TraceE3: Warnings.
**   TraceE2: Severe errors but recoverable. Will not crash, can continue.
**   TraceE1: Fatal errors could crash. Probably can't continue.
** Each Trace macro takes a msg type bitmap which is checked against the
** global TrcMsg variable. */

#define Trace(m,l,  fmt, d1,d2,d3,d4,d5,d6, line) do { if ((m) & trcMsg) TracePut(l, fmt,d1,d2,d3,d4,d5,d6,line); } while(0)
#define TraceE1(m,  fmt, d1,d2,d3,d4,d5,d6) do { Trace((m) | TrcMsgErr1, TrcMsgErr1, fmt,d1,d2,d3,d4,d5,d6, __LINE__); } while(0)
#define TraceE2(m,  fmt, d1,d2,d3,d4,d5,d6) do { Trace((m) | TrcMsgErr2, TrcMsgErr2, fmt,d1,d2,d3,d4,d5,d6, __LINE__); } while(0)
#define TraceE3(m,  fmt, d1,d2,d3,d4,d5,d6) do { Trace((m) | TrcMsgErr3, TrcMsgErr3, fmt,d1,d2,d3,d4,d5,d6, __LINE__); } while(0)
#define TraceInfo(m,fmt, d1,d2,d3,d4,d5,d6) do { Trace((m) | TrcMsgInfo, TrcMsgInfo, fmt,d1,d2,d3,d4,d5,d6, __LINE__); } while(0)
#ifdef __DEBUG
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6) do { Trace((m),  TrcMsgDbg,  fmt,d1,d2,d3,d4,d5,d6, __LINE__); } while(0)
#else
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6)
#endif

#else
#define Trace(m,l,  fmt, d1,d2,d3,d4,d5,d6, line)
#define TraceE1(m,  fmt, d1,d2,d3,d4,d5,d6)
#define TraceE2(m,  fmt, d1,d2,d3,d4,d5,d6)
#define TraceE3(m,  fmt, d1,d2,d3,d4,d5,d6)
#define TraceInfo(m,fmt, d1,d2,d3,d4,d5,d6)
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6)
#endif
#define TraceClear() traceHd = traceTl = 0;

void trcInit(void);
int trcGet(char *buf, int max_size, int which_end);
uint32_t traceShowN(unsigned int n);

/*
 * Only added for test applications that use trace and need a handle to the UART
 * to receive commands entered via console.
 */
int trcUartHandleGet(void);
        
#endif /* _TRACECMN_H_ */
