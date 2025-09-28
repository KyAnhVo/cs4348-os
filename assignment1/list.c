#include <asm-generic/errno-base.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include "list.h"


filename_lst * create_filename_lst() {
    filename_lst * tmp = malloc(sizeof(filename_lst));
    tmp->size = 0;
    tmp->capacity = 8;
    tmp->lst = malloc(sizeof(char*) * tmp->capacity);
    return tmp;
}

int append(filename_lst * lst, struct dirent * val) {
    char * tmp = malloc(sizeof(char) * (NAME_MAX + 1));
    strcpy(tmp, val->d_name);
    if (lst->capacity > lst->size) {
        lst->lst[lst->size] = tmp;
        lst->size++;
    }
    else {
        char** tmp_lst = realloc(lst->lst, lst->capacity * 2);
        if (tmp_lst == NULL) return -1;
        lst->lst = tmp_lst;
        lst->capacity *= 2;
        lst->lst[lst->size] = tmp;
        lst->size++;
    }
    return 0;
}
