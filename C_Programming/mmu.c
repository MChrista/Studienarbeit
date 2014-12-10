#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

// MMU Function
int convertVirtualToPhysical( int virtualAddr, uint32_t *page_directory ) {
  printf("Page Dir Address %016p\n",page_directory);
  int page_offset = virtualAddr & 0x00000FFF;
  //printf("Page Offset      : %08X\n", page_offset);
  int page_table_offset = (virtualAddr & 0x003FF000) >> 12;
  //printf("Page Table Offset: %08X\n", page_table_offset);
  int page_dir_offset = virtualAddr >> 22;
  //printf("Page Dir Offset  : %08X\n", page_dir_offset);

  
  // if page_table cannot be found, throw page fault
  if (!page_directory[page_dir_offset])
  {
      pageFault(virtualAddr, page_directory);
  }
  int * page_table = (void *) page_directory[page_dir_offset]; // vielleicht plus?
  
  printf ("Page Dir Entry   : %016p\n", page_table);
  int page_entry = * ((page_table) + page_table_offset);
  printf ("Page Table Entry : %016p\n", page_entry);


  return  page_entry + page_offset;
}

void pageFault( int virtualAddr, uint32_t page_directory[] ){
    printf("Page Fault at: %016p\n", virtualAddr );
    
    // Create Page Table
    uint32_t page_table[1024] __attribute__((align(4096)));
    int page_dir_offset = virtualAddr >> 22;
    page_directory[page_dir_offset] = ((unsigned int)page_table) ;
    printf("Physical Page Table Address: %016p\n", page_table );
    
    // Create Page in Page Table
       
    int page_table_offset = (virtualAddr & 0x003FF000) >> 12;
    
    int physical_page_addr = malloc(0x1000);
    printf("Physical Page Addr: %016p\n", physical_page_addr & 0xFFFFF000 | 3 );
    page_table[page_table_offset] = (physical_page_addr & 0xFFFFF000 | 3) ;
    
     
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
    return 1;
    
}

int main() {
  
  // Page Directory anlegen
   uint32_t page_directory[1024] __attribute__((align(4096)));
    
  //printf("Value of Directory: %08X\n", pageDir[1]);
  int testAddr = 0x00400CCC;

  printf("%p -> %p\n", testAddr, convertVirtualToPhysical(testAddr, page_directory));

   // set to not present
   int i;
    for(i = 0; i < 1024; i++)
    {
        // This sets the following flags to the pages:
        //   Supervisor: Only kernel-mode can access them
        //   Write Enabled: It can be both read from and written to
        //   Not Present: The page table is not present
        page_directory[i] = 0x00000002;
    }
   

}
