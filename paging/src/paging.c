#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include "paging.h"

#define PRESENT_ON_STORAGE 0x400
#define DIRTY 0x040
#define ACCESSED  0x020
#define USER_MODE 0x004
#define MAX_NUMBER_OF_PAGES 4
#define MAX_NUMBER_OF_STORGAE_PAGES 256

#define PDE(addr) ((addr & 0xFFC00000) >> 22)
#define PTE(addr) ((addr & 0x003FF000) >> 12)

#ifdef __DHBW_KERNEL__
// Linear address of data segment, defined in ldscript
// use only in Kernel context with x86 segmentation
// being enabled
extern unsigned long LD_DATA_START;
extern unsigned long LD_IMAGE_START;
#endif
/*
 * Declaration of Page Directory and Page tables
 */
uint32_t page_directory[1024] __attribute__((aligned(0x1000)));
//Create Page Tables
uint32_t kernel_page_table[1024] __attribute__((aligned(0x1000)));
uint32_t programm_page_table[1024] __attribute__((aligned(0x1000)));
uint32_t stack_page_table[1024] __attribute__((aligned(0x1000)));

//General Parameters
int startaddress = 0x200000; //Startaddress for Physical Memory

//Can be set down, but not higher than die maximum number of pages
int memoryPageCounter = MAX_NUMBER_OF_PAGES;

uint32_t physicalMemoryBitfield[MAX_NUMBER_OF_PAGES];

int startOfStorage = 0x300000;
int storagePageCounter = 0;

//Page replace parameters
int replace_pde_offset = 0;
int replace_pte_offset = 512;

//marks reserved pages with an bit. This array is needed to easy find an page to replace
uint32_t page_bitfield[1024][32];

struct storageEntry {
    short pde;
    short pte;
    uint32_t storageAddress;
};

struct storageEntry storageBitfield[MAX_NUMBER_OF_STORGAE_PAGES];

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

    //If page table exists in page directory
    if ((page_directory[page_dir_offset] & PRESENT_BIT) == PRESENT_BIT) {

        uint32_t *page_table;
        page_table = (uint32_t *) (page_directory[page_dir_offset] & 0xFFFFF000);

        //If page is not present in page table
        if ((*(page_table + page_table_offset) & PRESENT_BIT) != PRESENT_BIT) {

            /* Left 20 bits are memory address
             * 
             */
            uint32_t memoryAddress = getPageFrame();

            memoryAddress &= 0xFFFFF000;

            //If present on storage bit is set, load page from storage in memory
            if ((*(page_table + page_table_offset) & PRESENT_ON_STORAGE) == PRESENT_ON_STORAGE) {
                int indexStorageBitfield = indexOfDiskAddrByPdePte(page_dir_offset,page_table_offset);
                printf("Index in storage bitfield %i\n",indexStorageBitfield);
                //Update page in storageBitfield TODO
                //storageBitfield[indexStorageBitfield].pde = page_dir_offset;
                //storageBitfield[indexStorageBitfield].pte = page_table_offset;
                //storageBitfield[indexStorageBitfield].storageAddress = storageAddressOfPage;
                copyPage(storageBitfield[indexStorageBitfield].storageAddress, memoryAddress);
#ifdef __DHBW_KERNEL__
                memset(storageBitfield[indexStorageBitfield].storageAddress,0,0x1000);
#endif                
                storageBitfield[indexStorageBitfield].pde = 0;
                storageBitfield[indexStorageBitfield].pte = 0;
                
                
                //memoryAddress |= PRESENT_ON_STORAGE;
            }
            //Set flags on memory address
            memoryAddress = memoryAddress | PRESENT_BIT | RW_BIT | USER_MODE;
            *(page_table + page_table_offset) = memoryAddress;
            setPresentBit(page_dir_offset, page_table_offset, 1);
            
            //Set Bit in memory bitfield
            int indexInMemoryBitfield = (memoryAddress % startaddress) >> 12;
            physicalMemoryBitfield[indexInMemoryBitfield] = 1;
            
            ret_info.physical_address = memoryAddress & 0xFFFFF000;
            ret_info.flags = *(page_table + page_table_offset) & 0x00000FFF;

        } else {
            //printf("There is no Page Fault\n\n");
            ret_info.flags = *(page_table + page_table_offset) & 0x00000FFF;
            ret_info.physical_address = *(page_table + page_table_offset) & 0xFFFFF000;
        }

    } else {
        //printf("Segmentation Fault. Page Table is not present.\n");
        ret_info.physical_address = 0xFFFFFFFF;
        ret_info.flags = 0x0;
    }
    return &ret_info;
}

