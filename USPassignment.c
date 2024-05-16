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
#include "processes.h"

#define MAX_FILES 100 // max number of .usp files to handle

int pipe_fd[2];  // File descriptors for the pipe
int results_pipe_fd[2]; // File descriptors for the pipe to receive results from child

void parent_process(int pipe_fd[], int results_pipe_fd[]);

int main(int argc, char *argv[]) 
{
    // Create pipes using the new external function
    if (create_pipes(pipe_fd, results_pipe_fd) == -1) 
    {
        return EXIT_FAILURE; // Exit if pipe creation failed
    }

    pid_t pid = fork();
    if (pid == -1) 
    {
        perror("Error forking process");
        return EXIT_FAILURE;
    } else if (pid == 0) 
    {
        // In child process
        close(pipe_fd[1]); // Close write-end of main pipe
        close(results_pipe_fd[0]); // Close read-end of results pipe
        child_process(pipe_fd, results_pipe_fd);
        exit(EXIT_SUCCESS);  // Ensure child exits after completion
    } else 
    {
        // In parent process
        close(pipe_fd[0]); // Close read-end of main pipe
        close(results_pipe_fd[1]); // Close write-end of results pipe
        parent_process(pipe_fd, results_pipe_fd);
    }

    return EXIT_SUCCESS;
}
