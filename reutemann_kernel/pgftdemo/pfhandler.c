#define PRESENT_ON_STORAGE 0x400
#define DIRTY 0x040
#define ACCESSED  0x020
#define USER_MODE 0x004
#define MAX_NUMBER_OF_PAGES 4
#define MAX_NUMBER_OF_STORGAE_PAGES 256
#define TESTBITFIELD 0
#define PRESENT_BIT 1
#define NOT_PRESENT_BIT 0
#define RW_BIT 2
#define OFFSET_KERNEL_PT 0
#define OFFSET_PROGRAMM_PT 32
#define OFFSET_STACK_PT 1023
#define RW_BIT 2

#define PDE(addr) (((addr) & 0xFFC00000) >> 22)
#define PTE(addr) (((addr) & 0x003FF000) >> 12)

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
unsigned long page_directory[1024] __attribute__((aligned(0x1000)));
//Create Page Tables
unsigned long kernel_page_table[1024] __attribute__((aligned(0x1000)));
unsigned long programm_page_table[1024] __attribute__((aligned(0x1000)));
unsigned long stack_page_table[1024] __attribute__((aligned(0x1000)));

//General Parameters
int startaddress = 0x200000; //Startaddress for Physical Memory

//Can be set down, but not higher than die maximum number of pages
int memoryPageCounter = MAX_NUMBER_OF_PAGES;

unsigned long physicalMemoryBitfield[MAX_NUMBER_OF_PAGES];

int startOfStorage = 0x300000;
int storagePageCounter = 0;

//Page replace parameters
int replace_pde_offset = 0;
int replace_pte_offset = 512;

//marks reserved pages with an bit. This array is needed to easy find an page to replace
unsigned long page_bitfield[1024][32];

//against unused parameters
unsigned long unusedPar;

typedef struct pg_struct {
    unsigned long ft_addr; // faulting linear memory address
    unsigned long pde; // Page Directory Entry
    unsigned long pte; // Page Table Entry
    unsigned long off; // Page Offset
    unsigned long ph_addr; // Physical Address
    unsigned long flags; // Flags = TBD
    unsigned long vic_addr; // victim page address
    unsigned long sec_addr; // secondary storage address
} pg_struct_t;

struct storageEntry {
    short pde;
    short pte;
    unsigned long storageAddress;
};

struct storageEntry storageBitfield[MAX_NUMBER_OF_STORGAE_PAGES];

unsigned long* init_paging();
int setPresentBit(int, int, int);
int isPresentBit(int, int);
int getClassOfPage(int);
void copyPage(unsigned long, unsigned long);
unsigned long getAddressOfPageToReplace();
int isPresentBit(int, int);
unsigned long getPageFrame();
unsigned long swap(unsigned long);
int getIndexOfFrameOnDisk(unsigned long);
int indexOfDiskAddrByPdePte(unsigned short, unsigned short);
void freePageInMemory(int, int);
void freeAllPages();

static pg_struct_t pg_struct;
<<<<<<< HEAD
=======

unsigned long dbg_ft_addr;
>>>>>>> hwpaging

