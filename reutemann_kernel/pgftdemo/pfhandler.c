#include "pfhandler.h"


#ifdef __DHBW_KERNEL__
#include<stdarg.h>
#include "kprintf.h"
// Linear address of data segment, defined in ldscript
// use only in Kernel context with x86 segmentation
// being enabled
extern uint32_t LD_DATA_START;
extern uint32_t LD_IMAGE_START;
#else
#include<stdarg.h>
#include <stdio.h>
int kprintf(const char*, ...);
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
uint32_t startaddress = 0x200000; //Startaddress for Physical Memory

//Can be set down, but not higher than die maximum number of pages
uint32_t memoryPageCounter = PAGES_PHYSICAL_NUM;



uint32_t startOfStorage = 0x300000;
uint32_t storagePageCounter = 0;

//Page replace parameters
uint32_t replace_pde_offset = 0;
uint32_t replace_pte_offset = 512;

uint32_t dbg_ft_addr;

struct pageEntry {
    uint32_t pde;
    uint32_t pte;
    uint32_t memAddr;
};

struct pageEntry storageBitfield[PAGES_SWAPPED_NUM];
struct pageEntry physicalMemoryBitfield[PAGES_PHYSICAL_NUM];

static pg_struct_t pg_struct;


uint32_t setPresentBit(uint32_t, uint32_t, uint32_t);
uint32_t removePresentBit(uint32_t, uint32_t);
uint32_t isPresentBit(uint32_t, uint32_t);
uint32_t getClassOfPage(uint32_t);
uint32_t getAddressOfPageToReplace();
uint32_t isPresentBit(uint32_t, uint32_t);
uint32_t getPageFrame();
uint32_t swap(uint32_t virtAddr);
uint32_t getFreeMemoryAddress();
uint32_t getVirtAddrOfFrameOnDisk(uint32_t, uint32_t);
uint32_t getIndexInStorageBitfield(uint32_t, uint32_t);
void freePageInMemory(uint32_t, uint32_t);
void copyPage(uint32_t, uint32_t);
void clearPage(uint32_t);
void invalidate_addr(uint32_t);

pg_struct_t *
pfhandler(uint32_t ft_addr) {

    int page_dir_offset = (ft_addr >> 22) & 0x3FF;
    int page_table_offset = (ft_addr & 0x003FF000) >> 12;

    dbg_ft_addr = ft_addr;

    pg_struct.pde = page_dir_offset;
    pg_struct.pte = page_table_offset;
    pg_struct.off = ft_addr & 0x00000FFF;
    pg_struct.ft_addr = ft_addr;
    pg_struct.vic_addr = 0xFFFFFFFF;
    pg_struct.sec_addr = 0xFFFFFFFF;

    //If page table exists in page directory
    if ((page_directory[page_dir_offset] & PAGE_IS_PRESENT) == PAGE_IS_PRESENT) {

        uint32_t *page_table;
#ifdef __DHBW_KERNEL__
        page_table = (uint32_t *) ((page_directory[page_dir_offset] & 0xFFFFF000) - (uint32_t) & LD_DATA_START);
#else
        page_table = (uint32_t *) (page_directory[page_dir_offset] & 0xFFFFF000);
#endif

        //If page is not present in page table
        if ((*(page_table + page_table_offset) & PAGE_IS_PRESENT) != PAGE_IS_PRESENT) {
            /* Left 20 bits are memory address
             * 
             */
            uint32_t memoryAddress = getPageFrame();
            memoryAddress &= 0xFFFFF000;


            //TODO: Save swap bit
            memoryAddress = memoryAddress | PAGE_IS_PRESENT | PAGE_IS_RW | PAGE_IS_USER;

            if ((*(page_table + page_table_offset) & PAGE_IS_SWAPPED) == PAGE_IS_SWAPPED) {
                memoryAddress |= PAGE_IS_SWAPPED;
            }

            *(page_table + page_table_offset) = memoryAddress;
            setPresentBit(page_dir_offset, page_table_offset, (memoryAddress & 0xFFFFF000));

            //If present on storage bit is set, load page from storage in memory
            if ((*(page_table + page_table_offset) & PAGE_IS_SWAPPED) == PAGE_IS_SWAPPED) {

                uint32_t strVirtAddr = getVirtAddrOfFrameOnDisk(page_dir_offset, page_table_offset);

                copyPage(strVirtAddr, ft_addr & PAGE_ADDR_MASK);
                //Remove Dirty Bit, because this page wasn't changed
                *(page_table + page_table_offset) &= 0xFFFFFFBF;
            }
            //Set flags on memory address

            pg_struct.ph_addr = memoryAddress & 0xFFFFF000;
            pg_struct.flags = *(page_table + page_table_offset) & 0x00000FFF;

        } else {
            //printf("There is no Page Fault\n\n");
            pg_struct.flags = *(page_table + page_table_offset) & 0x00000FFF;
            pg_struct.ph_addr = *(page_table + page_table_offset) & 0xFFFFF000;
        }

    } else {
        //printf("Segmentation Fault. Page Table is not present.\n");
        pg_struct.ph_addr = 0xFFFFFFFF;
        pg_struct.flags = 0x0;
    }
    return &pg_struct;
} //END OF PFHANDLER

