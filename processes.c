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

int read_from_pipe(int fd, char *buffer, size_t size);
void process_file(const char *filename, char *result, int *result_length);

int child_process(int pipe_fd[], int results_pipe_fd[]) 
{
    close(pipe_fd[1]); // Close the write-end of the main pipe in the child
    close(results_pipe_fd[0]); // Close the read-end of the results pipe in the child

    int num_files;
    if (read(pipe_fd[0], &num_files, sizeof(num_files)) == -1) 
    {
        perror("Error reading number of files from pipe");
        return -1; // return an error code
    }
    printf("Received count of .usp files from parent: %d\n", num_files);

    char buffer[READ_BUFFER_SIZE];
    char result[RESULT_BUFFER_SIZE];
    int result_length = 0;

    for (int i = 0; i < num_files; i++) 
    {
        if (read_from_pipe(pipe_fd[0], buffer, sizeof(buffer)) <= 0) 
        {
            return -2; // return an error code if read_from_pipe fails
        }
        process_file(buffer, result, &result_length);
    }

    if (write(results_pipe_fd[1], result, result_length) == -1) 
    {
        perror("Error writing results to pipe");
        return -3; // return an error code
    }
    close(pipe_fd[0]);
    close(results_pipe_fd[1]);
    return 0; // return success
}

int parent_process(int pipe_fd[], int results_pipe_fd[]) 
{
    close(pipe_fd[0]);  // Close the read-end of the pipe in the parent
    close(results_pipe_fd[1]); // Close the write-end of the results pipe in the parent

    DIR *dir;
    if ((dir = opendir(".")) == NULL) 
    {
        perror("Failed to open directory");
        return -1;
    }

    int num_files = count_usp_files(dir);
    if (write(pipe_fd[1], &num_files, sizeof(num_files)) == -1) 
    {
        perror("Error writing number of files to pipe");
        closedir(dir);
        return -2;
    }
    printf("Sent count of .usp files to child: %d\n", num_files);
    send_filenames(dir, pipe_fd[1]);
    closedir(dir);

    int fd_result = open("result.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd_result == -1) 
    {
        perror("Failed to open result.txt");
        return -3;
    }
    read_results_and_write(fd_result, results_pipe_fd);

    wait(NULL);
    close(results_pipe_fd[0]);
    return 0;
}

int count_usp_files(DIR *dir) 
{
    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) 
    {
        if (strstr(entry->d_name, ".usp") != NULL) 
        {
            count++;
        }
    }
    rewinddir(dir);
    return count;
}
