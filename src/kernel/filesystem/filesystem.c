#include <stddef.h>
#include "filesystem.h"
#include "../libc/string.h"
#include "../libc/stdio.h"
#include "../libc/stdlib.h"

struct filesystem_node* filesystems = NULL;

int register_fs(struct filesystem* fs) {
    struct filesystem_node* new_fs = malloc(sizeof(struct filesystem_node));
    new_fs->fs = fs;
    new_fs->next = filesystems;
    filesystems = new_fs;

    return 1;
}

int fs_read(char* path, char* return_data, int read_length) {
    struct filesystem_node* current = filesystems;
    struct filesystem_node* best_filesystem_match = NULL;
    int best_match_len = -1;

    while (current != NULL) {
        int match_len = 0;
        while (current->fs->rootPath[match_len] && path[match_len] && current->fs->rootPath[match_len] == path[match_len]) {
            match_len++;
        }

        if ((current->fs->rootPath[0] == '/' && current->fs->rootPath[1] == '\0') || (current->fs->rootPath[match_len] == '\0' && (path[match_len] == '/' || path[match_len] == '\0'))) {
            if (match_len > best_match_len) {
                best_match_len = match_len;
                best_filesystem_match = current;
            }
        }
        current = current->next;
    }

    if (best_filesystem_match != NULL) {
        return best_filesystem_match->fs->read(path, return_data, read_length);
    } else {
        return -3;
    }
}


int fs_write(char* path, char* write_data, int write_length) {
    struct filesystem_node* current = filesystems;
    struct filesystem_node* best_filesystem_match = NULL;
    int best_match_len = -1;

    while (current != NULL) {
        int match_len = 0;
        while (current->fs->rootPath[match_len] && path[match_len] && current->fs->rootPath[match_len] == path[match_len]) {
            match_len++;
        }

        if ((current->fs->rootPath[0] == '/' && current->fs->rootPath[1] == '\0') || (current->fs->rootPath[match_len] == '\0' && (path[match_len] == '/' || path[match_len] == '\0'))) {
            if (match_len > best_match_len) {
                best_match_len = match_len;
                best_filesystem_match = current;
            }
        }
        current = current->next;
    }

    if (best_filesystem_match != NULL) {
        return best_filesystem_match->fs->read(path, write_data, write_length);
    } else {
        return -3;
    }
}