//==============================================================================
//START OF MEMORY FUNCTIONS//
//==============================================================================

uint32_t
getPageFrame() {
    /*
     * Returns a memory address
     * left 20 bits are memory address
     * right 12 bits are the index in storageBitfield
     */

    //Maximum allowed pages in memory at actual time.
    uint32_t memoryAddress;
    memoryAddress = getFreeMemoryAddress();
    if (memoryAddress != INVALID_ADDR) {
        return memoryAddress;
    }
    //There is no page left
    //get virtual address of page to replace
    uint32_t virtAddr = getAddressOfPageToReplace();
    pg_struct.vic_addr = virtAddr;
    memoryAddress = swap(virtAddr);
    return memoryAddress;
}

uint32_t
setPresentBit(uint32_t pde_offset, uint32_t pte_offset, uint32_t memAddr) {
    for (uint32_t i = 0; i < memoryPageCounter; i++) {
        if (physicalMemoryBitfield[i].pde == 0 && physicalMemoryBitfield[i].pte == 0) {
            physicalMemoryBitfield[i].pde = pde_offset;
            physicalMemoryBitfield[i].pte = pte_offset;
            physicalMemoryBitfield[i].memAddr = memAddr;
            return 1;
        }
    }
    return 0;
}

uint32_t
removePresentBit(uint32_t pde_offset, uint32_t pte_offset) {
    for (uint32_t i = 0; i < PAGES_PHYSICAL_NUM; i++) {
        if (physicalMemoryBitfield[i].pde == pde_offset && physicalMemoryBitfield[i].pte == pte_offset) {
            physicalMemoryBitfield[i].pde = 0;
            physicalMemoryBitfield[i].pte = 0;
            physicalMemoryBitfield[i].memAddr = 0;
            return 1;
        }
    }
    return 0;
}

uint32_t
isPresentBit(uint32_t pde_offset, uint32_t pte_offset) {
    for (uint32_t i = 0; i < memoryPageCounter; i++) {
        if (physicalMemoryBitfield[i].pde == pde_offset && physicalMemoryBitfield[i].pte == pte_offset) {
            return 1;
        }
    }
    return 0;
}

uint32_t
getFreeMemoryAddress() {
    for (uint32_t i = 0; i < PAGES_PHYSICAL_NUM; i++) {
        if (physicalMemoryBitfield[i].pde == 0 && physicalMemoryBitfield[i].pte == 0) {
            return ((uint32_t) (PAGES_PHYSICAL_START + i * PAGE_SIZE));
        }
    }
    return INVALID_ADDR;
}

