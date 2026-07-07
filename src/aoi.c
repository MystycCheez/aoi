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

// Action creation and modification functions

void InitActionData(aoiData* Data, unsigned long capacity)
{
    Data->ActionData.hash = 0; 
    Data->ActionData.count = 0;
    Data->ActionData.capacity = capacity;
    Data->ActionData.items = malloc(sizeof(HashEntry) * capacity);
    for (size_t i = 0; i < capacity; i++) {
        Data->ActionData.items[i].value = NULL;
        Data->ActionData.items[i].key = NULL;
    }
}

void SetActionHashFunction(aoiData* Data, unsigned long(*hash_fn)(char* name, unsigned long hash))
{
    Data->ActionHashFunction = hash_fn;
}

Action* NewAction(void (action)(aoiData*), const char* name, const char* desc)
{
    Action* a = malloc(sizeof(Action));
    Action tmp = {action, name, desc};
    memcpy(a, &tmp, sizeof(Action));

    return a;
}

void ResizeActionHashTable(aoiData* Data)
{
    HashEntry* tmp = malloc(sizeof(HashEntry) * Data->ActionData.capacity * 1.5);
    for (size_t i = 0; i < Data->ActionData.capacity; i++) tmp[i].key = NULL;

    unsigned long index;
    for (size_t i = 0; i < Data->ActionData.capacity; i++) {
        if (!(char*)Data->ActionData.items[i].key) continue;
        unsigned long hash = Data->ActionHashFunction((char*)Data->ActionData.items[i].key, Data->ActionData.hash);
        index = hash % Data->ActionData.capacity;
        tmp[index].key = Data->ActionData.items[i].key;
        tmp[index].value = Data->ActionData.items[i].value;
    }
    Data->ActionData.capacity *= 1.5;
    Data->ActionData.items = realloc(Data->ActionData.items, sizeof(HashEntry) * Data->ActionData.capacity);
    if (!Data->ActionData.items) {
        fprintf(stderr, "realloc failed!\n");
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < Data->ActionData.capacity; i++) {
        Data->ActionData.items[i].value = NULL;
        Data->ActionData.items[i].key = NULL;
    }
    mempcpy(Data->ActionData.items, tmp, Data->ActionData.capacity);
    free(tmp);
}

// TODO: make this more like ResizeActionHashTable
void ResizeUserDataHashTable(aoiData* Data)
{
    HashEntry* tmp = malloc(sizeof(HashEntry) * Data->UserData.capacity * 1.5);
    for (size_t i = 0; i < Data->UserData.capacity; i++) tmp[i].key = NULL;

    unsigned long index;
    for (size_t i = 0; i < Data->UserData.capacity; i++) {
        if (!(char*)Data->UserData.items[i].key) continue;
        unsigned long hash = Data->UserDataHashFunction((char*)Data->UserData.items[i].key, Data->UserData.hash);
        index = hash % Data->UserData.capacity;
        tmp[index].key = Data->UserData.items[i].key;
        tmp[index].value = Data->UserData.items[i].value;
    }
    Data->UserData.capacity *= 1.5;
    Data->UserData.items = realloc(Data->UserData.items, sizeof(HashEntry) * Data->UserData.capacity);
    mempcpy(Data->UserData.items, tmp, Data->UserData.capacity);
    free(tmp);
}

void AddActionWithStruct(aoiData* Data, Action* action, ActionBinding Bindings)
{
    if (Data->ActionData.count > (Data->ActionData.capacity * 0.75)) ResizeActionHashTable(Data);
    
    char* str = malloc(6);
    memset(str, 0, 6);
    memcpy(str, Bindings.binding, 5);
    unsigned long hash = Data->ActionHashFunction(str, Data->ActionData.hash);
    unsigned long index = hash % Data->ActionData.capacity;
    // printf("hash: %ld\n", hash);
    // printf("index: %ld\n", index);
    Data->ActionData.items[index].key = str;
    Data->ActionData.items[index].value = action;

    Data->ActionData.count++;
}

void SetActionWithStruct(aoiData* Data, Action* action, ActionBinding Bindings)
{
    unsigned long hash = Data->ActionHashFunction((char*)Bindings.binding, Data->ActionData.hash);
    unsigned long index = hash % Data->ActionData.capacity;
    Data->ActionData.items[index].value = action;   
}

Action* GetActionFromStruct(aoiData* Data, ActionBinding Bindings)
{
    unsigned long hash = Data->ActionHashFunction((char*)Bindings.binding, Data->ActionData.hash);
    unsigned long index = hash % Data->ActionData.capacity;
    if (!Data->ActionData.items[index].value) return GetActionFromName(Data, "Do Nothing");
    // printf("%s\n", ((Action*)Data->ActionData.items[index].value)->name);
    return Data->ActionData.items[index].value;
}

