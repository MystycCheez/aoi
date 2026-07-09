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

void SetHashFunction(HashData* Table, uint16_t(*hash_fn)(void* key))
{
    Table->HashFunction = hash_fn;
}

void ResizeHashTable(HashData* Table)
{
    KV_Pair* tmp = malloc(sizeof(KV_Pair) * Table->capacity * 1.5);
    for (size_t i = 0; i < Table->capacity; i++) tmp[i].key = NULL;

    uint16_t index;
    for (size_t i = 0; i < Table->capacity; i++) {
        if (!Table->items[i].key) continue;
        uint16_t hash = Table->HashFunction((uint16_t*)Table->items[i].key);
        index = hash % Table->capacity;
        tmp[index].key = Table->items[i].key;
        tmp[index].value = Table->items[i].value;
    }
    Table->capacity *= 1.5;
    Table->items = realloc(Table->items, sizeof(KV_Pair) * Table->capacity);
    if (!Table->items) {
        fprintf(stderr, "realloc failed!\n");
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < Table->capacity; i++) {
        Table->items[i].value = NULL;
        Table->items[i].key = NULL;
    }
    mempcpy(Table->items, tmp, Table->capacity);
    free(tmp);
}

//

uint16_t* ConvertBinding(aoiData* Data, Binding* binding)
{
    uint16_t* b_list = calloc(Data->BindingData.capacity + 1, 1);
    for (uint16_t i = 0; i < Data->BindingData.capacity; i++) {
        uint16_t hash = Data->BindingData.HashFunction((uint16_t*)binding[i].name);
        uint16_t index = hash % Data->BindingData.capacity;       
        b_list[index] = *(uint16_t*)Data->BindingData.items[index].value; 
    }
    return b_list;
}

void InitBindings(aoiData* Data, uint16_t capacity)
{
    if (!capacity) {
        fprintf(stderr, "Capacity must be greater than 0!\n");
        exit(EXIT_FAILURE);
    }
    Data->BindingData.capacity = capacity;
    Data->BindingData.count = 0;
    Data->BindingData.items = malloc(sizeof(KV_Pair) * capacity);
    for (size_t i = 0; i < capacity; i++) {
        Data->BindingData.items[i].value = NULL;
        Data->BindingData.items[i].key = NULL;
    }
}

void AddBinding(aoiData* Data, char* name)
{
    if (Data->BindingData.count > (Data->BindingData.capacity * 0.75)) ResizeHashTable(&Data->BindingData);

    uint16_t hash = Data->BindingData.HashFunction(name);
    uint16_t index = hash % Data->BindingData.capacity;

    Data->BindingData.items[index].key = name;
    Data->BindingData.items[index].value = 0;

    Data->BindingData.count++;
}

// Action creation and modification functions

void InitActionData(aoiData* Data, uint16_t capacity)
{
    Data->ActionData.hash = 0; 
    Data->ActionData.count = 0;
    Data->ActionData.capacity = capacity;
    Data->ActionData.items = malloc(sizeof(KV_Pair) * capacity);
    for (size_t i = 0; i < capacity; i++) {
        Data->ActionData.items[i].value = NULL;
        Data->ActionData.items[i].key = NULL;
    }
}

Action* NewAction(void (action)(aoiData*), const char* name, const char* desc)
{
    Action* a = malloc(sizeof(Action));
    Action tmp = {action, name, desc};
    memcpy(a, &tmp, sizeof(Action));

    return a;
}

void AddActionByStr(aoiData* Data, Action* action, uint16_t* binding)
{
    if (Data->ActionData.count > (Data->ActionData.capacity * 0.75)) ResizeHashTable(&(Data->ActionData));

    uint16_t hash = Data->ActionData.HashFunction(binding);
    uint16_t index = hash % Data->ActionData.capacity;

    Data->ActionData.items[index].key = binding;
    Data->ActionData.items[index].value = action;

    Data->ActionData.count++;
}

void AddActionByBinding(aoiData* Data, Action* action, Binding* binding)
{
    if (Data->ActionData.count > (Data->ActionData.capacity * 0.75)) ResizeHashTable(&(Data->ActionData));

    uint16_t* b_list = ConvertBinding(Data, binding);

    uint16_t hash = Data->ActionData.HashFunction(b_list);
    uint16_t index = hash % Data->ActionData.capacity;

    Data->ActionData.items[index].key = b_list;
    Data->ActionData.items[index].value = action;

    Data->ActionData.count++;
}

