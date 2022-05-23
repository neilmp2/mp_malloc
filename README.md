# Malloc MP | CS240/340

A well known (and my personal favorite) assignment from UIUC's Systems Programming classes, this MP (machine problem) involves the implmentation of the core C function: Malloc.

All the code for the implmentations of memory allocation, array memory allocation and memory reallocation is in _alloc.c_.

## Functions Overview

### Calloc

To allocate memory for an array, the function creates memory using malloc and sets its values to zero for an array to fill.

### (Struct) Metadata

This structure occupies a small amount of space for every block of memory allocated, and stores key information that allows the system to run. Memory is stored as a doubly linked list.

- Size of corresponding memory block and usage status
- Pointers to previous and next blocks of memory
- A third reverse pointer that makes removal and addition of blocks simpler

### Split

Helper function -- splits a block of memory into two separate blocks based on a desired size.

Traverses linked-list looking for an appropriately sized block to split and splits if one is found. A key place where this implmentation could be optimized is in the threshold which determines a block's eligibility to be split.

### Malloc

The main function -- allocates memory in the system. It firsts checks to see if any memory is free to allocate, and if not, asks the system to give it the required amount.

### Coalesce

Helper function that combines consecutive free blocks of memory into a larger piece. Checks its forward and previous neigbors, and combines with them if either or both are unused.

### Free

Unallocates the specified block of data, and checks of coalescing is possible.

### Realloc

Reallocates memory that is being used to a different block of a specified size. The main use case is a certain program needing more memory than was initially allocated.