uint32_t
getPageFrame() {
    /*
     * Returns a memory address
     * left 20 bits are memory address
     * right 12 bits are the index in storageBitfield
     */
    
    //Maximum allowed pages in memory at actual time.
    int limit;
    if(memoryPageCounter < MAX_NUMBER_OF_PAGES){
        limit = memoryPageCounter;
    }else{
        limit = MAX_NUMBER_OF_PAGES;
    }
    
    for (int i = 0; i < limit; i++) {
        if (physicalMemoryBitfield[i] == 0) {
            uint32_t next_address = (uint32_t) (startaddress + i * 0x1000);
            physicalMemoryBitfield[i] = 1;
            return next_address;
        }
    }
    //There is no page left
    //get virtual address of page to replace
    uint32_t virtAddr = getAddressOfPageToReplace();
    uint32_t memoryAddress = swap(virtAddr);
    return memoryAddress;
}

void copyPage(uint32_t src_address, uint32_t dst_address) {
#ifdef __DHBW_KERNEL__
    memcpy(dst_address, src_address, 0x1000);
#else
    printf("Copying page from 0x%08X to 0x%08X.\n", src_address, dst_address);
#endif

}

int indexOfDiskAddrByPdePte(uint32_t pde, uint32_t pte) {
    for (int i = 0; i < MAX_NUMBER_OF_STORGAE_PAGES; i++) {
        if (storageBitfield[i].pde == pde && storageBitfield[i].pte == pte) {
            return i;
        }
    }
    return -1;
}

uint32_t getFreeFrameOnDisk() {
    int indexOfFreeFrame = indexOfDiskAddrByPdePte(0, 0);
    printf("Index of Frame on Disk: %i\n", indexOfFreeFrame);
    if (indexOfFreeFrame >= 0) {
        return (uint32_t) (startOfStorage + indexOfFreeFrame * 0x1000);
    }
    return -1;
}

int getIndexOfFrameOnDisk(uint32_t storageAddr) {
    int indexStorageBitfield = (storageAddr % startOfStorage) >> 12;
    return indexStorageBitfield;
}

uint32_t swap(uint32_t virtAddr) {

    // Compute Parameters
    int pde = PDE(virtAddr);
    int pte = PTE(virtAddr);

    //printf("Swap:\nPDE: %x PTE: %x\n",pde,pte);
    uint32_t storageAddr;
    uint32_t * page_table = (uint32_t *) (page_directory[pde] & 0xFFFFF000);
    uint32_t memoryAddr = page_table[pte] & 0xFFFFF000;
    int flags = page_table[pte] & 0xFFF;

    

    // Check if page to swap is on disk
    if ((flags & PRESENT_ON_STORAGE) == PRESENT_ON_STORAGE) {
        print_debug("\nSwap with page in storage\n");
        //printf("Memory Address is %08X\n\n", memoryAddr);
        // Check if page was modified, only save it then
        if ((flags & DIRTY) == DIRTY) {
            int pageAddrOnStorageIndex = indexOfDiskAddrByPdePte(pde, pte);
            // Get address of page copy on disk
            storageAddr = storageBitfield[pageAddrOnStorageIndex].storageAddress;
            // Overwrite copy on disk with modified page 
            copyPage(memoryAddr, storageAddr);
        }
    } else {
        print_debug("\nSwap without page on storage\n");
        // Get free storage address to save page to
        storageAddr = getFreeFrameOnDisk();
        int index = getIndexOfFrameOnDisk(storageAddr);
        storageBitfield[index].pde = pde;
        storageBitfield[index].pte = pte;
        storageBitfield[index].storageAddress = storageAddr;
        copyPage(memoryAddr, storageAddr);
    }

    // Store disk address of page copy in its page table entry.
    page_table[pte] |= PRESENT_ON_STORAGE;

    // Reset present bit
    setPresentBit(pde, pte, 0);
    
    //Reset in memory Bitfield
    int indexInMemoryBitfield = (memoryAddr % startaddress) >> 12;
    printf("Before reseting bitfield entry with index: %d\n", indexInMemoryBitfield);
    physicalMemoryBitfield[indexInMemoryBitfield] = 0;
    page_table[pte] &= 0xFFFFFFFE;

    return memoryAddr;
}

int
setPresentBit(int pde_offset, int pte_offset, int bool) {
    if (pde_offset < 0 || pde_offset > 1023 || pte_offset < 0 || pte_offset > 1023) {
        //Offsets are not in range
        return 0;
    } else {
        int index = pte_offset / 32;
        if (bool == NOT_PRESENT_BIT) {
            //Set 0 in bitfield
            page_bitfield[pde_offset][index] &= (~(PRESENT_BIT << (pte_offset % 32)));
        } else {
            //Set 1 in bitfield
            page_bitfield[pde_offset][index] |= (PRESENT_BIT << (pte_offset % 32));
        }
        return 1;
    }
}

void
freePageInMemory(int pde, int pte){
    uint32_t virtAddr = 0;
    virtAddr |= pde << 22;
    virtAddr |= pte << 12;
    swap(virtAddr);
}

void
freeAllPages(){
    //For all present bits, do free page in Memory
    for(int pde=0; pde<1024; pde++){
        for(int pte=0; pte<1024;pte++){
            if(isPresentBit(pde,pte)){
                freePageInMemory(pde,pte);
            }
        }
    }
}

