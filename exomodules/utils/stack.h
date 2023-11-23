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
/** stack.h

  @Company
    ExoTerra

  @File Name
    stack.h

  @Summary
    declare APIs for interacting with the stack utility

 */
/* ************************************************************************** */

#ifndef _STACK_H    /* Guard against multiple inclusion */
#define _STACK_H

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif


#define DEFAULT_STACK_SIZE 100

typedef struct{
    int top;
    int capacity;
    void* array[DEFAULT_STACK_SIZE];
} stack_t; 


int stack_init(stack_t* stack);
int stack_push(stack_t* stack, void* item);
void* stack_pop(stack_t* stack);
int stack_peek(stack_t* stack, void* item);
int stack_top_get(stack_t* stack); 

    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
