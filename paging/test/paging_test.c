#include <stdio.h>
#include <inttypes.h>
#include "paging.h"
#include <math.h>

void testPaging( int virtualAddr, uint32_t * page_directory ){
    	int page_dir_offset = (virtualAddr & 0xFFC00000) >> 22;
	int page_table_offset = (virtualAddr & 0x003FF000) >> 12;

    	printf("0x%08X", virtualAddr );
    	printf("\t0x%03X",page_dir_offset );
	printf("\t0x%03X", page_table_offset);
	printf("\t0x%03X", virtualAddr &  0xFFF);

	if( (page_directory[page_dir_offset] & PRESENT_BIT)){ //if table present Bit is set
        	uint32_t *page_table;
        	page_table = (uint32_t *)(page_directory[page_dir_offset] & 0xFFFFF000);
        	if((*(page_table + page_table_offset) & PRESENT_BIT)){ //if page present Bit is set
			printf("\t");        
		}else{
            		printf("\tFPTE");
            		pageFault( virtualAddr );
        	}
		printf("\t0x%08X\n",*(page_table + page_table_offset));
	
    	}else{
        	printf("\tFPDE\n");
    	}
}

void testBitfield(){
    init_paging();
    setPresentBit(32,1,1);
    setPresentBit(32,2,1);
    setPresentBit(32,3,1);
    setPresentBit(32,4,1);
    setPresentBit(32,5,1);
    setPresentBit(32,6,1);
    setPresentBit(32,7,1);
    setPresentBit(32,9,1);
    setPresentBit(32,10,1);
    setPresentBit(32,11,1);
    setPresentBit(32,12,1);
    setPresentBit(32,13,1);
    setPresentBit(32,14,1);
    setPresentBit(32,15,1);
    setPresentBit(32,33,1);
    setPresentBit(32,31,1);
    setPresentBit(0,0,0);
    setPresentBit(0,-1,1);
    setPresentBit(-1,0,1);
    setPresentBit(-1,-1,1);
    setPresentBit(1023,1023,1);
    setPresentBit(1024,1023,1);
    setPresentBit(1023,1024,1);
    setPresentBit(1022,1023,1);
    setPresentBit(1023,1022,1);
    setPresentBit(512,0,1);
    setPresentBit(512,1023,1);
    setPresentBit(512,1024,1);
    setPresentBit(512,1,1);
    setPresentBit(512,1,0);
    setPresentBit(1023,1023,0);
    setPresentBit(0,0,0);
    setPresentBit(0,32,0);
    
    for(int i=0; i<1024;i++){
        for(int j=0; j<1024;j++){
            if(isPresentBit(i,j)){
                printf("PDE: %d PTE: %d PTO: %d \n",i,(j/32),(j%32));
            }
        }
    }
    
}

int main( int argc, char** argv )
{	
	uint32_t * pageDir = init_paging();
	if (argc > 1){
		FILE * f;
		f = fopen(argv[1], "r");
		if (f == NULL) {
			fprintf(stderr, "Can't open input file\n"); 
			exit(1);
		} 
		int testAddr;
		printf("\nAddress\t\tPDE\tPTE\tOffset\tFault?\tFrame Addr\n");
		while (fscanf(f, "%08X", &testAddr) != EOF) {
			testPaging(testAddr, pageDir);
		}
		printf("EOF");		
		
	} else {	
		printf("No Testdata given. Using default.\n");
		printf("\nAddress\t\tPDE\tPTE\tOffset\tFault?\tFrame Addr\n");
        	testPaging(0x00010000, pageDir);
		testPaging(0x08048000, pageDir);
		testPaging(0x60000000, pageDir);
		testPaging(0x08048FFF, pageDir);
                printf("Testing Bitfield\n");
                testBitfield();
		printf("\nTESTING OVER\n\n");
	}

}
