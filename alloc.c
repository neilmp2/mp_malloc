/**
 * Malloc
 * CS 240 - Fall 2020
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
    // allocate new memory
    void * new_memory = malloc(num * size);
    // initalize all of the new memory to 0 (set array to zero)
    if (new_memory != NULL) {
        memset(new_memory, 0, num*size);  //THIS IS WHAT LED ME TO FIND MEMSET: http://stackoverflow.com/questions/201101/how-to-initialize-all-members-of-an-array-to-the-same-value
        return new_memory;
    }
    return NULL;
}


/**
 * @brief Stuct holding the metadata for allocate.
 *  THIS CAN BE OPTIMIZED BY REMOVING THE PREV POINTER, BUT THIS WOULD REQUIRE CHANGING MOST OF THE CODE,
 *  AS THE PREV POINTER IS VERY CONVIENIENT FOR ACCESSING THE PREVIOUS FREE BLOCK AND REARRANGING POINTERS
 *  WHEN BLOCKS ARE REMOVED/ADDED TO THE FREELIST.
 */
typedef struct _metadata_t {
  unsigned int size;     // The size of the memory block.
  unsigned char isUsed;  // 0 if the block is free; 1 if the block is used.

  // Data to implement a double linked list
  struct _metadata_t * next;
  struct _metadata_t * prev; 
  struct _metadata_t * prev_real; 
} metadata_t;


//initialize head/tail ptrs to null
metadata_t * head = NULL; //head and tail of the metadata linked list. only free blocks have active pointers. 
metadata_t * tail = NULL;

//tail of the metadata reverse linked list. All blocks have a previous pointer pointing to the block of memory right before it. 
metadata_t * tail_real = NULL; 
void *startOfHeap = NULL;


/**
 * @brief implementation of block splitting
 * 
 */
void split(metadata_t * current_block, int desired_size) {
    int THRESHOLD = 1;
    // found a block of adequate size
    if (current_block->size >= desired_size) {
        // is the found block big enough to split? See Threshold value (bytes)
        //printf("Current block size is %d \n", current_block->size);
        //printf("Desired size is %d \n", desired_size);
        
        if (abs(current_block->size - desired_size) >= THRESHOLD) {
            //printf("%s\n", "block splitting");
            metadata_t * new_block = (void *)current_block + sizeof(metadata_t) + desired_size; 
            new_block->isUsed = 0; 
            new_block->size = current_block->size - desired_size - sizeof(metadata_t);
            new_block->prev_real = current_block; 

            // add the newly freed block to the end of the free list
            /* tail->next = new_block; 
            new_block->prev = tail; 
            new_block->next = NULL; 
            tail = new_block;  */

            // NEW IDEA: ADD THE NEWLY FREED BLOCK TO THE START OF THE FREE LIST! 
            if (head == NULL) {
                head = new_block;
                new_block->next = NULL;
                new_block->prev = NULL;
                tail = new_block;
            } else {
                new_block->prev= NULL;
                new_block->next = head; 
                head->prev = new_block;
                head = new_block;
            }


            //printf("New block size %d \n", new_block->size);

            // prepare current_block for returning (it is to be allocated)
            current_block->size = desired_size; 
            //printf("Returned block size is %d \n", current_block->size);
        }
    }
}


/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) {
    //fprintf(stderr, "Inside malloc\n");  
    // If we have not recorded the start of the heap, record it:
    if (startOfHeap == NULL) {
        startOfHeap = sbrk(0);
    }

    void * ptr; //this is the pointer to the START of the memory to return. 

    if (size == 0) {
        return NULL;
    }

    // Want to implement FIRST FIT! 
    if (head != NULL) {
        //head is not null, time to do first fit strategy!
        metadata_t * current = head;

        while(current != NULL) { 
            // found block of adequate size
            if (current->size >= size) {
                //make this the memory to Allocate
                split(current, size); // block split as close as possible
                current->isUsed = 1;
                current->size = size; // set size
                
                if (current != head && current != tail) {
                    current->prev->next = current->next; 
                    current->next->prev = current->prev; 
                } else if (current == head && current == tail) {
                    head = NULL;
                    tail = NULL; // no more free blocks now that current block is allocated
                } else if (current == head) {
                    head = current->next; 
                } else if (current == tail) {
                    tail = current->prev; 
                }
                
                current->prev = NULL;
                current->next = NULL; //removing current from the linked list of free blocks

                current = (void *)current + sizeof(metadata_t); // move current pointer to the start of the metadata "current's" memory block
                ptr = current;
                //printf("%s\n", "memory found, returning...");
                return ptr; 


            } // must continue searching the linked list of free blocks 
            else {
                current = current->next; 
            }
        }
    } 

    // adequate free memory not found.  need to allocate NEW memory using sbrk!
   
    metadata_t *meta = sbrk(sizeof(metadata_t) ); //allocate heap memory for the metadata struct
    meta->size = size;
    meta->isUsed = 1;
    meta->next = NULL;
    meta->prev = NULL; // this node is not free, so it's prev and next ptrs must be initalized to NULL. 
    
    ptr = sbrk(size); // allocate memory using sbrk system call
    if (tail_real == NULL) {
        tail_real = meta;
        meta->prev_real = NULL;
    } else {
        meta->prev_real = tail_real;
        tail_real = meta; //add new block to the all block single reverse linked list
        //printf("%s\n", "line 161");
    }
    //printf("%s\n", "NEW MEMORY ALLOCATED USING SBRK");
    // Return the pointer for the requested memory:
    return ptr; //ptr points to the start of the allocated program memory. right before ptr is the associated metadata memory
}




