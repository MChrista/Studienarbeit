#include <inttypes.h>
#define DEBUG 1
#define TESTBITFIELD 0
#define PRESENT_BIT 1
#define NOT_PRESENT_BIT 0
#define RW_BIT 2
#define OFFSET_KERNEL_PT 0
#define OFFSET_PROGRAMM_PT 32
#define OFFSET_STACK_PT 1023
#define RW_BIT 2



typedef struct pg_struct {
    int ft_addr;
    int pde;
    int pte;
    int off;
    int ph_addr;
    int flags;
    int vic_addr; // victim page address
    int sec_addr; // secondary storage address
}pg_struct_t;

pg_struct_t * pfhandler(uint32_t);
uint32_t* init_paging();
uint32_t setPresentBit(uint32_t , uint32_t , uint32_t);
uint32_t isPresentBit(uint32_t, uint32_t);
uint32_t getClassOfPage(uint32_t);
void copyPage(uint32_t , uint32_t );
uint32_t getAddressOfPageToReplace();
uint32_t isPresentBit(uint32_t , uint32_t );
uint32_t getPageFrame();
uint32_t swap(uint32_t virtAddr);
void print_debug(char*);
uint32_t getIndexOfFrameOnDisk(uint32_t);
uint32_t indexOfDiskAddrByPdePte(uint32_t,uint32_t);
void freePageInMemory(uint32_t, uint32_t);
void freeAllPages();