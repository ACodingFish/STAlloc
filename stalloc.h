#ifndef _STALLOC_H_
#define _STALLOC_H_

#define STALLOC_MAX_ALLOCS      1000U           // Max Allocations in Registry
#define STALLOC_MAX_BUF         100000U         // Max Allocated Memory

void Stalloc_Init(void);        //  Initializes Static Malloc Buffer
void * Stalloc(uint32_t size);  //  Static Malloc
void Stfree(void* data);        //  Static Free

#endif // _STALLOC_H_
