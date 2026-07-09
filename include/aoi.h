/*

TODO: Put introduction here

*/

#ifndef AOI_H
#define AOI_H

#include <stdint.h>

typedef struct aoiData aoiData;

typedef struct KV_Pair {
    const void* key;
    void* value;
} KV_Pair;

typedef struct Binding {
    const char* name;
    uint16_t value;
} Binding;

typedef struct HashData {
    KV_Pair* items;
    uint16_t(*HashFunction)(void* key);
    uint16_t count;
    uint16_t capacity;
    uint16_t hash;
} HashData;

typedef struct Action {
    void (*const action)(aoiData*);
    const char* name;
    const char* desc;
} Action;

typedef struct aoiData {
    uint16_t* ActiveBindings;
    HashData ActionData;
    HashData UserData;
    HashData BindingData;
} aoiData;

void A_DoNothing(aoiData* Data);

void SetHashFunction(HashData* Table, uint16_t(*hash_fn)(void* key));
void ResizeHashTable(HashData* Table);

uint16_t* ConvertBinding(aoiData* Data, Binding* binding);
void InitBindings(aoiData* Data, uint16_t capacity);
void AddBinding(aoiData* Data, char* name);

void InitActionData(aoiData* Data, uint16_t capacity);
Action* NewAction(void (action)(aoiData*), const char* name, const char* desc);
void AddActionByStr(aoiData* Data, Action* action, uint16_t* binding);
void AddActionByBinding(aoiData* Data, Action* action, Binding* binding);
void AddAction_(aoiData* Data, Action* action, Binding binding[]);
void SetActionWithBinding(aoiData* Data, Action* action, Binding* binding);
Action* GetActionFromBinding(aoiData* Data, Binding* binding);
Action* GetActionFromStr(aoiData* Data, uint16_t* binding);
Action* GetActionFromName(aoiData* Data, const char* name);
Action* GetActionFromCurrentBindings(aoiData* Data);

void ActionHandlerBinding(aoiData* Data, Binding* binding);
void ActionHandlerStr(aoiData* Data, uint16_t* binding);
void ActionHandlerActiveBinding(aoiData* Data);

void InitUserData(aoiData* Data, uint16_t capacity);
void AddUserData(aoiData* Data, char* name, void* data);
void* GetUserData(aoiData* Data, char* name);

aoiData* aoiInit(
    uint16_t UserDataCapacity, 
    uint16_t ActionDataCapacity, 
    uint16_t BindingCapacity, 
    uint16_t(*UserDataHashFunction)(void* key), 
    uint16_t(*ActionDataHashFunction)(void* key),
    uint16_t(*BindingDataHashFunction)(void* key)
);
void aoiCleanup(aoiData* Data);

#define AddAction(aoiData, Action, ...) \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Winitializer-overrides\"") \
    AddAction_(aoiData, Action, (Binding[]){__VA_ARGS__, {NULL, 0}}) \
    _Pragma("GCC diagnostic pop") 

#endif