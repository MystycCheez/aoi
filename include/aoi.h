/*

TODO: Put introduction here

*/

#ifndef AOI_BASE_H
#define AOI_BASE_H

// Taken from: https://www.youtube.com/watch?v=95M6V3mZgrI
#define da_append(da, item)\
    do {\
        if (da.count >= da.capacity) {\
            da.capacity *= 1.5;\
            da.items = realloc(da.items, da.capacity * sizeof(*da.items));\
        }\
        da.items[da.count++] = item;\
    } while(0)

typedef struct DA {
    void** items;
    unsigned long count;
    unsigned long capacity;
} DA;

typedef struct aoiData aoiData;

// NOTE: When creating the list of actions, make the list const
// Make a way for the user to be unable to change the members, at least
typedef struct Action {
    void (*const action)(aoiData* Data);
    const char* name;
    const char* desc;
} Action;

typedef struct ActionInfo {
    int key;
    int modifier;
    int mouseButton;
    int mouseDrag;
    int scope;
} ActionInfo;

typedef struct aoiData {
    ActionInfo InfoCounts;
    ActionInfo ActionData;
    DA RegisteredActions;
    DA UserData;
    Action** ActionTable;
} aoiData;

Action* newAction(aoiData* Data, void (action)(aoiData*), const char* name, const char* desc);
unsigned int getIndexFromStruct(aoiData* Data, struct ActionInfo ActionData);
void addActionWithStruct(aoiData* Data, Action* action, struct ActionInfo ActionData);
Action* getActionByStruct(aoiData* Data, struct ActionInfo ActionData);
Action* getActionByName(aoiData* Data, const char* str);
void setActionByStruct(aoiData* Data, Action* action, struct ActionInfo ActionData);

aoiData* aoi_Init();
void ActionHandlerFromStruct(aoiData* Data, ActionInfo ActionData);
void ActionHandler(aoiData* Data);

// Taken from https://x.com/vkrajacic/status/1749816169736073295
#define addAction(Data, Action, ...) \
    addActionWithStruct((Data), (Action), (ActionInfo){__VA_ARGS__})

#endif