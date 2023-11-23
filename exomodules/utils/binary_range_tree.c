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
 *      See header
 * 
 * Created on June 11, 2021, 9:12 AM
 */

#include <stddef.h>
#include <stdint.h>

#include "binary_range_tree.h"

/**
 * Search for the key.  The key can be any value within a given range (inclusive).
 * For example, a key of 0x7DF will return the node for range 0x7DF-0x7FF.  Also,
 * 0x7FF will return the same node.  Another example would be 0x181 will return
 * a node with the range 0x181-0x181 (single value)
 * @param root pointer to the tree root
 * @param key key to search for
 * @return pointer to node containing the key if found, NULL otherwise
 */
range_node_t* binary_range_search(range_node_t* root, uint16_t key)
{
    range_node_t *ret = NULL;
    // Traverse until root reaches to dead end or node is found
    while (root != NULL && ret == NULL) {
        // pass right subtree as new tree
        if(key > root->range_high) {
            root = root->right;
        } else if(key < root->range_low) {
             // pass left subtree as new tree
            root = root->left;
        } else if(key >= root->range_low && key <= root->range_high) {
            // if the key is in range return it
            ret = root;
        }
    }
    return ret;
}

/**
 * Insert a node in the range tree.  If an overlapping range is detected, this
 * will return an error.
 * @param root pointer to root node
 * @param newnode pointer to the new node
 * @return pointer to the root node or NULL on error
 */
range_node_t* binary_range_insert(range_node_t *root, range_node_t *newnode)
{
    int err = 0;
    
    newnode->left = NULL;
    newnode->right = NULL;
    
    if(root == NULL) {
        root = newnode;
    } else {
        range_node_t *prev = NULL;
        range_node_t *curr = root;
        while(curr != NULL && !err) {
            prev = curr;
            if(newnode->range_low  < curr->range_low &&
               newnode->range_high < curr->range_low) {
                /* completely Left */
                curr = curr->left;
            } else if(newnode->range_low > curr->range_high &&
                      newnode->range_high > curr->range_high) {
                /* Complete Right */
                /* The second case (n->h > c->h) may not be necessary, however,
                 * it would indicate that something is wrong with the range in 
                 * one of the node.  Maybe always check range_high is greater
                 * than range low, because any new node with this case is an
                 * error
                 */
                curr = curr->right;
            } else {
                /* The new node is not completely left or right, it overlaps
                 * or is contained within another node, which is an error
                 */
                err = -1;
            }
        }
        if(!err) {
            if(prev->range_low  < newnode->range_low &&
               prev->range_high < newnode->range_low) { // previous is less than
                prev->right = newnode;
            } else {
                prev->left = newnode;
            }
        }
    }
    return (err == 0) ? root:NULL;
}