#ifndef LIST_H
#define LIST_H

#include <linux/limits.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>

struct file_metadata {
    char    name[NAME_MAX];
    ino_t   serial_num;
};

typedef struct dirent_lst {
    struct file_metadata** lst;
    int capacity;
    int size;
} dirent_lst;

dirent_lst * create_dirent_lst();
int append(dirent_lst *, struct dirent *);

#endif
