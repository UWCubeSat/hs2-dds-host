/*
 * MIT License
 * 
 * Copyright (c) 2021 Eli Reed
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef CFG_H_
#define CFG_H_

#include <stdint.h>
#include <stdbool.h>

// corresponds with a single key/value pair
// in a config block
typedef struct cfg_elmnt_t {
    char *key;
    char *value;
} ConfigElement;

// corresponds with a single config block
typedef struct cfg_block_t {
    char *key;
    unsigned int num_elements;
    ConfigElement *elements;
} ConfigBlock;

// corresponds with a single config file
typedef struct cfg_file_t {
    int fd;
    unsigned int num_blocks;
    ConfigBlock *blocks;
} ConfigFile;

// opens config file and instantiates variables
ConfigFile * CFG_OpenConfigFile(const char * path);

// parses the config file and fills in 
bool CFG_ParseConfigFile(ConfigFile *file);

// closes the config file and frees up any other resources
void CFG_CloseConfigFile(ConfigFile *file);

// creates a config file based on the blocks in the structure
void CFG_GenerateConfigFromBlocks(ConfigBlock *blocks, int num_blocks);

// frees a single block
void CFG_FreeConfigBlock(ConfigBlock *block);

// frees an element of a block
void CFG_FreeConfigElement(ConfigElement *element);

#endif  // CFG_H_