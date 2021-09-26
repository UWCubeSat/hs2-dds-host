// C
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>    // for bool type
#include <unistd.h>     // for fstat
#include <fcntl.h>      // for POSIX
#include <sys/stat.h>   // for fstat
#include <errno.h>      // for errno
#include <string.h>     // for string things

// CFG
#include "cfg.h"

// parses a single element out of a string
static bool CFG_ParseElement(char *str, ConfigElement *out) {
  return true;
}

// parses a config block
static bool CFG_ParseBlock(char *str, ConfigBlock *out) {
  if (out == NULL || str == NULL) {
    return false;
  }

  // extract the block name
  const char *start = str;
  const char *tok = strtok(str, "\"");
  size_t size = tok - start;
  if (size > MAX_LENGTH - 1) {
    return false;
  }

  

  memcpy(out->key, start, size);
  out->key[size] = '\0';

  // start going line-by-line and getting the keys and values
  for (tok = strtok(NULL, "\n"); tok && *tok && !(*tok == CLOSE_BRACK); tok = strtok(NULL, "\n")) {
    start = tok;

  }
  
  return true;
}

static int CFG_GetBlockCount(char *str) {
    // first, we need to do a pass through the buffer to find 
    // the number of blocks
    int num_blocks = 0;
    bool first_found = false;
    size_t buf_len = strlen(str);
    for (int i = 0; i < buf_len; i++) {
        char c = str[i];
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
            return -1;
        }
        // else we simply continue
    }
    return num_blocks;
}

// parses a buffer containing some unknown number of blocks
static bool CFG_ParseBuffer(unsigned char *buf, Config *out) {
    int num_blocks;
    ConfigBlock *blocks;

    if (buf == NULL) {
        return false;
    }

    if ((num_blocks = CFG_GetBlockCount(buf)) < 0) {
      return false;
    }
    out->num_blocks = num_blocks;
    return true;
    // blocks = (ConfigBlock *) malloc(sizeof(ConfigBlock) * num_blocks);

    // steps:
      // 1) navigate to start of block from current position in buffer
      // 2) consume characters until we encounter either an opening bracket or a space (short-circuit on space)
      // 3) start consuming newlines, each time checking if the next character is a comment character
      //    3.5) if it is a comment character, move on to the next new-line
      // 4) in the absence of a comment character, consume characters until we're not at a space
      // 5) save pointer to beginning of this part of string
      // 6) consume characters until we encounter the element delimiter
      // 7) copy most recent string into current block key
      // 8) save pointer to beginning of value
      // 9) navigate until we encounter newline or comment character
      // 10) compute value string and store it in current config block value
    
    // TODO: figure out how to handle comments
    // const char *tok;
    // unsigned int current_block = 0;
    // for (tok = strtok(buf, "\""); tok && *tok; tok = strtok(NULL, "\"")) {
    //   // extract the config block string from the buffer
    //   const char * start = tok;
    //   tok = strtok(NULL, "}");
    //   if (tok == NULL) {
    //     fprintf(stderr, "Malformed config file detected. Please check formatting.\n");
    //     free(blocks);
    //     return false;
    //   }
    //   size_t size = tok - start;
    //   char block_str[size + 1];
    //   memcpy(block_str, start, size);
    //   block_str[size + 1] = '\0';

    //   if (!CFG_ParseBlock(block_str, (blocks + current_block))) {
    //     fprintf(stderr, "Failed to parse config block.\n");
    //     free(blocks);
    //     return false;
    //   }
    //   current_block++;
    // }
    

    // out->num_blocks = num_blocks;
    // out->blocks = blocks;
    // return true;
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
    char *buf = (char *) malloc(info.st_size + 1);
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