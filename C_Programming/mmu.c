#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "mmu.h"
#define DEBUG



uint32_t *page_directory;
const int NumberOfPageTables = 10;
int page_tables[10];

// MMU Function
int convertVirtualToPhysical( int virtualAddr, uint32_t *page_directory ) {
  //printf("Page Dir Address %p\n",page_directory);
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
  uint32_t * page_table = (void *) page_directory[page_dir_offset]; 
  
  //printf ("Page Dir Entry   : %p\n", page_table);
  int page_entry = * ((page_table) + page_table_offset);
  //printf ("Page Table Entry : %p\n", page_entry);


  return  page_entry + page_offset;
}

void pageFault( int virtualAddr, uint32_t page_directory[] ){
    printf("Page Fault at: %p\n", virtualAddr );
    
    // Create Page Table
    uint32_t page_table[1024] __attribute__((align(4096)));
    int page_dir_offset = virtualAddr >> 22;
    page_directory[page_dir_offset] = ((unsigned int)page_table) ;
    //printf("Physical Page Table Address: %p\n", page_table );
    
    // Create Page in Page Table
       
    int page_table_offset = (virtualAddr & 0x003FF000) >> 12;
    
    int physical_page_addr = malloc(0x1000);
    //printf("Physical Page Addr: %p\n", physical_page_addr & 0xFFFFF000 | 3 );
    page_table[page_table_offset] = (physical_page_addr & 0xFFFFF000 | 3) ;
    
     
}


int init_paging() {
#ifdef DEBUG
    printf("Debugging Modus");
#else
    printf("Normal Modus");
#endif
    // Initialize Page Directoy
    uint32_t local_directory[1024] __attribute__((align(4096)));
    page_directory = local_directory;
    
    
    
    //Copy Kernel to First Page
    uint32_t kernel_page_table[1024] __attribute__((align(4096)));
    
    
    
    // Initialize Pool of Pages
    for(int i = 0;i < NumberOfPageTables; i++){
        uint32_t page_table[1024] __attribute__((align(4096)));
        page_tables[i] = page_table;
    }
    
    /*Access to Page Table
     * int *pointer = page_tables[0];
     * *(pointer+1) = 100;    
     */
    
    
    
    
    // Subsequent are Testaddresses
    /*
    int testAddr = 0x00400CCC;
    printf("%p -> %p (Page Fault expected)\n", testAddr, convertVirtualToPhysical(testAddr, page_directory));
    printf("%p -> %p\n", testAddr, convertVirtualToPhysical(testAddr, page_directory));

    testAddr = 0x00400AAA;
    printf("%p -> %p\n", testAddr, convertVirtualToPhysical(testAddr, page_directory));

    testAddr = 0x00800AAA;
    printf("%p -> %p (Page Fault expected)\n ", testAddr, convertVirtualToPhysical(testAddr, page_directory));
     */
    //printf("Value of Directory: %08X\n", pageDir[1]);
}
