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

// Hash related

void SetHashFunction(HashData* Table, uint64_t(*hash_fn)(const void* key))
{
    Table->HashFunction = hash_fn;
}

void ResizeHashTable(HashData* Table)
{
    // printf("ResizeHashTable\n");
    // printf("Current Capacity: %lu\n", Table->capacity);
    // printf("Current Count: %lu\n\n", Table->count);
    KV_Pair* tmp = malloc(sizeof(KV_Pair) * Table->capacity * 1.5);
    if (!tmp) {
        fprintf(stderr, "malloc failed!\n");
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < (uint64_t)(Table->capacity * 1.5); i++) {
        // printf("%zu\n", i);
        tmp[i].key = NULL;
        tmp[i].value = NULL;
    }

    uint64_t index;
    for (size_t i = 0; i < Table->capacity; i++) {
        printf("items[%zu].key: %p\n", i, Table->items[i].key);
        printf("items[%zu].value: %p\n\n", i, Table->items[i].value);
        if (!Table->items[i].key) continue;
        uint64_t hash = Table->HashFunction(Table->items[i].key);
        index = hash % Table->capacity;
        tmp[index].key = Table->items[i].key;
        tmp[index].value = Table->items[i].value;
    }
    // printf("New Capacity: %u\n\n", (uint16_t)(Table->capacity * 1.5));
    Table->capacity *= 1.5;
    Table->items = realloc(Table->items, sizeof(KV_Pair) * Table->capacity);
    if (!Table->items) {
        fprintf(stderr, "realloc failed!\n");
        exit(EXIT_FAILURE);
    }
    mempcpy(Table->items, tmp, sizeof(KV_Pair) * Table->capacity);
    free(tmp);
}

//

uint16_t* ConvertBinding(aoiData* Data, Binding* binding)
{
    uint16_t* b_list = calloc(Data->BindingData.capacity, sizeof(uint16_t));

    for (uint16_t i = 0; i < Data->BindingData.capacity; i++) {
        if (!binding[i].name) break;
        b_list[i] = binding[i].value;
    }
    return b_list;
}

uint16_t* ConvertKV_Pair(aoiData* Data, KV_Pair* pairs)
{
    uint16_t* b_list = calloc(Data->BindingData.capacity, sizeof(uint16_t));

    for (uint16_t i = 0; i < Data->BindingData.capacity; i++) {
        b_list[i] = *(uint16_t*)pairs[i].value;
    }

    return b_list;
}

void InitBindingData(aoiData* Data, uint16_t capacity)
{
    if (!capacity) {
        fprintf(stderr, "Capacity must be greater than 0!\n");
        exit(EXIT_FAILURE);
    }
    Data->BindingData.capacity = capacity;
    Data->BindingData.count = 0;
    Data->BindingData.items = malloc(sizeof(KV_Pair) * capacity);
    for (size_t i = 0; i < capacity; i++) {
        Data->BindingData.items[i].key = NULL;
        Data->BindingData.items[i].value = malloc(sizeof(uint16_t));
        *(uint16_t*)Data->BindingData.items[i].value = 0;
    }
    Data->ActiveBindings = malloc(sizeof(uint16_t*) * Data->BindingData.capacity);
    SetActiveBindings(Data);
}

void AddBinding(aoiData* Data, char* name)
{
    if (Data->BindingData.count >= (Data->BindingData.capacity * 0.75)) {
        ResizeHashTable(&Data->BindingData);
        Data->ActiveBindings = realloc(Data->ActiveBindings, sizeof(uint16_t*) * Data->BindingData.capacity);
    }

    uint64_t hash = Data->BindingData.HashFunction(name);
    uint64_t index = hash % Data->BindingData.capacity;

    Data->BindingData.items[index].key = name;
    *(uint16_t*)Data->BindingData.items[index].value = 0;

    Data->BindingData.count++;
    SetActiveBindings(Data);
}

void SetBinding(aoiData* Data, Binding binding)
{
    // printf("Binding:\n Name: %s\n Value: %u\n", binding.name, binding.value);

    uint64_t hash = Data->BindingData.HashFunction(binding.name);
    uint64_t index = hash % Data->BindingData.capacity;

    // printf("hash: %lu\n\n", hash);

    *(uint16_t*)Data->BindingData.items[index].value = binding.value;
}

void SetActiveBindings(aoiData* Data)
{
    for (size_t i = 0; i < Data->BindingData.capacity; i++) {
        Data->ActiveBindings[i] = NULL;
        if (!Data->BindingData.items[i].key) continue;
        // printf("key: %s\n", (char*)Data->BindingData.items[i].key);
        // printf("key: %u\n", *(uint16_t*)Data->BindingData.items[i].value);
        uint64_t hash = Data->BindingData.HashFunction(Data->BindingData.items[i].key);
        uint64_t index = hash % Data->BindingData.capacity;
        Data->ActiveBindings[index] = Data->BindingData.items[index].value;
    }
}

