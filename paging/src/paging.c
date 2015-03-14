#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include "paging.h"
#define DEBUG


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

uint32_t page_directory[1024] __attribute__((aligned(0x1000)));

//Create Page Tables
page_t kernel_page_table[1024] __attribute__((align(0x1000)));
page_t programm_page_table[1024] __attribute__((align(0x1000)));
page_t stack_page_table[1024] __attribute__((align(0x1000)));

//General Parameters
int startaddress = 0x2000000;
int page_counter = 0;
int numOfPages = 20;




void bin_output(int val) 
{
    int i;
    char str[33];
    for (i = 31; i >= 0; i--)
	str[31-i] = '0' + ((val >> i) & 0x1);
    str[32] = 0;
    puts(str);
    return;
}





void pageFault( int virtualAddr){
#ifdef DEBUG
    printf("\nPage Fault at: %x\n", virtualAddr );
#endif
    int page_dir_offset = virtualAddr >> 22;
    int page_table_offset = (virtualAddr & 0x003FF000) >> 12;

    printf("Page directory Offset is: %i\n", page_dir_offset);
    printf("Page table Offset is: %i\n", page_table_offset);
#ifdef DEBUG_Detail
    printf("Page Directory offset Address is: %x\n",page_directory[page_dir_offset] );
    printf("Page Directory present Address is: %i\n",(page_directory[page_dir_offset] & 0x1) );
#endif
    if( (page_directory[page_dir_offset] & 0x1) == 1 ){ //if present Bit is set
        printf("Page Table is present\n");
        uint32_t *page_table;
        page_table = (uint32_t *)(page_directory[page_dir_offset] & 0xFFFFF000);
        if((*(page_table + page_table_offset) & 0x1) == 1){ //if present Bit is set
            printf("Page is already present at physical address %x\n", *(page_table + page_table_offset) & 0xFFFFF000);
        }else{
            uint32_t next_address = (uint32_t)(startaddress + page_counter++ * 0x1000 + 3);
            *(page_table + page_table_offset) = next_address;
            printf("New Page were reserved at physical address %x\n", next_address & 0xFFFFF000);
            // Get next free Page and return new virtual Adress
        }
    }else{
        printf("Segmentation Fault. Page Table is not present.\n");
    }
    
    //int physical_page_addr = malloc(0x1000);
    //printf("Physical Page Addr: %p\n", physical_page_addr & 0xFFFFF000 | 3 );
    //page_table[page_table_offset] = (physical_page_addr & 0xFFFFF000 | 3) ;
    
     
}
/*
int clear_address(int virtualAddr){
    int page_dir_offset = virtualAddr >> 22;
    int page_table_offset = (virtualAddr & 0x003FF000) >> 12;
    uint32_t *page_table;
    page_table = (uint32_t *)page_directory[page_dir_offset] & 0xFFFFF000;
}*/


uint32_t* init_paging() {
#ifdef DEBUG
    printf("Debugging Modus\n");
#else
    printf("Normal Modus");
#endif
    // Initialize Page Directoy
    
    //Set Directory to blank
    for(int i=0; i<1024;i++){
        *(page_directory+i) = *(page_directory+i) & 0x00000000;
    }
    
#ifdef DEBUG
    printf("Zeige die ersten vier Adressen des PageDirectories\n");
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
    *(page_directory) = (uint32_t)kernel_page_table | 3;
    *(page_directory+32) = (uint32_t)programm_page_table | 3;
    *(page_directory+1023) = (uint32_t)stack_page_table | 3;
    
#ifdef DEBUG
    printf("Kernel Page Table Address: %x\n", kernel_page_table[1]);
    printf("Kernel Page Table Address: %p\n", kernel_page_table);
    printf("Page Directory Address: %x\n", *(page_directory));
#endif
   
    printf("Address of page Directory%p\n",page_directory);
    return page_directory;
    
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
