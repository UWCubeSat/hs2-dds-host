/*
 * MIT License
 * 
 * Copyright (c) 2020 Eli Reed
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

#ifndef CSV_H_
#define CSV_H_

#include <stdio.h>
#include <stdbool.h>

#define MAX_CELL_LENGTH       1024

typedef struct csv_file_st {
  unsigned long long numRows;
  unsigned long long numCols;
  FILE *fp;
} CSVFile;

// opens a CSV file to be used with other functions
// in this interface
CSVFile * CSV_Open(const char * fileName);

// releases resources associated with input file
bool CSV_Close(CSVFile *file);

// reads a single element from the CSV. Returns
// true if successful, false otherwise.
char * CSV_ReadElement(CSVFile *file, unsigned long long row, unsigned long long col);

#endif  // CSV_H_