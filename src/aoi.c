#include "aoi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Dummy action

void A_DoNothing(aoiData* Data)
{
    (void)Data;
    return;
}

// aoi specific

aoiData* aoiInit(
    uint64_t UserDataCapacity, 
    uint64_t ActionDataCapacity, 
    uint64_t BindingCapacity
)
{
    aoiData* Data = malloc(sizeof(aoiData));
    if (!Data) return NULL;
    
    Data->BindingData = InitBindingData(BindingCapacity);
    Data->UserData = InitUserData(UserDataCapacity);
    Data->ActionData = InitActionData(ActionDataCapacity, BindingCapacity);

    Data->ActiveBindings = malloc(sizeof(uint16_t*) * BindingCapacity);
    for (size_t i = 0; i < BindingCapacity; i++) {
        Data->ActiveBindings[i] = malloc(sizeof(uint16_t));
        *Data->ActiveBindings[i] = 0;
    }

    return Data;
}

void aoiCleanup(aoiData* Data)
{
    // UserDataCleanup(Data);
    // ActionDataCleanup(Data);

    free(Data->UserData->entries);
    free(Data->ActionData->entries);
    free(Data->UserData->entries);
    free(Data);
}

// Modified from https://github.com/haipome/fnv
uint64_t HashStr(const char* name)
{
    unsigned long len = strlen(name);
    uint64_t hval = 0xcbf29ce484222325ULL;
    // printf("hasher\n");
    uint8_t* bp = (uint8_t*)name;
    uint8_t* be = bp + len;

    while (bp < be) {
        // printf("bp: ");
        // printf("%p: %u\n", bp, (uint8_t)*bp);
	    hval ^= (uint8_t)*bp++;

	    hval += 
        (hval << 1) + 
        (hval << 4) + 
        (hval << 5) +
	    (hval << 7) + 
        (hval << 8) + 
        (hval << 40);
    }
    // printf("hval: %zu\n", hval);

    return hval;
}

uint64_t HashPattern(const uint16_t* pattern, uint64_t len)
{
    uint64_t hval = 0xcbf29ce484222325ULL;
    if (!pattern) {
        fprintf(stderr, "Error, pattern null!\n");
        // exit(EXIT_FAILURE);
    }
    // printf("len: %zu\n", len);
    // printf("%p\n", pattern);
    
    // for (size_t i = 0; i < len; i++) {
    //     printf("%u ", pattern[i]);
    // }
    // printf("\n");
    uint16_t* bp = (uint16_t*)pattern;
    uint16_t* be = bp + len;

    while (bp < be) {
        // printf("bp: ");
        // printf("%p: %u\n", bp, (uint16_t)*bp);
	    hval ^= (uint16_t)*bp++;

	    hval += 
        (hval << 1) + 
        (hval << 4) + 
        (hval << 5) +
	    (hval << 7) + 
        (hval << 8) + 
        (hval << 40);
    }
    // printf("hval: %zu\n", hval);

    return hval;
}