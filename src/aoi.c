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

// User Data

//

aoiData* aoiInit(
    uint16_t UserDataCapacity, 
    uint16_t ActionDataCapacity, 
    uint16_t BindingCapacity, 
    uint64_t(*UserDataHashFunction)(const void* key), 
    uint64_t(*ActionDataHashFunction)(const void* key),
    uint64_t(*BindingDataHashFunction)(const void* key)
)
{
    aoiData* Data = malloc(sizeof(aoiData));
    if (!Data) return NULL;
    
    InitActionData(Data, ActionDataCapacity);
    InitUserData(Data, UserDataCapacity);
    InitBindingData(Data, BindingCapacity);

    SetHashFunction(&Data->ActionData, ActionDataHashFunction);
    SetHashFunction(&Data->UserData, UserDataHashFunction);
    SetHashFunction(&Data->BindingData, BindingDataHashFunction);

    return Data;
}

void aoiCleanup(aoiData* Data)
{
    // UserDataCleanup(Data);
    // ActionDataCleanup(Data);

    free(Data->UserData.items);
    free(Data->ActionData.items);
    free(Data);
}