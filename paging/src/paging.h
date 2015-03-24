#include <inttypes.h>
#define DEBUG 1
#define PRESENT_BIT 1
#define RW_BIT 2
#define IS_PRESENT(address) ((address & 0x1) == (1) ? (0) : (1))
void pageFault( int);
uint32_t* init_paging();
