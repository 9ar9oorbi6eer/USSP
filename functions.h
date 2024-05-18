#ifndef FUNCTIONS_H
#define FUNCTIONS_H


int calculate_age(const char* dob); 
int create_pipes(int pipe_fd[2], int results_pipe_fd[2]);

// child process functions
void process_file(const char *filename, char *result, int *result_length);
int read_from_pipe(int fd, char *buffer, size_t size);

//parent process functions
void send_filenames(DIR *dir, int pipe_fd);
void read_results_and_write(int fd_result, int results_pipe_fd[]);

int setup_pipes(int pipe_fd[2], int results_pipe_fd[2]);
int process_fork(int pipe_fd[2], int results_pipe_fd[2]);


#endif