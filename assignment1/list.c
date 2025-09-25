#include <asm-generic/errno-base.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include "list.h"


dirent_lst * create_dirent_lst() {
    dirent_lst * lst;
    lst = (dirent_lst *) malloc(sizeof(dirent_lst));
    if (lst == NULL) {
        return NULL;
    }
    lst->capacity = 8;
    lst->size = 0;
    lst->lst = (struct dirent **) malloc(sizeof(struct dirent *));
    if (lst->lst == NULL) {
        free(lst);
        return NULL;
    }
    return lst;
}

int append(dirent_lst * lst, struct dirent * val) {
    struct dirent * tmp = malloc(sizeof(struct dirent));
    tmp->d_ino = val->d_ino;
    strcpy(val->d_name, tmp->d_name);
    if (lst->size < lst->capacity) {
        lst->lst[lst->size] = val;
        lst->size++;
        return 0;
    }
    struct dirent ** tmp_lst = realloc(lst->lst, lst->capacity * 2 * sizeof(struct dirent*));
    if (tmp_lst == NULL) {
        return ENOMEM;
    }
    lst->lst = tmp_lst;
    lst->lst[lst->size] = val;
    lst->size++;
    lst->capacity *= 2;
    return 0;
}
