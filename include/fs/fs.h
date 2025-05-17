#ifndef FS_H
#define FS_H

#include "../kernel/kernel.h"
#include <sys/stat.h>
#include <dirent.h>

// Filesystem constants
#define MAX_FILES_PER_DIR 1024
#define MAX_DIR_DEPTH 32
#define MAX_FILENAME_LENGTH 256
#define MAX_PATH_LENGTH 1024
#define MAX_SYMLINK_DEPTH 8

// File types
typedef enum {
    FS_TYPE_REGULAR,
    FS_TYPE_DIRECTORY,
    FS_TYPE_SYMLINK,
    FS_TYPE_FIFO,
    FS_TYPE_SOCKET,
    FS_TYPE_CHAR_DEV,
    FS_TYPE_BLOCK_DEV,
    FS_TYPE_UNKNOWN
} FileType;

// File permissions
typedef struct {
    mode_t mode;
    uid_t uid;
    gid_t gid;
    time_t atime;
    time_t mtime;
    time_t ctime;
} FilePermissions;

// File entry
typedef struct {
    char name[MAX_FILENAME_LENGTH];
    char path[MAX_PATH_LENGTH];
    FileType type;
    FilePermissions perms;
    off_t size;
    void *data;
    struct FileEntry *parent;
    struct FileEntry *children[MAX_FILES_PER_DIR];
    int child_count;
} FileEntry;

// Filesystem operations
int fs_init();
void fs_cleanup();
FileEntry *fs_create_file(const char *path, FileType type);
int fs_delete_file(const char *path);
FileEntry *fs_get_file(const char *path);
int fs_list_directory(const char *path);
int fs_change_directory(const char *path);
int fs_create_directory(const char *path);
int fs_remove_directory(const char *path);
int fs_copy_file(const char *src, const char *dst);
int fs_move_file(const char *src, const char *dst);
int fs_create_symlink(const char *target, const char *linkpath);
char *fs_read_symlink(const char *path);
int fs_change_permissions(const char *path, mode_t mode);
int fs_change_owner(const char *path, uid_t uid, gid_t gid);
off_t fs_get_file_size(const char *path);
int fs_is_directory(const char *path);
int fs_is_regular_file(const char *path);
int fs_is_symlink(const char *path);
int fs_exists(const char *path);
char *fs_get_current_directory();
int fs_set_current_directory(const char *path);
char *fs_get_absolute_path(const char *path);
char *fs_get_relative_path(const char *path);
int fs_create_hard_link(const char *target, const char *linkpath);
int fs_rename(const char *oldpath, const char *newpath);
int fs_truncate(const char *path, off_t length);
int fs_sync();
int fs_mount(const char *source, const char *target, const char *type);
int fs_umount(const char *target);
int fs_stat(const char *path, struct stat *st);
int fs_lstat(const char *path, struct stat *st);
int fs_access(const char *path, int mode);
int fs_chmod(const char *path, mode_t mode);
int fs_chown(const char *path, uid_t owner, gid_t group);
int fs_utime(const char *path, const struct utimbuf *times);
int fs_mkfifo(const char *path, mode_t mode);
int fs_mknod(const char *path, mode_t mode, dev_t dev);
int fs_readlink(const char *path, char *buf, size_t bufsiz);
int fs_symlink(const char *target, const char *linkpath);
int fs_link(const char *oldpath, const char *newpath);
int fs_unlink(const char *path);
int fs_rmdir(const char *path);
int fs_mkdir(const char *path, mode_t mode);
int fs_rmdir(const char *path);
int fs_opendir(const char *path);
int fs_closedir(DIR *dirp);
struct dirent *fs_readdir(DIR *dirp);
int fs_rewinddir(DIR *dirp);
long fs_telldir(DIR *dirp);
void fs_seekdir(DIR *dirp, long loc);
int fs_dirfd(DIR *dirp);
int fs_fstat(int fd, struct stat *st);
int fs_fchmod(int fd, mode_t mode);
int fs_fchown(int fd, uid_t owner, gid_t group);
int fs_ftruncate(int fd, off_t length);
int fs_fsync(int fd);
int fs_fdatasync(int fd);
int fs_lockf(int fd, int cmd, off_t len);
int fs_fcntl(int fd, int cmd, ...);
int fs_ioctl(int fd, unsigned long request, ...);
int fs_pipe(int pipefd[2]);
int fs_dup(int oldfd);
int fs_dup2(int oldfd, int newfd);
int fs_dup3(int oldfd, int newfd, int flags);
int fs_fcntl(int fd, int cmd, ...);
int fs_open(const char *path, int flags, ...);
int fs_close(int fd);
ssize_t fs_read(int fd, void *buf, size_t count);
ssize_t fs_write(int fd, const void *buf, size_t count);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_fallocate(int fd, int mode, off_t offset, off_t len);
int fs_posix_fallocate(int fd, off_t offset, off_t len);
int fs_posix_fadvise(int fd, off_t offset, off_t len, int advice);
int fs_sync_file_range(int fd, off_t offset, off_t nbytes, unsigned int flags);
int fs_vmsplice(int fd, const struct iovec *iov, unsigned long nr_segs, unsigned int flags);
int fs_splice(int fd_in, loff_t *off_in, int fd_out, loff_t *off_out, size_t len, unsigned int flags);
int fs_tee(int fd_in, int fd_out, size_t len, unsigned int flags);
int fs_sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
int fs_copy_file_range(int fd_in, loff_t *off_in, int fd_out, loff_t *off_out, size_t len, unsigned int flags);

#endif // FS_H 