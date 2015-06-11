#define PRESENT_BIT 1
#define NOT_PRESENT_BIT 0
#define RW_BIT 2
#define OFFSET_KERNEL_PT 0
#define OFFSET_PROGRAMM_PT 32
#define OFFSET_STACK_PT 1023
#define RW_BIT 2
#define USER_MODE_BIT 4
#define PRESENT_ON_STORAGE 0x400
#define DIRTY 0x040
#define MAX_NUMBER_OF_PAGES 4

#define PDE(addr) ((addr & 0xFFC00000) >> 22)
#define PTE(addr) ((addr & 0x003FF000) >> 12)

#ifdef __DHBW_KERNEL__
// Linear address of data segment, defined in ldscript
// use only in Kernel context with x86 segmentation
// being enabled
extern unsigned long LD_DATA_START;
#endif


/*
 * Declaration of Page Directory and Page tables
 */
unsigned long page_directory[1024] __attribute__((aligned(0x1000)));
unsigned long kernel_page_table[1024] __attribute__((aligned(0x1000)));
unsigned long programm_page_table[1024] __attribute__((aligned(0x1000)));
unsigned long stack_page_table[1024] __attribute__((aligned(0x1000)));

//General Parameters
int startaddress = 0x20000; //Startaddress for Physical Memory
int page_counter = 0;

int startOfStorage = 0x300000;
int storagePageCounter = 0;

unsigned long page_bitfield[1024][32];
unsigned long page_addresses_on_storage[MAX_NUMBER_OF_PAGES][3];
unsigned long test;

//Page replace parameters
int replace_pde_offset = 0;
int replace_pte_offset = 512;

typedef struct pg_struct {
    unsigned long ft_addr; // faulting linear memory address
    unsigned long pde; // Page Directory Entry
    unsigned long pte; // Page Table Entry
    unsigned long off; // Page Offset
    unsigned long ph_addr; // Physical Address
    unsigned long flags; // Flags = TBD
} pg_struct_t;


static pg_struct_t pg_struct;
int setPresentBit(int, int, int);
int isPresentBit(int, int);
int replacePage(int, int);
int getClassOfPage(int);
int getClassOfPage(int);
void loadPageFromStorage(unsigned long, unsigned long);
unsigned long getAddressOfPageToReplace();
int isPresentBit(int, int);
unsigned long getPageFrame();
unsigned long swap(unsigned long);

pg_struct_t *
pfhandler(unsigned long ft_addr) {
    int page_dir_offset = (ft_addr >> 22) & 0x3FF;
    int page_table_offset = (ft_addr & 0x003FF000) >> 12;

    pg_struct.ft_addr = ft_addr;
    pg_struct.pde = page_dir_offset;
    pg_struct.pte = page_table_offset;
    pg_struct.off = ft_addr & 0x00000FFF;


    if ((page_directory[page_dir_offset] & PRESENT_BIT) == PRESENT_BIT) { //if present Bit is set

        unsigned long *page_table;
#ifdef __DHBW_KERNEL__
        page_table = (unsigned long *) ((page_directory[page_dir_offset] & 0xFFFFF000) - (unsigned long) &LD_DATA_START);
#else
        page_table = (unsigned long *) (page_directory[page_dir_offset] & 0xFFFFF000);
#endif

        if ((*(page_table + page_table_offset) & PRESENT_BIT) != PRESENT_BIT) { //if present Bit is not set
            unsigned long memoryAddress = getPageFrame();

            int counter = memoryAddress & 0xFFF;
            memoryAddress &= 0xFFFFF000;
            page_addresses_on_storage[counter][0] = page_dir_offset;
            page_addresses_on_storage[counter][1] = page_table_offset;

            if ((*(page_table + page_table_offset) & 0x400) == 0x400) {
                page_addresses_on_storage[counter][2] = *(page_table + page_table_offset) & 0xFFFFF000;
                loadPageFromStorage(memoryAddress, *(page_table + page_table_offset) & 0xFFFFF000);
                memoryAddress |= PRESENT_ON_STORAGE;

            }
            memoryAddress = memoryAddress | PRESENT_BIT | RW_BIT;
            *(page_table + page_table_offset) = memoryAddress;
            setPresentBit(page_dir_offset, page_table_offset, 1);
            pg_struct.ph_addr = memoryAddress & 0xFFFFF000;
            pg_struct.flags = *(page_table + page_table_offset) & 0x00000FFF;

        } else {
            //printf("There is no Page Fault\n\n");
            pg_struct.ph_addr = *(page_table + page_table_offset) & 0xFFFFF000;
            pg_struct.flags = *(page_table + page_table_offset) & 0x00000FFF;
        }

    } else {
        pg_struct.ph_addr = 0xFFFFFFFF;
        pg_struct.flags = 0x0;
    }
    return &pg_struct;
} /* end of pfhandler */

unsigned long
getPageFrame() {
    if (page_counter < MAX_NUMBER_OF_PAGES) {
        unsigned long next_address = (unsigned long) (startaddress + page_counter * 0x1000);
        next_address |= page_counter;
        page_counter++;
        return next_address;
    } else {
        unsigned long virtAddr = getAddressOfPageToReplace();
        unsigned long memoryAddress = swap(virtAddr);
        return memoryAddress;

    }
}

