/*
 * File: umalloc.c
 * Name: Cole Amlong
 * EID: CA34894
 * 
 * Each block has 16 B of meta-data: An 8 Byte unsigned long for the size of its 
 * payload and An 8 Byte pointer to a "next" block (used for traversing an explicit 
 * free list). Each free block in the explicit free list is sorted in ascending 
 * order by its memory address. When the allocator is called, it finds the first 
 * block in the free list big enough to contain it. The machine then trims the 
 * designated free block so it exactly fits the needed space (using the split function).
 */
#include "umalloc.h"
#include "csbrk.h"
#include "ansicolors.h"
#include <stdio.h>
#include <assert.h>

const char author[] = ANSI_BOLD ANSI_COLOR_RED "COLE AMLONG | CA34894" ANSI_RESET;

// A pointer to the start of the explicit free list.
memory_block_t *free_head;

/*
 * HELPER METHODS
 */

/*
 * is_allocated - returns true if a block is marked as allocated.
 */
bool is_allocated(memory_block_t *block) {
    assert(block != NULL);
    return block->block_size_alloc & 0x1;
}

/*
 * deallocate - marks a block as unallocated.
 */
void deallocate(memory_block_t *block) {
    assert(block != NULL);
    block->block_size_alloc &= ~0x1;
}

/*
 * get_size - gets the size of the block.
 */
size_t get_size(memory_block_t *block) {
    assert(block != NULL);
    return block->block_size_alloc & ~(ALIGNMENT-1);
}

/*
 * get_next - gets the next block in the free list.
 */
memory_block_t *get_next(memory_block_t *block) {
    assert(block != NULL);
    return block->next;
}

/*
 * put_block - puts a block struct into memory at the specified address.
 * Initializes the size and allocated fields, along with Nulling out the next 
 * field.
 */
void put_block(memory_block_t *block, size_t size, bool alloc) {
    assert(block != NULL);
    assert(size % ALIGNMENT == 0);
    assert(alloc >> 1 == 0);
    block->block_size_alloc = size | alloc;
    block->next = NULL;
}

/*
 * get_payload - gets the payload of the block.
 */
void *get_payload(memory_block_t *block) {
    assert(block != NULL);
    return (void*)(block + 1);
}

/*
 * get_block - given a payload, returns the block.
 */
memory_block_t *get_block(void *payload) {
    assert(payload != NULL);
    return ((memory_block_t *)payload) - 1;
}

/*
 * START OF UMALLOC IMPLEMENTATION
 */

/*
 * find - finds a free block that can satisfy the umalloc request.
 */
memory_block_t *find(size_t size) {
    // align size to ALIGNMENT (padding)
    size = ALIGN(size);

    if (free_head == NULL) {
        // no space is left, expand heap
        free_head = extend(PAGESIZE);
    }

    // traverse free list to find space for new allocated block
    memory_block_t *block = free_head;
    while (block) {
        if (get_size(block) >= size) {
            // found sufficient space, return tailored allocated space
            return split(block, size);
        }

        block = get_next(block);
    }

    // failed to find sufficient space, extend heap, return tailored allocated space
    return split(extend(size), size);
}

/*
 * extend - extends the heap if more memory is required.
 */
memory_block_t *extend(size_t size) {
    memory_block_t *expansion = csbrk(size);
    put_block(expansion, size, false);

    if (free_head == NULL) {
        // no other free blocks: no need to order
        return expansion;
    }

    // move pointer to end of free list
    memory_block_t *block = free_head;
    while (get_next(block)) {
        block = get_next(block);
    }
    
    // add expansion to the end of the free list
    block->next = expansion;
    return expansion;
}

/*
 * split - splits a given block in parts, one allocated, one free.
 */
memory_block_t *split(memory_block_t *block, size_t size) {
    // create a new allocated block in the latter half of the free block
    // so there is no need to update the free list
    memory_block_t *alloc_block = block + (get_size(block) - size) / sizeof(memory_block_t);
    put_block(alloc_block, size, true);

    if (alloc_block == block) {
        // the new allocated block is the same size as the free block
        if (free_head == alloc_block) {
            // special case: we are dealing with the free header
            free_head = get_next(free_head);
        } else {
            // traverse the free list
            memory_block_t *free_block = free_head;
            while (get_next(free_block) && get_next(free_block) != block) {
                free_block = get_next(free_block);
            }

            // remove the free block from the list
            free_block->next = get_next(block);
        }
    }

    // decrease free block size
    block->block_size_alloc -= size;
    
    return alloc_block;
}

/*
 * uinit - Used initialize metadata required to manage the heap
 * along with allocating initial memory.
 */
int uinit() {
    // initialize the heap as one block of free memory
    free_head = csbrk(PAGESIZE);
    put_block(free_head, PAGESIZE, false);
    return 0;
}

/*
 * umalloc -  allocates size bytes and returns a pointer to the allocated memory.
 * TODO: add new_block to alloc_head
 */
void *umalloc(size_t size) {
    memory_block_t *new_block = find(size + sizeof(memory_block_t));
    return get_payload(new_block);
}

/*
 * ufree - frees the memory space pointed to by ptr, which must have been called
 * by a previous call to malloc.
 * TODO: Update allochead if it is removed;
 */
void ufree(void *ptr) {
    memory_block_t *block = get_block(ptr);
    deallocate(block);
    
    // traverse the free list, find the index to insert the block to the free list.
    memory_block_t *free_block = free_head;
    while (get_next(free_block) && get_next(free_block) <= block) {
        free_block = get_next(free_block);
    }

    if (get_next(free_block)) {
        // not at the end of the free list, update block pointer
        block->next = get_next(free_block);
    }

    // update previous block pointer
    free_block->next = block;
}