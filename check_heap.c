/*
 * File: check_heap.c
 * Name: Cole Amlong
 * EID: CA34894
 * 
 * This file contains checks to make sure the custom implemented heap is valid.
 * Checks include: 
 *  1. Is every block in the free list marked as free?
 *  2. Are all free blocks at the correct alignment?
 *  3. All free block sizes are greater than 0?
 *  4. Free list is in proper memory address order?
 */

#include "umalloc.h"

// A pointer to the beginning of the explicit free list.
extern memory_block_t *free_head;

/*
 * 1.  Is every block in the free list marked as free?
 */
bool all_free_marked() {
    // traverse through free list
    memory_block_t *block = free_head;
    while (block) {
        if (is_allocated(block)) {
            // block is marked as allocated in the free list, failed
            return false;
        }

        block = get_next(block);
    }

    // all free blocks checked, passed
    return true;
}

/*
 * 3.  Are all free blocks at the correct alignment?
 */
bool all_free_aligned() {
    // traverse through free list
    memory_block_t *block = free_head;
    while (get_next(block)) {
        if (get_size(block) % ALIGNMENT != 0) {
            // block is not aligned, failed
            return false;
        }
       
        block = get_next(block);
    }

    // all free blocks checked, passed
    return true;
}

/*
 * CUSTOM: All free block sizes are greater than 0?
 * This test, although simple, was critical for me in debugging.
 */
bool all_size_positive() {
    // traverse the free list
    memory_block_t *block = free_head;
    while (block) {
        if (get_size(block) <= 0) {
            // block size is <= 0, failed
            return false;
        }

        block = get_next(block);
    }
    
    // all blocks have a proper size, passed
    return true;
}

/*
 * 11.  Free list is in proper memory address order?
 */
bool free_list_ordered() {
    if (! get_next(free_head)) {
        // list only has one element, therefore it is ordered
        return true;
    }

    // traverse through free list
    memory_block_t *block = free_head;
    while (get_next(block)) {
        if (block >= get_next(block)) {
            // next free block's memory address is before this block, free list is not ordered
            return false;
        }

        block = get_next(block);
    }

    // all free blocks listed have been checked, free list is ordered
    return true;
}

/*
 * check_heap -  used to check that the heap is still in a consistent state.
 * Required to be completed for checkpoint 1.
 * Should return 0 if the heap is still consistent, otherwise return a non-zero
 * return code. Asserts are also a useful tool here.
 */
int check_heap() {
    // CUSTOM: All free block sizes are greater than 0? Check 1
    if (! all_size_positive()) {
         return -1;
    }

    // 1.  Is every block in the free list marked as free? Check 2
    if (! all_free_marked()) {
        return -1;
    }

    // 3.  Are all blocks at the correct alignment? Check 3
    if (! all_free_aligned()) {
        return -1;
    }

    // 11.  If you are maintaining the free list in memory order,
    // are you maintaining that order after inserting afreed block 
    // into the free list? Check 4
    if (! free_list_ordered()) {
        return -1;
    }
    
    return 0;
}