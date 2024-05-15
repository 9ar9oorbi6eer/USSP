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


int read_from_pipe(int fd, char *buffer, size_t size);
void process_file(const char *filename, char *result, int *result_length);

void child_process(int pipe_fd[], int results_pipe_fd[]) {
    close(pipe_fd[1]);
    close(results_pipe_fd[0]);

    int num_files;
    read(pipe_fd[0], &num_files, sizeof(num_files));  // First read the number of .usp files
    printf("Received count of .usp files from parent: %d\n", num_files);


    char buffer[1024];
    char result[10240];  // Increased buffer size for potential large results
    int result_length = 0;

    for (int i = 0; i < num_files; i++) {
        if (read_from_pipe(pipe_fd[0], buffer, sizeof(buffer)) > 0) {
            process_file(buffer, result, &result_length);
        }
    }

    write(results_pipe_fd[1], result, result_length);
    close(pipe_fd[0]);
    close(results_pipe_fd[1]);
}


void parent_process(int pipe_fd[], int results_pipe_fd[]) {
    close(pipe_fd[0]);  // Close the read-end of the pipe in the parent
    close(results_pipe_fd[1]); // Close the write-end of the results pipe in the parent

    DIR *dir;
    if ((dir = opendir(".")) == NULL) {
        perror("Failed to open directory");
        return;  // Modified from 'return 0;' since the function returns void
    }

    int num_files = count_usp_files(dir);
    write(pipe_fd[1], &num_files, sizeof(num_files));  // Send the count of .usp files to the child
    printf("Sent count of .usp files to child: %d\n", num_files);
    send_filenames(dir, pipe_fd[1]);
    closedir(dir);

    int fd_result = open("result.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (fd_result == -1) {
        perror("Failed to open result.txt");
        return;  // Modified from 'return 0;' since the function returns void
    }
    read_results_and_write(fd_result, results_pipe_fd);

    wait(NULL);
    close(results_pipe_fd[0]);
}



int count_usp_files(DIR *dir) {
    int count = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".usp") != NULL) {
            count++;
        }
    }
    rewinddir(dir);  // Reset directory stream position for future iterations
    return count;
}
