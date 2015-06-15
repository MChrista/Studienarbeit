#include <stdio.h>
#include <inttypes.h>
#include "paging.h"
#include <math.h>
#include <unistd.h>
#include <stdlib.h>

#define PDE(addr) ((addr & 0xFFC00000) >> 22)
#define PTE(addr) ((addr & 0x003FF000) >> 12)

#define PRESENT   0x00000001
#define READWRITE 0x00000002
#define USER      0x00000004
#define ACCESSED  0x00000020
#define DIRTY     0x00000040

struct page_fault_result {
    int fault_address;
    int pde;
    int pte;
    int offset;
    int physical_address;
    int flags;
};

void testPageClass();

void testPageClass() {
    /*
    printf("Class of Page %d\n",getClassOfPage(0));
    printf("Class of Page %d\n",getClassOfPage(1));
    printf("Class of Page %d\n",getClassOfPage(2));
    printf("Class of Page %d\n",getClassOfPage(32));
    printf("Class of Page %d\n",getClassOfPage(64));
    printf("Class of Page %d\n",getClassOfPage(96));
    printf("Class of Page %d\n",getClassOfPage(99));
    printf("Class of Page %d\n",getClassOfPage(67));
     */
}

void testPageFault(int virtualAddr) {
    struct page_fault_result * pf_result;
    pf_result = pageFault(virtualAddr);
    printf("[PFRES] 0x%08X\t0x%03X\t0x%03X\t0x%03X\t\t0x%08X\t%03X\n",
            pf_result->fault_address,
            pf_result->pde,
            pf_result->pte,
            pf_result->offset,
            pf_result->physical_address,
            pf_result->flags
            );
}

void setFlags(int virtualAddr, uint32_t flags, uint32_t * page_directory) {

    uint32_t * page_table = (uint32_t *) (page_directory[PDE(virtualAddr)]&0xFFFFF000);
    int * physicalAddr = (int *) (page_table[PTE(virtualAddr)]&0xFFFFF000);

    page_table[PTE(virtualAddr)] |= flags;

}

void testPaging(int virtualAddr, uint32_t * page_directory) {

    struct page_fault_result * pf_result;

    int page_dir_offset = (virtualAddr & 0xFFC00000) >> 22;
    int page_table_offset = (virtualAddr & 0x003FF000) >> 12;

    printf("[PFOUT] 0x%08X", virtualAddr);
    printf("\t0x%03X", page_dir_offset);
    printf("\t0x%03X", page_table_offset);
    printf("\t0x%03X", virtualAddr & 0xFFF);

    if ((page_directory[page_dir_offset] & PRESENT_BIT)) { //if table present Bit is set
        uint32_t *page_table;
        page_table = (uint32_t *) (page_directory[page_dir_offset] & 0xFFFFF000);
        if ((*(page_table + page_table_offset) & PRESENT_BIT)) { //if page present Bit is set
            printf("\t");
            pf_result = pageFault(virtualAddr);
        } else {
            printf("\tFPTE");
            pf_result = pageFault(virtualAddr);
        }
        printf("\t0x%08X\n", *(page_table + page_table_offset));

        printf("[PFRES] 0x%08X\t0x%03X\t0x%03X\t0x%03X\t\t0x%08X\t%03X\n",
                pf_result->fault_address,
                pf_result->pde,
                pf_result->pte,
                pf_result->offset,
                pf_result->physical_address,
                pf_result->flags
                );
    } else {
        printf("\tFPDE\n");
    }
}

void testBitfield() {
    init_paging();
    setPresentBit(32, 1, 1);
    setPresentBit(32, 2, 1);
    setPresentBit(32, 3, 1);
    setPresentBit(32, 4, 1);
    setPresentBit(32, 5, 1);
    setPresentBit(32, 6, 1);
    setPresentBit(32, 7, 1);
    setPresentBit(32, 9, 1);
    setPresentBit(32, 10, 1);
    setPresentBit(32, 11, 1);
    setPresentBit(32, 12, 1);
    setPresentBit(32, 13, 1);
    setPresentBit(32, 14, 1);
    setPresentBit(32, 15, 1);
    setPresentBit(32, 33, 1);
    setPresentBit(32, 31, 1);
    setPresentBit(0, 0, 0);
    setPresentBit(0, -1, 1);
    setPresentBit(-1, 0, 1);
    setPresentBit(-1, -1, 1);
    setPresentBit(1023, 1023, 1);
    setPresentBit(1024, 1023, 1);
    setPresentBit(1023, 1024, 1);
    setPresentBit(1022, 1023, 1);
    setPresentBit(1023, 1022, 1);
    setPresentBit(512, 0, 1);
    setPresentBit(512, 1023, 1);
    setPresentBit(512, 1024, 1);
    setPresentBit(512, 1, 1);
    setPresentBit(512, 1, 0);
    setPresentBit(1023, 1023, 0);
    setPresentBit(0, 0, 0);
    setPresentBit(0, 32, 0);

    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < 1024; j++) {
            if (isPresentBit(i, j)) {
                printf("PDE: %d PTE: %d PTO: %d \n", i, (j / 32), (j % 32));
            }
        }
    }

}

int main(int argc, char** argv) {
    printf("Test: starting main");
    uint32_t * pageDir = init_paging();
    if (argc > 1) {
        FILE * f;
        f = fopen(argv[1], "r");
        if (f == NULL) {
            fprintf(stderr, "Can't open input file\n");
            exit(1);
        }
        int testAddr;
        printf("\nType\tAddress\t\tPDE\tPTE\tOffset\tFault?\tFrame Addr\tFlags\n");
        while (fscanf(f, "%08X", &testAddr) != EOF) {
            //testPaging(testAddr, pageDir);
            testPageFault(testAddr);
        }
        printf("EOF");

    } else {
        printf("No Testdata given. Using default.\n");
        testPageClass();
        printf("\n        Address\t\tPDE\tPTE\tOffset\tFault?\tFrame Addr\n");
        testPageFault(0x08048000);
        testPageFault(0x08049000);
        testPageFault(0x08050000);
        testPageFault(0x08051000);
        printf("Set Flags\n");
        setFlags(0x08048000, ACCESSED | DIRTY, pageDir);
        testPageFault(0x08048000);
        setFlags(0x08049000, ACCESSED | DIRTY, pageDir);
        testPageFault(0x08049000);
        setFlags(0x08051000, ACCESSED | DIRTY, pageDir);
        testPageFault(0x08051000);
        printf("Remove 08050000\n");
        testPageFault(0x08052000);
        setFlags(0x08052000, ACCESSED | DIRTY, pageDir);
        printf("Remove 08048000\n");
        testPageFault(0x08053000);
        setFlags(0x08053000, ACCESSED | DIRTY, pageDir);
        printf("Read 08048000\n");
        testPageFault(0x08048000);
        
        //printf("Testing Bitfield\n");
        //testBitfield();
        printf("\nTESTING OVER\n\n");
    }
    exit(1);

}
