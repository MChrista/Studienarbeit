#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "paging.h"

#define KernelEntwicklung



int isMemoryAddress(int memoryAddress);
int convertCharToHex(char *args);
void testIsPresent();
void testReplacePage();

int main(int argc, char *argv[]){
    
    testReplacePage();
    
    
    
    
    /*
    printf("Init Paging Address %p\n",init_paging());
    pageFault(0xFFFFFFFC);
    pageFault(0xFFFFFFFC);
    pageFault(0xFFFFFFFC);
    pageFault(0xFFFFFFFC);
    pageFault(0xFFFFFFFC);
*/
#ifndef KernelEntwicklung
    for(i=1;i<argc;i++){
        int memoryAddress = convertCharToHex(argv[i]);
#ifdef DEBUG
        printf("Memory Addresse is: %x\n",memoryAddress);
        //isMemoryAddress(argv[i]);
        convertCharToHex(argv[i]);
        //printf("\n%s",argv[i]);
#endif
        if(isMemoryAddress(memoryAddress) == 0){
            //call paging
        }
    }
    
    
    char str[100];
    
    while(1){
        memset(&str[0],0,sizeof(str));
        printf("Paging> ");
        scanf("%s", str);
        //printf( "You entered: %s", str);
        char *ptr = strtok(str, " ");
        while(ptr != NULL){
            switch(ptr[0]){
                case 'p': 
                    printf("page\n");
                    break;
                case 'd': 
                    printf("directory\n");
                    break;
                case 'e': 
                    return 0;
                default: 
                    break;
            }
            ptr = strtok(NULL," ");
        }
            
          // Testfälle
    }
    //Float: %.2f
#endif

    return 0;

}

int convertCharToHex(char* args){
    int result = (int)strtol(args, NULL, 16);
    /*
     *  If we want to use Prefix
        const char *hexstring = "0xabcdef0";
        int number = (int)strtol(hexstring, NULL, 0); 
    }
     */
#ifdef DEBUG
    printf("Der Hexstring ist: %x\n",result);
#endif
    return result;
}

int isMemoryAddress(int memoryAddress){
    if(memoryAddress >= 0 && memoryAddress <= 0xFFFFFFFF){
        return 0;
    }else{
#ifdef DEBUG
        printf("Memory Address %x is not in range",memoryAddress);
#endif
        return 1;
    }
}


void testReplacePage(){
    printf("Start of testing replace pages");
    init_paging();
    pageFault(0x08048000);
    pageFault(0x08049000);
    pageFault(0x08050000);
    pageFault(0x08051000);

    printf("Replace\n");
    pageFault(0x08052000);
    printf("Replace\n");
    pageFault(0x08053000);
    pageFault(0x08052000);
    pageFault(0x08050000);
    printf("Replace\n");
    pageFault(0x08049000);
    pageFault(0x08049000);
    pageFault(0x08053000);
    printf("Replace\n");
    pageFault(0x08060000);
    printf("Replace\n");
    pageFault(0x08061000);
    printf("Replace\n");
    pageFault(0x08062000);
    pageFault(0x08063000);
    pageFault(0x08064000);
    
    printf("Zweiter Test\n");
    
    pageFault(0x08055000);
    pageFault(0x08054000);
    pageFault(0x08053000);
    pageFault(0x08052000);
    pageFault(0x08051000);
    pageFault(0x08050000);
    pageFault(0x08049000);
    pageFault(0x08048000);
    
    
}

void testIsPresent(){
    //08048000
    //20
    //32
    //2
    //72
    printf("Page present Bit is %d\n", isPresentBit(0x20,0x48));
    setPresentBit(0x20,0x47,1);
    setPresentBit(0x20,0x48,1);
    setPresentBit(0x20,0x49,1);
    printf("Page present Bit is %d\n", isPresentBit(0x20,0x47));
    printf("Page present Bit is %d\n", isPresentBit(0x20,0x48));
    printf("Page present Bit is %d\n", isPresentBit(0x20,0x49));
    setPresentBit(0x20,0x48,0);
    setPresentBit(0x20,0x49,0);
    printf("Page present Bit is %d\n", isPresentBit(0x20,0x47));
    printf("Page present Bit is %d\n", isPresentBit(0x20,0x48));
    printf("Page present Bit is %d\n", isPresentBit(0x20,0x49));
    setPresentBit(0,1,1);
    setPresentBit(0,2,1);
    setPresentBit(1,0,1);
    setPresentBit(1023,1023,1);
}

