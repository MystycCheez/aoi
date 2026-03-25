#include "aoi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Default action functions

static void A_DoNothing(aoiData* Data)
{
    (void)Data;
    return;
}

static void A_Exit(aoiData* Data)
{
    (void)Data;
    exit(EXIT_SUCCESS);
}

// Misc Functions

// Returns a pointer to the newly created action
Action* newAction(aoiData* Data, void (action)(aoiData*), const char* name, const char* desc)
{
    Action tmp = {.action = action, .name = name, .desc = desc};
    Action* a_ptr = malloc(sizeof(*a_ptr));

    if (a_ptr == NULL) {
        fprintf(stderr, "newAction\n");
        fprintf(stderr, "malloc\n");
        exit(EXIT_FAILURE);
    }

    memcpy(a_ptr, &tmp, sizeof(*a_ptr));

    da_append(Data->RegisteredActions, a_ptr);
    
    if (Data->RegisteredActions.items[Data->RegisteredActions.count - 1] == NULL) {
        fprintf(stderr, "newAction\n");
        fprintf(stderr, "da_append\n");
        exit(EXIT_FAILURE);
    }
    return a_ptr;
}

unsigned int getIndexFromStruct(aoiData* Data, ActionInfo ActionData)
{
    // v * (W * X * Y * Z) + w * (X * Y * Z) + x * (Y * Z) + y * Z + z

    // This shouldn't happen, but I'll keep it here incase it does
    if ((ActionData.key < 0) ||
        (ActionData.modifier < 0) ||
        (ActionData.mouseButton < 0) ||
        (ActionData.mouseDrag < 0) ||
        (ActionData.scope < 0)
    ) return 0;

    unsigned int index = 0;
    
    int keyCount = Data->InfoCounts.key;
    int modifierCount = Data->InfoCounts.modifier;
    int mouseButtonCount = Data->InfoCounts.mouseButton;
    int isMoving = Data->InfoCounts.mouseDrag;

    index = (unsigned int){
        (ActionData.scope * (keyCount * modifierCount * mouseButtonCount * isMoving)) +
        (ActionData.key * (modifierCount * mouseButtonCount * isMoving)) +
        (ActionData.modifier * (mouseButtonCount * isMoving)) +
        (ActionData.mouseButton * isMoving) +
        (ActionData.mouseDrag)
    };
    printf("%d\n", ActionData.key);
    printf("%d\n", ActionData.modifier);
    printf("%d\n", ActionData.mouseButton);
    printf("%d\n", ActionData.mouseDrag);
    printf("%d\n", ActionData.scope);
    return index;
}

void addActionWithStruct(aoiData* Data, Action* action, ActionInfo ActionData)
{
    unsigned int index = getIndexFromStruct(Data, ActionData);
    printf("%s: %u\n", action->name, index);
    Data->ActionTable[index] = action;
}

Action* getActionFromStruct(aoiData* Data, ActionInfo ActionData)
{
    unsigned int index = getIndexFromStruct(Data, ActionData);
    printf("Index: %u\n", index);

    Action* action = Data->ActionTable[index];
    if (action == NULL) {
        return Data->ActionTable[0];
    }
    return action;
}

Action* getActionFromName(aoiData* Data, const char* str)
{
    unsigned int index = 0;
    
    DA* RegisteredActions = &Data->RegisteredActions;
    Action* a_ptr = RegisteredActions->items[index];

    while (a_ptr != NULL) {
        // printf("%s\n", a_ptr->name);
        if (strcmp(a_ptr->name, str) == 0) return a_ptr;
        a_ptr = RegisteredActions->items[++index];
    }
    fprintf(stderr, "Action not found\n");
    fprintf(stderr, "Returning \"Do Nothing\"\n");
    return RegisteredActions->items[0];
}

void setActionWithStruct(aoiData* Data, Action* action, ActionInfo ActionData)
{
    int index = getIndexFromStruct(Data, ActionData);
    Data->ActionTable[index] = action;
}

//

aoiData* aoi_Init()
{
    aoiData* Data = malloc(sizeof(aoiData));

    Data->RegisteredActions.count = 0;
    Data->RegisteredActions.capacity = 4;
    Data->RegisteredActions.items = malloc(sizeof(Action*) * Data->RegisteredActions.capacity);

    Data->UserData.count = 0;
    Data->UserData.capacity = 4;
    Data->UserData.items = malloc(sizeof(void*) * Data->UserData.capacity);

    Data->InfoCounts = (ActionInfo){.key = 260, .modifier = 8, .mouseButton = 8, .scope = 2, .mouseDrag = 2};

    Data->ActionData = (ActionInfo){0};

    unsigned int size = Data->InfoCounts.key * Data->InfoCounts.modifier * Data->InfoCounts.mouseButton * Data->InfoCounts.mouseDrag;

    Data->ActionTable = calloc(sizeof(Action*), size);
    if (Data->ActionTable == NULL) {
        fprintf(stderr, "ActionTable malloc\n");
        exit(EXIT_FAILURE);
    }

    Action* DO_NOTHING = newAction(
        Data,
        A_DoNothing,
        "Do Nothing",
        "Do nothing"
    );
    addAction(Data, DO_NOTHING);

    newAction(
        Data,
        A_Exit,
        "Exit",
        "Exit program"
    );

    return Data;
}

void ActionHandlerFromStruct(aoiData* Data, ActionInfo ActionData)
{
    Action* action = getActionFromStruct(Data, ActionData);
    action->action(Data);
}

void ActionHandler(aoiData *Data)
{
    ActionHandlerFromStruct(Data, Data->ActionData);
}