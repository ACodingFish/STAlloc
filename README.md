# STAlloc
## Description
A statically-allocated implementation of malloc for embedded systems

## Configuration (stalloc.h)
The following parameters can be adjusted to fit the needs of the system:
* #define STALLOC_MAX_ALLOCS      1000U           // Max Allocations in Registry
    * This is the max number of registry entries (allocations) allowed
    * Size of Registry buffer will be (STALLOC_MAX_ALLOCS * 2 * sizeof(uint8_t*))
* #define STALLOC_MAX_BUF         100000U         // Max Allocated Memory

## Initialization - void Stalloc_Init(void)
Initializes registry and buffer values to 0. 

## Malloc Equivalent - void * Stalloc(uint32_t size)
Statically allocates memory in a "malloc"-like fashion. Tracks pointers in registry.
* uint32_t size - specifies byte count to be allocated
* Return Value: Pointer to allocated memory, or NULL on failure.

## Free Equivalent - void Stfree(void* data)
"Free"s previously allocated memory from registry. Resets all buffer data back to 0.
* void* data - Pointer to previously allocated memory. Will ignore NULL values.
    * Non-Null data that is not present in the registry is an unhandled condition

