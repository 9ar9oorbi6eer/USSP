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

int main(int argc, char *argv[]) 
{
    // Define return codes
    const int SUCCESS = 0;
    const int PIPE_FAILURE = -1;
    const int FORK_FAILURE = -2;

    if (setup_pipes(pipe_fd, results_pipe_fd) != SUCCESS) 
    {
        return PIPE_FAILURE;
    }

    if (process_fork(pipe_fd, results_pipe_fd) != SUCCESS)
    {
        return FORK_FAILURE;
    }

    return SUCCESS;
}
