#define _GNU_SOURCE
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const size_t headerSize = 0x800*0x10;



void extractFile(const char* path,void* buffer,size_t length) {
  static size_t total = 0;
  total += length;
  FILE* f = fopen(path,"wb");
/*
  size_t chunk = 0x10000;
  size_t remaining = length;
  while(remaining) {
    remaining -= fwrite(buffer,1,remaining<chunk?remaining:chunk,f);
  }
*/
  fwrite(buffer,1,length,f);
  fclose(f);  
  printf("Exported %lu bytes in total\n",total);
  return;
}

int main(int argc, char* argv[]) {
  off_t lastNeedle = headerSize;
  off_t needle = lastNeedle;
  int iso = open(argv[1],O_RDONLY);
  size_t length = lseek(iso,0,SEEK_END);
  uint8_t* haystack = mmap(NULL,length,PROT_READ,MAP_SHARED,iso,0);
  printf("File mapped to memory! (%lu bytes)\n",length);

  while(needle < length) {
    printf("Looking for needle!\n");
    uint8_t* needlePointer = memmem(&haystack[lastNeedle+1],length-needle,"\001CD001",6);
    printf("Found needle?\n");
    if (needlePointer == NULL) {
      printf("No more needles!\n");
      needlePointer = &haystack[length + headerSize];
    }
    needle = needlePointer - haystack;
    printf("Found possible needle at %lu\n",needle);

    // Extract a file now
    printf("\n");
    static unsigned int isoIndex = 0;
    off_t fileOffset = lastNeedle - headerSize;
    size_t fileLength = needle - lastNeedle;
    if (atoi(argv[2]) == -1) {
      fileLength = length - fileOffset;
    }
    if (fileLength > 0) {
      isoIndex++;
      if ((isoIndex == atoi(argv[2])) || (atoi(argv[2]) < 1)) {
        printf("Extracting iso %u, length: %lu\n",isoIndex,fileLength);
        char buffer[128];
        sprintf(buffer,"%s.chunk%u.iso",argv[1],isoIndex);
        extractFile(buffer,&haystack[fileOffset],fileLength);
      }
    }
    printf("\n");

    lastNeedle = needle;
  }

  munmap(haystack,length);
  close(iso);
  return 0;
}
