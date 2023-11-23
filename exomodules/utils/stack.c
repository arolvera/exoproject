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
#include "stack.h"

void* STACK_ERROR = 0;

int stack_init(stack_t* stack)
{
    int err = 0;
    if(stack == 0){
        err = -1;
    } else {
        stack->top = -1;
        stack->capacity = DEFAULT_STACK_SIZE;
    }
    return err;
}


int stack_push(stack_t* stack, void* item)
{
    int err = 0;
    if((stack == 0) || (item == 0) || (stack->top + 1) > stack->capacity){
        err = -1;
    } else {
        stack->array[++stack->top] = item;
    }
    return err;
}


void* stack_pop(stack_t* stack)
{
    if(stack->top >= 0){
        return stack->array[stack->top--];
    } else {
        return STACK_ERROR;
    }
}


int stack_peek(stack_t* stack, void* item)
{
    int err = 0;
    if((stack == 0) || (item == 0)){
        err = -1;
    } else {
        item = stack->array[stack->top];
    }
    return err;
}


int stack_top_get(stack_t* stack)
{
    return stack->top;
}