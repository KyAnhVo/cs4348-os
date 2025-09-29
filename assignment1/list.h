/**
 * Ky Anh Vo
 * CS4348_HON
 * Fall 2025
 */
#ifndef LIST_H
#define LIST_H

#include <linux/limits.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>

// essentially a vector, arraylist, ...

typedef struct filename {
    char * file_name;
} filename;

typedef struct filename_lst {
    filename** lst;
    int capacity;
    int size;
} filename_lst;

filename_lst* create_filename_lst();
int append_from_dirent(filename_lst *, struct dirent *);
int append(filename_lst *, char *);
void free_filename_lst(filename_lst *);

#endif
