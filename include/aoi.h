/*

TODO: Put introduction here

*/

#ifndef AOI_H
#define AOI_H

#include <stdint.h>
#include <stdbool.h>

#define DEFAULT_CAPACITY 8

typedef struct aoiData aoiData;

enum Flags {
    AOI_STRICT,
    AOI_IGNORE,
    AOI_IGNORE_ALL_ELSE,
};

typedef struct BindingEntry {
    const char* name;
    uint16_t patternElement;
} BindingEntry;

typedef struct BindingTable {
    BindingEntry* entries;
    struct BindingTable* chain;
    uint64_t count;
    uint64_t capacity;
} BindingTable;

typedef struct ActionEntry {
    const uint16_t* pattern;
    struct Action* action;
} ActionEntry;

typedef struct ActionTable {
    ActionEntry* entries;
    struct ActionTable* chain;
    uint64_t count;
    uint64_t capacity;
} ActionTable;

typedef struct UserDataEntry {
    const char* name;
    void* ptr;
} UserDataEntry;

typedef struct UserDataTable {
    UserDataEntry* entries;
    struct UserDataTable* chain;
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

// aoi_binding.c
uint64_t HashBinding(const char* name);
BindingTable* InitBindingData(uint64_t capacity);
BindingTable* GetBindingChain(BindingTable* Table);
void ResizeBindingTable(BindingTable* Table);
void AddBinding(BindingTable* Table, const char* name, uint16_t patternElement);
uint16_t* ConvertBinding(BindingTable* Table, BindingEntry* binding);
void SetActiveBindings(aoiData* Data);
void ResetBindings(aoiData* Data);

// aoi_action.c
uint64_t HashAction(const char* name);
ActionTable* InitActionData(uint64_t capacity);
Action* NewAction(void (action)(aoiData*), const char* name, const char* desc);
ActionTable* InitActionTable(uint64_t capacity);
ActionTable* GetActionChain(ActionTable* Table);
void ResizeActionTable(ActionTable* Table);
bool DoesKeyMatchPattern(const uint16_t* key, const uint16_t* pattern, uint64_t len);
ActionEntry* GetActionEntryFromPattern(ActionTable* Table, const uint16_t* pattern);
void AddActionFromPattern(ActionTable* Table, Action* action, const uint16_t* pattern);
void AddActionFromEntry(ActionTable* Table, ActionEntry* entry);
void AddActionFromBinding(aoiData* Data, Action* action, BindingEntry* binding);
void SetActionFromKeyAction(ActionTable* Table, Action* action, const uint16_t* pattern);
void SetActionFromBinding(aoiData* Data, Action* action, BindingEntry* binding);
void ActionHandler(aoiData* Data);

// aoi_userdata.c
uint64_t HashUserData(const char* name);
UserDataTable* InitUserData(uint64_t capacity);
void AddUserData(aoiData* Data, char* name, void* data);
UserDataTable* InitUserDataTable(uint64_t capacity);
UserDataTable* GetUserDataChain(UserDataTable* Table);
void ResizeUserDataTable(UserDataTable* Table);
void AddUserData_(UserDataTable* Table, const char* name, void* ptr);
void AddUserDataWithStruct(UserDataTable* Table, UserDataEntry* entry);
void* GetUserData(aoiData* Data, char* name);

//
aoiData* aoiInit(
    uint16_t UserDataCapacity, 
    uint16_t ActionDataCapacity, 
    uint16_t BindingCapacity
);
void aoiCleanup(aoiData* Data);

#define SetAction(aoiData, Action, ...) \
    SetAction_(aoiData, Action, (Binding[]){__VA_ARGS__, {NULL, 0}})

#endif