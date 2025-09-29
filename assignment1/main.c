#include "list.h"

#include <string.h>
#include <errno.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#include <signal.h>     // kill child signals
#include <unistd.h>     // fork
#include <sys/wait.h>   // wait children
#include <dirent.h>     // dir manager
#include <fcntl.h>      // open files

#define FIRST_CHILD         0
#define SECOND_CHILD        1

// Error codes
#define OK                  0
#define PIPE_CREATE_ERR     1
#define FORK_ERR            2
#define DIR_OPEN_ERR        3
#define DIR_READ_ERR        4
#define FILENAME_SIZE       NAME_MAX + 1

void create_pipes(int**, int**);
void child_main_flow(int*, int*, char*, int);
void read_dir_file(char*, filename_lst*);
void write_dir_to_pipe(filename_lst *, int *);
void read_dir_from_pipe(filename_lst *, int *);
void write_file_content_to_pipe(char*, int*);
void read_file_content_from_pipe(char *, int *); 

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
        child_main_flow(pipe_c1_to_c2, pipe_c2_to_c1, argv[1], FIRST_CHILD);
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
        child_main_flow(pipe_c2_to_c1, pipe_c1_to_c2, argv[2], SECOND_CHILD);
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

void child_main_flow(int* send_pipe, int* recv_pipe, char* dir, int child_id) {
    // just to be safe
    close(send_pipe[0]);
    close(recv_pipe[1]);

    filename_lst * this_filenames = create_filename_lst();
    filename_lst * other_filenames = create_filename_lst();
    read_dir_file(dir, this_filenames);

    if (child_id == FIRST_CHILD) {
        write_dir_to_pipe(this_filenames, send_pipe);
        read_dir_from_pipe(other_filenames, recv_pipe);
        for (int i = 0; i < this_filenames->size; i++) {
            char file_path[PATH_MAX + 1];
            snprintf(file_path, sizeof(file_path), "%s/%s", dir, this_filenames->lst[i]->file_name);
            write_file_content_to_pipe(file_path, send_pipe);
        }
        for (int i = 0; i < other_filenames->size; i++) {
            char file_path[PATH_MAX + 1];
            snprintf(file_path, sizeof(file_path), "%s/%s", dir, other_filenames->lst[i]->file_name);
            read_file_content_from_pipe(file_path, recv_pipe);
        }
    }
    else {
        read_dir_from_pipe(other_filenames, recv_pipe);
        write_dir_to_pipe(this_filenames, send_pipe);
        for (int i = 0; i < other_filenames->size; i++) {
            char file_path[PATH_MAX + 1];
            snprintf(file_path, sizeof(file_path), "%s/%s", dir, other_filenames->lst[i]->file_name);
            read_file_content_from_pipe(file_path, recv_pipe);
        }
        for (int i = 0; i < this_filenames->size; i++) {
            char file_path[PATH_MAX + 1];
            snprintf(file_path, sizeof(file_path), "%s/%s", dir, this_filenames->lst[i]->file_name);
            write_file_content_to_pipe(file_path, send_pipe);
        }
    }


}

void read_dir_file(char* dir_name, filename_lst * filenames) {
    errno = 0;
    DIR * dir = opendir(dir_name);
    struct dirent * curr_file;
    while ((curr_file = readdir(dir)) != NULL) {
        if (strcmp(curr_file->d_name, ".") == 0 || strcmp(curr_file->d_name, "..") == 0)
            continue;
        append_from_dirent(filenames, curr_file);
    }

    if (errno != 0)
        errno = DIR_READ_ERR;
    else
        errno = OK;
}

void write_dir_to_pipe(filename_lst * filenames, int * send_pipe) {
    write(send_pipe[1], &(filenames->size), sizeof(int));
    for (int i = 0; i < filenames->size; i++) {
        write(send_pipe[1], filenames->lst[i]->file_name, FILENAME_SIZE);
    }
}

void read_dir_from_pipe(filename_lst * filenames, int * recv_pipe) {
    // first 4 bytes: read amount of files.
    // the rest FILENAME_SIZE * #files are for filenames
    int file_count;
    char tmp[FILENAME_SIZE];
    read(recv_pipe[0], &file_count, sizeof(file_count));
    for (int i = 0; i < file_count; i++) {
        read(recv_pipe[0], tmp, FILENAME_SIZE);
        append(filenames, tmp);
    }
}

void write_entirely(int * send_pipe, void * buffer, int write_size) {
    int len_written = 0;
    while (len_written < write_size) {
        int curr_written = write(send_pipe[1], buffer + len_written, write_size - len_written);
        len_written += curr_written;
    }
}

void write_file_content_to_pipe(char* filename, int* send_pipe) {
    char msg[4096];
    uint32_t write_len;
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("nah\n");
        return;
    }

    do {
        uint32_t read_len = fread(msg, 1, sizeof(msg), file);
        write(send_pipe[1], &read_len, 4);
        if (read_len == 0)
            break;
        write_entirely(send_pipe, msg, read_len);
    } while (1);
}

void read_entirely(int * recv_pipe, void * buffer, int read_size) {
    int len_read = 0;
    while (len_read < read_size) {
        int curr_read = read(recv_pipe[0], buffer + len_read, read_size - len_read);
        len_read += curr_read;
    }
}

void read_file_content_from_pipe(char * filename, int * recv_pipe) {
    char msg[4096] = {0};
    uint32_t read_len;
    FILE* file = fopen(filename, "wb+");
    if (!file) {
        return;
    }

    do {
        read(recv_pipe[0], &read_len, sizeof(int));
        if (read_len == 0)
            break;
        read_entirely(recv_pipe, msg, read_len);
        fwrite(msg, 1, read_len, file);
    } while(1);
}

