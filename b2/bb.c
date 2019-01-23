#include "bb.h"
#include "file.h"
#include "cruncher.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char * argv[])
{
  File myFile;
  File myBBFile;
  char* fileName;
  bool isExecutable = false;
  bool isRelocated = false;
  uint address = 0;

  if((argc != 2 && argc != 4) ||
     (strcmp(argv[1], "-h") == 0) ||
     (strcmp(argv[1], "-help") == 0)){
    printf("Usage: b2 [-[c|e|r] xxxx] <filename>\n");
    printf("   -c: Make executable with start address xxxx.\n");
    printf("   -e: Same as -c :P.\n");
    printf("   -r: Relocate file to hex address xxxx.\n");
    return 0;
  }

  if(argc == 2) {
    fileName = argv[1];
  } else {
    int i;
    char *s = argv[2];
    fileName = argv[3];

    if((strcmp(argv[1], "-c") == 0) ||
       (strcmp(argv[1], "-e") == 0))
      isExecutable = true;
    else if(strcmp(argv[1], "-r") == 0)
      isRelocated = true;
    else {
      printf("Don't understand, aborting.\n");
      return -1;
    }
    if(strlen(s) != 4){
      printf("Don't understand, aborting.\n");
      return -1;
    }

    for(i = 0; i < 4; ++i){
      byte c;
      if(s[i] >= '0' && s[i] <= '9') c = s[i] - '0';
      if(s[i] >= 'a' && s[i] <= 'f') c = s[i] - 'a' + 10;
      if(s[i] >= 'A' && s[i] <= 'F') c = s[i] - 'A' + 10;
      address *= 16;
      address += c;
    }
  }

  if(!readFile(&myFile, fileName)) {
    printf("Error (B-1): Open file \"%s\", aborting.\n", fileName);
    return -1;
  }

  if(!crunch(&myFile, &myBBFile, address, isExecutable, isRelocated)) {
    freeFile(&myFile);
    return -1;
  }

  if(!writeFile(&myBBFile, myFile.name)) {
    printf("Error (B-2): Write file \"%s\", aborting.\n", myBBFile.name);
  }

  printf("B2: \"%s\" -> \"%s\"\n", myFile.name, myBBFile.name);

  freeFile(&myFile);
  freeFile(&myBBFile);

  return 0;
}
