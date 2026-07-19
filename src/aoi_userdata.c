#include "aoi.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint64_t HashUserData(const char* name)
{

}

UserDataTable* InitUserDataTable(uint64_t capacity)
{
    UserDataTable* Table = malloc(sizeof(UserDataTable));

    Table->capacity = capacity;
    Table->count = 0;
    Table->chain = NULL;
    Table->entries = malloc(sizeof(UserDataEntry) * Table->capacity);

    return Table;
}

UserDataTable* GetUserDataChain(UserDataTable* Table)
{
    return Table->chain;
}

void ResizeUserDataTable(UserDataTable* Table)
{
    UserDataTable* list = InitUserDataTable(Table->capacity * 4);
    if (!list) {
        fprintf(stderr, "ResizeUserDataTable:\n");
        fprintf(stderr, "malloc failed!\n");
        exit(EXIT_FAILURE);
    }

    size_t index = 0;
    for (size_t i = 0; i < Table->count; i++) {
        if (!Table->entries[index++].name) continue;
        list->entries[list->count++].name = Table->entries[index++].name;
        list->entries[list->count++].ptr = Table->entries[index++].ptr;
    }

    UserDataTable* Chain = NULL;
    while ((Chain = GetUserDataChain(Table))) {
        index = 0;
        for (size_t i = 0; i < Chain->count; i++) {
            list->entries[list->count++].name = Chain->entries[index++].name;
            list->entries[list->count++].ptr = Chain->entries[index++].ptr;
        }
    }

    uint64_t newCap = list->count * 1.5 * 1.5;

    UserDataEntry* tmp = malloc(sizeof(UserDataEntry) * newCap);
    if (!tmp) {
        fprintf(stderr, "ResizeUserDataTable:\n");
        fprintf(stderr, "malloc failed!\n");
        exit(EXIT_FAILURE);
    }

    index = 0;
    for (size_t i = 0; list->entries[index].name; i++) {
        tmp[i].name = list->entries[index].name;
        tmp[i].ptr = list->entries[index].ptr;
    }
    free(list->entries);
    free(list);

    Table->capacity = newCap;
    Table->entries = realloc(Table->entries, sizeof(UserDataEntry) * Table->capacity);
    if (!Table->entries) {
        fprintf(stderr, "ResizeUserDataTable:\n");
        fprintf(stderr, "realloc failed!\n");
        exit(EXIT_FAILURE);
    }

    mempcpy(Table->entries, tmp, sizeof(UserDataEntry) * Table->capacity);
    free(tmp);
}

void AddUserData_(UserDataTable* Table, const char* name, void* ptr)
{
    uint64_t hash = HashUserData(name);
    uint64_t i = hash % Table->capacity;
    if (Table->entries[i].name) {
        if (Table->chain) {
            AddUserData_(Table->chain, name, ptr);
        } else {
            Table->chain = InitUserDataTable(DEFAULT_CAPACITY);
            AddUserData_(Table->chain, name, ptr);
        }
    } else {
        Table->entries[i].name = name;
        Table->entries[i].ptr = ptr;
    }
    Table->count++;
    if (Table->count >= (Table->capacity * 0.75)) ResizeUserDataTable(Table);
}

void AddUserDataWithStruct(UserDataTable* Table, UserDataEntry* entry)
{
    AddUserData_(Table, entry->name, entry->ptr);
}

