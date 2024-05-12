#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_FILES 100 // max number of .usp files to handle

// Global variables
int pipe_fd[2];  // File descriptors for the pipe
char *file_names[MAX_FILES];  // Array to store file names
int file_count = 0;  // Counter for number of files

int main(int argc, char *argv[]) 
{
    // Initialize local variables
    DIR *dir; // a pointer to a directory stream
    struct dirent *entry; // a pointer to a struct representing directory enteries

    // Create a pipe
    if (pipe(pipe_fd) == -1) // this creates a pipe and stores file descripters in pip_fd
    {
        perror("pipe"); // this is if the pipe fails
        exit(EXIT_FAILURE);
    }

    // Process command-line arguments if necessary
    if (argc > 1) 
    {
        printf("Processing extra arguments...\n");
    }

    // Open the directory containing the executable if it fails then print an error
    if ((dir = opendir(".")) == NULL) 
    {
        perror("Failed to open directory");
        exit(EXIT_FAILURE);
    }

    // Here you can add the code to read and filter .usp files
    // Placeholder for reading directory entries
    while ((entry = readdir(dir)) != NULL) 
    {
            if (strstr(entry->d_name, ".usp") != NULL && file_count < MAX_FILES) // check count limit
            {
                // Allocate memory for the filename
                file_names[file_count] = malloc(strlen(entry->d_name) + 1); 
                if (file_names[file_count] == NULL) // check memory allocation success
                {
                    perror("Failed to allocate memory for filename");
                    exit(EXIT_FAILURE);
                }
                strcpy(file_names[file_count], entry->d_name);
                printf("Found .usp file: %s\n", file_names[file_count]);
                file_count++;
            }
        }
    // Close the directory
    closedir(dir);

    // printing the list, making sure stuff is working:
    printf("\nList of .usp files:\n");
    for (int i = 0; i < file_count; i++) 
    {
        printf("%s\n", file_names[i]);
    }

    for (int i = 0; i < file_count; i++) {
        free(file_names[i]);
    }
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    return EXIT_SUCCESS;
}
