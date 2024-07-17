/* *********************************************************************************
MIT License

Copyright (c) 2024 Jonathan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*********************************************************************************** */

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
static uint32_t st_registry_count = 0;
static uint8_t st_buf[STALLOC_MAX_BUF];     // Needs to be protected with a mutex

static uint8_t* stalloc_registry_find_space(uint32_t size);
static void stalloc_registry_allocate(uint8_t* p_start, uint32_t size);
static void stalloc_registry_free(uint8_t* p_start);

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


static uint8_t* stalloc_registry_find_space(uint32_t size)
{
    uint8_t *p_start=NULL;
    uint8_t * next_p_start=st_buf;
    uint32_t buff_remaining=0;

    for (uint32_t i = 0; i < STALLOC_MAX_ALLOCS; i++)
    {  
        if (st_registry[i].end == 0) // Unregistered entry
        {
            buff_remaining = ((uint32_t)STALLOC_MAX_BUF - (uint32_t)(next_p_start-st_buf));
            if (size <= buff_remaining) // unregistered spot in registry and there is space
            {
                p_start=next_p_start; // store
            } else
            {
                p_start=NULL;    // Nothing found
            }
            break; // Sorted array, so break regardless since we are at the end of known values
        } else if ((st_registry[i].start - next_p_start) >= size) // Registered entry, check against last entry
        {
            p_start=next_p_start; // store
            break;
        }
        
        next_p_start=st_registry[i].end+1;
    }
    return p_start;
}

static void stalloc_registry_allocate(uint8_t* p_start, uint32_t size)
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
            st_registry[i].end=p_start+size-1;
            st_registry_count++;
            break;
        }
    }
}

static void stalloc_registry_free(uint8_t* p_start)
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

        memset((uint8_t*)st_registry[index].start, 0, (size_t)((uint8_t*)st_registry[index].end + 1 - (uint8_t*)st_registry[index].start)); // Reset values to 0

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