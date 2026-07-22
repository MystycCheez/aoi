#include "aoi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BindingTable* InitBindingData(uint64_t capacity)
{
    BindingTable* Table = malloc(sizeof(BindingTable));

    Table->capacity = capacity;
    Table->count = 0;
    Table->chain = NULL;
    Table->entries = malloc(sizeof(BindingEntry) * Table->capacity);

    return Table;
}

BindingTable* GetBindingStructure(aoiData* Data)
{
    return &Data->BindingData;
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

void AddBinding(BindingTable* Table, const char* name)
{
    uint64_t hash = Hash(name);
    uint64_t i = hash % Table->capacity;
    if (Table->entries[i].name) {
        if (Table->chain) {
            AddBinding(Table->chain, name);
        } else {
            Table->chain = InitBindingData(DEFAULT_CAPACITY);
            AddBinding(Table->chain, name);
        }
    } else {
        Table->entries[i].name = name;
        Table->entries[i].patternElement = 0;
    }
    Table->count++;
    if (Table->count >= (Table->capacity * 0.75)) ResizeBindingTable(Table);
    // Consider resizing ActiveBindings here if still used
    // Data->ActiveBindings = realloc(Data->ActiveBindings, sizeof(uint16_t*) * Data->BindingData.capacity);
}

BindingEntry* GetBindingEntry(BindingTable* Table, char* name)
{
    static size_t err_count = 1;
    
    uint64_t hash = Hash(name);
    uint64_t index = hash % Table->capacity;

    if (index > Table->capacity) {
        fprintf(stderr, "err count: %zu - ", err_count++);
        fprintf(stderr, "index out of bounds!\n\n");
        return NULL;
    }
    BindingEntry* entry = &Table->entries[index];
    if (!entry) {
        fprintf(stderr, "Binding Entry not found.\n");
    }
    return entry;
}

uint16_t* ConvertBindingsToPattern(BindingTable* Table, BindingEntry binding[])
{
    uint16_t* pattern = calloc(Table->capacity, sizeof(uint16_t));

    for (size_t i = 0; i < Table->capacity; i++) {
        if (!binding[i].name) break;

        uint64_t hash = Hash(binding[i].name);
        uint64_t index = hash % Table->capacity;

        pattern[index] = binding[i].patternElement;
    }
    return pattern;
}

uint16_t* ConvertBindingsToFuzzyPattern(BindingTable* Table, BindingEntry binding[])
{
    uint16_t* pattern = calloc(Table->capacity, sizeof(uint16_t));
    memset(pattern, PATTERN_IGNORE, Table->capacity);

    for (size_t i = 0; i < Table->capacity; i++) {
        if (!binding[i].name) break;

        uint64_t hash = Hash(binding[i].name);
        uint64_t index = hash % Table->capacity;

        pattern[index] = binding[i].patternElement;
    }
    return pattern;
}

void SetBindings_(BindingTable* Table, BindingEntry binding[])
{
    uint16_t* pattern = ConvertBindingsToFuzzyPattern(Table, binding);
    for (size_t i = 0; i < Table->capacity; i++) {
        if (pattern[i] == PATTERN_IGNORE) continue;
        Table->entries[i].patternElement = pattern[i];
    }
}

void SetActiveBindings(aoiData *Data, BindingEntry* entries)
{
    for (size_t i = 0; i < Data->BindingData.capacity; i++) {
        Data->ActiveBindings[i] = &entries[i].patternElement;
    }
}

void ResetBindings(aoiData* Data)
{
    for (uint64_t i = 0; i < Data->BindingData.capacity; i++) {
        if (!Data->ActiveBindings[i]) continue;
        *Data->ActiveBindings[i] = 0;
    }
}