void
loadPageFromStorage(unsigned long memory_address, unsigned long storage_address) {
    test = memory_address + storage_address;
}

void
savePageToStorage(unsigned long memory_address, unsigned long storage_address) {
    test = memory_address + storage_address;
}

int indexOfDiskAddrByPdePte(unsigned long pde, unsigned long pte) {
    for (int i = 0; i < MAX_NUMBER_OF_PAGES; i++) {
        if (page_addresses_on_storage[i][0] == pde
                && page_addresses_on_storage[i][1] == pte) {
            return i;
        }
    }
    return -1;
}

unsigned long getFreeFrameOnDisk() {
    return (unsigned long) (startOfStorage + storagePageCounter++ * 0x1000);
}

unsigned long swap(unsigned long virtAddr) {

    // Compute Parameters
    int pde = PDE(virtAddr);
    int pte = PTE(virtAddr);

    unsigned long storageAddr;
#ifdef __DHBW_KERNEL__
    unsigned long * page_table = (unsigned long *) ((page_directory[pde] & 0xFFFFF000) - (unsigned long) &LD_DATA_START);
#else
    unsigned long * page_table = (unsigned long *) (page_directory[pde] & 0xFFFFF000);
#endif
    unsigned long memoryAddr = page_table[pte] & 0xFFFFF000;
    int flags = page_table[pte] & 0xFFF;

    int pageAddrOnStorageIndex = indexOfDiskAddrByPdePte(pde, pte);

    // Check if page to swap is on disk
    if ((flags & PRESENT_ON_STORAGE) == PRESENT_ON_STORAGE) {
        // Get address of page copy on disk
        storageAddr = page_addresses_on_storage[pageAddrOnStorageIndex][2];

        // Check if page was modified, only save it then
        if ((flags & DIRTY) == DIRTY) {
            // Overwrite copy on disk with modified page 
            savePageToStorage(memoryAddr, storageAddr);
        }
    } else {
        // Get free storage address to save page to
        storageAddr = getFreeFrameOnDisk();
        savePageToStorage(memoryAddr, storageAddr);
    }

    // Store disk address of page copy in its page table entry.
    page_table[pte] = storageAddr | PRESENT_ON_STORAGE;

    // Reset present bit
    setPresentBit(pde, pte, 0);
    page_table[pte] &= 0xFFFFFFFE;

    return memoryAddr | pageAddrOnStorageIndex;
}

int
setPresentBit(int pde_offset, int pte_offset, int bool) {
    if (pde_offset < 0 || pde_offset > 1023 || pte_offset < 0 || pte_offset > 1023) {
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
    int start_pde, start_pte, counter_pde, counter_pte, class, flags, tmp_class;

    start_pde = replace_pde_offset;
    start_pte = replace_pte_offset;
    counter_pde = replace_pde_offset;
    counter_pte = replace_pte_offset;
    class = 4; //Start at highest class + 1 to fetch the first page with class 3 if all pages have class 3

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
#ifdef __DHBW_KERNEL__
            temp_page_table = (unsigned long *) ((page_directory[counter_pde] & 0xFFFFF000) - (unsigned long) &LD_DATA_START);
#else
            temp_page_table = (unsigned long *) (page_directory[counter_pde] & 0xFFFFF000);
#endif
            flags = *(temp_page_table + counter_pte) & 0xFFF;
            tmp_class = getClassOfPage(flags);
            *(temp_page_table + counter_pte) &= 0xFFFFFFDF; //Remove access bit
            if (class > tmp_class) {
                class = tmp_class;
                replace_pde_offset = counter_pde;
                replace_pte_offset = counter_pte;
            }
        }
    } while ((counter_pde != start_pde || counter_pte != start_pte) && (class != 0)); //Until walk through bitfield is complete


    //printf("Replace Offsets are %d %d\n", replace_pde_offset, replace_pte_offset);
    //Get page table, in which is the page you want to replace and get the physical address of this page

    unsigned long virtAddr = 0;
    virtAddr |= replace_pde_offset << 22;
    virtAddr |= replace_pte_offset << 12;
    return virtAddr;
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

unsigned long* init_paging() {

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
        kernel_page_table[i] = (unsigned long) (i * 0x1000 + 3);
        setPresentBit(0, i, 1);
    }
    *(page_directory + OFFSET_KERNEL_PT) = (unsigned long) kernel_page_table | PRESENT_BIT | RW_BIT;
    *(page_directory + OFFSET_PROGRAMM_PT) = (unsigned long) programm_page_table | PRESENT_BIT | RW_BIT | USER_MODE_BIT;
    *(page_directory + OFFSET_STACK_PT) = (unsigned long) stack_page_table | PRESENT_BIT | RW_BIT | USER_MODE_BIT;

#ifdef __DHBW_KERNEL__
    *(page_directory + OFFSET_KERNEL_PT) += (unsigned long) &LD_DATA_START;
    *(page_directory + OFFSET_PROGRAMM_PT) += (unsigned long) &LD_DATA_START;
    *(page_directory + OFFSET_STACK_PT) += (unsigned long) &LD_DATA_START;
#endif

    return page_directory;
}