/**
 * @brief implmentation of memory coalescing 
 * 
 */
void coalesce(metadata_t * current) {
    //The pointer to the metadata of a newly freed memory block is passed in. 
    //Want to check forward and backward to see if it's neighbors are also free.
    //If both neighbors are free, condense forward into current, then current into backward. 
    //If only forward is free, condense forward into current. If only backward is free, condense current into backward. 
    metadata_t * forward = (void *)current + current->size + sizeof(metadata_t); //move forward from current to the next metadata structure
    metadata_t * backward = current->prev_real;
 
    void *endOfHeap = sbrk(0);
    int forward_free = 0;
    int backward_free = 0;
    //check forward
    if ((void *)forward < endOfHeap) {
        if (forward->isUsed == 0) {forward_free = 1;}
    }
    //check backward
    if ((void *)backward >= startOfHeap) {
        //printf("Backward is used %d\n ", backward->isUsed);
        if (backward->isUsed == 0) {backward_free =1;}
    }

    //printf("Prev real is %x\n", backward);
    
    //printf("Forward free is %d\n ", forward_free);
    //printf("Backward free is %d\n", backward_free);
    
    //coalesce forward into current (if applicable)
    if (forward_free == 1) {
        current->size = current->size + forward->size + sizeof(metadata_t); // current size now current's og size, the forward's size and the forward's metadata
        if (forward == head) {
            head = forward->next;
            forward->next->prev = NULL; 
            forward->next = NULL;  
        } else {
            forward->prev->next = forward->next;
            forward->next->prev = forward->prev;
        }

        //deal with the prev_real pointer
        metadata_t * above_forward = (void *)forward + forward->size + sizeof(metadata_t); //the block above forward
        if ((void *) above_forward < endOfHeap) { //check if it is a valid block
            above_forward->prev_real = current; //reassign prev_real pointer 
        }
        //null all forward pointers
        forward->size = 0; //flag that shows coalesced
        forward->prev = NULL;
        forward->prev_real = NULL;
        forward->next = NULL;
        //now we have a larger current block
        //printf("%s\n", "memory combined forward");
    }

    //coalesce current into backward (if applicable)
    if (backward_free == 1) {
        backward->size = backward->size + current->size + sizeof(metadata_t); 
        tail = current->prev;
        current->prev->next = NULL;
        current->prev = NULL;
        //deal with prev_real pointer from the block above current (forward!)
        if ((void *) forward < endOfHeap) {
            forward->prev_real = backward;
        }
        current->size = 0;
        current->next = NULL;
        current->prev = NULL;
        current->prev_real = NULL;
        //printf("%s\n", "memory combined backward");
    }
}


/**
 * @brief Prints heap data, useful for debugging.
 * 
 */
void printHeap() {
    // Print out data about each metadata chunk:
    metadata_t *curMeta = startOfHeap;
    void *endOfHeap = sbrk(0);
    printf("-- Start of Heap (%p) --\n", startOfHeap);
    while ((void *)curMeta < endOfHeap) {   // While we're before the end of the heap...
        printf("metadata for memory %p: (%p, size=%d, isUsed=%d)\n", (void *)curMeta + sizeof(metadata_t), curMeta, curMeta->size, curMeta->isUsed);
        curMeta = (void *)curMeta + curMeta->size + sizeof(metadata_t);
    }
    printf("-- End of Heap (%p) --\n\n", endOfHeap);
}




/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free (void *ptr) {
   
    if (ptr != NULL) {
        // Find the metadata located immediately before `ptr`:
        metadata_t *meta = ptr - sizeof( metadata_t );
        // Mark the allocation is free:
        meta->isUsed = 0;

        //add the newly freed memory to the end of the Free List
        if (head == NULL) { //if there's nothing else free, so this is the first in the double LL
            head = meta; 
            tail = meta; //if head is NULL, then tail should also be null! 
            meta->next = NULL;
            meta->prev = NULL;
            
        } else {
        // standard add to end stuff
        tail->next = meta; 
        meta->prev = tail; 
        tail = meta; 
        meta->next = NULL; //just in case!
        }
        coalesce(meta); //pass in to check for free blocks and merge them
    }
}



/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
    // implement realloc:
    if (ptr == NULL) {
        return malloc(size);
    }

    metadata_t * ptr_meta = ptr - sizeof(metadata_t);
    
    if (ptr_meta->size >= size) {
        //split(ptr_meta, size);  /. THIS DOES NOT WORK. Trying to shrink memory blocks uses too much memory because metadata is too big. Optimization: use 1 less ptr for metadata.
        return ptr; //For this particular implmentation -- not reallocating a smaller size is more efficient? (splitting fails tester4)
    } else { //need to allocate memory
        void * new_memory = malloc(size); //allocate new memory
        if (new_memory == NULL) {return NULL;}
        memcpy(new_memory, ptr, ptr_meta->size); //copy memory over
        free(ptr); //free the unneeded memory -- it has been copied.
        return new_memory;
    }
}