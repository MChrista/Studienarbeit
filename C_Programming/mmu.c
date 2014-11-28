#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

// MMU Function
int convertVirtualToPhysical( int virtualAddr, int *pageDir[] ) {
  printf("Page Dir Address %p\n",pageDir);
  int page_offset = virtualAddr & 0x00000FFF;
  //printf("Page Offset      : %08X\n", page_offset);
  int page_table_offset = (virtualAddr & 0x003FF000) >> 12;
  //printf("Page Table Offset: %08X\n", page_table_offset);
  int page_dir_offset = virtualAddr >> 22;
  //printf("Page Dir Offset  : %08X\n", page_dir_offset);

  int * page_table = (void *) pageDir[1]; // vielleicht plus?
  
  printf ("Page Dir Entry   : %p\n", page_table);
  int page_entry = * (page_table + page_table_offset);
  printf ("Page Table Entry : %08X\n", page_entry);


  return  page_entry + page_offset;
}


/*
 * This function should translate 16MB from Floppy 
 * 16MB=2²⁷=8 000 000 Hex = 134 217 728 Bit
 * 4KB=2¹⁵=8 000 Hex = 32768 Bit
*/
int translate(){
    int counter = 0;
    for(int i=0x0000000;i<0x8000000;i+=0x8000){
        printf("Aktuelle Roundnumber: %i \n",counter++);
        printf("Aktueller Hexwert: %x\n",i);
    }
    fflush(stdout);
    return 1;
    
}

int main() {
    /*
  static int * pageDir[10];
  static int pageTable[10];
  
  printf("Address of Directory %p\n",pageDir);
  printf("Address of pageTable %p\n",pageTable);
  pageDir[1] = pageTable;
  pageTable[0] = 0x12345000;
 
  //printf("Value of Directory: %08X\n", pageDir[1]);
  int testAddr = 0x00400CCC;

  printf("%08X -> %08X\n", testAddr, convertVirtualToPhysical(testAddr, pageDir));
  */
    translate();
}
