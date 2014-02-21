#pragma once

namespace ByteBoozer
{

typedef enum {
  noDecr = 0,
  cleanDecr,
  normalDecr,
  loadInitDecr
} decruncherType;

typedef struct {
  size_t size;
  byte *data;
} File;

typedef unsigned int uint;

bool crunch(File *aSource, File *aTarget, uint startAdress, uint decrFlag, bool isRelocated, bool isLight);

void freeFile(File *aFile);

#define memSize 65536

}
