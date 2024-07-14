#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>

#ifdef __APPLE__
#include <sys/clonefile.h>
#elif __linux__
#include <sys/ioctl.h>
#include <linux/btrfs.h>
#endif

#define MAX_PATH 1024

int clone_item(const char *source, const char *target, int preserve_permissions);
int clone_directory(const char *source, const char *target, int preserve_permissions);
int clone_symlink(const char *source, const char *target);

int main(int argc, char *argv[])
{
    int preserve_permissions = 0;
    int source_index = 1;
    int target_index = 2;

    if (argc == 4 && strcmp(argv[1], "--no-permission-changes") == 0)
    {
        preserve_permissions = 1;
        source_index = 2;
        target_index = 3;
    }
    else if (argc != 3)
    {
        fprintf(stderr, "Usage: %s [--no-permission-changes] [source] [target]\n", argv[0]);
        return 1;
    }

    const char *source = argv[source_index];
    const char *target = argv[target_index];

    struct stat st;
    if (stat(source, &st) != 0)
    {
        perror("Error checking source");
        return 1;
    }

    if (!S_ISDIR(st.st_mode))
    {
        fprintf(stderr, "Source must be a directory\n");
        return 1;
    }

    return clone_directory(source, target, preserve_permissions);
}

int clone_file(const char *source, const char *target)
{
#ifdef __APPLE__
    return clonefile(source, target, 0);
#elif __linux__
    int src_fd = open(source, O_RDONLY);
    if (src_fd == -1)
    {
        perror("Error opening source file");
        return -1;
    }

    int dst_fd = open(target, O_WRONLY | O_CREAT, 0644);
    if (dst_fd == -1)
    {
        perror("Error opening target file");
        close(src_fd);
        return -1;
    }

    int result = ioctl(dst_fd, BTRFS_IOC_CLONE, src_fd);

    close(src_fd);
    close(dst_fd);

    return result;
#else
#error "Unsupported platform"
#endif
}

int clone_item(const char *source, const char *target, int preserve_permissions)
{
    struct stat st;
    if (lstat(source, &st) != 0)
    {
        perror("Error checking source item");
        return 1;
    }

    if (S_ISDIR(st.st_mode))
    {
        return clone_directory(source, target, preserve_permissions);
    }
    else if (S_ISLNK(st.st_mode))
    {
        return clone_symlink(source, target);
    }
    else
    {
        if (access(target, F_OK) != 0)
        {
            // Target doesn't exist, clone it
            if (clone_file(source, target) != 0)
            {
                perror("Error cloning file");
                return 1;
            }
        }
        else
        {
            // Target exists, check if it's different from source
            struct stat target_st;
            if (lstat(target, &target_st) != 0)
            {
                perror("Error checking target item");
                return 1;
            }

            if (st.st_mtime > target_st.st_mtime)
            {
                // Source is newer, update target
                if (unlink(target) != 0)
                {
                    perror("Error removing old target file");
                    return 1;
                }
                if (clone_file(source, target) != 0)
                {
                    perror("Error updating file");
                    return 1;
                }
            }
        }

        // Adjust permissions if needed
        if (!preserve_permissions)
        {
            mode_t new_mode = st.st_mode | S_IWUSR;
            if (chmod(target, new_mode) != 0)
            {
                perror("Error adjusting file permissions");
                return 1;
            }
        }
    }
    return 0;
}

int clone_directory(const char *source, const char *target, int preserve_permissions)
{
    DIR *dir;
    struct dirent *entry;
    struct stat st;

    if (stat(source, &st) != 0)
    {
        perror("Error checking source directory");
        return 1;
    }

    if (access(target, F_OK) != 0)
    {
        mode_t dir_mode = preserve_permissions ? (st.st_mode & 0777) : ((st.st_mode & 0777) | S_IWUSR);
        if (mkdir(target, dir_mode) != 0)
        {
            perror("Error creating target directory");
            return 1;
        }
    }
    else if (!preserve_permissions)
    {
        mode_t new_mode = st.st_mode | S_IWUSR;
        if (chmod(target, new_mode) != 0)
        {
            perror("Error adjusting directory permissions");
            return 1;
        }
    }

    dir = opendir(source);
    if (dir == NULL)
    {
        perror("Error opening source directory");
        return 1;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char source_path[MAX_PATH];
        char target_path[MAX_PATH];
        snprintf(source_path, sizeof(source_path), "%s/%s", source, entry->d_name);
        snprintf(target_path, sizeof(target_path), "%s/%s", target, entry->d_name);

        if (clone_item(source_path, target_path, preserve_permissions) != 0)
        {
            closedir(dir);
            return 1;
        }
    }

    closedir(dir);
    return 0;
}

int clone_symlink(const char *source, const char *target)
{
    char link_target[MAX_PATH];
    ssize_t len = readlink(source, link_target, sizeof(link_target) - 1);

    if (len == -1)
    {
        perror("Error reading symlink");
        return 1;
    }

    link_target[len] = '\0';

    // Check if the target symlink already exists
    if (access(target, F_OK) == 0)
    {
        // If it exists, check if it's different
        char existing_link_target[MAX_PATH];
        ssize_t existing_len = readlink(target, existing_link_target, sizeof(existing_link_target) - 1);

        if (existing_len == -1)
        {
            perror("Error reading existing symlink");
            return 1;
        }

        existing_link_target[existing_len] = '\0';

        if (strcmp(link_target, existing_link_target) == 0)
        {
            // Symlinks are the same, no need to update
            return 0;
        }

        // Symlinks are different, remove the existing one
        if (unlink(target) != 0)
        {
            perror("Error removing existing symlink");
            return 1;
        }
    }

    // Create the new symlink
    if (symlink(link_target, target) != 0)
    {
        perror("Error creating symlink");
        return 1;
    }

    return 0;
}