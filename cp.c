#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <fcntl.h>
#include <dirent.h>

void copy_file(const char *source, const char *destination, int interactive, int verbose)
{
// Check if -v option is triggered	
    if (verbose)
    {
        char destination_path[1024];
        snprintf(destination_path, sizeof(destination_path), "%s/%s", destination, source);
        printf("'%s' -> '%s' \n", source, destination_path);
    
    }
    struct stat dest_stat;
    
    if (stat(destination, &dest_stat) == 0 && S_ISDIR(dest_stat.st_mode))
    {
        char destination_path[1024];
        snprintf(destination_path, sizeof(destination_path), "%s/%s", destination, source);
        destination = destination_path;
    }

    // Check if the destination file already exists
    if (access(destination, F_OK) == 0)
    {
        if (interactive)
        {
            printf("Destination file %s already exists. Overwrite? (y/n): ", destination);
            char response;
            scanf(" %c", &response);
            if (response != 'y' && response != 'Y')
            {
                return;
            }
        }
    }

    // Open source file for reading
    int source_fd = open(source, O_RDONLY);
    if (source_fd == -1)
    {
        perror("Error opening source file");
        return;
    }

    // Create or open destination file for writing
    int destination_fd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (destination_fd == -1)
    {
        perror("Error opening destination file");
        close(source_fd);
        return;
    }

    // Copy data from source to destination
    char buffer[4096];
    ssize_t bytes_read, bytes_written;
    while ((bytes_read = read(source_fd, buffer, sizeof(buffer))) > 0)
    {
        bytes_written = write(destination_fd, buffer, bytes_read);
        if (bytes_written == -1)
        {
            perror("Error writing to destination file");
            close(source_fd);
            close(destination_fd);
            return;
        }
    }

    // Close file descriptors
    close(source_fd);
    close(destination_fd);
}

void copy_directory(const char *source, const char *destination, int interactive, int verbose)
{
    struct dirent *entry;
    DIR *dir = opendir(source);

    if (!dir)
    {
        perror("Error opening source directory");
        return;
    }

    // Create destination directory if it doesn't exist
    mkdir(destination, 0755);

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char source_path[1024];
        char destination_path[1024];
        snprintf(source_path, sizeof(source_path), "%s/%s", source, entry->d_name);
        snprintf(destination_path, sizeof(destination_path), "%s/%s", destination, entry->d_name);

        struct stat st;
        if (lstat(source_path, &st) == -1)
        {
            perror("Error stat-ing source");
            continue;
        }

        if (S_ISDIR(st.st_mode))
        {
            // Recursively copy subdirectories
            copy_directory(source_path, destination_path, interactive, verbose);
        }
        else if (S_ISREG(st.st_mode))
        {
            // Copy regular files
            copy_file(source_path, destination_path, interactive, verbose);
        }
        else
        {
            // Handle other file types if needed
            fprintf(stderr, "Skipping file '%s' with unsupported type.\n", source_path);
        }
    }

    closedir(dir);
}

int my_cp(int argc, char *argv[])
{
    int c, interactive = 0, recursive = 0, verbose = 0;
    const char *target_directory = NULL;

    // Define the command-line options
    static struct option long_options[] = {
        {"interactive", no_argument, NULL, 'i'},
        {"recursive", no_argument, NULL, 'r'},
        {"target-directory", required_argument, NULL, 't'},
        {"verbose", no_argument, NULL, 'v'},
        {NULL, 0, NULL, 0}};

    while ((c = getopt_long(argc, argv, "irt:v", long_options, NULL)) != -1)
    {
        switch (c)
        {
        case 'i':
            interactive = 1;
            break;
        case 'r':
            recursive = 1;
            break;
        case 't':
            target_directory = optarg;
            break;
        case 'v':
            verbose = 1;
            break;
        default:
            fprintf(stderr, "Usage: my_cp [-i] [-r] [-t DIRECTORY] [-v] source1 source2 ... destination\n");
            exit(EXIT_FAILURE);
        }
    }

    if (!target_directory && optind + 1 > argc)
    {
        fprintf(stderr, "Error: Both source(s) and destination must be specified\n");
        fprintf(stderr, "Usage: my_cp [-i] [-r] [-t DIRECTORY] [-v] source1 source2 ... destination\n");
        exit(EXIT_FAILURE);
    }

    const char *destination = argv[argc - 1];

    if (target_directory)
    {
        destination = argv[optind - 1];
        for (int i = optind; i < argc; i++)
        {
            const char *source = argv[i];
            if (recursive)
            {
                copy_directory(source, destination, interactive, verbose);
            }
            else
            {
                copy_file(source, destination, interactive, verbose);
            }
        }
    }
    else
    {
        for (int i = optind; i < argc - 1; i++)
        {

            const char *source = argv[i];
            if (recursive)
            {
                copy_directory(source, destination, interactive, verbose);
            }
            else
            {
                copy_file(source, destination, interactive, verbose);
            }
        }
    }

    return 0;
}
