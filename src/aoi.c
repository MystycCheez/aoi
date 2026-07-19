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

uint64_t HashBinding(const char* name)
{

}

uint64_t HashAction(uint16_t* key)
{

}

uint64_t HashUserData(const char* name)
{

}

BindingTable* InitBindingTable(uint64_t capacity)
{
    BindingTable* Table = malloc(sizeof(BindingTable));

    Table->capacity = capacity;
    Table->count = 0;
    Table->chain = NULL;
    Table->entries = malloc(sizeof(BindingEntry) * Table->capacity);

    return Table;
}

uint16_t GetBindingKeyElement(BindingTable* Table, const char* name)
{
    uint64_t hash = HashBinding(name);
    uint64_t i = hash % Table->capacity;
    return Table->entries[i].keyElement;
}

void CopyBinding(BindingTable* src, BindingEntry* dst, uint64_t index)
{
    uint64_t hash = HashBinding(src->entries[index].name);
    uint64_t i = hash % src->capacity;
    if (!src->chain) {
        dst[i].name = src->entries[index].name;
        dst[i].keyElement = src->entries[index].keyElement;
    } else {
        
    }
}

BindingTable* GetChain(BindingTable* Table)
{
    return Table->chain;
}

void ResizeBindingTable(BindingTable* Table)
{
    BindingTable* list = InitBindingTable(Table->capacity * 4);
    if (!list) {
        fprintf(stderr, "ResizeBindingTable:\n");
        fprintf(stderr, "malloc failed!\n");
        exit(EXIT_FAILURE);
    }

    size_t index = 0;
    for (size_t i = 0; i < Table->count; i++) {
        if (!Table->entries[index++].name) continue;
        list->entries[list->count++].name = Table->entries[index++].name;
        list->entries[list->count++].keyElement = Table->entries[index++].keyElement;
    }

    BindingTable* Chain = NULL;
    while ((Chain = GetChain(Table))) {
        index = 0;
        for (size_t i = 0; i < Chain->count; i++) {
            list->entries[list->count++].name = Chain->entries[index++].name;
            list->entries[list->count++].keyElement = Chain->entries[index++].keyElement;
        }
    }

    uint64_t newCap = list->count * 1.5 * 1.5;

    BindingEntry* tmp = malloc(sizeof(BindingEntry) * newCap);
    if (!tmp) {
        fprintf(stderr, "ResizeBindingTable:\n");
        fprintf(stderr, "malloc failed!\n");
        exit(EXIT_FAILURE);
    }

    index = 0;
    for (size_t i = 0; list->entries[index].name; i++) {
        tmp[i].name = list->entries[index].name;
        tmp[i].keyElement = list->entries[index].keyElement;
    }
    free(list->entries);
    free(list);

    Table->capacity = newCap;
    Table->entries = realloc(Table->entries, sizeof(BindingEntry) * Table->capacity);
    if (!Table->entries) {
        fprintf(stderr, "ResizeBindingTable:\n");
        fprintf(stderr, "realloc failed!\n");
        exit(EXIT_FAILURE);
    }
    
    mempcpy(Table->entries, tmp, sizeof(BindingEntry) * Table->capacity);
    free(tmp);
}

void AddBinding_(BindingTable* Table, const char* name, uint16_t keyElement)
{
    uint64_t hash = HashBinding(name);
    uint64_t i = hash % Table->capacity;
    if (Table->entries[i].name) {
        if (Table->chain) {
            AddBinding_(Table->chain, name, keyElement);
        } else {
            Table->chain = InitBindingTable();
            AddBinding_(Table->chain, name, keyElement);
        }
    } else {
        Table->entries[i].name = name;
        Table->entries[i].keyElement = keyElement;
    }
    Table->count++;
    if (Table->count >= (Table->capacity * 0.75)) ResizeBindingTable(Table);
    // Consider resizing ActiveBindings here if still used
    // Data->ActiveBindings = realloc(Data->ActiveBindings, sizeof(uint16_t*) * Data->BindingData.capacity);
}

