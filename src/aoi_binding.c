#include "aoi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint64_t HashBinding(const char* name)
{

}

BindingTable* InitBindingData(uint64_t capacity)
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
    BindingTable* list = InitBindingData(Table->capacity * 4);
    if (!list) {
        fprintf(stderr, "ResizeBindingTable:\n");
        fprintf(stderr, "malloc failed!\n");
        exit(EXIT_FAILURE);
    }

    size_t index = 0;
    for (size_t i = 0; i < Table->count; i++) {
        if (!Table->entries[index++].name) continue;
        list->entries[list->count++].name = Table->entries[index++].name;
        list->entries[list->count++].patternElement = Table->entries[index++].patternElement;
    }

    BindingTable* Chain = NULL;
    while ((Chain = GetBindingChain(Table))) {
        index = 0;
        for (size_t i = 0; i < Chain->count; i++) {
            list->entries[list->count++].name = Chain->entries[index++].name;
            list->entries[list->count++].patternElement = Chain->entries[index++].patternElement;
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
        tmp[i].patternElement = list->entries[index].patternElement;
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

void AddBinding(BindingTable* Table, const char* name, uint16_t patternElement)
{
    uint64_t hash = HashBinding(name);
    uint64_t i = hash % Table->capacity;
    if (Table->entries[i].name) {
        if (Table->chain) {
            AddBinding(Table->chain, name, patternElement);
        } else {
            Table->chain = InitBindingData(DEFAULT_CAPACITY);
            AddBinding(Table->chain, name, patternElement);
        }
    } else {
        Table->entries[i].name = name;
        Table->entries[i].patternElement = patternElement;
    }
    Table->count++;
    if (Table->count >= (Table->capacity * 0.75)) ResizeBindingTable(Table);
    // Consider resizing ActiveBindings here if still used
    // Data->ActiveBindings = realloc(Data->ActiveBindings, sizeof(uint16_t*) * Data->BindingData.capacity);
}

uint16_t* ConvertBinding(BindingTable* Table, BindingEntry* binding)
{
    uint16_t* b_list = calloc(Table->capacity, sizeof(uint16_t));

    for (size_t i = 0; i < Table->capacity; i++) {
        if (!binding[i].name) break;

        uint64_t hash = HashBinding(binding[i].name);
        uint64_t index = hash % Table->capacity;

        b_list[index] = binding[i].patternElement;
    }
    return b_list;
}

void SetActiveBindings(aoiData* Data)
{
    for (size_t i = 0; i < Data->BindingData.capacity; i++) {
        Data->ActiveBindings[i] = NULL;
        if (!Data->BindingData.entries[i].name) continue;
        // printf("key: %s\n", (char*)Data->BindingData.items[i].key);
        // printf("key: %u\n", *(uint16_t*)Data->BindingData.items[i].value);
        uint64_t hash = HashBinding(Data->BindingData.entries[i].name);
        uint64_t index = hash % Data->BindingData.capacity;
        Data->ActiveBindings[index] = &Data->BindingData.entries[index].patternElement;
    }
}

void ResetBindings(aoiData* Data)
{
    for (uint64_t i = 0; i < Data->BindingData.capacity; i++) {
        if (!Data->ActiveBindings[i]) continue;
        *Data->ActiveBindings[i] = 0;
    }
}