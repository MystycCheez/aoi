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