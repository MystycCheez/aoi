/*

TODO: Put introduction here

*/

#ifndef AOI_H
#define AOI_H

#include <stdint.h>
#include <stdbool.h>

#define DEFAULT_CAPACITY 8
#define PATTERN_IGNORE UINT16_MAX

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
    uint64_t patternLen;
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
    BindingTable* BindingData;
    ActionTable* ActionData;
    UserDataTable* UserData;
} aoiData;

void A_DoNothing(aoiData* Data);

// aoi_binding.c
BindingTable* InitBindingData(uint64_t capacity);
BindingTable* GetBindingStructure(aoiData* Data);
BindingTable* GetBindingChain(BindingTable* Table);
void ResizeBindingTable(aoiData* Data, BindingTable* Table);
void AddBinding(aoiData* Data, BindingTable* Table, const char* name);
BindingEntry* GetBindingEntry(BindingTable* Table, char* name);
uint16_t* ConvertBindingsToPattern(BindingTable* Table, BindingEntry binding[]);
uint16_t* ConvertBindingsToFuzzyPattern(BindingTable* Table, BindingEntry binding[]);
void SetBindings_(BindingTable* Table, BindingEntry binding[]);
void SetActiveBindings(aoiData *Data);
void ResetBindings(aoiData* Data);

// aoi_action.c
ActionTable* InitActionData(uint64_t capacity, uint64_t pattenLen);
ActionTable* GetActionStructure(aoiData* Data);
ActionTable* GetActionChain(ActionTable* Table);
Action* NewAction(void (action)(aoiData*), const char* name, const char* desc);
void ResizeActionTable(ActionTable* Table);
bool DoesKeyMatchPattern(const uint16_t* key, const uint16_t* pattern, uint64_t len);
ActionEntry* GetActionEntryFromPattern(ActionTable* Table, const uint16_t* pattern);
ActionEntry* GetActionEntryFromCurrentBindings(aoiData* Data);
void AddActionFromPattern(ActionTable* Table, Action* action, const uint16_t* pattern);
void AddActionFromEntry(ActionTable* Table, ActionEntry* entry);
void AddActionFromBinding(aoiData* Data, Action* action, BindingEntry* binding);
void SetActionFromKeyAction(ActionTable* Table, Action* action, const uint16_t* pattern);
void SetActionFromBinding(aoiData* Data, Action* action, BindingEntry* binding);
void ActionHandler(aoiData* Data);
ActionEntry* GetActionEntry(ActionTable* Table, char* name);

// aoi_userdata.c
UserDataTable* InitUserData(uint64_t capacity);
UserDataTable* GetUserDataStructure(aoiData* Data);
UserDataTable* GetUserDataChain(UserDataTable* Table);
void AddUserData(aoiData* Data, char* name, void* data);
void ResizeUserDataTable(UserDataTable* Table);
void AddUserData_(UserDataTable* Table, const char* name, void* ptr);
void AddUserDataWithStruct(UserDataTable* Table, UserDataEntry* entry);
UserDataEntry* GetUserDataEntry(UserDataTable* Table, char* name);

//
aoiData* aoiInit(
    uint64_t UserDataCapacity, 
    uint64_t ActionDataCapacity, 
    uint64_t BindingCapacity
);
void aoiCleanup(aoiData* Data);
uint64_t HashStr(const char* name);
uint64_t HashPattern(const uint16_t* pattern, uint64_t len);

#define AddAction(Table, Action, ...) \
    AddActionFromBinding(Table, Action, (BindingEntry[]){__VA_ARGS__, {NULL, 0}})

#define SetAction(aoiData, Action, ...) \
    SetActionFromBinding(aoiData, Action, (BindingEntry[]){__VA_ARGS__, {NULL, 0}})

#define SetBindings(Table, ...) \
    SetBindings_(Table, (BindingEntry[]){__VA_ARGS__, {NULL, 0}})

#endif