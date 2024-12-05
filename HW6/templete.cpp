#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <cstring>

int my_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    // TODO
    return 0;
}

int my_getattr(const char *path, struct stat *st) {
    // TODO
    return 0;
}

int my_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
    // TODO
    return 0;
}

int readlink(const char *path, void *buffer, size_t size) {
    // TODO
    return 0;
}

// 設定 FUSE 操作結構
static struct fuse_operations op;

int main(int argc, char *argv[]) {
    memset(&op, 0, sizeof(op));
    op.getattr = my_getattr;
    op.readdir = my_readdir;
    op.read = my_read;
    op.readlink = readlink;

    return fuse_main(argc, argv, &op, nullptr);
}