Action* GetActionFromName(aoiData* Data, const char* name)
{
    for (size_t i = 0; i < Data->ActionData.count; i++) {
        if (strcmp(((Action*)Data->ActionData.items[i].value)->name, name)) return Data->ActionData.items[i].value;
    }
    fprintf(stderr, "Action not found\n");
    return NULL;
}

//

void ActionHandlerFromStruct(aoiData* Data, ActionBinding Bindings)
{
    Action* a = GetActionFromStruct(Data, Bindings);
    if (!a->action) A_DoNothing(Data); else a->action(Data);
}

void ActionHandler(aoiData* Data)
{
    ActionHandlerFromStruct(Data, Data->ActiveBindings);
}

// User Data

void InitUserData(aoiData* Data, unsigned long capacity, unsigned long initHash)
{
    if (!capacity) {
        fprintf(stderr, "Capacity must be greater than 0!\n");
        exit(EXIT_FAILURE);
    }
    Data->UserData.capacity = capacity;
    Data->UserData.count = 0;
    Data->UserData.items = malloc(sizeof(HashEntry) * capacity);
    Data->UserData.hash = initHash;
    for (size_t i = 0; i < capacity; i++) {
        Data->UserData.items[i].value = NULL;
        Data->UserData.items[i].key = NULL;
    }
}

void SetUserDataHashFunction(aoiData* Data, unsigned long(*hash_fn)(char* name, unsigned long hash))
{
    Data->UserDataHashFunction = hash_fn;
}

void AddUserData(aoiData* Data, char* name, void* data)
{
    if (Data->UserData.count > (Data->UserData.capacity * 0.75)) {
        Data->UserData.capacity *= 1.5;
        Data->UserData.items = realloc(Data->UserData.items, sizeof(HashEntry) * Data->UserData.capacity);
    }

    unsigned long hash = Data->UserDataHashFunction(name, Data->UserData.hash);
    unsigned long index = hash % Data->UserData.capacity;
    // printf("hash: %ld\n", hash);
    // printf("index: %ld\n", index);
    Data->UserData.items[index].key = strdup(name);
    Data->UserData.items[index].value = data;
    Data->UserData.count++;
}

void* GetUserData(aoiData* Data, char* name)
{
    unsigned long hash = Data->UserDataHashFunction(name, Data->UserData.hash);
    unsigned long index = hash % Data->UserData.capacity;
    return Data->UserData.items[index].value;
}

//

void InitBindings(aoiData* Data, unsigned long capacity, unsigned long initHash)
{
    if (!capacity) {
        fprintf(stderr, "Capacity must be greater than 0!\n");
        exit(EXIT_FAILURE);
    }
    Data->Bindings.capacity = capacity;
    Data->Bindings.count = 0;
    Data->Bindings.items = malloc(sizeof(HashEntry) * capacity);
    Data->Bindings.hash = initHash;
    for (size_t i = 0; i < capacity; i++) {
        Data->Bindings.items[i].value = NULL;
        Data->Bindings.items[i].key = NULL;
    }
}

void SetBindingsHashFunction(aoiData* Data, unsigned long(*hash_fn)(char* name, unsigned long hash))
{
    Data->BindingsHashFunction = hash_fn;
}

void AddBinding(aoiData* Data, char* name, void* data)
{
    if (Data->Bindings.count > (Data->Bindings.capacity * 0.75)) {
        Data->Bindings.capacity *= 1.5;
        Data->Bindings.items = realloc(Data->Bindings.items, sizeof(HashEntry) * Data->Bindings.capacity);
    }

    unsigned long hash = Data->BindingsHashFunction(name, Data->Bindings.hash);
    unsigned long index = hash % Data->Bindings.capacity;
    // printf("hash: %ld\n", hash);
    // printf("index: %ld\n", index);
    Data->Bindings.items[index].key = strdup(name);
    Data->Bindings.items[index].value = data;
    Data->Bindings.count++;
}

//

aoiData* aoiInit(
    unsigned long hashSeed, 
    unsigned long UD_capacity, 
    unsigned long AD_capacity, 
    unsigned long(*ud_hash_fn)(char* name, unsigned long hash), 
    unsigned long(*ad_hash_fn)(char* name, unsigned long hash),
    unsigned long(*bn_hash_fn)(char* name, unsigned long hash)
)
{
    aoiData* Data = malloc(sizeof(aoiData));
    if (!Data) return NULL;
    InitActionData(Data, AD_capacity);
    InitUserData(Data, UD_capacity, hashSeed);

    SetUserDataHashFunction(Data, ud_hash_fn);
    SetActionHashFunction(Data, ad_hash_fn);
    SetBindingsHashFunction(Data, bn_hash_fn);

    // Why did I add LogLevel???????????????
    Data->LogLevel = LOG_DEFAULT;
    
    Data->ActiveBindings = (ActionBinding){0};

    Action* A_DONOTHING = NewAction(A_DoNothing, "Do Nothing", "Do nothing");
    AddAction(Data, A_DONOTHING);

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