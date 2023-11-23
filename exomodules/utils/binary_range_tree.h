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
/**
 * @Company
 *      Exoterra
 * @Author
 *      Joshua Meyers
 * @File
 *      binary_range_tree.h
 * @Summary
 *      Provided binary search tree based on a high & low range
 * @Description
 *      Provide binary search tree capability, but instead of doing a single
 *      value, it is based on a ranges of values in each node. 
 * @Notes
 *      - Overlapping ranges are not supported
 *      - Range values are inclusive
 *      - Single values are supported by setting high & low to the same value
 * 
 * Created on June 11, 2021, 9:12 AM
 */
#ifndef BINARY_RANGE_TREE_H
#define	BINARY_RANGE_TREE_H

#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct RangeNode {
    uint16_t range_low;
    uint16_t range_high;
    struct RangeNode *left, *right;
} range_node_t;

range_node_t* binary_range_search(range_node_t* root, uint16_t key);
range_node_t* binary_range_insert(range_node_t *root, range_node_t *newnode);

#ifdef	__cplusplus
}
#endif

#endif	/* BINARY_RANGE_TREE_H */

