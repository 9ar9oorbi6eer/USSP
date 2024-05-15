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

int calculate_age(const char* dob) 
{
    struct tm birth_tm = {0};
    strptime(dob, "%d-%m-%Y", &birth_tm);
    time_t birth_time = mktime(&birth_tm);
    time_t current_time = time(NULL);
    return difftime(current_time, birth_time) / (365.24 * 24 * 3600);
}

int create_pipes(int pipe_fd[2], int results_pipe_fd[2]) 
{
    if (pipe(pipe_fd) == -1 || pipe(results_pipe_fd) == -1) 
    {
        perror("pipe");
        return -1; // Return error indicator
    }
    return 0; // Return success indicator
}

// child process functions
int read_from_pipe(int fd, char *buffer, size_t size) 
{
    ssize_t read_size = read(fd, buffer, size - 1);
    if (read_size > 0) {
        buffer[read_size] = '\0';
    }
    return read_size;
}

void process_file(const char *filename, char *result, int *result_length) 
{
    char read_buffer[1024];
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        return;
    }

    int bytes_read = read(fd, read_buffer, sizeof(read_buffer) - 1);
    if (bytes_read < 0) {
        perror("Read failed");
        close(fd);
        return;
    }
    read_buffer[bytes_read] = '\0';

    char *name = strtok(read_buffer, "\n");
    char *dob = strtok(NULL, "\n");
    int years = calculate_age(dob);
    *result_length += snprintf(result + *result_length, 10240 - *result_length, "%s:%d\n", name, years);

    close(fd);
}


// parent process functions
void send_filenames(DIR *dir, int pipe_fd) 
{
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".usp") != NULL) {
            write(pipe_fd, entry->d_name, strlen(entry->d_name) + 1);
            printf("Processing file: %s\n", entry->d_name);
            sleep(1);
        }
    }
    close(pipe_fd);
}

void read_results_and_write(int fd_result, int results_pipe_fd[]) 
{
    if (fd_result == -1) 
    {
        perror("Failed to open result.txt");
        exit(EXIT_FAILURE);
    }

    char results_buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = read(results_pipe_fd[0], results_buffer, sizeof(results_buffer) - 1)) > 0) {
        results_buffer[bytes_read] = '\0';
        write(fd_result, results_buffer, strlen(results_buffer));
        write(fd_result, "\n", 1);
    }
    close(fd_result);
}