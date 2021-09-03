#include "cfg.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>    // for bool type
#include <unistd.h>     // for fstat
#include <fcntl.h>      // for POSIX
#include <sys/stat.h>   // for fstat
#include <errno.h>      // for errno

ConfigFile * CFG_OpenConfigFile(const char * path) {
    // exit early if the string is a null pointer
    if (path == NULL) {
        return NULL;
    }

    // open our file and initialize its members
    ConfigFile * cfg = (ConfigFile *) malloc(sizeof(ConfigFile));
    cfg->fd = open(path, O_RDONLY);
    cfg->blocks = NULL;
    cfg->num_blocks = 0;
    return cfg;
}

void CFG_CloseConfigFile(ConfigFile *file) {
    if (file == NULL) {
        // nothing to do
        return;
    }

    // first close the file
    close(file->fd);

    // free all the block structs
    for (int i = 0; i < file->num_blocks; i++) {
        CFG_FreeConfigBlock(file->blocks + i);
    }

    // finally, actually free the file struct itself
    free(file);
}

static bool CFG_ParseElement(char *str) {

}

static bool CFG_ParseBlock(char *str) {

}

static bool CFG_ParseBuffer(char *buf) {

}

bool CFG_ParseConfigFile(ConfigFile *file) {
    if (file == NULL) {
        // nothing to do
        return false;
    }

    // first pass, we want to count how many blocks there
    // this is going to correspond with the number of '{' '}' pairs
    // we can detect
    // if we reach EOF before a bracket closes, we'll immediately return false

    // first, stat the file so we know how much memory we need
    struct stat info;
    if (fstat(file->fd, &info) < 0) {
        perror("Failed to stat config file.");
        return false;
    }
    int bytes_read = 0;
    unsigned char *buf = (unsigned char *) malloc(sizeof(unsigned char));
    const unsigned int kReadSize = 4096;
    while (bytes_read < info.st_size) {
        // read out some bytes into our buffer
        bytes_read += read(file->fd, buf + bytes_read, kReadSize);
        if (bytes_read < 0) {
            perror("Failed to read from file.");
            free(buf);
            return false;
        }
    }
    return true;
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