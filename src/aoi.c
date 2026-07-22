#include "aoi.h"

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
    uint16_t UserDataCapacity, 
    uint16_t ActionDataCapacity, 
    uint16_t BindingCapacity
)
{
    aoiData* Data = malloc(sizeof(aoiData));
    if (!Data) return NULL;
    
    Data->ActionData = *InitActionData(ActionDataCapacity);
    Data->UserData = *InitUserData(UserDataCapacity);
    Data->BindingData = *InitBindingData(BindingCapacity);

    return Data;
}

void aoiCleanup(aoiData* Data)
{
    // UserDataCleanup(Data);
    // ActionDataCleanup(Data);

    free(Data->UserData.entries);
    free(Data->ActionData.entries);
    free(Data->UserData.entries);
    free(Data);
}

// Modified from https://github.com/haipome/fnv
uint64_t Hash(const char* name)
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