int
isPresentBit(int pde_offset, int pte_offset) {
    if (pde_offset < 0 || pde_offset > 1023 || pte_offset < 0 || pte_offset > 1023) {
        //Offsets are not in range
        return 0;
    } else {
        int index = pte_offset / 32;
        return ((page_bitfield[pde_offset][index] & (PRESENT_BIT << (pte_offset % 32))) != NOT_PRESENT_BIT);
    }
}

uint32_t
getAddressOfPageToReplace() {
    /* Implementation of NRU
     * 
    A=0, M=0 (nicht gelesen, nicht verändert)
    A=0, M=1 (nicht gelesen, aber verändert)
    A=1, M=0 (gelesen, aber nicht verändert)
    A=1, M=1 (gelesen und verändert)
     */
    uint32_t *temp_page_table;
    int start_pde;
    int start_pte;
    int counter_pde;
    int counter_pte;
    int class;
    int flags;
    int tmp_class;

    //Save pde and pte of last replace
    start_pde = replace_pde_offset;
    start_pte = replace_pte_offset;
    //Inititialize counter on offsets from last replace
    counter_pde = replace_pde_offset;
    counter_pte = replace_pte_offset;
    //Start at highest class + 1 to fetch the first page with class 3 if all pages have class 3
    class = 4;

    //Start to search in bitfield for present pages until one cycle is through or a page with class 0 is found
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
            //Found present page
#ifdef __DHBW_KERNEL__
            temp_page_table = (uint32_t *) ((page_directory[counter_pde] & 0xFFFFF000) - (unsigned long) &LD_DATA_START);
#else
            temp_page_table = (uint32_t *) (page_directory[counter_pde] & 0xFFFFF000);
#endif
            flags = *(temp_page_table + counter_pte) & 0xFFF;
            tmp_class = getClassOfPage(flags);
            //Remove access bit
            *(temp_page_table + counter_pte) &= 0xFFFFFFDF;
            //If class of page is lower than actual class, save pde and pte
            if (class > tmp_class) {
                class = tmp_class;
                replace_pde_offset = counter_pde;
                replace_pte_offset = counter_pte;
            }
        }
        //printf("Bool of While %d\n",counter_pde != start_pde && counter_pte != start_pte);
    } while ((counter_pde != start_pde || counter_pte != start_pte) && (class != 0)); //Until walk through bitfield is complete



    //Create a virtual address with the indices of the page which is to replace
    uint32_t virtAddr = 0;
    virtAddr |= replace_pde_offset << 22;
    virtAddr |= replace_pte_offset << 12;
    return virtAddr;
}

int getClassOfPage(int flags) {
    //Bit 5: accesed
    //Bit 6: dirty
    if ((flags & ACCESSED) == ACCESSED) {
        if ((flags & DIRTY) == DIRTY) {
            return 3;
        } else {
            return 2;
        }
    } else {
        if ((flags & DIRTY) == DIRTY) {
            return 1;
        } else {
            return 0;
        }
    }
}

void print_debug(char*str) {
#ifdef __DHBW_KERNEL__

#else
    printf("%s", str);
#endif
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

    //set physical memory bitfield to blank
    for (int i = 0; i < MAX_NUMBER_OF_PAGES; i++) {
        physicalMemoryBitfield[i] = 0;
    }

    //set storage bitfield to blank
    for (int i = 0; i < MAX_NUMBER_OF_STORGAE_PAGES; i++) {
        storageBitfield[i].pde = 0;
        storageBitfield[i].pte = 0;
        storageBitfield[i].storageAddress = 0;
    }

    //Copy Kernel to First Page Table
    //for the first MB
    for (int i = 0; i < 256; i++) {
#ifdef __DHBW_KERNEL__
        if(i >= (LD_IMAGE_START >> 12) && i < (LD_DATA_START >> 12)){
            kernel_page_table[i] = (uint32_t) (i * 0x1000 + PRESENT_BIT);
        }else{
            kernel_page_table[i] = (uint32_t) (i * 0x1000 + PRESENT_BIT + RW_BIT + USER_MODE);
        }
#else
        if(i >= (0x10000 >> 12) && i < (0x20000 >> 12)){
            kernel_page_table[i] = (uint32_t) (i * 0x1000 + PRESENT_BIT);
        }else{
            kernel_page_table[i] = (uint32_t) (i * 0x1000 + PRESENT_BIT + RW_BIT + USER_MODE);
        }
#endif
        setPresentBit(0, i, 1);
    }
    *(page_directory + OFFSET_KERNEL_PT) = (uint32_t) kernel_page_table | PRESENT_BIT | RW_BIT | USER_MODE;
    *(page_directory + OFFSET_PROGRAMM_PT) = (uint32_t) programm_page_table | PRESENT_BIT | RW_BIT | USER_MODE;
    *(page_directory + OFFSET_STACK_PT) = (uint32_t) stack_page_table | PRESENT_BIT | RW_BIT | USER_MODE;

    //printf("Address of page Directory %p\n",page_directory);
    return page_directory;
}
