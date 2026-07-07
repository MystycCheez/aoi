/*

TODO: Put introduction here

*/

#ifndef AOI_H
#define AOI_H

typedef struct DA {
    void** items;
    unsigned long count;
    unsigned long capacity;
} DA;

typedef struct HashEntry {
    const char* key;
    void* value;
} HashEntry;

typedef struct HashData {
    HashEntry* items;
    unsigned long count;
    unsigned long capacity;
    unsigned long hash;
} HashData;

typedef struct aoiData aoiData;

typedef struct Action {
    void (*const action)(aoiData* Data);
    const char* name;
    const char* desc;
} Action;

typedef union ActionBinding {
    struct {
        char key;
        char modifier;
        char mouseButton;
        char mouseDrag;
        char scope;
    };
    char binding[5];
} ActionBinding;

typedef enum LogLevels {
    LOG_DEFAULT,
    LOG_DEBUG,
} LogLevels;

typedef struct aoiData {
    LogLevels LogLevel;
    ActionBinding ActiveBindings;
    HashData ActionData;
    HashData UserData;
    HashData Bindings;
    unsigned long(*ActionHashFunction)(char* name, unsigned long hash);
    unsigned long(*UserDataHashFunction)(char* name, unsigned long hash);
    unsigned long(*BindingsHashFunction)(char* name, unsigned long hash);
} aoiData;

void A_DoNothing(aoiData* Data);

void InitActionData(aoiData* Data, unsigned long capacity);
void SetActionHashFunction(aoiData* Data, unsigned long(*hash_fn)(char* name, unsigned long hash));
Action* NewAction(void (action)(aoiData*), const char* name, const char* desc);
void AddActionWithStruct(aoiData* Data, Action* action, ActionBinding Bindings);
Action* GetActionFromStruct(aoiData* Data, ActionBinding Bindings);
Action* GetActionFromName(aoiData* Data, const char* name);
void SetActionWithStruct(aoiData* Data, Action* action, ActionBinding Bindings);

void ActionHandlerFromStruct(aoiData* Data, ActionBinding Bindings);
void ActionHandler(aoiData* Data);

void InitUserData(aoiData* Data, unsigned long capacity, unsigned long initHash);
void SetUserDataHashFunction(aoiData* Data, unsigned long(*hash_fn)(char* name, unsigned long hash));
void AddUserData(aoiData* Data, char* name, void* data);
void* GetUserData(aoiData* Data, char* name);

aoiData* aoiInit(
    unsigned long hashSeed, 
    unsigned long UD_capacity, 
    unsigned long AD_capacity, 
    unsigned long(*ud_hash_fn)(char* name, unsigned long hash), 
    unsigned long(*ad_hash_fn)(char* name, unsigned long hash),
    unsigned long(*bn_hash_fn)(char* name, unsigned long hash)
);
void aoiCleanup(aoiData* Data);

// Taken from https://x.com/vkrajacic/status/1749816169736073295
#define aoi_Init(...) \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Winitializer-overrides\"") \
    aoi_InitWithStruct((ActionBinding){.key = 255, .modifier = 8, .mouseButton = 8, .scope = 2, .mouseDrag = 2, __VA_ARGS__}) \
    _Pragma("GCC diagnostic pop") 

#define AddAction(aoiData, Action, ...) \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Winitializer-overrides\"") \
    AddActionWithStruct((aoiData), (Action), (ActionBinding){__VA_ARGS__}) \
    _Pragma("GCC diagnostic pop") 

#define SetAction(aoiData, Action, ...) \
    SetActionWithStruct((aoiData), (Action), (ActionBinding){__VA_ARGS__})

#endif