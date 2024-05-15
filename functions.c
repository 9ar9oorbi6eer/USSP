#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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

int create_pipes(int pipe_fd[2], int results_pipe_fd[2]) {
    if (pipe(pipe_fd) == -1 || pipe(results_pipe_fd) == -1) {
        perror("pipe");
        return -1; // Return error indicator
    }
    return 0; // Return success indicator
}