pg_struct_t *
pfhandler(unsigned long ft_addr) {

    int page_dir_offset = (ft_addr >> 22) & 0x3FF;
    int page_table_offset = (ft_addr & 0x003FF000) >> 12;

<<<<<<< HEAD
=======
    dbg_ft_addr = ft_addr;

>>>>>>> hwpaging
    pg_struct.pde = page_dir_offset;
    pg_struct.pte = page_table_offset;
    pg_struct.off = ft_addr & 0x00000FFF;
    pg_struct.ft_addr = ft_addr;
<<<<<<< HEAD
=======
    pg_struct.vic_addr = 0xFFFFFFFF;
    pg_struct.sec_addr = 0xFFFFFFFF;
>>>>>>> hwpaging

    //If page table exists in page directory
    if ((page_directory[page_dir_offset] & PRESENT_BIT) == PRESENT_BIT) {

        unsigned long *page_table;
#ifdef __DHBW_KERNEL__
        page_table = (unsigned long *) ((page_directory[page_dir_offset] & 0xFFFFF000) - (unsigned long) &LD_DATA_START);
#else
        page_table = (unsigned long *) (page_directory[page_dir_offset] & 0xFFFFF000);
#endif

        //If page is not present in page table
        if ((*(page_table + page_table_offset) & PRESENT_BIT) != PRESENT_BIT) {

            /* Left 20 bits are memory address
             * 
             */
            unsigned long memoryAddress = getPageFrame();
            
            memoryAddress &= 0xFFFFF000;
            
            //If present on storage bit is set, load page from storage in memory
            if ((*(page_table + page_table_offset) & PRESENT_ON_STORAGE) == PRESENT_ON_STORAGE) {
                int indexStorageBitfield = indexOfDiskAddrByPdePte(page_dir_offset, page_table_offset);
                //printf("Index in storage bitfield %i\n", indexStorageBitfield);
                //Update page in storageBitfield TODO
                //storageBitfield[indexStorageBitfield].pde = page_dir_offset;
                //storageBitfield[indexStorageBitfield].pte = page_table_offset;
                //storageBitfield[indexStorageBitfield].storageAddress = storageAddressOfPage;
                copyPage(storageBitfield[indexStorageBitfield].storageAddress, memoryAddress);
#ifdef __DHBW_KERNEL__
                //memset(storageBitfield[indexStorageBitfield].storageAddress, 0, 0x1000);
#endif                
                //Reset index in storage Bitfield
                //storageBitfield[indexStorageBitfield].pde = 0;
                //storageBitfield[indexStorageBitfield].pte = 0;


                memoryAddress |= PRESENT_ON_STORAGE;
            }
            //Set flags on memory address
            memoryAddress = memoryAddress | PRESENT_BIT | RW_BIT | USER_MODE;
            
            *(page_table + page_table_offset) = memoryAddress;
            
            setPresentBit(page_dir_offset, page_table_offset, 1);
            
            //Set Bit in memory bitfield
            unsigned long indexInMemoryBitfield = (memoryAddress % startaddress) >> 12;
            physicalMemoryBitfield[indexInMemoryBitfield] = 1;
            
<<<<<<< HEAD
            
=======
            pg_struct.ph_addr = memoryAddress & 0xFFFFF000;
>>>>>>> hwpaging
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
}

unsigned long
getPageFrame() {
    /*
     * Returns a memory address
     * left 20 bits are memory address
     * right 12 bits are the index in storageBitfield
     */

<<<<<<< HEAD
=======

>>>>>>> hwpaging
    //Maximum allowed pages in memory at actual time.
    int limit;
    if (memoryPageCounter < MAX_NUMBER_OF_PAGES) {
        limit = memoryPageCounter;
    } else {
        limit = MAX_NUMBER_OF_PAGES;
    }

    for (int i = 0; i < limit; i++) {
        if (physicalMemoryBitfield[i] == 0) {
            unsigned long next_address = (unsigned long) (startaddress + i * 0x1000);
            physicalMemoryBitfield[i] = 1;
            return next_address;
        }
    }
    //There is no page left
    //get virtual address of page to replace
    unsigned long virtAddr = getAddressOfPageToReplace();
<<<<<<< HEAD
    pg_struct.ph_addr = virtAddr;
=======
    pg_struct.vic_addr = virtAddr;
>>>>>>> hwpaging
    unsigned long memoryAddress = swap(virtAddr);
    
    return memoryAddress;
}

<<<<<<< HEAD
void copyPage(unsigned long src_address, unsigned long dst_address) {
    unusedPar = src_address + dst_address;
=======
unsigned long dbg_copy_src_addr;
unsigned long dbg_copy_dst_addr;

void copyPage(unsigned long src_address, unsigned long dst_address) {
    unusedPar = src_address + dst_address;
    dbg_copy_src_addr = src_address;
    dbg_copy_dst_addr = dst_address;
>>>>>>> hwpaging
#ifdef __DHBW_KERNEL__
    //memcpy((unsigned long *)dst_address,(unsigned long *) src_address, 0x1000);
#else
    printf("Copying page from 0x%08X to 0x%08X.\n", src_address, dst_address);
#endif

}

int indexOfDiskAddrByPdePte(unsigned short pde, unsigned short pte) {
    for (int i = 0; i < MAX_NUMBER_OF_STORGAE_PAGES; i++) {
        if (storageBitfield[i].pde == pde && storageBitfield[i].pte == pte) {
            return i;
        }
    }
    return -1;
}

unsigned long getFreeFrameOnDisk() {
    int indexOfFreeFrame = indexOfDiskAddrByPdePte(0, 0);
    //printf("Index of Frame on Disk: %i\n", indexOfFreeFrame);
    if (indexOfFreeFrame >= 0) {
        return (unsigned long) (startOfStorage + indexOfFreeFrame * 0x1000);
    }
    return -1;
}

int getIndexOfFrameOnDisk(unsigned long storageAddr) {
    int indexStorageBitfield = (storageAddr % startOfStorage) >> 12;
    return indexStorageBitfield;
}
<<<<<<< HEAD
=======

unsigned long dbg_swap_addr;
unsigned long dbg_swap_result;
>>>>>>> hwpaging

unsigned long swap(unsigned long virtAddr) {
    
    // Compute Parameters
    unsigned int pde = PDE(virtAddr);
    unsigned int pte = PTE(virtAddr);
    
<<<<<<< HEAD
=======
    dbg_swap_addr = virtAddr;
>>>>>>> hwpaging

    //printf("Swap:\nPDE: %x PTE: %x\n",pde,pte);
    unsigned long storageAddr;

#ifdef __DHBW_KERNEL__
    unsigned long * page_table = (unsigned long *) ((page_directory[pde] & 0xFFFFF000) - (unsigned long) & LD_DATA_START);
#else
    unsigned long * page_table = (unsigned long *) (page_directory[pde] & 0xFFFFF000);
#endif

    unsigned long memoryAddr = page_table[pte] & 0xFFFFF000;
    int flags = page_table[pte] & 0xFFF;



    // Check if page to swap is on disk
    if ((flags & PRESENT_ON_STORAGE) == PRESENT_ON_STORAGE) {
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
        // Get free storage address to save page to
        storageAddr = getFreeFrameOnDisk();
<<<<<<< HEAD
=======
        pg_struct.sec_addr = storageAddr;
>>>>>>> hwpaging
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
<<<<<<< HEAD

    //Reset in memory Bitfield
    unsigned long indexInMemoryBitfield = (memoryAddr % startaddress) >> 12;
    //printf("Before reseting bitfield entry with index: %d\n", indexInMemoryBitfield);
    physicalMemoryBitfield[indexInMemoryBitfield] = 0;
    page_table[pte] &= 0xFFFFFFFE;
=======

    //Reset in memory Bitfield
    unsigned long indexInMemoryBitfield = (memoryAddr % startaddress) >> 12;
    //printf("Before reseting bitfield entry with index: %d\n", indexInMemoryBitfield);
    physicalMemoryBitfield[indexInMemoryBitfield] = 0;
    page_table[pte] &= 0xFFFFFFFE;

    dbg_swap_result = memoryAddr;

>>>>>>> hwpaging
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
freePageInMemory(int pde, int pte) {
    unsigned long virtAddr = 0;
    virtAddr |= pde << 22;
    virtAddr |= pte << 12;
    swap(virtAddr);
}

void
freeAllPages() {
    //For all present bits, do free page in Memory
    for (int pde = 0; pde < 1024; pde++) {
        for (int pte = 0; pte < 1024; pte++) {
            if (isPresentBit(pde, pte)) {
                if (!(pde == 0 && pte <= 512)) {
                    freePageInMemory(pde, pte);
                }
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

unsigned long
getAddressOfPageToReplace() {
    /* Implementation of NRU
     * 
    A=0, M=0 (nicht gelesen, nicht verändert)
    A=0, M=1 (nicht gelesen, aber verändert)
    A=1, M=0 (gelesen, aber nicht verändert)
    A=1, M=1 (gelesen und verändert)
     */
    unsigned long *temp_page_table;
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
            temp_page_table = (unsigned long *) ((page_directory[counter_pde] & 0xFFFFF000) - (unsigned long) &LD_DATA_START);
#else
            temp_page_table = (unsigned long *) (page_directory[counter_pde] & 0xFFFFF000);
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
    unsigned long virtAddr = 0;
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

unsigned long*
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
    for (unsigned long i = 0; i < 256; i++) {
#ifdef __DHBW_KERNEL__
        if (i >= (LD_IMAGE_START >> 12) && i < (LD_DATA_START >> 12)) {
            kernel_page_table[i] = (unsigned long) (i * 0x1000 + PRESENT_BIT);
        } else {
            kernel_page_table[i] = (unsigned long) (i * 0x1000 + PRESENT_BIT + RW_BIT + USER_MODE);
        }
#else
        if (i >= (0x10000 >> 12) && i < (0x20000 >> 12)) {
            kernel_page_table[i] = (unsigned long) (i * 0x1000 + PRESENT_BIT);
        } else {
            kernel_page_table[i] = (unsigned long) (i * 0x1000 + PRESENT_BIT + RW_BIT + USER_MODE);
        }
#endif
        setPresentBit(0, i, 1);
    }
    *(page_directory + OFFSET_KERNEL_PT) = (unsigned long) kernel_page_table | PRESENT_BIT | RW_BIT | USER_MODE;
    *(page_directory + OFFSET_PROGRAMM_PT) = (unsigned long) programm_page_table | PRESENT_BIT | RW_BIT | USER_MODE;
    *(page_directory + OFFSET_STACK_PT) = (unsigned long) stack_page_table | PRESENT_BIT | RW_BIT | USER_MODE;

#ifdef __DHBW_KERNEL__
    *(page_directory + OFFSET_KERNEL_PT) += (unsigned long) &LD_DATA_START;
    *(page_directory + OFFSET_PROGRAMM_PT) += (unsigned long) &LD_DATA_START;
    *(page_directory + OFFSET_STACK_PT) += (unsigned long) &LD_DATA_START;
#endif

    //printf("Address of page Directory %p\n",page_directory);
    return page_directory;
}



