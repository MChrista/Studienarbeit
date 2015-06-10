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
uint32_t kernel_page_table[1024] __attribute__((aligned(0x1000)));
uint32_t programm_page_table[1024] __attribute__((aligned(0x1000)));
uint32_t stack_page_table[1024] __attribute__((aligned(0x1000)));

//General Parameters
int startaddress = 0x200000; //Startaddress for Physical Memory
int page_counter = 0;

int startOfStorage = 0x300000;
int storagePageCounter = 0;


const int maxNumberOfPages = 4; //Maximum Number of Pages
uint32_t page_bitfield[1024][32];
uint32_t page_addresses_on_storage[4][3];



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

struct page_fault_result * pageFault(int virtualAddr) {

    int page_dir_offset = (virtualAddr >> 22) & 0x3FF;
    int page_table_offset = (virtualAddr & 0x003FF000) >> 12;

    ret_info.pde = page_dir_offset;
    ret_info.pte = page_table_offset;
    ret_info.offset = virtualAddr & 0x00000FFF;
    ret_info.fault_address = virtualAddr;
    
    if ((page_directory[page_dir_offset] & PRESENT_BIT) == PRESENT_BIT) { //if present Bit is set
        uint32_t *page_table;
        page_table = (uint32_t *) (page_directory[page_dir_offset] & 0xFFFFF000);
        if ((*(page_table + page_table_offset) & PRESENT_BIT) != PRESENT_BIT) { //if present Bit is not set
            getPageFrame(page_table,page_dir_offset,page_table_offset);
        } else {
            //printf("There is no Page Fault\n\n");
            ret_info.flags = *(page_table + page_table_offset) & 0x00000FFF;
            ret_info.physical_address = *(page_table + page_table_offset) & 0xFFFFF000;
        }

    ret_info.flags = page_table[page_table_offset] & 0x00000FFF;
    } else {
        //printf("Segmentation Fault. Page Table is not present.\n");
        ret_info.physical_address = 0xFFFFFFFF;
        ret_info.flags = 0x0;
    }
    
    return &ret_info;
}

void 
getPageFrame(uint32_t *page_table,int pde ,int pte){
    if (page_counter < maxNumberOfPages) {
        uint32_t next_address = (uint32_t) (startaddress + page_counter++ * 0x1000 + PRESENT_BIT + RW_BIT);
        //If Storage bit is set
        if((*(page_table + pte) & 0x400) == 0x400){
            page_addresses_on_storage[page_counter-1][0] = pde;
            page_addresses_on_storage[page_counter-1][0] = pte;
            page_addresses_on_storage[page_counter-1][0] = *(page_table + pte) & 0xFFFFF000;
            loadPageFromStorage(next_address, *(page_table + pte) & 0xFFFFF000);
        }
        *(page_table + pte) = next_address;
        setPresentBit(pde, pte, 1);
        ret_info.physical_address = next_address & 0xFFFFF000;
        ret_info.flags = *(page_table + pte) & 0x00000FFF;
    } else {
        //printf("Replace Page\n"); 
        replacePage(pde, pte);

    }
}

void
loadPageFromStorage(uint32_t memory_address, uint32_t storage_address){
    
}

void
savePageToStorage(uint32_t memory_address, uint32_t storage_address){
    
}

int
setPresentBit(int pde_offset, int pte_offset, int bool) {
    if (pde_offset < 0 || pde_offset > 1023 || pte_offset < 0 || pte_offset > 1023) {
        //printf("Offset is no in range\n");
        return 0;
    } else {
        int index = pte_offset / 32;
        if (bool == NOT_PRESENT_BIT) {
            page_bitfield[pde_offset][index] &= (~(PRESENT_BIT << (pte_offset % 32)));
        } else {
            page_bitfield[pde_offset][index] |= (PRESENT_BIT << (pte_offset % 32));
        }
        return 1;
    }
}

int
isPresentBit(int pde_offset, int pte_offset) {
    if (pde_offset < 0 || pde_offset > 1023 || pte_offset < 0 || pte_offset > 1023) {
        //printf("Offset is no in range\n");
        return 0;
    } else {
        int index = pte_offset / 32;
        return ((page_bitfield[pde_offset][index] & (PRESENT_BIT << (pte_offset % 32))) != NOT_PRESENT_BIT);
    }
}

