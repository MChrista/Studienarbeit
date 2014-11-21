#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

static uint8_t memory[16777216]; //Array with 16MB


uint8_t * generateMemory(){
	for(int i=0;i<16777216;i++){
		memory[i]=0;
	}
	return memory;
}



int main(){
	uint8_t *p = generateMemory();
	printf("Hello World\n");
	printf("%d",*(p+1));
	return 0;
}
