#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>
#include "functions.h"


int read_from_pipe(int fd, char *buffer, size_t size);
void process_file(const char *filename, char *result, int *result_length);

void child_process(int pipe_fd[], int results_pipe_fd[]) 
{
    char buffer[1024];
    char result[1024 * 10];
    int result_length = 0;

    close(pipe_fd[1]);
    close(results_pipe_fd[0]);

    while (read_from_pipe(pipe_fd[0], buffer, sizeof(buffer)) > 0) 
    {
        process_file(buffer, result, &result_length);
    }

    write(results_pipe_fd[1], result, result_length);
    close(pipe_fd[0]);
    close(results_pipe_fd[1]);
    exit(EXIT_SUCCESS);
}


void parent_process(int pipe_fd[], int results_pipe_fd[]) 
{
    close(pipe_fd[0]);  // Close the read-end of the pipe in the parent
    close(results_pipe_fd[1]); // Close the write-end of the results pipe in the parent

    DIR *dir;
    if ((dir = opendir(".")) == NULL) 
    {
        perror("Failed to open directory");
        exit(EXIT_FAILURE);
    }

    send_filenames(dir, pipe_fd[1]);
    closedir(dir);

    read_results_and_write(open("result.txt", O_WRONLY | O_CREAT | O_APPEND, 0666), results_pipe_fd);

    wait(NULL);
    close(results_pipe_fd[0]);
}
