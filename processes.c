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