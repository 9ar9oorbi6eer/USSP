// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <dirent.h>
// #include <string.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <sys/wait.h>
// #include <time.h>

// #define MAX_FILES 100 // max number of .usp files to handle

// // Global variables
// int pipe_fd[2];  // File descriptors for the pipe
// int results_pipe_fd[2]; // File descriptors for the pipe to receive results from child
// char *file_names[MAX_FILES];  // Array to store file names
// int file_count = 0;  // Counter for number of files

// void child_process() 
// {
//     char buffer[256];
//     close(pipe_fd[1]);  // Close the write-end of the pipe in the child
//     close(results_pipe_fd[0]); // Close the read-end of the results pipe in the child

//     // Read filenames from the pipe
//     while (read(pipe_fd[0], buffer, sizeof(buffer)) > 0) 
//     {
//         printf("Child received: %s\n", buffer);
//         // Process each file here (placeholder)
//         write(results_pipe_fd[1], "Processed", 10); // Send results back to parent
//     }
//     close(pipe_fd[0]);
//     close(results_pipe_fd[1]);
//     exit(EXIT_SUCCESS);
// }


// int main(int argc, char *argv[]) 
// {
//     // Initialize local variables
//     DIR *dir; // a pointer to a directory stream
//     struct dirent *entry; // a pointer to a struct representing directory enteries
//     pid_t pid;
//     FILE *result_file;

//     // Create a pipe
//     if (pipe(pipe_fd) == -1 || pipe(results_pipe_fd) == -1) // this creates a pipe and stores file descripters in pip_fd
//     {
//         perror("pipe"); // this is if the pipe fails
//         exit(EXIT_FAILURE);
//     }

// // Fork a child process
//     pid = fork();
//     if (pid == -1) 
//     {
//         perror("fork");
//         exit(EXIT_FAILURE);

//     } 
//     else if (pid == 0) 
//     {
//         // Child process
//         child_process();
//     } 
//     else 
//     {
//         // Parent process
//         close(pipe_fd[0]);  // Close the read-end of the pipe in the parent
//         close(results_pipe_fd[1]); // Close the write-end of the results pipe in the parent



//     // Open the directory containing the executable if it fails then print an error
//     if ((dir = opendir(".")) == NULL) 
//     {
//         perror("Failed to open directory");
//         exit(EXIT_FAILURE);
//     }

//     // Here you can add the code to read and filter .usp files
//     // Placeholder for reading directory entries
//     while ((entry = readdir(dir)) != NULL) 
//     {
//             if (strstr(entry->d_name, ".usp") != NULL && file_count < MAX_FILES) // check count limit
//             {
//                 // Allocate memory for the filename
//                 file_names[file_count] = malloc(strlen(entry->d_name) + 1); 
//                 if (file_names[file_count] == NULL) // check memory allocation success
//                 {
//                     perror("Failed to allocate memory for filename");
//                     exit(EXIT_FAILURE);
//                 }
//                 strcpy(file_names[file_count], entry->d_name);
//                 write(pipe_fd[1], file_names[file_count], strlen(file_names[file_count]) + 1);
//                 file_count++;
//             }
//         }
//     // Close the directory
//         closedir(dir);
//         close(pipe_fd[1]);

//         result_file = fopen("result.txt", "w");
//         if (!result_file)
//         {
//             perror("Failed to open result.txt");
//             exit(EXIT_FAILURE);
//         }

//         char results_buffer[256];
//         while (read(results_pipe_fd[0], results_buffer, sizeof(results_buffer)) > 0) 
//         {
//             fprintf(result_file, "%s\n", results_buffer);
//         }
//         fclose(result_file);

//         for (int i = 0; i < file_count; i++) 
//         {
//             free(file_names[i]);
//         }

//         wait(NULL);
//         close(results_pipe_fd[0]);
//     }
//     return EXIT_SUCCESS;
// }
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

#define MAX_FILES 100 // max number of .usp files to handle

// Global variables
int pipe_fd[2];  // File descriptors for the pipe
int results_pipe_fd[2]; // File descriptors for the pipe to receive results from child

void child_process() 
{
    char buffer[256];
    int fd;
    char read_buffer[1024];
    ssize_t read_size;

    close(pipe_fd[1]);  // Close the write-end of the pipe in the child
    close(results_pipe_fd[0]); // Close the read-end of the results pipe in the child

    while ((read_size = read(pipe_fd[0], buffer, sizeof(buffer) - 1)) > 0) 
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
        char result[300];
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

        char results_buffer[300];
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
