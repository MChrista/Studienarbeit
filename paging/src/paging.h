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

struct page_fault_result;

struct page_fault_result * pageFault( int);
uint32_t* init_paging();
int setPresentBit(int , int , int);
int isPresentBit(int, int);
int getClassOfPage(int);
void loadPageFromStorage(uint32_t , uint32_t );
uint32_t getAddressOfPageToReplace();
int isPresentBit(int , int );
uint32_t getPageFrame();
uint32_t swap(uint32_t virtAddr);