void
freePageInMemory(uint32_t pde, uint32_t pte) {
    uint32_t virtAddr = 0;
    virtAddr |= pde << 22;
    virtAddr |= pte << 12;
    swap(virtAddr);
}

void
freeAllPages() {
    uint32_t pde;
    uint32_t pte;
    uint32_t virtAddr = 0;
    uint32_t *page_table;

    //For all present bits, do free page in Memory
    for (uint32_t i = 0; i < PAGES_PHYSICAL_NUM; i++) {
        pde = physicalMemoryBitfield[i].pde;
        pte = physicalMemoryBitfield[i].pte;
        if (isPresentBit(pde, pte)) {

            virtAddr |= pde << PDE_SHIFT;
            virtAddr |= pte << PTE_SHIFT;
            clearPage(virtAddr);

#ifdef __DHBW_KERNEL__
            page_table = (uint32_t *) ((page_directory[pde] & 0xFFFFF000) - (uint32_t) & LD_DATA_START);
#else
            page_table = (uint32_t *) (page_directory[pde] & 0xFFFFF000);
#endif
            page_table[pte] &= 0xFFFFFFFE;
#ifdef __DHBW_KERNEL__
            invalidate_addr(virtAddr);
#endif
            removePresentBit(pde, pte);

        }
    }
}

//==============================================================================
//END OF MEMORY FUNCTIONS//
//==============================================================================

uint32_t dbg_copy_src_addr;
uint32_t dbg_copy_dst_addr;
uint32_t unsusedPar;

void copyPage(uint32_t src_address, uint32_t dst_address) {
    //unsusedPar = src_address + dst_address;
#ifdef __DHBW_KERNEL__
    uint32_t *src = (uint32_t *) ((src_address & PAGE_ADDR_MASK) - (uint32_t) & LD_DATA_START);
    uint32_t *dst = (uint32_t *) ((dst_address & PAGE_ADDR_MASK) - (uint32_t) & LD_DATA_START);
    for (int i = 0; i < (PAGE_SIZE / 4); i++) {
        *(dst++) = *(src++);
    }
    //src_address &= 1;
    //dst_address &= 1;

#else
    unsusedPar = src_address + dst_address;
#endif

}

void clearPage(uint32_t address) {
#ifdef __DHBW_KERNEL__
    uint32_t *addr = (uint32_t *) ((address & PAGE_ADDR_MASK) - (uint32_t) & LD_DATA_START);
    for (int i = 0; i < (PAGE_SIZE / 4); i++) {
        *(addr++) = 0x00000000;
    }
#else
    address &= 1;
#endif
}



//==============================================================================
//START OF DISK FUNCTIONS//
//==============================================================================

uint32_t getVirtAddrOfFrameOnDisk(uint32_t pde, uint32_t pte) {
    for (uint32_t i = 0; i < PAGES_SWAPPED_NUM; i++) {
        if (storageBitfield[i].pde == pde && storageBitfield[i].pte == pte) {
            //PDE is zero
            return ((SWAPPED_START_ADDR + i) << PTE_SHIFT);
        }
    }
    return INVALID_ADDR;
}

uint32_t getIndexInStorageBitfield(uint32_t pde, uint32_t pte) {
    for (uint32_t i = 0; i < PAGES_SWAPPED_NUM; i++) {
        if (storageBitfield[i].pde == pde && storageBitfield[i].pte == pte) {
            //PDE is zero
            return i;
        }
    }
    return INVALID_INDEX;
}

uint32_t dbg_swap_addr;
uint32_t dbg_swap_result;

