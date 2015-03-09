#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "paging.h"
#define DEBUG
#define KernelEntwicklung



int isMemoryAddress(int memoryAddress);
int convertCharToHex(char *args);

int main(int argc, char *argv[]){
    init_paging();
#ifndef KernelEntwicklung
    int i;
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
