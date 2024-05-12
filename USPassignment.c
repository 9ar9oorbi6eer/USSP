#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// Global variables
int pipe_fd[2];  // File descriptors for the pipe

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
    while ((entry = readdir(dir)) != NULL) // reads each entry in directory until NULL is returned
    {
        if (strstr(entry->d_name, ".usp") != NULL) // checks if it contains extention .usp
        {
            // Process each .usp file
            printf("Found .usp file: %s\n", entry->d_name);
        }
    }

    // Close the directory
    closedir(dir);

    // Cleanup and exit
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    return EXIT_SUCCESS;
}
