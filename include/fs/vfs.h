#ifndef VFS_H
#define VFS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>

#define VFS_MAX_FILENAME 256
#define VFS_MAX_PATH 1024
#define VFS_MAX_FILES 1024
#define VFS_MAX_CHILDREN 128

typedef enum {
    VFS_TYPE_FILE,
    VFS_TYPE_DIRECTORY,
    VFS_TYPE_SYMLINK
} VFSNodeType;

typedef struct VFSNode {
    char name[VFS_MAX_FILENAME];
    VFSNodeType type;
    time_t created;
    time_t modified;
    time_t accessed;
    mode_t permissions;
    uid_t uid;
    gid_t gid;
    size_t size;
    void *data;
    struct VFSNode *parent;
    struct VFSNode *children[VFS_MAX_CHILDREN];
    int child_count;
} VFSNode;

// VFS initialization and cleanup
int vfs_init(void);
void vfs_cleanup(void);

// File operations
VFSNode *vfs_create_file(const char *path, mode_t mode);
VFSNode *vfs_create_directory(const char *path, mode_t mode);
VFSNode *vfs_create_symlink(const char *path, const char *target, mode_t mode);
int vfs_write(const char *path, const void *buffer, size_t size, off_t offset);
int vfs_read(const char *path, void *buffer, size_t size, off_t offset);
int vfs_delete(const char *path);
int vfs_rename(const char *old_path, const char *new_path);
int vfs_chmod(const char *path, mode_t mode);
int vfs_chown(const char *path, uid_t uid, gid_t gid);
int vfs_truncate(const char *path, off_t size);

// Path resolution and node access
VFSNode *vfs_get_node(const char *path);
VFSNode *vfs_get_parent(const char *path);
char *vfs_get_filename(const char *path);
char *vfs_get_absolute_path(const char *path);
char *vfs_get_canonical_path(const char *path);

// Directory operations
VFSNode **vfs_list_directory(const char *path, int *count);
int vfs_change_directory(const char *path);
char *vfs_get_current_directory(void);

// Utility functions
int vfs_exists(const char *path);
int vfs_is_directory(const char *path);
int vfs_is_file(const char *path);
int vfs_is_symlink(const char *path);
int vfs_is_readable(const char *path);
int vfs_is_writable(const char *path);
int vfs_is_executable(const char *path);
size_t vfs_get_size(const char *path);
time_t vfs_get_mtime(const char *path);
time_t vfs_get_ctime(const char *path);
time_t vfs_get_atime(const char *path);
char *vfs_resolve_symlink(const char *path);

// VFS state
extern VFSNode *vfs_root;
extern char vfs_current_directory[VFS_MAX_PATH];

#endif // VFS_H 