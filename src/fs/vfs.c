#include "../../include/fs/vfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>

// Global VFS state
VFSNode *vfs_root = NULL;
char vfs_current_directory[VFS_MAX_PATH] = "/";

// Initialize the virtual filesystem
int vfs_init(void) {
    // Create root directory
    vfs_root = (VFSNode *)malloc(sizeof(VFSNode));
    if (!vfs_root) {
        return -1;
    }
    
    // Initialize root directory
    memset(vfs_root, 0, sizeof(VFSNode));
    strcpy(vfs_root->name, "/");
    vfs_root->type = VFS_TYPE_DIRECTORY;
    vfs_root->created = time(NULL);
    vfs_root->modified = vfs_root->created;
    vfs_root->accessed = vfs_root->created;
    vfs_root->permissions = 0755;
    vfs_root->uid = getuid();
    vfs_root->gid = getgid();
    vfs_root->size = 0;
    vfs_root->data = NULL;
    vfs_root->parent = vfs_root; // Root is its own parent
    vfs_root->child_count = 0;
    
    // Create standard directories
    vfs_create_directory("/bin", 0755);
    vfs_create_directory("/etc", 0755);
    vfs_create_directory("/home", 0755);
    vfs_create_directory("/tmp", 0777);
    vfs_create_directory("/usr", 0755);
    vfs_create_directory("/var", 0755);
    
    // Create user home directory
    char home_path[VFS_MAX_PATH];
    struct passwd *pw = getpwuid(getuid());
    if (pw) {
        snprintf(home_path, VFS_MAX_PATH, "/home/%s", pw->pw_name);
        vfs_create_directory(home_path, 0755);
        
        // Set current directory to user's home
        strcpy(vfs_current_directory, home_path);
    }
    
    return 0;
}

// Clean up the virtual filesystem
void vfs_cleanup(void) {
    // Free all nodes (recursive function would be better)
    if (vfs_root) {
        // Free children
        for (int i = 0; i < vfs_root->child_count; i++) {
            // This is a simplification - should recursively free
            free(vfs_root->children[i]->data);
            free(vfs_root->children[i]);
        }
        
        // Free root
        free(vfs_root->data);
        free(vfs_root);
        vfs_root = NULL;
    }
}

// Helper function to split path into components
static char **split_path(const char *path, int *count) {
    char *path_copy = strdup(path);
    char **components = (char **)malloc(VFS_MAX_PATH * sizeof(char *));
    *count = 0;
    
    // Handle absolute vs relative path
    char *token;
    if (path_copy[0] == '/') {
        components[(*count)++] = strdup("/");
        token = strtok(path_copy + 1, "/");
    } else {
        token = strtok(path_copy, "/");
    }
    
    while (token && *count < VFS_MAX_PATH - 1) {
        if (strcmp(token, ".") != 0) {  // Skip "." components
            if (strcmp(token, "..") == 0) {
                // Handle ".." components
                if (*count > 1) {
                    free(components[--(*count)]);
                }
            } else {
                components[(*count)++] = strdup(token);
            }
        }
        token = strtok(NULL, "/");
    }
    
    free(path_copy);
    return components;
}

// Helper function to free path components
static void free_path_components(char **components, int count) {
    for (int i = 0; i < count; i++) {
        free(components[i]);
    }
    free(components);
}

// Get a node from path
VFSNode *vfs_get_node(const char *path) {
    if (!path || !vfs_root) {
        return NULL;
    }
    
    // Handle root directory
    if (strcmp(path, "/") == 0) {
        return vfs_root;
    }
    
    // Split path into components
    int component_count = 0;
    char **components = split_path(path, &component_count);
    
    // Start at root or current directory
    VFSNode *current = vfs_root;
    int start_index = 0;
    
    if (components[0][0] != '/') {
        // Relative path, start from current directory
        current = vfs_get_node(vfs_current_directory);
    } else {
        // Absolute path, start from root
        start_index = 1;
    }
    
    // Traverse path
    for (int i = start_index; i < component_count && current; i++) {
        if (current->type != VFS_TYPE_DIRECTORY) {
            current = NULL;
            break;
        }
        
        // Look for component in current directory
        int found = 0;
        for (int j = 0; j < current->child_count; j++) {
            if (strcmp(current->children[j]->name, components[i]) == 0) {
                current = current->children[j];
                found = 1;
                break;
            }
        }
        
        if (!found) {
            current = NULL;
            break;
        }
    }
    
    free_path_components(components, component_count);
    return current;
}

