#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include "paging.h"



typedef struct page
{
   uint32_t present    : 1;   // Page present in memory
   uint32_t rw         : 1;   // Read-only if clear, readwrite if set
   uint32_t user       : 1;   // Supervisor level only if clear
   uint32_t accessed   : 1;   // Has the page been accessed since last refresh?
   uint32_t dirty      : 1;   // Has the page been written to since last refresh?
   uint32_t unused     : 7;   // Amalgamation of unused and reserved bits
   uint32_t frame      : 20;  // Frame address (shifted right 12 bits)
} page_t;



/*
 * Declaration of Page Directory and Page tables
 */
uint32_t page_directory[1024] __attribute__((aligned(0x1000)));

//Create Page Tables
page_t kernel_page_table[1024] __attribute__((align(0x1000)));
page_t programm_page_table[1024] __attribute__((align(0x1000)));
page_t stack_page_table[1024] __attribute__((align(0x1000)));

//General Parameters
int startaddress = 0x2000000; //Startaddress for Physical Memory
int page_counter = 0;
int numOfPages = 20; //Maximum Number of Pages



void pageFault( int virtualAddr){

    printf("\nPage Fault at: %x\n", virtualAddr );

    int page_dir_offset = virtualAddr >> 22;
    int page_table_offset = (virtualAddr & 0x003FF000) >> 12;
    
#if DEBUG >= 1
    printf("Page directory Offset is: %i\n", page_dir_offset);
    printf("Page table Offset is: %i\n", page_table_offset);
#endif
#if DEBUG >= 2
    printf("Page Directory offset Address is: %x\n",page_directory[page_dir_offset] );
    printf("Page Directory present Address is: %i\n",(page_directory[page_dir_offset] & 0x1) );
#endif
    if( (page_directory[page_dir_offset] & 0x1) == 1 ){ //if present Bit is set
#if DEBUG >= 1
        printf("Page Table is present\n");
#endif
        uint32_t *page_table;
        page_table = (uint32_t *)(page_directory[page_dir_offset] & 0xFFFFF000);
        if((*(page_table + page_table_offset) & 0x1) == PAGE_PRESENT){ //if present Bit is set
#if DEBUG >= 1
            printf("Page is already present at physical address %x\n", *(page_table + page_table_offset) & 0xFFFFF000);
#endif
        }else{
            uint32_t next_address = (uint32_t)(startaddress + page_counter++ * 0x1000 + PAGE_PRESENT + PAGE_RW);
            *(page_table + page_table_offset) = next_address;
            printf("New Page were reserved at physical address %x\n", next_address & 0xFFFFF000);
            // Get next free Page and return new virtual Adress
        }
    }else{
        printf("Segmentation Fault. Page Table is not present.\n");
    }
}
/*
int clear_address(int virtualAddr){
    int page_dir_offset = virtualAddr >> 22;
    int page_table_offset = (virtualAddr & 0x003FF000) >> 12;
    uint32_t *page_table;
    page_table = (uint32_t *)page_directory[page_dir_offset] & 0xFFFFF000;
}*/


uint32_t* init_paging() {
#if DEBUG >= 1
    printf("Debugging Modus\n");
#else
    printf("Normal Modus\n");
#endif
    // Initialize Page Directoy
    
    //Set Directory to blank
    for(int i=0; i<1024;i++){
        *(page_directory+i) = *(page_directory+i) & 0x00000000;
    }
    
#if DEBUG >= 1
    printf("Show the first four Addresses of PageDirectory\n");
    for(int i=0; i<4;i++){
        printf("%p\n",(page_directory+i));
    }
#endif
    
    
    
    //Copy Kernel to First Page Table
    //for the first MB
    for(int i = 0; i<256; i++){
        kernel_page_table[i].present = 1;
        kernel_page_table[i].rw = 1;
        kernel_page_table[i].frame= (uint32_t)i * 0x1;
       // kernel_page_table[i] = (uint32_t)(i * 0x1000 + 3);
    }
    *(page_directory) = (uint32_t)kernel_page_table | PAGE_PRESENT | PAGE_RW;
    *(page_directory+32) = (uint32_t)programm_page_table | PAGE_PRESENT | PAGE_RW;
    *(page_directory+1023) = (uint32_t)stack_page_table | PAGE_PRESENT | PAGE_RW;
    
    
    
#if DEBUG >= 1
    printf("Kernel Page Table Address: %x\n", kernel_page_table[1]);
    printf("Kernel Page Table Address: %p\n", kernel_page_table);
    printf("Page Directory Address: %x\n", *(page_directory));
#endif
   
    printf("Address of page Directory %p\n",page_directory);
    return page_directory;
}
