#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <fcntl.h>
#include <dirent.h>

void move_file(const char *source, const char *destination, int interactive, const char *backup_suffix)
{
    // Check if the destination is an existing directory
    struct stat dest_stat;
    if (stat(destination, &dest_stat) == 0 && S_ISDIR(dest_stat.st_mode))
    {
        // Append the source filename to the destination directory
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
        else if (backup_suffix)
        {
            // Create a backup with the specified suffix
            char backup_destination[1024];
            snprintf(backup_destination, sizeof(backup_destination), "%s%s", destination, backup_suffix);
            if (rename(destination, backup_destination) != 0)
            {
                perror("Error creating backup file");
                return;
            }
        }
    }

    // Perform the file move
    if (rename(source, destination) != 0)
    {
        perror("Error moving file");
    }
}

int my_mv(int argc, char *argv[])
{
    int c, interactive = 0, backup = 0;
    const char *suffix = NULL;
    const char *target_directory = NULL;

    // Define the command-line options
    static struct option long_options[] = {
        {"interactive", no_argument, NULL, 'i'},
        {"suffix", required_argument, NULL, 'S'},
        {"backup", no_argument, NULL, 'b'},
        {"target-directory", required_argument, NULL, 't'},
        {NULL, 0, NULL, 0}};

    while ((c = getopt_long(argc, argv, "iS:bt:", long_options, NULL)) != -1)
    {
        switch (c)
        {
        case 'i':
            interactive = 1;
            break;
        case 'S':
            suffix = optarg;
            break;
        case 'b':
            backup = 1;
            break;
        case 't':
            target_directory = optarg;
            break;
        default:
            fprintf(stderr, "Usage: my_mv [-i] [-S SUFFIX] [-b] [-t DIRECTORY] source destination\n");
            exit(EXIT_FAILURE);
        }
    }

    if (!target_directory && optind + 1 > argc)
    {
        fprintf(stderr, "Error: Both source(s) and destination must be specified\n");
        fprintf(stderr, "Usage: my_mv [-i] [-S SUFFIX] [-b] [-t DIRECTORY] source destination\n");
        exit(EXIT_FAILURE);
    }

    const char *destination = argv[argc - 1];

    if (suffix && backup)
    {
        fprintf(stderr, "Error: Both -S and -b options cannot be used together\n");
        fprintf(stderr, "Usage: my_mv [-i] [-S SUFFIX] [-b] [-t DIRECTORY] source destination\n");
        exit(EXIT_FAILURE);
    }

    if (backup)
    {
        // Use a default backup suffix
        suffix = "~";
    }

    if (target_directory)
    {
        destination = argv[optind -1];
        for (int i = optind; i < argc; i++)
        {
            const char *source = argv[i];
            printf("src %s, dest %s \n",source, destination);
            move_file(source, destination, interactive, suffix);
        }
    }
    else
    {
        printf("src %s, dest %s \n",argv[optind], destination);
        move_file(argv[optind], destination, interactive, suffix);
    }

    return 0;
}