// Get parent directory of path
VFSNode *vfs_get_parent(const char *path) {
    // Get absolute path
    char abs_path[VFS_MAX_PATH];
    if (path[0] == '/') {
        strncpy(abs_path, path, VFS_MAX_PATH - 1);
    } else {
        snprintf(abs_path, VFS_MAX_PATH, "%s/%s", vfs_current_directory, path);
    }
    
    // Find last slash
    char *last_slash = strrchr(abs_path, '/');
    if (!last_slash || last_slash == abs_path) {
        // Root directory or direct child of root
        return vfs_root;
    }
    
    // Null-terminate at last slash to get parent path
    *last_slash = '\0';
    if (strlen(abs_path) == 0) {
        return vfs_root;
    }
    
    return vfs_get_node(abs_path);
}

// Extract filename from path
char *vfs_get_filename(const char *path) {
    const char *last_slash = strrchr(path, '/');
    if (!last_slash) {
        return strdup(path);
    }
    return strdup(last_slash + 1);
}

// Create a file
VFSNode *vfs_create_file(const char *path, mode_t mode) {
    if (!path || !vfs_root) {
        return NULL;
    }
    
    // Get parent directory
    VFSNode *parent = vfs_get_parent(path);
    if (!parent || parent->type != VFS_TYPE_DIRECTORY) {
        return NULL;
    }
    
    // Get filename
    char *filename = vfs_get_filename(path);
    if (!filename || strlen(filename) == 0) {
        free(filename);
        return NULL;
    }
    
    // Check if file already exists
    for (int i = 0; i < parent->child_count; i++) {
        if (strcmp(parent->children[i]->name, filename) == 0) {
            free(filename);
            return NULL;  // File already exists
        }
    }
    
    // Check if parent has room for another child
    if (parent->child_count >= VFS_MAX_CHILDREN) {
        free(filename);
        return NULL;
    }
    
    // Create new file node
    VFSNode *file = (VFSNode *)malloc(sizeof(VFSNode));
    if (!file) {
        free(filename);
        return NULL;
    }
    
    // Initialize file node
    memset(file, 0, sizeof(VFSNode));
    strncpy(file->name, filename, VFS_MAX_FILENAME - 1);
    file->type = VFS_TYPE_FILE;
    file->created = time(NULL);
    file->modified = file->created;
    file->accessed = file->created;
    file->permissions = mode & 0777;  // Apply umask
    file->uid = getuid();
    file->gid = getgid();
    file->size = 0;
    file->data = NULL;
    file->parent = parent;
    file->child_count = 0;
    
    // Add file to parent directory
    parent->children[parent->child_count++] = file;
    parent->modified = time(NULL);
    
    free(filename);
    return file;
}

// Create a directory
VFSNode *vfs_create_directory(const char *path, mode_t mode) {
    if (!path || !vfs_root) {
        return NULL;
    }
    
    // Get parent directory
    VFSNode *parent = vfs_get_parent(path);
    if (!parent || parent->type != VFS_TYPE_DIRECTORY) {
        return NULL;
    }
    
    // Get directory name
    char *dirname = vfs_get_filename(path);
    if (!dirname || strlen(dirname) == 0) {
        free(dirname);
        return NULL;
    }
    
    // Check if directory already exists
    for (int i = 0; i < parent->child_count; i++) {
        if (strcmp(parent->children[i]->name, dirname) == 0) {
            free(dirname);
            return NULL;  // Directory already exists
        }
    }
    
    // Check if parent has room for another child
    if (parent->child_count >= VFS_MAX_CHILDREN) {
        free(dirname);
        return NULL;
    }
    
    // Create new directory node
    VFSNode *dir = (VFSNode *)malloc(sizeof(VFSNode));
    if (!dir) {
        free(dirname);
        return NULL;
    }
    
    // Initialize directory node
    memset(dir, 0, sizeof(VFSNode));
    strncpy(dir->name, dirname, VFS_MAX_FILENAME - 1);
    dir->type = VFS_TYPE_DIRECTORY;
    dir->created = time(NULL);
    dir->modified = dir->created;
    dir->accessed = dir->created;
    dir->permissions = mode & 0777;  // Apply umask
    dir->uid = getuid();
    dir->gid = getgid();
    dir->size = 0;
    dir->data = NULL;
    dir->parent = parent;
    dir->child_count = 0;
    
    // Add directory to parent
    parent->children[parent->child_count++] = dir;
    parent->modified = time(NULL);
    
    free(dirname);
    return dir;
}

