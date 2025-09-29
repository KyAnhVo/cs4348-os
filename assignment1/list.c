/**
 * Ky Anh Vo
 * CS4348_HON
 * Fall 2025
 */
#include <asm-generic/errno-base.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>

#include "list.h"

// create a list, let it have some capacity
filename_lst * create_filename_lst() {
    filename_lst * tmp = (filename_lst*) malloc(sizeof(filename_lst));
    tmp->size = 0;
    tmp->capacity = 8;
    tmp->lst = malloc(sizeof(filename*) * tmp->capacity);
    return tmp;
}

// the usual double cap when filled implementation of list, but from dirent
int append_from_dirent(filename_lst * lst, struct dirent * entry) {
    if (lst->capacity == lst->size) {
        lst->capacity *= 2;
        lst->lst = (filename**) realloc(lst->lst, sizeof(filename*) * lst->capacity);
    }

    // populate new filename
    filename * f = (filename*) malloc(sizeof(*f));
    f->file_name = (char*) malloc(NAME_MAX + 1);
    memcpy(f->file_name, entry->d_name, NAME_MAX + 1);
    
    // append filename in lst->lst
    lst->lst[lst->size] = f;
    lst->size++;

    return 0;
}

// this one uses name only
int append(filename_lst * lst, char* entry) {
    if (lst->capacity == lst->size) {
        lst->capacity *= 2;
        lst->lst = (filename**) realloc(lst->lst, sizeof(filename*) * lst->capacity);
    }
    
    // populate new filename
    filename * f = (filename*) malloc(sizeof(*f));
    f->file_name = (char*) malloc(NAME_MAX + 1);
    memcpy(f->file_name, entry, NAME_MAX + 1);

    // append filename in lst->lst
    lst->lst[lst->size] = f;
    lst->size++;

    return 0;
}

void free_filename_lst(filename_lst* lst) {
    for (int i = 0; i < lst->size; i++) {
        free(lst->lst[i]->file_name);
        free(lst->lst[i]);
    }
    free(lst->lst);
    free(lst);
}
