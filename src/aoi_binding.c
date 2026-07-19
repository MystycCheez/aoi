#include "aoi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint64_t HashBinding(const char* name)
{

}

BindingTable* InitBindingTable(uint64_t capacity)
{
    BindingTable* Table = malloc(sizeof(BindingTable));

    Table->capacity = capacity;
    Table->count = 0;
    Table->chain = NULL;
    Table->entries = malloc(sizeof(BindingEntry) * Table->capacity);

    return Table;
}

BindingTable* GetBindingChain(BindingTable* Table)
{
    return Table->chain;
}

void ResizeBindingTable(BindingTable* Table)
{
    BindingTable* list = InitBindingTable(Table->capacity * 4);
    if (!list) {
        fprintf(stderr, "ResizeBindingTable:\n");
        fprintf(stderr, "malloc failed!\n");
        exit(EXIT_FAILURE);
    }

    size_t index = 0;
    for (size_t i = 0; i < Table->count; i++) {
        if (!Table->entries[index++].name) continue;
        list->entries[list->count++].name = Table->entries[index++].name;
        list->entries[list->count++].keyElement = Table->entries[index++].keyElement;
    }

    BindingTable* Chain = NULL;
    while ((Chain = GetBindingChain(Table))) {
        index = 0;
        for (size_t i = 0; i < Chain->count; i++) {
            list->entries[list->count++].name = Chain->entries[index++].name;
            list->entries[list->count++].keyElement = Chain->entries[index++].keyElement;
        }
    }

    uint64_t newCap = list->count * 1.5 * 1.5;

    BindingEntry* tmp = malloc(sizeof(BindingEntry) * newCap);
    if (!tmp) {
        fprintf(stderr, "ResizeBindingTable:\n");
        fprintf(stderr, "malloc failed!\n");
        exit(EXIT_FAILURE);
    }

    index = 0;
    for (size_t i = 0; list->entries[index].name; i++) {
        tmp[i].name = list->entries[index].name;
        tmp[i].keyElement = list->entries[index].keyElement;
    }
    free(list->entries);
    free(list);

    Table->capacity = newCap;
    Table->entries = realloc(Table->entries, sizeof(BindingEntry) * Table->capacity);
    if (!Table->entries) {
        fprintf(stderr, "ResizeBindingTable:\n");
        fprintf(stderr, "realloc failed!\n");
        exit(EXIT_FAILURE);
    }

    mempcpy(Table->entries, tmp, sizeof(BindingEntry) * Table->capacity);
    free(tmp);
}

void AddBinding(BindingTable* Table, const char* name, uint16_t keyElement)
{
    uint64_t hash = HashBinding(name);
    uint64_t i = hash % Table->capacity;
    if (Table->entries[i].name) {
        if (Table->chain) {
            AddBinding(Table->chain, name, keyElement);
        } else {
            Table->chain = InitBindingTable(DEFAULT_CAPACITY);
            AddBinding(Table->chain, name, keyElement);
        }
    } else {
        Table->entries[i].name = name;
        Table->entries[i].keyElement = keyElement;
    }
    Table->count++;
    if (Table->count >= (Table->capacity * 0.75)) ResizeBindingTable(Table);
    // Consider resizing ActiveBindings here if still used
    // Data->ActiveBindings = realloc(Data->ActiveBindings, sizeof(uint16_t*) * Data->BindingData.capacity);
}

