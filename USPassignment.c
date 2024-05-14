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

#define MAX_FILES 100 // max number of .usp files to handle

int pipe_fd[2];  // File descriptors for the pipe
int results_pipe_fd[2]; // File descriptors for the pipe to receive results from child

int main(int argc, char *argv[]) 
{
    DIR *dir;
    struct dirent *entry;
    pid_t pid;
    int fd_result;

    // Create pipes
    if (pipe(pipe_fd) == -1 || pipe(results_pipe_fd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) 
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) 
    {
        child_process(pipe_fd, results_pipe_fd);
    }
    else 
    {
        close(pipe_fd[0]);  // Close the read-end of the pipe in the parent
        close(results_pipe_fd[1]); // Close the write-end of the results pipe in the parent

        if ((dir = opendir(".")) == NULL) 
        {
            perror("Failed to open directory");
            exit(EXIT_FAILURE);
        }
        while ((entry = readdir(dir)) != NULL) 
        {
            if (strstr(entry->d_name, ".usp") != NULL)
            {
                write(pipe_fd[1], entry->d_name, strlen(entry->d_name) + 1);
                printf("Processing file: %s\n", entry->d_name); // Add this line
                sleep(1); // this was 1
            }
        }
        closedir(dir);
        close(pipe_fd[1]); // Done sending data

        sleep(2); // Sleep for 2 seconds to allow the child to process

        fd_result = open("result.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
        if (fd_result == -1)
        {
            perror("Failed to open result.txt");
            exit(EXIT_FAILURE);
        }

        char results_buffer[1024]; // was 300
        ssize_t bytes_read;
        while ((bytes_read = read(results_pipe_fd[0], results_buffer, sizeof(results_buffer) - 1)) > 0)
        {
            results_buffer[bytes_read] = '\0';
            write(fd_result, results_buffer, strlen(results_buffer));
            write(fd_result, "\n", 1);
        }
        close(fd_result);

        wait(NULL);
        close(results_pipe_fd[0]);
    }
    return EXIT_SUCCESS;
}