// Write to a file
int vfs_write(const char *path, const void *buffer, size_t size, off_t offset) {
    if (!path || !buffer || !vfs_root) {
        return -1;
    }
    
    // Get file node
    VFSNode *file = vfs_get_node(path);
    if (!file || file->type != VFS_TYPE_FILE) {
        return -1;
    }
    
    // Resize if needed
    if (offset + size > file->size) {
        void *new_data = realloc(file->data, offset + size);
        if (!new_data) {
            return -1;
        }
        file->data = new_data;
        
        // Initialize new space to zero
        if (offset > file->size) {
            memset((char *)file->data + file->size, 0, offset - file->size);
        }
        
        file->size = offset + size;
    }
    
    // Write data
    memcpy((char *)file->data + offset, buffer, size);
    file->modified = time(NULL);
    file->accessed = file->modified;
    
    return size;
}

// Read from a file
int vfs_read(const char *path, void *buffer, size_t size, off_t offset) {
    if (!path || !buffer || !vfs_root) {
        return -1;
    }
    
    // Get file node
    VFSNode *file = vfs_get_node(path);
    if (!file || file->type != VFS_TYPE_FILE) {
        return -1;
    }
    
    // Check bounds
    if (offset >= file->size) {
        return 0;  // EOF
    }
    
    // Adjust size if reading past end of file
    if (offset + size > file->size) {
        size = file->size - offset;
    }
    
    // Read data
    memcpy(buffer, (char *)file->data + offset, size);
    file->accessed = time(NULL);
    
    return size;
}

// Delete a file or directory
int vfs_delete(const char *path) {
    if (!path || !vfs_root) {
        return -1;
    }
    
    // Cannot delete root
    if (strcmp(path, "/") == 0) {
        return -1;
    }
    
    // Get node
    VFSNode *node = vfs_get_node(path);
    if (!node) {
        return -1;
    }
    
    // Cannot delete non-empty directory
    if (node->type == VFS_TYPE_DIRECTORY && node->child_count > 0) {
        return -1;
    }
    
    // Find node in parent's children
    VFSNode *parent = node->parent;
    for (int i = 0; i < parent->child_count; i++) {
        if (parent->children[i] == node) {
            // Remove from parent's children
            for (int j = i; j < parent->child_count - 1; j++) {
                parent->children[j] = parent->children[j + 1];
            }
            parent->child_count--;
            parent->modified = time(NULL);
            
            // Free node
            free(node->data);
            free(node);
            
            return 0;
        }
    }
    
    return -1;  // Node not found in parent's children
}

// Change current directory
int vfs_change_directory(const char *path) {
    if (!path || !vfs_root) {
        return -1;
    }
    
    // Get node
    VFSNode *node = vfs_get_node(path);
    if (!node || node->type != VFS_TYPE_DIRECTORY) {
        return -1;
    }
    
    // Get absolute path
    char abs_path[VFS_MAX_PATH];
    if (path[0] == '/') {
        strncpy(abs_path, path, VFS_MAX_PATH - 1);
    } else {
        if (strcmp(vfs_current_directory, "/") == 0) {
            snprintf(abs_path, VFS_MAX_PATH, "/%s", path);
        } else {
            snprintf(abs_path, VFS_MAX_PATH, "%s/%s", vfs_current_directory, path);
        }
    }
    
    // Normalize the path (remove .. and .)
    char *canonical = vfs_get_canonical_path(abs_path);
    if (!canonical) {
        return -1;
    }
    
    // Update current directory
    strncpy(vfs_current_directory, canonical, VFS_MAX_PATH - 1);
    free(canonical);
    
    return 0;
}

