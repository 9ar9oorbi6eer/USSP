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


void child_process(int pipe_fd[], int results_pipe_fd[]) 
{
    char buffer[1024];
    int fd;
    char read_buffer[1024];
    char result[1024 * 10];
    ssize_t read_size;
    int result_length = 0;

    close(pipe_fd[1]);
    close(results_pipe_fd[0]);

    while ((read_size = read(pipe_fd[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[read_size] = '\0';
        printf("Child received: %s\n", buffer);

        fd = open(buffer, O_RDONLY);
        if (fd == -1) {
            perror("Failed to open file");
            continue;
        }

        int bytes_read = read(fd, read_buffer, sizeof(read_buffer) - 1);
        if (bytes_read < 0) {
            perror("Read failed");
            close(fd);
            continue;
        }
        read_buffer[bytes_read] = '\0';

        char *name = strtok(read_buffer, "\n");
        char *dob = strtok(NULL, "\n");
        int years = calculate_age(dob);
        result_length += snprintf(result + result_length, sizeof(result) - result_length, "%s:%d\n", name, years);

        close(fd);
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
    struct dirent *entry;
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
            printf("Processing file: %s\n", entry->d_name);
            sleep(1);
        }
    }
    closedir(dir);
    close(pipe_fd[1]); // Done sending data

    sleep(2); // Allow the child to process

    int fd_result = open("result.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd_result == -1)
    {
        perror("Failed to open result.txt");
        exit(EXIT_FAILURE);
    }

    char results_buffer[1024];
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
