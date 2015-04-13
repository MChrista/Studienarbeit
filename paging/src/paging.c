#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include "paging.h"


/*
 * Declaration of Page Directory and Page tables
 */
uint32_t page_directory[1024] __attribute__((aligned(0x1000)));

//Create Page Tables
uint32_t kernel_page_table[1024] __attribute__((align(0x1000)));
uint32_t programm_page_table[1024] __attribute__((align(0x1000)));
uint32_t stack_page_table[1024] __attribute__((align(0x1000)));

//General Parameters
int startaddress = 0x2000000; //Startaddress for Physical Memory
int page_counter = 0;
int numOfPages = 20; //Maximum Number of Pages
uint32_t page_bitfield[1024][32] = {0};



void pageFault( int virtualAddr){
#if DEBUG >= 1
    printf("\nPage Fault at: %x\n", virtualAddr );
#endif
    int page_dir_offset = (virtualAddr >> 22) & 0x3FF;
    int page_table_offset = (virtualAddr & 0x003FF000) >> 12;
    
#if DEBUG >= 1
    printf("Page directory Offset is: %i\n", page_dir_offset);
    printf("Page table Offset is: %i\n", page_table_offset);
#endif
#if DEBUG >= 2
    printf("Page Directory offset Address is: %x\n",page_directory[page_dir_offset] );
    printf("Page Directory present Address is: %i\n",(page_directory[page_dir_offset] & 0x1) );
#endif
    
    if( (page_directory[page_dir_offset] & PRESENT_BIT) == PRESENT_BIT ){ //if present Bit is set
#if DEBUG >= 1
        printf("Page Table is present\n");
#endif
        uint32_t *page_table;
        page_table = (uint32_t *)(page_directory[page_dir_offset] & 0xFFFFF000);
#if DEBUG >= 2
        if(IS_PRESENT(*(page_table + page_table_offset) == 0)){
            printf("Makro sagt present\n");
        }else{
            printf("Makro sagt nicht present\n");
        }
#endif
        if((*(page_table + page_table_offset) & PRESENT_BIT) == PRESENT_BIT){ //if present Bit is set
#if DEBUG >= 1
            printf("Page is already present at physical address %x\n", *(page_table + page_table_offset) & 0xFFFFF000);
#endif
        }else{
            if(page_counter <= numOfPages){
                uint32_t next_address = (uint32_t)(startaddress + page_counter++ * 0x1000 + PRESENT_BIT + RW_BIT);
                *(page_table + page_table_offset) = next_address;
#if DEBUG >= 1      
          printf("New Page were reserved at physical address %x\n", next_address & 0xFFFFF000);
#endif             
   // Get next free Page and return new virtual Adress
            }else{
#if DEBUG >= 1
                printf("The Maximum Number of Pages have been reached already. This Page can't be reserved");
#endif
            }
            
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

int setPresentBit(int pde_offset, int pte_offset, int bool){
    if(pde_offset < 0 || pde_offset > 1023 || pte_offset < 0 || pte_offset > 1023){
        printf("Offset is no in range\n");
        return 1;     
    }else{
        int index = pte_offset/32;
        if(bool == 0){
            page_bitfield[pde_offset][index] |= 0 << (pte_offset%32);
        }else{
            page_bitfield[pde_offset][index] |= 1 << (pte_offset%32);
        }
        
#if DEBUG >= 1
        printf("Index in PDE is %d\n",pde_offset);
        printf("Index in PTE is %d\n",index);
        printf("New Value is %d\n",page_bitfield[pde_offset][index]);
#endif
        return 0;
        
    }
}

int isPresentBit(int pde_offset, int pte_offset){
    if(pde_offset < 0 || pde_offset > 1023 || pte_offset < 0 || pte_offset > 1023){
        printf("Offset is no in range\n");
        return 1;     
    }else{
        int index = pte_offset/32;
        
#if DEBUG >= 1
        printf("Index in PDE is %d\n",pde_offset);
        printf("Index in PTE is %d\n",index);
#endif
        return ((page_bitfield[pde_offset][index] & (1 << (pte_offset%32)) ) != 0);
    }
}




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
       kernel_page_table[i] = (uint32_t)(i * 0x1000 + 3);
    }
    *(page_directory) = (uint32_t)kernel_page_table | PRESENT_BIT | RW_BIT;
    *(page_directory+32) = (uint32_t)programm_page_table | PRESENT_BIT | RW_BIT;
    *(page_directory+1023) = (uint32_t)stack_page_table | PRESENT_BIT | RW_BIT;
    
    
    
#if DEBUG >= 1
    printf("Kernel Page Table Address: %x\n", kernel_page_table[1]);
    printf("Kernel Page Table Address: %p\n", kernel_page_table);
    printf("Page Directory Address: %x\n", *(page_directory));
#endif
   
    printf("Address of page Directory %p\n",page_directory);
    return page_directory;
}
