#include <stdio.h>
#include <inttypes.h>
#include "../pfhandler.h"
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define PRESENT   0x00000001
#define READWRITE 0x00000002
#define USER      0x00000004
#define ACCESSED  0x00000020
#define DIRTY     0x00000040

void setFlags(int, uint32_t, uint32_t *);


void testPageFault(char *mode, int virtualAddr, uint32_t * pageDir) {
    pg_struct_t *pg_struct;
    printf("%s %08X\n", mode, virtualAddr);
    if ((pageDir[PDE(virtualAddr)] & PRESENT) == PRESENT) {
        uint32_t * page_table = (uint32_t *) (pageDir[PDE(virtualAddr)] & 0xFFFFF000);

        if ((page_table[PTE(virtualAddr)] & PRESENT) != PRESENT) {
            pg_struct = pfhandler(virtualAddr);
            printf("Page fault @ 0x%08X -> %08X %08X %08X\n",
                    pg_struct->ft_addr,
                    pg_struct->ph_addr,
                    pg_struct->vic_addr,
                    pg_struct->sec_addr
                    );
        }
    }
    if (strncmp(mode, "W", sizeof (char)) == 0) {
        setFlags(virtualAddr, DIRTY, pageDir);
    }else{
        printf("%08X: 00000000\n",virtualAddr);
    }
    setFlags(virtualAddr, ACCESSED, pageDir);
}

void setFlags(int virtualAddr, uint32_t flags, uint32_t * page_directory) {

    uint32_t * page_table = (uint32_t *) (page_directory[PDE(virtualAddr)]&0xFFFFF000);
    //int * physicalAddr = (int *) (page_table[PTE(virtualAddr)]&0xFFFFF000);

    page_table[PTE(virtualAddr)] |= flags;

}

void testPaging(int virtualAddr, uint32_t * page_directory) {

    pg_struct_t * pf_result;
    
    int page_dir_offset = (virtualAddr & 0xFFC00000) >> 22;
    int page_table_offset = (virtualAddr & 0x003FF000) >> 12;

    printf("[PFOUT] 0x%08X", virtualAddr);
    printf("\t0x%03X", page_dir_offset);
    printf("\t0x%03X", page_table_offset);
    printf("\t0x%03X", virtualAddr & 0xFFF);

    if ((page_directory[page_dir_offset] & PRESENT)) { //if table present Bit is set
        uint32_t *page_table;
        page_table = (uint32_t *) (page_directory[page_dir_offset] & 0xFFFFF000);
        if ((*(page_table + page_table_offset) & PRESENT)) { //if page present Bit is set
            printf("\t");
            pf_result = pfhandler(virtualAddr);
        } else {
            printf("\tFPTE");
            pf_result = pfhandler(virtualAddr);
        }
        printf("\t0x%08X\n", *(page_table + page_table_offset));

        printf("[PFRES] 0x%08X\t0x%03X\t0x%03X\t0x%03X\t\t0x%08X\t%03X\n",
                pf_result->ft_addr,
                pf_result->pde,
                pf_result->pte,
                pf_result->off,
                pf_result->ph_addr,
                pf_result->flags
                );
    } else {
        printf("\tFPDE\n");
    }
}

void testBitfield() {
    init_paging();

}

int main(int argc, char** argv) {
    uint32_t * pageDir = init_paging();
    if (argc > 1) {
        FILE * f;
        f = fopen(argv[1], "r");
        if (f == NULL) {
            fprintf(stderr, "Can't open input file\n");
            exit(1);
        }
        uint32_t testAddr;
        char *mode = malloc(2);

        //printf("\nType\tAddress\t\tPDE\tPTE\tOffset\tFault?\tFrame Addr\tFlags\n");
        while (fscanf(f, "%s %08X\n", mode, &testAddr) != EOF) {
            //testPaging(testAddr, pageDir);
            testPageFault(mode, testAddr, pageDir);
        }
        printf("EOF");

    } else {
        printf("No Testdata given. Using default.\n");
        printf("\n        Address\t\tPDE\tPTE\tOffset\tFault?\tFrame Addr\n");
        //testBitfield();
        printf("\nTESTING OVER\n\n");
    }
    exit(1);

}