// Get current directory
char *vfs_get_current_directory(void) {
    return strdup(vfs_current_directory);
}

// List directory contents
VFSNode **vfs_list_directory(const char *path, int *count) {
    if (!path || !count || !vfs_root) {
        return NULL;
    }
    
    // Get directory node
    VFSNode *dir = vfs_get_node(path);
    if (!dir || dir->type != VFS_TYPE_DIRECTORY) {
        return NULL;
    }
    
    // Create array for children
    *count = dir->child_count;
    if (*count == 0) {
        return NULL;
    }
    
    VFSNode **children = (VFSNode **)malloc(*count * sizeof(VFSNode *));
    if (!children) {
        return NULL;
    }
    
    // Copy children
    for (int i = 0; i < *count; i++) {
        children[i] = dir->children[i];
    }
    
    return children;
}

// Get absolute path
char *vfs_get_absolute_path(const char *path) {
    if (!path) {
        return NULL;
    }
    
    char *abs_path = (char *)malloc(VFS_MAX_PATH);
    if (!abs_path) {
        return NULL;
    }
    
    if (path[0] == '/') {
        strncpy(abs_path, path, VFS_MAX_PATH - 1);
    } else {
        if (strcmp(vfs_current_directory, "/") == 0) {
            snprintf(abs_path, VFS_MAX_PATH, "/%s", path);
        } else {
            snprintf(abs_path, VFS_MAX_PATH, "%s/%s", vfs_current_directory, path);
        }
    }
    
    return abs_path;
}

// Get canonical path (resolve . and ..)
char *vfs_get_canonical_path(const char *path) {
    if (!path) {
        return NULL;
    }
    
    // Get absolute path
    char *abs_path = vfs_get_absolute_path(path);
    if (!abs_path) {
        return NULL;
    }
    
    // Split into components
    int component_count = 0;
    char **components = split_path(abs_path, &component_count);
    free(abs_path);
    
    // Build canonical path
    char *canonical = (char *)malloc(VFS_MAX_PATH);
    if (!canonical) {
        free_path_components(components, component_count);
        return NULL;
    }
    
    // Start with root
    strcpy(canonical, "/");
    
    // Add components
    for (int i = 1; i < component_count; i++) {
        if (strcmp(canonical, "/") != 0) {
            strcat(canonical, "/");
        }
        strcat(canonical, components[i]);
    }
    
    free_path_components(components, component_count);
    
    // Handle empty path
    if (strlen(canonical) == 0) {
        strcpy(canonical, "/");
    }
    
    return canonical;
}

// Check if a path exists
int vfs_exists(const char *path) {
    return vfs_get_node(path) != NULL;
}

// Check if a path is a directory
int vfs_is_directory(const char *path) {
    VFSNode *node = vfs_get_node(path);
    return node && node->type == VFS_TYPE_DIRECTORY;
}

// Check if a path is a file
int vfs_is_file(const char *path) {
    VFSNode *node = vfs_get_node(path);
    return node && node->type == VFS_TYPE_FILE;
}

// Get file size
size_t vfs_get_size(const char *path) {
    VFSNode *node = vfs_get_node(path);
    return node ? node->size : 0;
}

// Get file modification time
time_t vfs_get_mtime(const char *path) {
    VFSNode *node = vfs_get_node(path);
    return node ? node->modified : 0;
}

// Check if a file is readable
int vfs_is_readable(const char *path) {
    VFSNode *node = vfs_get_node(path);
    if (!node) {
        return 0;
    }
    
    // Check owner permission
    if (node->uid == getuid()) {
        return (node->permissions & S_IRUSR) != 0;
    }
    
    // Check group permission
    if (node->gid == getgid()) {
        return (node->permissions & S_IRGRP) != 0;
    }
    
    // Check other permission
    return (node->permissions & S_IROTH) != 0;
}

// Check if a file is writable
int vfs_is_writable(const char *path) {
    VFSNode *node = vfs_get_node(path);
    if (!node) {
        return 0;
    }
    
    // Check owner permission
    if (node->uid == getuid()) {
        return (node->permissions & S_IWUSR) != 0;
    }
    
    // Check group permission
    if (node->gid == getgid()) {
        return (node->permissions & S_IWGRP) != 0;
    }
    
    // Check other permission
    return (node->permissions & S_IWOTH) != 0;
}