int
replacePage(int pde, int pte) {
    /* Implementation of NRU
     * 
    A=0, M=0 (nicht gelesen, nicht verändert)
    A=0, M=1 (nicht gelesen, aber verändert)
    A=1, M=0 (gelesen, aber nicht verändert)
    A=1, M=1 (gelesen und verändert)
     */
    uint32_t *temp_page_table;
    int start_pde, start_pte, counter_pde, counter_pte, class, flags, tmp_class;



    start_pde = replace_pde_offset;
    start_pte = replace_pte_offset;
    counter_pde = replace_pde_offset;
    counter_pte = replace_pte_offset;
    class = 4; //Start at highest class + 1 to fetch the first page with class 3 if all pages have class 3

    //printf("Start to search new replacing page%d %d\n", start_pde, start_pte);
    do {

        counter_pte++;
        if (counter_pte > 1023) {
            if (counter_pde == 1023) {
                counter_pde = 0;
                counter_pte = 512; //Do not remove Kernel Pages
            } else {
                counter_pte = 0;
                counter_pde++;
            }

        }
        if (isPresentBit(counter_pde, counter_pte)) {
            //printf("Found page at %d %d\n", counter_pde, counter_pte);
#ifdef __DHBW_KERNEL__
            temp_page_table = (uint32_t *) ((page_directory[counter_pde] & 0xFFFFF000) - (unsigned long) &LD_DATA_START);
#else
            temp_page_table = (uint32_t *) (page_directory[counter_pde] & 0xFFFFF000);
#endif
            flags = *(temp_page_table + counter_pte) & 0xFFF;
            tmp_class = getClassOfPage(flags);
            //printf("Flags %i\n", flags);
            //printf("Class of Page %i\n", tmp_class);
            if (class > tmp_class) {
                class = tmp_class;
                replace_pde_offset = counter_pde;
                replace_pte_offset = counter_pte;
            }
        }
        //printf("Bool of While %d\n",counter_pde != start_pde && counter_pte != start_pte);
    } while ((counter_pde != start_pde || counter_pte != start_pte) && (class != 0)); //Until walk through bitfield is complete


    //printf("Replace Offsets are %d %d\n", replace_pde_offset, replace_pte_offset);
    //Get page table, in which is the page you want to replace and get the physical address of this page

#ifdef __DHBW_KERNEL__
    temp_page_table = (uint32_t *) ((page_directory[counter_pde] & 0xFFFFF000) - (unsigned long) &LD_DATA_START);
#else
    temp_page_table = (uint32_t *) (page_directory[counter_pde] & 0xFFFFF000);
#endif
    uint32_t replace_phy_address = *(temp_page_table + replace_pte_offset) & 0xFFFFF000;

    //printf("PTE: %i PDE: %i Physical Address of Page: %x\n",page_dir_offset,page_table_offset, replace_phy_address);
    //printf("Check Bitfield Offsets: %i\n", isPresentBit(replace_pde_offset,replace_pte_offset));
    
    //Remove old page
    //Search for entry in page_addresses_on_storage
    /*
    int counter = 0;
    uint32_t storage_address = 0;
    while(counter < maxNumberOfPages && storage_address==0){
        if(page_addresses_on_storage[counter][0] == replace_pde_offset && page_addresses_on_storage[counter][1] == replace_pte_offset){
            //Page was already on Storage
            storage_address = page_addresses_on_storage[counter][2];
            page_addresses_on_storage[counter][0] = pde;
            page_addresses_on_storage[counter][1] = pte;
            page_addresses_on_storage[counter][2] = 0;
        }
    }
    if((*(temp_page_table + replace_pte_offset) & 0x40) == 0x40){ //If Dirty Bit is set
        //Copy page to storage
    }
    if((*(temp_page_table + replace_pte_offset) & 0x400) == 0x400){ //If page is already present on storage
        
    }
    */
    *(temp_page_table + replace_pte_offset) &= 0x0;
    setPresentBit(replace_pde_offset, replace_pte_offset, 0);

    /*Map new page
     * Page Table is already present
     */
    uint32_t *page_table;
#ifdef __DHBW_KERNEL__
    page_table = (uint32_t *) ((page_directory[pde] & 0xFFFFF000) - (unsigned long) &LD_DATA_START);
#else
    page_table = (uint32_t *) (page_directory[pde] & 0xFFFFF000);
#endif
    *(page_table + pte) = (replace_phy_address + RW_BIT + PRESENT_BIT);
    ret_info.physical_address = replace_phy_address & 0xFFFFF000;
    ret_info.flags = *(page_table + pte) & 0x00000FFF;
    //Now set Present Bit in bitmap matrix
    setPresentBit(pde, pte, 1);
    return 1;
}

int getClassOfPage(int flags) {
    //Bit 5: accesed
    //Bit 6: dirty
    if ((flags & 0x20) == 0x20) {
        if ((flags & 0x40) == 0x40) {
            return 3;
        } else {
            return 2;
        }
    } else {
        if ((flags & 0x40) == 0x40) {
            return 1;
        } else {
            return 0;
        }
    }
}

uint32_t*
init_paging() {

    // Initialize Page Directory

    //Set Directory to blank
    for (int i = 0; i < 1024; i++) {
        *(page_directory + i) = *(page_directory + i) & 0x00000000;
    }

    //set Bitfield to blank
    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < 32; j++) {
            page_bitfield[i][j] = 0;
        }
    }

    //Copy Kernel to First Page Table
    //for the first MB
    for (int i = 0; i < 256; i++) {
        kernel_page_table[i] = (uint32_t) (i * 0x1000 + 3);
        setPresentBit(0, i, 1);
    }
    *(page_directory + OFFSET_KERNEL_PT) = (uint32_t) kernel_page_table | PRESENT_BIT | RW_BIT;
    *(page_directory + OFFSET_PROGRAMM_PT) = (uint32_t) programm_page_table | PRESENT_BIT | RW_BIT;
    *(page_directory + OFFSET_STACK_PT) = (uint32_t) stack_page_table | PRESENT_BIT | RW_BIT;

    //printf("Address of page Directory %p\n",page_directory);
    return page_directory;
}
