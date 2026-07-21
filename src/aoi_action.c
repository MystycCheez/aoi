#include "aoi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

uint64_t HashAction(const char* name)
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
        if (!Table->entries[index++].pattern) continue;
        list->entries[list->count++].pattern = Table->entries[index++].pattern;
        list->entries[list->count++].action = Table->entries[index++].action;
    }

    ActionTable* Chain = NULL;
    while ((Chain = GetActionChain(Table))) {
        index = 0;
        for (size_t i = 0; i < Chain->count; i++) {
            list->entries[list->count++].pattern = Chain->entries[index++].pattern;
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
    for (size_t i = 0; list->entries[index].pattern; i++) {
        tmp[i].pattern = list->entries[index].pattern;
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

bool DoesKeyMatchPattern(const uint16_t* key, const uint16_t* pattern, uint64_t len)
{
    size_t index = 0;
    while (index < len) {
        if (pattern[index] == UINT16_MAX) continue;
        if (key[index] != pattern[index]) return false;
    }
    return true;
}

ActionEntry* GetActionEntryFromPattern(ActionTable* Table, const uint16_t* key)
{
    for (size_t i = 0; i < Table->capacity; i++) {
        if (DoesKeyMatchPattern(key, Table->entries[i].pattern, Table->capacity)) return &Table->entries[i];
    }
    return NULL;
}

void AddActionFromPattern(ActionTable* Table, Action* action, const uint16_t* pattern)
{
    uint64_t hash = HashAction(action->name);
    uint64_t i = hash % Table->capacity;
    if (Table->entries[i].pattern) {
        if (Table->chain) {
            AddActionFromPattern(Table->chain, action, pattern);
        } else {
            Table->chain = InitActionTable(DEFAULT_CAPACITY);
            AddActionFromPattern(Table->chain, action, pattern);
        }
    } else {
        Table->entries[i].pattern = pattern;
        Table->entries[i].action = action;
    }
    Table->count++;
    if (Table->count >= (Table->capacity * 0.75)) ResizeActionTable(Table);
}

void AddActionFromEntry(ActionTable* Table, ActionEntry* entry)
{
    AddActionFromPattern(Table, entry->action, entry->pattern);
}

void AddActionFromBinding(ActionTable* Table, Action* action, BindingEntry* binding)
{
    uint16_t* pattern = ConvertBinding(binding);
    AddActionFromPattern(Table, action, pattern);
}

//

void SetActionFromKeyAction(ActionTable* Table, Action* action, const uint16_t* pattern)
{
    ActionEntry* ptr = GetActionEntryFromPattern(Table, pattern);
    if (!ptr) {
        fprintf(stderr, "Error: Action not found.\n");
        return;
    }
    ptr->action = action;
}

void SetActionFromBinding(ActionTable* Table, Action* action, BindingEntry* binding)
{
    uint16_t* pattern = ConvertBinding(binding);
    SetActionFromKeyAction(Table, action, pattern);
}

#define AddAction(Table, Action, ...) \
    AddActionWithBinding(Table, Action, (Binding[]){__VA_ARGS__, {NULL, 0}})