void ResetBindings(aoiData* Data)
{
    for (uint64_t i = 0; i < Data->BindingData.capacity; i++) {
        if (!Data->ActiveBindings[i]) continue;
        *Data->ActiveBindings[i] = 0;
    }
}

// Action creation and modification functions

void InitActionData(aoiData* Data, uint16_t capacity)
{
    Data->ActionData.hash = 0; 
    Data->ActionData.count = 0;
    Data->ActionData.capacity = capacity;
    Data->ActionData.items = malloc(sizeof(KV_Pair) * capacity);
    for (size_t i = 0; i < capacity; i++) {
        Data->ActionData.items[i].key = NULL;
        Data->ActionData.items[i].value = NULL;
    }
}

Action* NewAction(void (action)(aoiData*), const char* name, const char* desc)
{
    Action* a = malloc(sizeof(Action));
    Action tmp = {action, name, desc};
    memcpy(a, &tmp, sizeof(Action));

    return a;
}

void AddAction_(aoiData* Data, Action* action, Binding* binding)
{
    // printf("AddAction_\n");
    if (Data->ActionData.count >= (Data->ActionData.capacity * 0.75)) ResizeHashTable(&(Data->ActionData));

    uint16_t* b_list = malloc(sizeof(uint16_t) * Data->BindingData.capacity);
    memset(b_list, 0, sizeof(uint16_t) * Data->BindingData.capacity);

    for (size_t i = 0; i < Data->BindingData.capacity; i++) {
        // printf("Name: %s\n", binding[i].name);
        // printf("Name: %u\n", binding[i].value);
        if (!binding[i].name) break;
        uint64_t hash = Data->BindingData.HashFunction(binding[i].name);
        uint64_t index = hash % Data->BindingData.capacity;
        b_list[index] = binding[i].value;
    }

    for (uint16_t i = 0; i < Data->BindingData.capacity; i++) {
        printf("%u ", b_list[i]);
    }
    printf("\n");

    uint64_t hash = Data->BindingData.HashFunction(b_list);
    uint64_t index = hash % Data->BindingData.capacity;

    // printf("index: %zu\n\n", index);

    Data->ActionData.items[index].key = b_list;
    Data->ActionData.items[index].value = action;

    Data->ActionData.count++;
}

void SetAction_(aoiData* Data, Action* action, Binding* binding)
{
    uint16_t* b_list = ConvertBinding(Data, binding);

    uint64_t hash = Data->ActionData.HashFunction(b_list);
    uint64_t index = hash % Data->ActionData.capacity;

    Data->ActionData.items[index].value = action;
}

Action* GetAction_(aoiData* Data, Binding* binding)
{
    uint16_t* b_list = ConvertBinding(Data, binding);

    uint64_t hash = Data->ActionData.HashFunction(b_list);
    uint64_t index = hash % Data->ActionData.capacity;
    if (!Data->ActionData.items[index].value) {
        char* name = calloc(Data->ActionData.capacity, 1);
        hash = Data->ActionData.HashFunction(name);
        index = hash % Data->ActionData.capacity;
        free(name);
    }
    return Data->ActionData.items[index].value;
}

Action* GetActionFromCurrentBindings(aoiData* Data)
{
    uint16_t* values = malloc(sizeof(uint16_t) * Data->BindingData.capacity);
    memset(values, 0, sizeof(uint16_t) * Data->BindingData.capacity);
    for (size_t i = 0; i < Data->BindingData.capacity; i ++) {
        if (!Data->ActiveBindings[i]) continue;
        values[i] = *Data->ActiveBindings[i];
    }
    uint64_t hash = Data->ActionData.HashFunction(values);
    uint64_t index = hash % Data->ActionData.capacity;
    free(values);
    return Data->ActionData.items[index].value;
}

//

void ActionHandler(aoiData* Data)
{
    Action* a = GetActionFromCurrentBindings(Data);
    if ((!a) || (!a->action)) A_DoNothing(Data); else a->action(Data);
}

// User Data

void InitUserData(aoiData* Data, uint16_t capacity)
{
    if (!capacity) {
        fprintf(stderr, "Capacity must be greater than 0!\n");
        exit(EXIT_FAILURE);
    }
    Data->UserData.capacity = capacity;
    Data->UserData.count = 0;
    Data->UserData.items = malloc(sizeof(KV_Pair) * capacity);
    for (size_t i = 0; i < capacity; i++) {
        Data->UserData.items[i].value = NULL;
        Data->UserData.items[i].key = NULL;
    }
}

void AddUserData(aoiData* Data, char* name, void* data)
{
    if (Data->UserData.count >= (Data->UserData.capacity * 0.75)) ResizeHashTable(&(Data->UserData));

    uint64_t hash = Data->UserData.HashFunction(name);
    uint64_t index = hash % Data->UserData.capacity;

    Data->UserData.items[index].key = strdup(name);
    Data->UserData.items[index].value = data;
    Data->UserData.count++;
}

void* GetUserData(aoiData* Data, char* name)
{
    uint64_t hash = Data->UserData.HashFunction(name);
    uint64_t index = hash % Data->UserData.capacity;
    return Data->UserData.items[index].value;
}

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