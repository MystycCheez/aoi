#include "aoi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint64_t HashAction(const uint16_t* key)
{

}

ActionTable* InitActionTable(uint64_t capacity)
{
    ActionTable* Table = malloc(sizeof(ActionTable));

    Table->capacity = capacity;
    Table->count = 0;
    Table->chain = NULL;
    Table->entries = malloc(sizeof(ActionEntry) * Table->capacity);

    return Table;
}

ActionTable* GetActionChain(ActionTable* Table)
{
    return Table->chain;
}

void ResizeActionTable(ActionTable* Table)
{
    ActionTable* list = InitActionTable(Table->capacity * 4);
    if (!list) {
        fprintf(stderr, "ResizeActionTable:\n");
        fprintf(stderr, "malloc failed!\n");
        exit(EXIT_FAILURE);
    }

    size_t index = 0;
    for (size_t i = 0; i < Table->count; i++) {
        if (!Table->entries[index++].key) continue;
        list->entries[list->count++].key = Table->entries[index++].key;
        list->entries[list->count++].action = Table->entries[index++].action;
    }

    ActionTable* Chain = NULL;
    while ((Chain = GetActionChain(Table))) {
        index = 0;
        for (size_t i = 0; i < Chain->count; i++) {
            list->entries[list->count++].key = Chain->entries[index++].key;
            list->entries[list->count++].action = Chain->entries[index++].action;
        }
    }

    uint64_t newCap = list->count * 1.5 * 1.5;

    ActionEntry* tmp = malloc(sizeof(ActionEntry) * newCap);
    if (!tmp) {
        fprintf(stderr, "ResizeActionTable:\n");
        fprintf(stderr, "malloc failed!\n");
        exit(EXIT_FAILURE);
    }

    index = 0;
    for (size_t i = 0; list->entries[index].key; i++) {
        tmp[i].key = list->entries[index].key;
        tmp[i].action = list->entries[index].action;
    }
    free(list->entries);
    free(list);

    Table->capacity = newCap;
    Table->entries = realloc(Table->entries, sizeof(ActionEntry) * Table->capacity);
    if (!Table->entries) {
        fprintf(stderr, "ResizeActionTable:\n");
        fprintf(stderr, "realloc failed!\n");
        exit(EXIT_FAILURE);
    }

    mempcpy(Table->entries, tmp, sizeof(ActionEntry) * Table->capacity);
    free(tmp);
}

void AddActionFromKey(ActionTable* Table, Action* action, const uint16_t* key)
{
    uint64_t hash = HashAction(key);
    uint64_t i = hash % Table->capacity;
    if (Table->entries[i].key) {
        if (Table->chain) {
            AddActionFromKey(Table->chain, action, key);
        } else {
            Table->chain = InitActionTable(DEFAULT_CAPACITY);
            AddActionFromKey(Table->chain, action, key);
        }
    } else {
        Table->entries[i].key = key;
        Table->entries[i].action = action;
    }
    Table->count++;
    if (Table->count >= (Table->capacity * 0.75)) ResizeActionTable(Table);
}

void AddActionFromEntry(ActionTable* Table, ActionEntry* entry)
{
    AddActionFromKey(Table, entry->action, entry->key);
}

void AddActionFromBinding(ActionTable* Table, Action* action, BindingEntry* binding)
{
    uint16_t* b_list = ConvertBinding(binding);
    AddActionFromKey(Table, action, b_list);
}

//

ActionEntry* GetActionEntryFromKey(ActionTable* Table, const uint16_t* key)
{
    uint64_t hash = HashAction(key);
    uint64_t i = hash % Table->capacity;
    
    if (Table->entries[i].key) return &Table->entries[i];
    ActionTable* tmp;
    while ((tmp = GetActionChain(Table))) {
        uint64_t hash = HashAction(key);
        uint64_t i = hash % tmp->capacity;
        if (tmp->entries[i].key) return &tmp->entries[i];
    }
    fprintf(stderr, "Error: Action not found.\n");
    return NULL;
}

//

void SetActionFromKeyAction(ActionTable* Table, Action* action, const uint16_t* key)
{
    ActionEntry* ptr = GetActionEntryFromKey(Table, key);
    if (!ptr) {
        fprintf(stderr, "Error: Action not found.\n");
        return;
    }
    ptr->action = action;
}

void SetActionFromBinding(ActionTable* Table, Action* action, BindingEntry* binding)
{
    uint16_t* b_list = ConvertBinding(binding);
}

#define AddAction(Table, Action, ...) \
    AddActionWithBinding(Table, Action, (Binding[]){__VA_ARGS__, {NULL, 0}})