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

// Global variables
int pipe_fd[2];  // File descriptors for the pipe
int results_pipe_fd[2]; // File descriptors for the pipe to receive results from child

void child_process() 
{
    // these are declerations of buffer and file descriptor for reading data
    char buffer[1024]; // was 256
    int fd;
    char read_buffer[1024];
    ssize_t read_size;

    // close unused ends of pipes in child process
    close(pipe_fd[1]);  // Close the write-end of the pipe in the child
    close(results_pipe_fd[0]); // Close the read-end of the results pipe in the child

    while ((read_size = read(pipe_fd[0], buffer, sizeof(buffer) - 1)) > 0) // continously read from file until no more data is available
    {
        buffer[read_size] = '\0'; // Ensure null termination
        printf("Child received: %s\n", buffer);

        // Open and process the file using system calls
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

        // Split the content to process the date of birth and calculate age
        char *name = strtok(read_buffer, "\n");
        char *dob = strtok(NULL, "\n");
        struct tm birth_tm = {0};
        strptime(dob, "%d-%m-%Y", &birth_tm);
        time_t birth_time = mktime(&birth_tm);
        time_t current_time = time(NULL);
        double seconds = difftime(current_time, birth_time);
        int years = seconds / (365.24 * 24 * 3600);

        // Prepare the result string
        char result[1024]; // was 300
        sprintf(result, "%s:%d", name, years);
        write(results_pipe_fd[1], result, strlen(result) + 1); // Send results back to parent
        
        close(fd);
    }
    close(pipe_fd[0]);
    close(results_pipe_fd[1]);
    exit(EXIT_SUCCESS);
}

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
        child_process();
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
