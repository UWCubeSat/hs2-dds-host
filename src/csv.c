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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "dds-host/util/csv.h"


// computes the number of rows in the csv
static unsigned long long CSV_NumRows(CSVFile *);

// computes the number of columns in the csv
static unsigned long long CSV_NumCols(CSVFile *);

CSVFile * CSV_Open(const char * fileName) {
  FILE * fp = fopen(fileName, "r");

  if (fp == NULL) {
    perror("CSV_Open() failed");
    return NULL;
  }

  CSVFile * file = (CSVFile *)malloc(sizeof(CSVFile));

  if (file == NULL) {
    fprintf(stderr, "Failed to allocate CSVFile\n");
    return NULL;
  }

  file->fp = fp;
  file->numCols = CSV_NumCols(file);
  file->numRows = CSV_NumRows(file);

  // return to the beginning of the file
  if (fseek(file->fp, 0, SEEK_SET) != 0) {
    return NULL;
  }

  return file;
}

bool CSV_Close(CSVFile *file) {
  if (file == NULL) {
    fprintf(stderr, "CSV_Close()-> can't free a NULL pointer!\n");
    return false;
  }
  fclose(file->fp);
  free(file);
  return true;
}

const char * CSV_ReadElement(CSVFile *file, unsigned long long row, unsigned long long col) {
  if (file == NULL) {
    fprintf(stderr, "file can't be null\n");
    return NULL;
  }

  if (file->fp == NULL) {
    fprintf(stderr, "underlying file pointer isn't open\n");
    return NULL;
  }

  if (row > file->numRows) {
    fprintf(stderr, "ReadElement()->there aren't that many rows!\n");
    return NULL;
  }

  if (col > file->numCols) {
    fprintf(stderr, "ReadElement()->there aren't that many columns!\n");
    return NULL;
  }

  // navigate to the right row by consuming lines
  unsigned long long rowIdx = 1;
  while (rowIdx < row) {
    char c = fgetc(file->fp);
    if (c == '\n') {
      rowIdx++;
    }
  }

  // file position indicator should be at the right row
  // consume the entire line into buf, then get the field at that position
  const unsigned int kBufLen = 1024;
  char buf[kBufLen+1];
  memset(buf, '\0', kBufLen);

  fgets(buf, sizeof(buf), file->fp);
  const char * tok;
  for (tok = strtok(buf, ","); tok && *tok; tok = strtok(NULL, ",\n")) {
    if (!--col) {
      const char * tmp = strdup(tok);
      fseek(file->fp, 0, SEEK_SET);
      return tmp;
    }
  }
  fseek(file->fp, 0, SEEK_SET);
  return NULL;
}

static unsigned long long CSV_NumRows(CSVFile * file) {
  if (file == NULL) {
    fprintf(stderr, "file can't be null\n");
    return 0;
  }

  // simply count the number of new lines in the file
  char c;
  unsigned long long rows = 0;

  while ((c = fgetc(file->fp)) != EOF) {
    if (c == '\n') {
      rows++;
    }
  }
  if (c == EOF) {
    rows++;
    return rows;
  }
  return 0;
}

static unsigned long long CSV_NumCols(CSVFile *file) {
  if (file == NULL) {
    fprintf(stderr, "file can't be null\n");
    return 0;
  }

  // simply count the number of commas in the first record
  char c;
  unsigned long long cols = 0;

  while ((c = fgetc(file->fp)) != '\n') {
    if (c == ',') {
      cols++;
    }
  }
  if (c == '\n') {
    cols++;
    return cols;
  }
  return 0;
}

