#ifndef FUNCTIONS_H
#define FUNCTIONS_H


void child_process(int pipe_fd[], int results_pipe_fd[]);
int calculate_age(const char* dob); 


#endif