void CopyAction(ActionTable* src, ActionEntry* dst, uint64_t index)
{
    uint64_t hash = HashBinding(src->entries[index].name);
    uint64_t i = hash % src->capacity;
    dst[i].name = src->entries[index].name;
    dst[i].keyElement = src->entries[index].keyElement;
}

void ResizeActionTable(ActionTable* Table)
{
    ActionEntry* tmp = malloc(sizeof(ActionEntry) * Table->capacity * 1.5);
    if (!tmp) {
        fprintf(stderr, "ResizeActionTable:\n");
        fprintf(stderr, "malloc failed!\n");
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < (uint64_t)(Table->capacity * 1.5); i++) {
        tmp[i].key = NULL;
        tmp[i].ptr = NULL;
    }
    for (size_t i = 0; i < Table->capacity; i++) {
        if (!Table->entries[i].key) continue;
        CopyAction(Table, tmp, i);
    }
    Table->capacity *= 1.5;
    Table->entries = realloc(Table->entries, sizeof(ActionEntry) * Table->capacity);
    if (!Table->entries) {
        fprintf(stderr, "ResizeActionTable:\n");
        fprintf(stderr, "realloc failed!\n");
        exit(EXIT_FAILURE);
    }
    mempcpy(Table->entries, tmp, sizeof(ActionEntry) * Table->capacity);
    free(tmp);
}



//

uint16_t* ConvertBinding(aoiData* Data, Binding* binding)
{
    uint16_t* b_list = calloc(Data->BindingData.capacity, sizeof(uint16_t));

    for (size_t i = 0; i < Data->BindingData.capacity; i++) {
        if (!binding[i].name) break;
        // printf("Name: %s\n", binding[i].name);
        // printf("Value: %u\n", binding[i].value);
        uint64_t hash = Data->BindingData.HashFunction(binding[i].name);
        uint64_t index = hash % Data->BindingData.capacity;
        // printf(" index: %zu\n\n", index);
        b_list[index] = binding[i].value;
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

    uint16_t* b_list = ConvertBinding(Data, binding);

    for (uint16_t i = 0; i < Data->BindingData.capacity; i++) {
        printf("%u ", b_list[i]);
    }
    printf("\n\n");

    uint64_t hash = Data->ActionData.HashFunction(b_list);
    uint64_t index = hash % Data->ActionData.capacity;

    // printf("Blist index: %zu\n\n", index);
    if (!Data->ActionData.items[index].key) {
        Data->ActionData.items[index].key = b_list;
        Data->ActionData.items[index].value = action;
    } else {

    }

    Data->ActionData.count++;
}

void SetAction_(aoiData* Data, Action* action, Binding* binding)
{
    uint16_t* b_list = ConvertBinding(Data, binding);

    for (uint16_t i = 0; i < Data->BindingData.capacity; i++) {
        printf("%u ", b_list[i]);
    }
    printf("\n\n");

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
    uint16_t* b_list = malloc(sizeof(uint16_t) * Data->BindingData.capacity);
    memset(b_list, 0, sizeof(uint16_t) * Data->BindingData.capacity);

    for (size_t i = 0; i < Data->BindingData.capacity; i++) {
        if (!Data->ActiveBindings[i]) continue;
        b_list[i] = *Data->ActiveBindings[i];
    }

    uint64_t hash = Data->ActionData.HashFunction(b_list);
    uint64_t index = hash % Data->ActionData.capacity;
    free(b_list);
    // printf("Key: %s\n", (char*)Data->ActionData.items[index].key);
    // printf("Value: %p\n", Data->ActionData.items[index].value);
    return Data->ActionData.items[index].value;
}

//

void ActionHandler(aoiData* Data)
{
    Action* a = GetActionFromCurrentBindings(Data);
    printf("Action: %s\n", a->name);
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
    static size_t err_count = 1;
    
    uint64_t hash = Data->UserData.HashFunction(name);
    uint64_t index = hash % Data->UserData.capacity;

    if (index > Data->UserData.capacity) {
        fprintf(stderr, "err count: %zu - ", err_count++);
        fprintf(stderr, "index out of bounds!\n\n");
        return NULL;
    }
    
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