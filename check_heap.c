
#include "umalloc.h"

//Place any variables needed here from umalloc.c as an extern.
extern memory_block_t *free_head;
extern memory_block_t *alloc_head;

/*
 * 1.  Is every block in the free list marked as free?
 */
bool all_free_marked() {
    memory_block_t *current_free_block = free_head;

    while (current_free_block) {
        if (!is_allocated(current_free_block)) {
            return false;
        }

        current_free_block = current_free_block->next.free;
    }

    return true;
}

/*
 * 3.  Are all blocks at the correct alignment?
 */
bool all_aligned() {
    // check free blocks
    memory_block_t *current_free_block = free_head;

    while (current_free_block) {
        if (current_free_block->block_size_alloc % 16 != 0) {
            return false;
        }
       
        current_free_block = current_free_block->next.free;
    }

    // check allocated blocks
    memory_block_t *current_alloc_block = alloc_head;

    while (current_alloc_block) {
        if (current_alloc_block->block_size_alloc % 16 != 0) {
            return false;
        }

        current_alloc_block = current_alloc_block->next.allocated;
    }

    return true;
}

/*
 * 9.  If you are implementing coalescing, are there any 
 * contiguous free blocks that somehow escaped coa-lescing?
 */
bool all_coalesced() {
    memory_block_t *current_free_block = free_head;

    while (current_free_block) {
        if (current_free_block->next.free == current_free_block + current_free_block->block_size_alloc && !is_allocated(current_free_block->next.free)) {
            return false;
        }
    }

    return true;
}

/*
 * 11.  If you are maintaining the free list in memory order, 
 * are you maintaining that order after inserting a freed block 
 * into the free list?
 */
bool free_list_ordered() {
    memory_block_t *current_free_block = free_head->next.free;

    while (current_free_block->next.free) {
        if (&current_free_block >= &current_free_block->next.free) {
            return false;
        }

        current_free_block = current_free_block->next.free;
    }

    return true;
}

/*
 * check_heap -  used to check that the heap is still in a consistent state.
 * Required to be completed for checkpoint 1.
 * Should return 0 if the heap is still consistent, otherwise return a non-zero
 * return code. Asserts are also a useful tool here.
 */
int check_heap() {
    // 1.  Is every block in the free list marked as free? Check 1
    if (! all_free_marked()) {
        return -1;
    }

    // 3.  Are all blocks at the correct alignment? Check 2
    if (! all_aligned()) {
        return -1;
    }

    // 9.  If you are implementing coalescing, are there any 
    // contiguous free blocks that somehow escaped coa-lescing? Check 3
    if (! all_coalesced()) {
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