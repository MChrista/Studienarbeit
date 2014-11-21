#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

static uint8_t memory[16777216]; //Array with 16MB
static int * pointer_to_descriptor[1024];

uint8_t * generateMemory(){
	for(int i=0;i<16777216;i++){
		memory[i]=0;
	}
	memory[2]=10;
	return memory;
}

uint32_t * createDescriptorTable(){


}





int main(){
	uint8_t *p = generateMemory();
	printf("Hello World\n");
	printf("%d",p[2]);
	return 0;
}
