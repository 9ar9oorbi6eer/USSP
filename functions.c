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

#define READ_BUFFER_SIZE 1024
#define RESULT_BUFFER_SIZE 10240

int calculate_age(const char* dob) 
{
    struct tm birth_tm = {0};
    if (strptime(dob, "%d-%m-%Y", &birth_tm) == NULL) 
    {
        fprintf(stderr, "Error: Invalid date format in DOB: %s\n", dob);
        return -1; // Return -1 for invalid date format
    }
    time_t birth_time = mktime(&birth_tm);
    if (birth_time == -1) 
    {
        fprintf(stderr, "Error: mktime failed for DOB: %s\n", dob);
        return -1;
    }
    time_t current_time = time(NULL);
    return difftime(current_time, birth_time) / (365.24 * 24 * 3600);
}

int create_pipes(int pipe_fd[2], int results_pipe_fd[2]) 
{
    if (pipe(pipe_fd) == -1) 
    {
        perror("Error creating main communication pipe");
        return -1;
    }
    if (pipe(results_pipe_fd) == -1) 
    {
        perror("Error creating results pipe");
        return -1;
    }
    return 0;
}

// child process functions
int read_from_pipe(int fd, char *buffer, size_t size) 
{
    ssize_t read_size = read(fd, buffer, size - 1);
    if (read_size == -1) 
    {
        perror("Error reading from pipe");
        return -1;
    }
    if (read_size > 0) 
    {
        buffer[read_size] = '\0';
    }
    return read_size;
}

// Process individual file content and format output
void process_file(const char *filename, char *result, int *result_length) 
{
    char read_buffer[READ_BUFFER_SIZE];
    int fd = open(filename, O_RDONLY);
    if (fd == -1) 
    {
        perror("Failed to open file");
        return;
    }

    int bytes_read = read(fd, read_buffer, sizeof(read_buffer) - 1);
    if (bytes_read == -1) 
    {
        perror("Error reading from file");
        close(fd);
        return;
    } 
    else if (bytes_read == 0) 
    {
        fprintf(stderr, "Unexpected end of file: %s\n", filename);
        close(fd);
        return;
    }
    read_buffer[bytes_read] = '\0';

    char *name = strtok(read_buffer, "\n");
    char *dob = strtok(NULL, "\n");
    if (name == NULL || dob == NULL) 
    {
        fprintf(stderr, "Error: Invalid file format in file: %s\n", filename);
        close(fd);
        return;
    }
    int age = calculate_age(dob);
    if (age == -1) 
    {
        fprintf(stderr, "Error calculating age for file: %s\n", filename);
        close(fd);
        return;
    }

    // Adjust the snprintf to only include name and age in the desired format.
    *result_length += snprintf(result + *result_length, RESULT_BUFFER_SIZE - *result_length, "%s:%d\n", name, age);

    close(fd);
}

// parent process functions
void send_filenames(DIR *dir, int pipe_fd) 
{
    struct dirent *entry;
    int stop_flag = 0; // Flag to stop the loop
    while (!stop_flag && (entry = readdir(dir)) != NULL) 
    {
        if (strstr(entry->d_name, ".usp") != NULL) 
        {
            if (write(pipe_fd, entry->d_name, strlen(entry->d_name) + 1) == -1) 
            {
                perror("Error writing filename to pipe");
                stop_flag = 1; // Set flag to stop the loop
            }
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
        return;
    }

    char results_buffer[READ_BUFFER_SIZE];
    ssize_t bytes_read;
    int stop_flag = 0; // Flag to stop the loop
    while (!stop_flag && (bytes_read = read(results_pipe_fd[0], results_buffer, sizeof(results_buffer) - 1)) > 0) 
    {
        if (bytes_read == -1) 
        {
            perror("Error reading results from pipe");
            stop_flag = 1; // Set flag to stop the loop
            break;
        }
        results_buffer[bytes_read] = '\0';
        if (write(fd_result, results_buffer, strlen(results_buffer)) == -1) 
        {
            perror("Error writing results to result.txt");
            stop_flag = 1; // Set flag to stop the loop
            break;
        }
        if (write(fd_result, "\n", 1) == -1) 
        {
            perror("Error writing newline to result.txt");
            stop_flag = 1; // Set flag to stop the loop
            break;
        }
    }
    close(fd_result);
}
