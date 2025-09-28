#include "list.h"
#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>


#include <signal.h>     // kill child signals
#include <unistd.h>     // fork
#include <sys/wait.h>   // wait children
#include <dirent.h>     // dir manager

// Error codes
#define OK                  0
#define PIPE_CREATE_ERR     1
#define FORK_ERR            2
#define DIR_OPEN_ERR        3
#define DIR_READ_ERR        4

void create_pipes(int**, int**);
void main_flow(int*, int*, char*);
int read_dir_file(char*, filename_lst*);

int main(int argc, char** argv) {
    int * pipe_c1_to_c2, * pipe_c2_to_c1;
    pid_t child1, child2;

    create_pipes(&pipe_c1_to_c2, &pipe_c2_to_c1);
    if (pipe_c1_to_c2 == NULL) {
        printf("Pipe creation failed\n");
        return PIPE_CREATE_ERR;
    }
    
    // Create child 1
    child1 = fork();
    if (child1 == -1) {
        printf("Child 1 fork failed\n");
        return FORK_ERR;
    }

    // Child 1 code
    if (child1 == 0) {
        main_flow(pipe_c1_to_c2, pipe_c2_to_c1, argv[1]);
        return 0;
    }

    // Continue parent code

    // Create child 2
    child2 = fork();
    if (child2 == -1) {
        printf("Child 2 fork failed\n");
        kill(child1, SIGSEGV); // SIGTERM and SIGKILL are boring. Debugging might be hell tho.
        return FORK_ERR;
    }
    
    // Child 2 code
    if (child2 == 0) {
        main_flow(pipe_c2_to_c1, pipe_c1_to_c2, argv[2]);
        return 0;
    }
    
    // wait both children, return
    wait(NULL);
    wait(NULL);
    return 0;
}

void create_pipes(int** p1, int** p2) {
    * p1 = malloc(sizeof(int) * 2);
    * p2 = malloc(sizeof(int) * 2);
    
    // Create pipe 1, if failed then
    // free memory, and set ptrs to NULL

    if (pipe(*p1) < 0) {
        free(*p1);
        free(*p2);
        *p1 = NULL;
        *p2 = NULL;
        return;
    }

    // Create pipe 2, if failed then close pipe 1,
    // free memory, and set ptrs to NULL

    if (pipe(*p2) < 0) {
        close((*p1)[0]);
        close((*p1)[1]);
        free(*p1);
        free(*p2);
        *p1 = NULL;
        *p2 = NULL;
        return;
    }
}

void main_flow(int* send_pipe, int* recv_pipe, char* dir) {
    // just to be safe
    close(send_pipe[1]);
    close(recv_pipe[0]);

    filename_lst * filenames = create_filename_lst();
    read_dir_file(dir, )
}

int read_dir_file(char* dir_name, filename_lst * filenames) {
    errno = 0;
    DIR * dir = opendir(dir_name);
    struct dirent * curr_file;
    while ((curr_file = readdir(dir)) != NULL) {
        append(filenames, curr_file);
        errno = 0;
    }
    if (errno != 0) errno = DIR_READ_ERR;
    else            errno = OK;
    return errno;
}
