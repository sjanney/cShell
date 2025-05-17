#include "../../include/fs/fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>
#include <time.h>

// Global variables
static FileEntry *root = NULL;
static char current_dir[MAX_PATH_LENGTH];
static pthread_mutex_t fs_mutex = PTHREAD_MUTEX_INITIALIZER;
static FILE *fs_log = NULL;

// Function to write to filesystem log
static void write_fs_log(const char *format, ...) {
    if (!fs_log) return;

    time_t now;
    time(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    va_list args;
    va_start(args, format);
    fprintf(fs_log, "[%s] ", timestamp);
    vfprintf(fs_log, format, args);
    fprintf(fs_log, "\n");
    va_end(args);
}

// Initialize filesystem
int fs_init() {
    write_fs_log("Initializing filesystem...");

    // Create root directory
    root = (FileEntry *)malloc(sizeof(FileEntry));
    if (!root) {
        write_fs_log("Failed to allocate memory for root directory");
        return -1;
    }

    // Initialize root directory
    strcpy(root->name, "/");
    strcpy(root->path, "/");
    root->type = FS_TYPE_DIRECTORY;
    root->perms.mode = S_IRWXU | S_IRWXG | S_IRWXO;
    root->perms.uid = getuid();
    root->perms.gid = getgid();
    root->size = 0;
    root->data = NULL;
    root->parent = NULL;
    root->child_count = 0;

    // Set current directory to root
    strcpy(current_dir, "/");

    // Initialize logging
    fs_log = fopen("logs/fs.log", "a");
    if (!fs_log) {
        write_fs_log("Failed to open filesystem log file");
        return -1;
    }
    setvbuf(fs_log, NULL, _IOLBF, 0);

    write_fs_log("Filesystem initialized successfully");
    return 0;
}

// Cleanup filesystem
void fs_cleanup() {
    write_fs_log("Cleaning up filesystem...");

    // Free all file entries
    // TODO: Implement recursive cleanup of file tree

    // Close log file
    if (fs_log) {
        fclose(fs_log);
        fs_log = NULL;
    }

    write_fs_log("Filesystem cleanup completed");
}

// Create a new file
FileEntry *fs_create_file(const char *path, FileType type) {
    pthread_mutex_lock(&fs_mutex);

    write_fs_log("Creating file: %s (type: %d)", path, type);

    // Parse path
    char *path_copy = strdup(path);
    char *last_slash = strrchr(path_copy, '/');
    if (!last_slash) {
        free(path_copy);
        pthread_mutex_unlock(&fs_mutex);
        return NULL;
    }

    // Split path into directory and filename
    *last_slash = '\0';
    char *dirname = path_copy;
    char *filename = last_slash + 1;

    // Find parent directory
    FileEntry *parent = fs_get_file(dirname);
    if (!parent || parent->type != FS_TYPE_DIRECTORY) {
        free(path_copy);
        pthread_mutex_unlock(&fs_mutex);
        return NULL;
    }

    // Check if file already exists
    for (int i = 0; i < parent->child_count; i++) {
        if (strcmp(parent->children[i]->name, filename) == 0) {
            free(path_copy);
            pthread_mutex_unlock(&fs_mutex);
            return NULL;
        }
    }

    // Create new file entry
    FileEntry *file = (FileEntry *)malloc(sizeof(FileEntry));
    if (!file) {
        free(path_copy);
        pthread_mutex_unlock(&fs_mutex);
        return NULL;
    }

    // Initialize file entry
    strncpy(file->name, filename, MAX_FILENAME_LENGTH - 1);
    strncpy(file->path, path, MAX_PATH_LENGTH - 1);
    file->type = type;
    file->perms.mode = S_IRUSR | S_IWUSR;
    file->perms.uid = getuid();
    file->perms.gid = getgid();
    file->size = 0;
    file->data = NULL;
    file->parent = parent;
    file->child_count = 0;

    // Add to parent directory
    parent->children[parent->child_count++] = file;

    free(path_copy);
    pthread_mutex_unlock(&fs_mutex);
    write_fs_log("File created successfully: %s", path);
    return file;
}

// Get file by path
FileEntry *fs_get_file(const char *path) {
    pthread_mutex_lock(&fs_mutex);

    write_fs_log("Getting file: %s", path);

    // Handle root directory
    if (strcmp(path, "/") == 0) {
        pthread_mutex_unlock(&fs_mutex);
        return root;
    }

    // Start from root
    FileEntry *current = root;
    char *path_copy = strdup(path);
    char *token = strtok(path_copy, "/");

    // Traverse path
    while (token) {
        int found = 0;
        for (int i = 0; i < current->child_count; i++) {
            if (strcmp(current->children[i]->name, token) == 0) {
                current = current->children[i];
                found = 1;
                break;
            }
        }
        if (!found) {
            free(path_copy);
            pthread_mutex_unlock(&fs_mutex);
            return NULL;
        }
        token = strtok(NULL, "/");
    }

    free(path_copy);
    pthread_mutex_unlock(&fs_mutex);
    return current;
}

// List directory contents
int fs_list_directory(const char *path) {
    pthread_mutex_lock(&fs_mutex);

    write_fs_log("Listing directory: %s", path);

    FileEntry *dir = fs_get_file(path);
    if (!dir || dir->type != FS_TYPE_DIRECTORY) {
        pthread_mutex_unlock(&fs_mutex);
        return -1;
    }

    printf("Contents of %s:\n", path);
    for (int i = 0; i < dir->child_count; i++) {
        FileEntry *entry = dir->children[i];
        printf("%s%s  ", entry->name, entry->type == FS_TYPE_DIRECTORY ? "/" : "");
    }
    printf("\n");

    pthread_mutex_unlock(&fs_mutex);
    return 0;
}

// Change directory
int fs_change_directory(const char *path) {
    pthread_mutex_lock(&fs_mutex);

    write_fs_log("Changing directory to: %s", path);

    FileEntry *dir = fs_get_file(path);
    if (!dir || dir->type != FS_TYPE_DIRECTORY) {
        pthread_mutex_unlock(&fs_mutex);
        return -1;
    }

    strncpy(current_dir, path, MAX_PATH_LENGTH - 1);

    pthread_mutex_unlock(&fs_mutex);
    return 0;
}

// Get current directory
char *fs_get_current_directory() {
    return current_dir;
}

// Create directory
int fs_create_directory(const char *path) {
    FileEntry *dir = fs_create_file(path, FS_TYPE_DIRECTORY);
    return dir ? 0 : -1;
}

// Remove directory
int fs_remove_directory(const char *path) {
    pthread_mutex_lock(&fs_mutex);

    write_fs_log("Removing directory: %s", path);

    FileEntry *dir = fs_get_file(path);
    if (!dir || dir->type != FS_TYPE_DIRECTORY) {
        pthread_mutex_unlock(&fs_mutex);
        return -1;
    }

    // Check if directory is empty
    if (dir->child_count > 0) {
        pthread_mutex_unlock(&fs_mutex);
        return -1;
    }

    // Remove from parent
    FileEntry *parent = dir->parent;
    for (int i = 0; i < parent->child_count; i++) {
        if (parent->children[i] == dir) {
            // Shift remaining entries
            for (int j = i; j < parent->child_count - 1; j++) {
                parent->children[j] = parent->children[j + 1];
            }
            parent->child_count--;
            break;
        }
    }

    free(dir);
    pthread_mutex_unlock(&fs_mutex);
    return 0;
}

// Copy file
int fs_copy_file(const char *src, const char *dst) {
    pthread_mutex_lock(&fs_mutex);

    write_fs_log("Copying file from %s to %s", src, dst);

    FileEntry *src_file = fs_get_file(src);
    if (!src_file || src_file->type == FS_TYPE_DIRECTORY) {
        pthread_mutex_unlock(&fs_mutex);
        return -1;
    }

    // Create new file
    FileEntry *dst_file = fs_create_file(dst, src_file->type);
    if (!dst_file) {
        pthread_mutex_unlock(&fs_mutex);
        return -1;
    }

    // Copy file data
    if (src_file->data) {
        dst_file->data = malloc(src_file->size);
        if (!dst_file->data) {
            pthread_mutex_unlock(&fs_mutex);
            return -1;
        }
        memcpy(dst_file->data, src_file->data, src_file->size);
        dst_file->size = src_file->size;
    }

    pthread_mutex_unlock(&fs_mutex);
    return 0;
}

// Move file
int fs_move_file(const char *src, const char *dst) {
    if (fs_copy_file(src, dst) != 0) {
        return -1;
    }
    return fs_delete_file(src);
}

// Delete file
int fs_delete_file(const char *path) {
    pthread_mutex_lock(&fs_mutex);

    write_fs_log("Deleting file: %s", path);

    FileEntry *file = fs_get_file(path);
    if (!file) {
        pthread_mutex_unlock(&fs_mutex);
        return -1;
    }

    // Remove from parent
    FileEntry *parent = file->parent;
    for (int i = 0; i < parent->child_count; i++) {
        if (parent->children[i] == file) {
            // Shift remaining entries
            for (int j = i; j < parent->child_count - 1; j++) {
                parent->children[j] = parent->children[j + 1];
            }
            parent->child_count--;
            break;
        }
    }

    // Free file data
    if (file->data) {
        free(file->data);
    }
    free(file);

    pthread_mutex_unlock(&fs_mutex);
    return 0;
}

// Change file permissions
int fs_change_permissions(const char *path, mode_t mode) {
    pthread_mutex_lock(&fs_mutex);

    write_fs_log("Changing permissions for %s to %o", path, mode);

    FileEntry *file = fs_get_file(path);
    if (!file) {
        pthread_mutex_unlock(&fs_mutex);
        return -1;
    }

    file->perms.mode = mode;

    pthread_mutex_unlock(&fs_mutex);
    return 0;
}

// Change file owner
int fs_change_owner(const char *path, uid_t uid, gid_t gid) {
    pthread_mutex_lock(&fs_mutex);

    write_fs_log("Changing owner for %s to %d:%d", path, uid, gid);

    FileEntry *file = fs_get_file(path);
    if (!file) {
        pthread_mutex_unlock(&fs_mutex);
        return -1;
    }

    file->perms.uid = uid;
    file->perms.gid = gid;

    pthread_mutex_unlock(&fs_mutex);
    return 0;
}

// Get file size
off_t fs_get_file_size(const char *path) {
    pthread_mutex_lock(&fs_mutex);

    FileEntry *file = fs_get_file(path);
    if (!file) {
        pthread_mutex_unlock(&fs_mutex);
        return -1;
    }

    off_t size = file->size;
    pthread_mutex_unlock(&fs_mutex);
    return size;
}

// Check if path is a directory
int fs_is_directory(const char *path) {
    pthread_mutex_lock(&fs_mutex);

    FileEntry *file = fs_get_file(path);
    int is_dir = (file && file->type == FS_TYPE_DIRECTORY);

    pthread_mutex_unlock(&fs_mutex);
    return is_dir;
}

// Check if path is a regular file
int fs_is_regular_file(const char *path) {
    pthread_mutex_lock(&fs_mutex);

    FileEntry *file = fs_get_file(path);
    int is_reg = (file && file->type == FS_TYPE_REGULAR);

    pthread_mutex_unlock(&fs_mutex);
    return is_reg;
}

// Check if path is a symlink
int fs_is_symlink(const char *path) {
    pthread_mutex_lock(&fs_mutex);

    FileEntry *file = fs_get_file(path);
    int is_link = (file && file->type == FS_TYPE_SYMLINK);

    pthread_mutex_unlock(&fs_mutex);
    return is_link;
}

// Check if path exists
int fs_exists(const char *path) {
    pthread_mutex_lock(&fs_mutex);

    FileEntry *file = fs_get_file(path);
    int exists = (file != NULL);

    pthread_mutex_unlock(&fs_mutex);
    return exists;
} 