uint32_t swap(uint32_t virtAddr) {

    // Compute Parameters
    int pde = PDE(virtAddr);
    int pte = PTE(virtAddr);

    //printf("Swap:\nPDE: %x PTE: %x\n",pde,pte);
    uint32_t storageAddr;
    dbg_swap_addr = virtAddr;

#ifdef __DHBW_KERNEL__
    uint32_t * page_table = (uint32_t *) ((page_directory[pde] & 0xFFFFF000) - (uint32_t) & LD_DATA_START);
#else
    uint32_t * page_table = (uint32_t *) (page_directory[pde] & 0xFFFFF000);
#endif

    uint32_t memoryAddr = page_table[pte] & 0xFFFFF000;
    int flags = page_table[pte] & 0xFFF;



    // Check if page to swap is on disk
    if ((flags & PAGE_IS_SWAPPED) == PAGE_IS_SWAPPED) {
        //print_debug("\nSwap with page in storage\n");
        //printf("Memory Address is %08X\n\n", memoryAddr);
        kprintf("Page is swapped\n");

        // Check if page was modified, only save it then
        if ((flags & PAGE_IS_DIRTY) == PAGE_IS_DIRTY) {
            //int pageAddrOnStorageIndex = indexOfDiskAddrByPdePte(pde, pte);
            kprintf("Page is diry and swapped\n");
            // Get address of page copy on disk
            //storageAddr = storageBitfield[pageAddrOnStorageIndex].memAddr;
            storageAddr = getVirtAddrOfFrameOnDisk(pde, pte);

            if (storageAddr != INVALID_ADDR) {
                // Overwrite copy on disk with modified page 
                copyPage(virtAddr, storageAddr);
                pg_struct.sec_addr = storageAddr;
            }


        }
    } else {
        //print_debug("\nSwap without page on storage\n");
        // Get free storage address to save page to

        //printf("Storage Address is: %08x\n", storageAddr);

        //Get free page on Storage
        uint32_t index = getIndexInStorageBitfield(0, 0);
        storageBitfield[index].pde = pde;
        storageBitfield[index].pte = pte;

        storageAddr = getVirtAddrOfFrameOnDisk(pde, pte);
        pg_struct.sec_addr = storageAddr;
        copyPage(virtAddr, storageAddr);
        clearPage(virtAddr);

    }

    // Store disk address of page copy in its page table entry.
    page_table[pte] |= PAGE_IS_SWAPPED;

    // Reset present bit
    removePresentBit(pde, pte);

    page_table[pte] &= 0xFFFFFFFE;

    dbg_swap_result = memoryAddr;

    return memoryAddr;

} //END OF SWAP

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
    uint32_t start_pde;
    uint32_t start_pte;
    uint32_t counter_pde;
    uint32_t counter_pte;
    uint32_t class;
    uint32_t flags;
    uint32_t tmp_class;
    uint32_t virtAddr;

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
            //First invalidate TLB to have actual flags in page entry
            virtAddr = 0;
            virtAddr |= counter_pde << 22;
            virtAddr |= counter_pte << 12;
#ifdef __DHBW_KERNEL__
            invalidate_addr(virtAddr);
            temp_page_table = (uint32_t *) ((page_directory[counter_pde] & 0xFFFFF000) - (uint32_t) & LD_DATA_START);
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
    virtAddr = 0;
    virtAddr |= replace_pde_offset << 22;
    virtAddr |= replace_pte_offset << 12;
    return virtAddr;
} //END OF NRU

uint32_t getClassOfPage(uint32_t flags) {
    //Bit 5: accesed
    //Bit 6: dirty
    if ((flags & PAGE_IS_ACCESSED) == PAGE_IS_ACCESSED) {
        if ((flags & PAGE_IS_DIRTY) == PAGE_IS_DIRTY) {
            return 3;
        } else {
            return 2;
        }
    } else {
        if ((flags & PAGE_IS_DIRTY) == PAGE_IS_DIRTY) {
            return 1;
        } else {
            return 0;
        }
    }
}

//==============================================================================
//END OF DISK FUNCTIONS//
//==============================================================================




#ifndef __DHBW_KERNEL__

