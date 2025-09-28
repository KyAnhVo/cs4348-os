#ifndef LIST_H
#define LIST_H

#include <linux/limits.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>


typedef struct filename_lst {
    char ** lst;
    int capacity;
    int size;
} filename_lst;

filename_lst* create_filename_lst();
int append(filename_lst *, struct dirent *);

#endif
