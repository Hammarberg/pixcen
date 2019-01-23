#include "file.h"
#include <stdlib.h>
#include <string.h>

void freeFile(File *aFile)
{
  free(aFile->name);
  free(aFile->data);
}

bool readFile(File *aFile, const char *fileName)
{
  FILE *fp = NULL;
  struct stat fileStatus;

  aFile->name = (char *)strdup(fileName);

  if(stat(aFile->name, &fileStatus) == -1) {
    return false;
  }
  aFile->size = fileStatus.st_size;

  fp = fopen(aFile->name, "rb");
  if(fp == NULL) {
    return false;
  }

  aFile->data = (byte *)malloc(aFile->size);
  if(aFile->data == NULL) {
    fclose(fp);
    return false;
  }

  if(fread(aFile->data, 1, aFile->size, fp) != aFile->size) {
    fclose(fp);
    free(aFile->data);
    return false;
  }

  fclose(fp);
  return true;
}

bool writeFile(File *aFile, const char *fileName)
{
  FILE *fp = NULL;
  size_t length;
  char *ext;

  length = strlen(fileName);
  aFile->name = (char *)malloc(length + 4);

  if(aFile->name == NULL){
    return false;
  }

  strncpy(aFile->name, fileName, length);
  strncpy(aFile->name + length, ".b2\0", 4);

  fp = fopen(aFile->name, "wb");
  if(fp == NULL) {
    return false;
  }

  if(fwrite(aFile->data, 1, aFile->size, fp) != aFile->size) {
    fclose(fp);
    return false;
  }

  fclose(fp);
  return true;
}