int kprintf(const char * format, ...) {
    int rtn = 0;
    va_list args;
    va_start(args, format);
    rtn = vprintf(format, args);
    va_end(args);
    return rtn;

}
#endif

//==============================================================================
//Initialize paging//
//==============================================================================

uint32_t*
init_paging() {

    // Initialize Page Directory

    //Set Directory to blank
    for (uint32_t i = 0; i < 1024; i++) {
        *(page_directory + i) = *(page_directory + i) & 0x00000000;
    }

    //set physical memory bitfield to blank
    for (uint32_t i = 0; i < PAGES_PHYSICAL_NUM; i++) {
        physicalMemoryBitfield[i].pde = 0;
        physicalMemoryBitfield[i].pte = 0;
        physicalMemoryBitfield[i].memAddr = 0;
    }

    //Copy Kernel to First Page Table
    //for the first MB
    for (uint32_t i = 0; i < 256; i++) {
#ifdef __DHBW_KERNEL__
        if (i >= (LD_IMAGE_START >> 12) && i < (LD_DATA_START >> 12)) {
            kernel_page_table[i] = (uint32_t) (i * 0x1000 + PAGE_IS_PRESENT);
        } else if (i > (LD_DATA_START >> 12)) {
            kernel_page_table[i] = (uint32_t) (i * 0x1000 + PAGE_IS_PRESENT + PAGE_IS_RW);
        } else {
            kernel_page_table[i] = (uint32_t) (i * 0x1000 + PAGE_IS_PRESENT + PAGE_IS_RW + PAGE_IS_USER);
        }

#else
        if (i >= (0x10000 >> 12) && i < (0x20000 >> 12)) {
            kernel_page_table[i] = (uint32_t) (i * 0x1000 + PAGE_IS_PRESENT);
        } else if (i > (0x20000 >> 12)) {
            kernel_page_table[i] = (uint32_t) (i * 0x1000 + PAGE_IS_PRESENT + PAGE_IS_RW);
        } else {
            kernel_page_table[i] = (uint32_t) (i * 0x1000 + PAGE_IS_PRESENT + PAGE_IS_RW + PAGE_IS_USER);
        }
#endif
    }

    //Map swap space
    for (uint32_t i = SWAP_START_ADDR; i < (SWAP_START_ADDR + PAGES_SWAPPED_NUM); i++) {
        storageBitfield[i - SWAP_START_ADDR].memAddr = (uint32_t) (PAGES_SWAPPED_START + (i - SWAP_START_ADDR) * PAGE_SIZE);
        storageBitfield[i - SWAP_START_ADDR].pde = 0;
        storageBitfield[i - SWAP_START_ADDR].pte = 0;
        kernel_page_table[i] = (uint32_t) (PAGES_SWAPPED_START + (i - SWAP_START_ADDR) * PAGE_SIZE + PAGE_IS_PRESENT + PAGE_IS_RW + PAGE_IS_USER);
    }


    *(page_directory + PDE_KERNEL_PT) = (uint32_t) kernel_page_table | PAGE_IS_PRESENT | PAGE_IS_RW | PAGE_IS_USER;
    *(page_directory + PDE_PROGRAMM_PT) = (uint32_t) programm_page_table | PAGE_IS_PRESENT | PAGE_IS_RW | PAGE_IS_USER;
    *(page_directory + PDE_STACK_PT) = (uint32_t) stack_page_table | PAGE_IS_PRESENT | PAGE_IS_RW | PAGE_IS_USER;

#ifdef __DHBW_KERNEL__
    *(page_directory + PDE_KERNEL_PT) += (uint32_t) & LD_DATA_START;
    *(page_directory + PDE_PROGRAMM_PT) += (uint32_t) & LD_DATA_START;
    *(page_directory + PDE_STACK_PT) += (uint32_t) & LD_DATA_START;
#endif
    //printf("Address of page Directory %p\n",page_directory);
    return page_directory;
} //END OF INIT PAGING



