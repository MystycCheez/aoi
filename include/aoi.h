/*

TODO: Put introduction here

*/

#ifndef AOI_H
#define AOI_H

#include <stdint.h>

typedef struct aoiData aoiData;

enum Flags {
    AOI_STRICT,
    AOI_IGNORE,
    AOI_IGNORE_ALL_ELSE,
};

typedef struct BindingEntry {
    const char *name;
    uint16_t keyElement;
} BindingEntry;

typedef struct BindingTable {
    BindingEntry *pairs;
    struct BindingTable *chain;
    uint64_t count;
    uint64_t capacity;
} BindingTable;

typedef struct ActionEntry {
    const uint16_t *key;
    struct Action *ptr;
} ActionEntry;

typedef struct ActionTable {
    ActionEntry *pairs;
    struct ActionTable *chain;
    uint64_t count;
    uint64_t capacity;
} ActionTable;

typedef struct UserDataEntry {
    const char *name;
    void *ptr;
} UserDataEntry;

typedef struct UserDataTable {
    UserDataEntry *pairs;
    struct UserDataTable *chain;
    uint64_t count;
    uint64_t capacity;
} UserDataTable;

typedef struct Action {
    void (*const action)(aoiData*);
    const char* name;
    const char* desc;
} Action;

typedef struct aoiData {
    uint16_t** ActiveBindings;
    BindingTable BindingData;
    ActionTable ActionData;
    UserDataTable UserData;
} aoiData;

void A_DoNothing(aoiData* Data);

void SetHashFunction(HashData* Table, uint64_t(*hash_fn)(const void* key));
void ResizeHashTable(HashData* Table);

uint16_t* ConvertBinding(aoiData* Data, Binding* binding);
void InitBindingData(aoiData* Data, uint16_t capacity);
void AddBinding(aoiData* Data, char* name);
void SetBinding(aoiData* Data, Binding binding);
void SetActiveBindings(aoiData* Data);
void ResetBindings(aoiData* Data);

void InitActionData(aoiData* Data, uint16_t capacity);
Action* NewAction(void (action)(aoiData*), const char* name, const char* desc);
void AddAction_(aoiData* Data, Action* action, Binding* binding);
void SetAction_(aoiData* Data, Action* action, Binding* binding);
Action* GetAction_(aoiData* Data, Binding* binding);
Action* GetActionFromCurrentBindings(aoiData* Data);

void ActionHandler(aoiData* Data);

void InitUserData(aoiData* Data, uint16_t capacity);
void AddUserData(aoiData* Data, char* name, void* data);
void* GetUserData(aoiData* Data, char* name);

aoiData* aoiInit(
    uint16_t UserDataCapacity, 
    uint16_t ActionDataCapacity, 
    uint16_t BindingCapacity, 
    uint64_t(*UserDataHashFunction)(const void* key), 
    uint64_t(*ActionDataHashFunction)(const void* key),
    uint64_t(*BindingDataHashFunction)(const void* key)
);
void aoiCleanup(aoiData* Data);

#define AddAction(aoiData, Action, ...) \
    AddAction_(aoiData, Action, (Binding[]){__VA_ARGS__, {NULL, 0}})

#define SetAction(aoiData, Action, ...) \
    SetAction_(aoiData, Action, (Binding[]){__VA_ARGS__, {NULL, 0}})

#endif