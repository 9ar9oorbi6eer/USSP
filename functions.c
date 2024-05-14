#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "functions.h"

void child_process(int pipe_fd[], int results_pipe_fd[]) 
{
    char buffer[1024]; // was 256
    int fd;
    char read_buffer[1024];
    char result[1024 * 10]; // Increased size to accumulate results
    ssize_t read_size;
    int result_length = 0; // To keep track of the current length of the result buffer

    close(pipe_fd[1]);  // Close the write-end of the pipe in the child
    close(results_pipe_fd[0]); // Close the read-end of the results pipe in the child

    while ((read_size = read(pipe_fd[0], buffer, sizeof(buffer) - 1)) > 0) 
    {
        buffer[read_size] = '\0'; // Ensure null termination
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
        struct tm birth_tm = {0};
        strptime(dob, "%d-%m-%Y", &birth_tm);
        time_t birth_time = mktime(&birth_tm);
        time_t current_time = time(NULL);
        double seconds = difftime(current_time, birth_time);
        int years = seconds / (365.24 * 24 * 3600);
        // re read this snprint review this
        // Append the result to the result buffer
        result_length += snprintf(result + result_length, sizeof(result) - result_length, "%s:%d\n", name, years);

        close(fd);
    }

    // Send accumulated results back to parent
    write(results_pipe_fd[1], result, result_length);
    
    close(pipe_fd[0]);
    close(results_pipe_fd[1]);
    exit(EXIT_SUCCESS);
}