// Rename a file or directory
int vfs_rename(const char *old_path, const char *new_path) {
    if (!old_path || !new_path || !vfs_root) {
        return -1;
    }
    
    // Get source node
    VFSNode *src_node = vfs_get_node(old_path);
    if (!src_node) {
        return -1;
    }
    
    // Get target parent
    VFSNode *target_parent = vfs_get_parent(new_path);
    if (!target_parent || target_parent->type != VFS_TYPE_DIRECTORY) {
        return -1;
    }
    
    // Get target name
    char *target_name = vfs_get_filename(new_path);
    if (!target_name || strlen(target_name) == 0) {
        free(target_name);
        return -1;
    }
    
    // Check if target already exists
    for (int i = 0; i < target_parent->child_count; i++) {
        if (strcmp(target_parent->children[i]->name, target_name) == 0) {
            free(target_name);
            return -1;  // Target already exists
        }
    }
    
    // Check if target parent has room
    if (target_parent->child_count >= VFS_MAX_CHILDREN) {
        free(target_name);
        return -1;
    }
    
    // Remove from source parent
    VFSNode *src_parent = src_node->parent;
    int found = 0;
    for (int i = 0; i < src_parent->child_count; i++) {
        if (src_parent->children[i] == src_node) {
            // Remove from parent's children
            for (int j = i; j < src_parent->child_count - 1; j++) {
                src_parent->children[j] = src_parent->children[j + 1];
            }
            src_parent->child_count--;
            src_parent->modified = time(NULL);
            found = 1;
            break;
        }
    }
    
    if (!found) {
        free(target_name);
        return -1;
    }
    
    // Add to target parent
    strncpy(src_node->name, target_name, VFS_MAX_FILENAME - 1);
    src_node->parent = target_parent;
    src_node->modified = time(NULL);
    target_parent->children[target_parent->child_count++] = src_node;
    target_parent->modified = time(NULL);
    
    free(target_name);
    return 0;
}

// Create a symbolic link
VFSNode *vfs_create_symlink(const char *path, const char *target, mode_t mode) {
    if (!path || !target || !vfs_root) {
        return NULL;
    }
    
    // Get parent directory
    VFSNode *parent = vfs_get_parent(path);
    if (!parent || parent->type != VFS_TYPE_DIRECTORY) {
        return NULL;
    }
    
    // Get link name
    char *linkname = vfs_get_filename(path);
    if (!linkname || strlen(linkname) == 0) {
        free(linkname);
        return NULL;
    }
    
    // Check if file already exists
    for (int i = 0; i < parent->child_count; i++) {
        if (strcmp(parent->children[i]->name, linkname) == 0) {
            free(linkname);
            return NULL;  // File already exists
        }
    }
    
    // Check if parent has room for another child
    if (parent->child_count >= VFS_MAX_CHILDREN) {
        free(linkname);
        return NULL;
    }
    
    // Create new symlink node
    VFSNode *link = (VFSNode *)malloc(sizeof(VFSNode));
    if (!link) {
        free(linkname);
        return NULL;
    }
    
    // Initialize symlink node
    memset(link, 0, sizeof(VFSNode));
    strncpy(link->name, linkname, VFS_MAX_FILENAME - 1);
    link->type = VFS_TYPE_SYMLINK;
    link->created = time(NULL);
    link->modified = link->created;
    link->accessed = link->created;
    link->permissions = mode & 0777;  // Apply umask
    link->uid = getuid();
    link->gid = getgid();
    link->size = strlen(target);
    link->data = strdup(target);
    link->parent = parent;
    link->child_count = 0;
    
    // Add symlink to parent directory
    parent->children[parent->child_count++] = link;
    parent->modified = time(NULL);
    
    free(linkname);
    return link;
}

// Resolve a symbolic link
char *vfs_resolve_symlink(const char *path) {
    if (!path || !vfs_root) {
        return NULL;
    }
    
    // Get node
    VFSNode *node = vfs_get_node(path);
    if (!node || node->type != VFS_TYPE_SYMLINK) {
        return NULL;
    }
    
    return strdup((char *)node->data);
} 