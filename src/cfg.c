// C
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>    // for bool type
#include <unistd.h>     // for fstat
#include <fcntl.h>      // for POSIX
#include <sys/stat.h>   // for fstat
#include <errno.h>      // for errno

// CFG
#include "cfg.h"

// parses a single element out of a string
static ConfigElement * CFG_ParseElement(char *str) {
    return true;
}

// parses a config block
static ConfigBlock * CFG_ParseBlock(char *str) {
    return true;
}

// parses a buffer containing some unknown number of blocks
static bool CFG_ParseBuffer(unsigned char *buf, Config *out) {
    if (buf == NULL) {
        return false;
    }

    // first, we need to do a pass through the buffer to find 
    // the number of blocks
    int num_blocks = 0;
    bool first_found = false;
    for (int i = 0; i < strlen(buf); i++) {
        char c = buf[i];
        // Some possibilities
            // 1: neither found and next character is opening - start
            // 2: neither found and next character is close - error
            // 3: first found and next character is opening - error
            // 4: first found and next character is close - end
            // 5: next character is neither bracket, so we just move on - continue

        if (!first_found && c == OPEN_BRACK) {
            first_found = true;
        } else if (first_found && c == CLOSE_BRACK) {
            num_blocks++;

            // reset
            first_found = false;
        } else if (first_found && c == OPEN_BRACK ||
                    !first_found && c == CLOSE_BRACK) {
            // error
            fprintf(stderr, "Malformed config file. Please ensure that all blocks are enclosed in a {...} pair.\n");
            return false;
        }
    }
    // now we know how many blocks to allocate
    ConfigBlock *blocks = (ConfigBlock *) malloc(sizeof(ConfigBlock) * num_blocks);

    // second pass, we want to remove all comment strings from the file
    // comments are delimited by an opening ';' and a terminal '\n'
    bool comment_char_found = true;
    for (int i = 0; i < strlen(buf); i++) {
        char c = buf[i];

        if (!comment_char_found && c == COMMENT_CHAR) {
            comment_char_found = true;
        } else if (comment_char_found && c == '\n') {
            comment_char_found = false;
        } else {
            buf[i] = '\0';
        }
    }

    out->num_blocks = num_blocks;
    out->blocks = blocks;
    return true;
}

bool CFG_ParseConfigFile(int fd, Config *out) {
    if (fd < 0) {
        fprintf(stderr, "Invalid file descriptor.\n");
        return false;
    }

    // first, stat the file so we know how much memory we need
    struct stat info;
    if (fstat(fd, &info) < 0) {
        perror("Failed to stat config file.");
        return false;
    }

    if (info.st_size == 0) {
        fprintf(stderr, "File is empty.\n");
        return false;
    }

    int bytes_read = 0;
    // allocate sufficient buffer with space for null-character
    unsigned char *buf = (unsigned char *) malloc(info.st_size + 1);
    memset(buf, '\0', info.st_size + 1);

    const size_t kReadSize = 1024;
    while (bytes_read < info.st_size) {
        // read out some bytes into our buffer
        size_t r_bytes = read(fd, buf + bytes_read, kReadSize);
        if (bytes_read < 0) {
            perror("Failed to read from file.");
            free(buf);
            return false;
        }
        bytes_read += r_bytes;
    }
    bool status = CFG_ParseBuffer(buf, out);
    free(buf);
    return status;
}

void CFG_FreeConfigBlock(ConfigBlock *block) {
    if (block == NULL) {
        return;
    }

    for (int i = 0; i < block->num_elements; i++) {
        CFG_FreeConfigElement(block->elements + i);
    }

    free(block);
}

void CFG_FreeConfigElement(ConfigElement *element) {
    if (element == NULL) {
        return;
    }

    free(element->key);
    free(element->value);
    free(element);
}

bool CFG_GenerateConfigFromBlocks(ConfigBlock *blocks, int num_blocks, char * path) {
    if (blocks == NULL || num_blocks == 0) {
        // nothing to do
        return false;
    }

    // attempt to open the file at the given path with read/write perms for user/group
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);

    if (fd < 0) {
        perror("Failed to create new config file.");
        return false;
    }


}