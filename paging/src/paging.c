#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include "paging.h"

/*
 * Declaration of Page Directory and Page tables
 */
uint32_t page_directory[1024] __attribute__((aligned(0x1000)));

//uint32_t * page_directory = (uint32_t *) 0x100000;
//uint32_t * kernel_page_table = (uint32_t *) 0x101000;
//uint32_t * programm_page_table = (uint32_t *) 0x102000;
//uint32_t * stack_page_table = (uint32_t *) 0x103000;

//Create Page Tables
uint32_t kernel_page_table[1024] __attribute__((align(0x1000)));
uint32_t programm_page_table[1024] __attribute__((align(0x1000)));
uint32_t stack_page_table[1024] __attribute__((align(0x1000)));

//General Parameters
int startaddress = 0x200000; //Startaddress for Physical Memory
int page_counter = 0;
const int maxNumberOfPages = 4; //Maximum Number of Pages
uint32_t page_bitfield[1024][32] = {0};


//Page replace parameters
int replace_pde_offset = 0;
int replace_pte_offset = 512;

struct page_fault_result {
  int fault_address;
  int pde;
  int pte;
  int offset;
  int physical_address;
  int flags;  
};
struct page_fault_result ret_info;

struct page_fault_result * pageFault( int virtualAddr){    

    int page_dir_offset = (virtualAddr >> 22) & 0x3FF;
    int page_table_offset = (virtualAddr & 0x003FF000) >> 12;
    
    ret_info.pde = page_dir_offset;
    ret_info.pte = page_table_offset;
    ret_info.offset = virtualAddr & 0x00000FFF;
    ret_info.fault_address = virtualAddr;
    
    if( (page_directory[page_dir_offset] & PRESENT_BIT) == PRESENT_BIT ){ //if present Bit is set

        uint32_t *page_table;
        page_table = (uint32_t *)(page_directory[page_dir_offset] & 0xFFFFF000);
/*
        if(IS_PRESENT(*(page_table + page_table_offset) == 0)){
            printf("Makro sagt present\n");
        }else{
            printf("Makro sagt nicht present\n");
        }
*/
        if((*(page_table + page_table_offset) & PRESENT_BIT) != PRESENT_BIT){ //if present Bit is set
            printf("Debug step\n");
            if(page_counter < maxNumberOfPages){
                printf("Maximum number of pages has not been reached\n");
                 //=  *(page_table + page_table_offset) & 0xFFFFF000;
          
                uint32_t next_address = (uint32_t)(startaddress + page_counter++ * 0x1000 + PRESENT_BIT + RW_BIT);
                *(page_table + page_table_offset) = next_address;
                setPresentBit(page_dir_offset,page_table_offset,1);
     
                ret_info.physical_address = next_address & 0xFFFFF000;
   // Get next free Page and return new virtual Adress
            }else{
                printf("The Maximum Number of Pages have been reached already. This Page can't be reserved\n"); 
                //Get physical address of page you want to replace
                printf("Start PTE offset: %d\n", replace_pte_offset);
                do{
                    //printf(".\n");
                    replace_pte_offset++;
                    if(replace_pte_offset > 1023){
                        if(replace_pde_offset==1023){
                            replace_pde_offset=0;
                            replace_pte_offset=512;//Do not remove Kernel Pages
                        }else{
                            replace_pte_offset =0;
                            replace_pde_offset++;
                        }
                        
                    }
                }while(!isPresentBit(replace_pde_offset,replace_pte_offset));
                
                printf("Replace Offsets are %d %d\n",replace_pde_offset,replace_pte_offset);            
                //Get page table, in which is the page you want to replace and get the physical address of this page
                uint32_t *temp_page_table;
                temp_page_table = (uint32_t *)(page_directory[replace_pde_offset] & 0xFFFFF000);
                uint32_t replace_phy_address = *(temp_page_table + replace_pte_offset) & 0xFFFFF000;
                
                //Remove old page
                *(temp_page_table + replace_pte_offset) &= 0x0;
                
                /*Map new page
                 * Page Table is already present
                 */
                *(page_table + page_table_offset) = (replace_phy_address + RW_BIT + PRESENT_BIT);
                ret_info.physical_address = replace_phy_address && 0xFFFFF000;
                
                //Now set Present Bit in bitmap matrix
                setPresentBit(page_dir_offset,page_table_offset,1);
                
                
            }
        }
        ret_info.flags = *(page_table + page_table_offset) && 0x00000FFF;
    }else{
        printf("Segmentation Fault. Page Table is not present.\n");
        ret_info.physical_address = -1;
    }
    return &ret_info;
}

int setPresentBit(int pde_offset, int pte_offset, int bool){
    if(pde_offset < 0 || pde_offset > 1023 || pte_offset < 0 || pte_offset > 1023){
        printf("Offset is no in range\n");
        return 0;     
    }else{
        int index = pte_offset/32;
        if(bool == NOT_PRESENT_BIT){
            page_bitfield[pde_offset][index] &= (~(PRESENT_BIT << (pte_offset%32)));
        }else{
            page_bitfield[pde_offset][index] |= (PRESENT_BIT << (pte_offset%32));
        }
       // printf("PDE %d\t\t",pde_offset);
        //printf("PTE %d\t\t",index);
        //printf("Val %x\t\t",page_bitfield[pde_offset][index]);

        return 1;
    }
}

int isPresentBit(int pde_offset, int pte_offset){
    if(pde_offset < 0 || pde_offset > 1023 || pte_offset < 0 || pte_offset > 1023){
        printf("Offset is no in range\n");
        return 0;     
    }else{
        int index = pte_offset/32;
        return ((page_bitfield[pde_offset][index] & (PRESENT_BIT << (pte_offset%32)) ) != NOT_PRESENT_BIT);
    }
}




uint32_t* init_paging() {

    // Initialize Page Directory
    
    //Set Directory to blank
    for(int i=0; i<1024;i++){
        *(page_directory+i) = *(page_directory+i) & 0x00000000;
    }
    
    //set Bitfield to blank
    for(int i=0; i<1024;i++){
        for(int j=0; j<32;j++){
            page_bitfield[i][j]=0;
        }
    }
    
    //Copy Kernel to First Page Table
    //for the first MB
    for(int i = 0; i<256; i++){
       kernel_page_table[i] = (uint32_t)(i * 0x1000 + 3);
       setPresentBit(0,i,1);
    }
    *(page_directory + OFFSET_KERNEL_PT) = (uint32_t)kernel_page_table | PRESENT_BIT | RW_BIT;
    *(page_directory + OFFSET_PROGRAMM_PT) = (uint32_t)programm_page_table | PRESENT_BIT | RW_BIT;
    *(page_directory + OFFSET_STACK_PT) = (uint32_t)stack_page_table | PRESENT_BIT | RW_BIT;
   
    printf("Address of page Directory %p\n",page_directory);
    return page_directory;
}
