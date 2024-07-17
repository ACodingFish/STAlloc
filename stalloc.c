#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "stalloc.h"



typedef struct __attribute__((__packed__)) StallocEntry_t_def
{
    uint8_t* start;
    uint8_t* end;
} StallocEntry_t;

static StallocEntry_t st_registry[STALLOC_MAX_ALLOCS];  // Needs to be protected with a mutex
uint32_t st_registry_count = 0;
static uint8_t st_buf[STALLOC_MAX_BUF];     // Needs to be protected with a mutex

uint8_t* stalloc_registry_find_space(uint32_t size);
void stalloc_registry_allocate(uint8_t* p_start, uint32_t size);
void stalloc_registry_free(uint8_t* p_start);

void Stalloc_Init(void)
{
    memset(st_buf, 0, sizeof(st_buf));
    memset(st_registry, 0, sizeof(st_registry));
}

void * Stalloc(uint32_t size)
{
    uint8_t * p_start = NULL;
    if ((size > 0) && ((st_registry_count+1) < STALLOC_MAX_ALLOCS))
    {
        p_start = stalloc_registry_find_space(size);
    }
    if (p_start != NULL)
    {
        stalloc_registry_allocate(p_start, size);
    }
    return (void*)p_start;
}

void Stfree(void* data)
{
    // Free Data
    // Reset Data Values to 0
    // Unregister from Registry - Set start and end to 0
    if (st_registry_count > 0)
    {
        stalloc_registry_free((uint8_t*)data);
    }
}


uint8_t* stalloc_registry_find_space(uint32_t size)
{
    uint8_t *p_start=st_buf;
    uint32_t buff_remaining=0;

    for (uint32_t i = 0; i < st_registry_count; i++)
    {  
        if (st_registry[i].end == 0) // Unregistered entry
        {
            buff_remaining = (STALLOC_MAX_BUF - (uint32_t)(p_start-st_buf));
            if (size < buff_remaining) // unregistered spot in registry and there is space
            {
                // store
            } else
            {
                p_start=NULL;    // Nothing found
            }
            break; // Sorted array, so break regardless since we are at the end of known values
        } else if ((st_registry[i].start - p_start) > size) // Registered entry, check against last entry
        {
            break; // store
        }
        
        p_start=st_registry[i].end+1;
    }
    return p_start;
}

void stalloc_registry_allocate(uint8_t* p_start, uint32_t size)
{
    for (uint32_t i = 0; i <= st_registry_count; i++)
    {
        if (st_registry[i].start > p_start)
        {
            for (uint32_t j = st_registry_count; j > i; j--)
            {
                if (st_registry[j-1].start!=0) // skip unregistered values
                {
                    st_registry[j]=st_registry[j-1];// shift down
                }
            } 
        }
        if ((st_registry[i].start==0)||(st_registry[i].start > p_start)) // store
        {
            st_registry[i].start=p_start;
            st_registry[i].end=p_start+size;
            st_registry_count++;
            break;
        }
    }
}

void stalloc_registry_free(uint8_t* p_start)
{
    uint32_t index = 0;

    if (p_start !=NULL)
    {
        for (uint32_t i = 0; i < st_registry_count; i++)
        {
            if (st_registry[i].start == p_start) // Find pointer in registry
            {
                index=i;
                break;
            }
        }

        memset(p_start, 0, (size_t)((uint8_t*)st_registry[index].end - p_start)); // Reset values to 0

        for (uint32_t i = index; i < st_registry_count; i++)
        {
            if (((i+1) >= st_registry_count) || (((i+1) < st_registry_count)&&(st_registry[i+1].start == 0))) // Single Clear if the rest are unset
            {
                st_registry[i].start=0;
                st_registry[i].end=0;
                break;
            } else // need to shift
            {
                st_registry[i]=st_registry[i+1];
            }
        }
        st_registry_count--;
    }
}