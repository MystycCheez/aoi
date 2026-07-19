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
        list->entries[list->count++].ptr = Table->entries[index++].ptr;
    }

    ActionTable* Chain = NULL;
    while ((Chain = GetActionChain(Table))) {
        index = 0;
        for (size_t i = 0; i < Chain->count; i++) {
            list->entries[list->count++].key = Chain->entries[index++].key;
            list->entries[list->count++].ptr = Chain->entries[index++].ptr;
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
        tmp[i].ptr = list->entries[index].ptr;
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

void AddAction_(ActionTable* Table, const uint16_t* key, void* ptr)
{
    uint64_t hash = HashAction(key);
    uint64_t i = hash % Table->capacity;
    if (Table->entries[i].key) {
        if (Table->chain) {
            AddAction_(Table->chain, key, ptr);
        } else {
            Table->chain = InitActionTable(DEFAULT_CAPACITY);
            AddAction_(Table->chain, key, ptr);
        }
    } else {
        Table->entries[i].key = key;
        Table->entries[i].ptr = ptr;
    }
    Table->count++;
    if (Table->count >= (Table->capacity * 0.75)) ResizeActionTable(Table);
}

void AddActionWithStruct(ActionTable* Table, ActionEntry* entry)
{
    AddAction_(Table, entry->key, entry->ptr);
}

