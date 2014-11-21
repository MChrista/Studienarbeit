#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

// MMU Function
int convertVirtualToPhysical( int virtualAddr, int* pageDir ) {
  int page_offset = virtualAddr & 0x00000FFF;
  printf("Page Offset      : %08X\n", page_offset);
  int page_table_offset = (virtualAddr & 0x003FF000) >> 12;
  printf("Page Table Offset: %08X\n", page_table_offset);
  int page_dir_offset = virtualAddr >> 22;
  printf("Page Dir Offset  : %08X\n", page_dir_offset);

  int * page_table = pageDir + page_dir_offset; // vielleicht plus?
  printf ("Page Dir Entry   : %08X\n", &page_table);
  int page_entry = &page_table + page_table_offset;
  printf ("Page Table Entry : %08X\n", page_entry);


  return  ( page_entry % 4096 ) + page_offset;
}

int main() {
  int pageDir[10];
  int pageTable[10];

  pageDir[1] = * pageTable;
  pageTable[0] = 0x12345000;

  int testAddr = 0x00400CCC;

  printf("%08X -> %08X\n", testAddr, convertVirtualToPhysical(testAddr, * pageDir));

}