void AddAction_(aoiData* Data, Action* action, Binding binding[])
{
    if (Data->ActionData.count > (Data->ActionData.capacity * 0.75)) ResizeHashTable(&(Data->ActionData));

    uint16_t* b_list = ConvertBinding(Data, binding);

    uint16_t hash = Data->ActionData.HashFunction(b_list);
    uint16_t index = hash % Data->ActionData.capacity;

    Data->ActionData.items[index].key = b_list;
    Data->ActionData.items[index].value = action;

    Data->ActionData.count++;
}

void SetActionWithBinding(aoiData* Data, Action* action, Binding* binding)
{
    uint16_t* b_list = ConvertBinding(Data, binding);

    uint16_t hash = Data->ActionData.HashFunction(b_list);
    uint16_t index = hash % Data->ActionData.capacity;
    Data->ActionData.items[index].value = action;   
}

Action* GetActionFromBinding(aoiData* Data, Binding* binding)
{
    uint16_t* b_list = ConvertBinding(Data, binding);

    uint16_t hash = Data->ActionData.HashFunction(b_list);
    uint16_t index = hash % Data->ActionData.capacity;
    if (!Data->ActionData.items[index].value) return GetActionFromName(Data, "Do Nothing");
    return Data->ActionData.items[index].value;
}

Action* GetActionFromStr(aoiData* Data, uint16_t* binding)
{
    uint16_t hash = Data->ActionData.HashFunction(binding);
    uint16_t index = hash % Data->ActionData.capacity;
    if (!Data->ActionData.items[index].value) return GetActionFromName(Data, "Do Nothing");
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

Action* GetActionFromCurrentBindings(aoiData* Data)
{
    uint16_t hash = Data->ActionData.HashFunction(Data->ActiveBindings);
    uint16_t index = hash % Data->ActionData.capacity;
    return Data->ActionData.items[index].value;
}

//

void ActionHandlerBinding(aoiData* Data, Binding* binding)
{
    Action* a = GetActionFromBinding(Data, binding);
    if (!a->action) A_DoNothing(Data); else a->action(Data);
}

void ActionHandlerStr(aoiData* Data, uint16_t* binding)
{
    Action* a = GetActionFromStr(Data, binding);
    if (!a->action) A_DoNothing(Data); else a->action(Data);
}

void ActionHandlerActiveBinding(aoiData* Data)
{
    ActionHandlerStr(Data, Data->ActiveBindings);
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
    if (Data->UserData.count > (Data->UserData.capacity * 0.75)) {
        Data->UserData.capacity *= 1.5;
        Data->UserData.items = realloc(Data->UserData.items, sizeof(KV_Pair) * Data->UserData.capacity);
    }

    uint16_t hash = Data->UserData.HashFunction(name);
    uint16_t index = hash % Data->UserData.capacity;

    Data->UserData.items[index].key = strdup(name);
    Data->UserData.items[index].value = data;
    Data->UserData.count++;
}

void* GetUserData(aoiData* Data, char* name)
{
    uint16_t hash = Data->UserData.HashFunction(name);
    uint16_t index = hash % Data->UserData.capacity;
    return Data->UserData.items[index].value;
}

//

aoiData* aoiInit(
    uint16_t UserDataCapacity, 
    uint16_t ActionDataCapacity, 
    uint16_t BindingCapacity, 
    uint16_t(*UserDataHashFunction)(void* key), 
    uint16_t(*ActionDataHashFunction)(void* key),
    uint16_t(*BindingDataHashFunction)(void* key)
)
{
    aoiData* Data = malloc(sizeof(aoiData));
    if (!Data) return NULL;
    InitActionData(Data, ActionDataCapacity);
    InitUserData(Data, UserDataCapacity);
    InitBindings(Data, BindingCapacity);

    SetHashFunction(&Data->ActionData, ActionDataHashFunction);
    SetHashFunction(&Data->UserData, UserDataHashFunction);
    SetHashFunction(&Data->BindingData, BindingDataHashFunction);

    uint16_t* adn = calloc(1, sizeof(BindingCapacity));
    Action* A_DONOTHING = NewAction(A_DoNothing, "Do Nothing", "Do nothing");
    AddActionByStr(Data, A_DONOTHING, adn);

    Action* A_TESTACTION = NULL;
    Binding at[] = {
        {"Key", 34},
        {"State", 2}
    }; 

    AddActionByBinding(Data, A_TESTACTION, at);

    AddAction(Data, A_TESTACTION, 
        {"Key", 34},
        {"State", 2}
    );

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