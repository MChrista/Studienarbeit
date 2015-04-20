#include <inttypes.h>
#define DEBUG 0
#define PRESENT_BIT 1
#define NOT_PRESENT_BIT 0
#define RW_BIT 2
#define OFFSET_KERNEL_PT 0
#define OFFSET_PROGRAMM_PT 32
#define OFFSET_STACK_PT 1023
#define RW_BIT 2
#define IS_PRESENT(address) ((address & 0x1) == (1) ? (0) : (1))
void pageFault( int);
uint32_t* init_paging();
int setPresentBit(int , int , int);
int isPresentBit(int, int);