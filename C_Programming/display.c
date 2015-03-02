#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "mmu.h"
#define DEBUG



int isMemoryAddress(int memoryAddress);
int convertCharToHex(char *args);

int main(int argc, char *argv[]){
    //init_paging();
    
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
    
    /*
    char str[100];
    
    do{
        memset(&str[0],0,sizeof(str));
        printf("Paging>");
        scanf("%s", str);
        printf( "\nYou entered: %s\n", str);
          // Testfälle
    }while(strcmp(str , "exit"));
    //Float: %.2f
    */

    return 0;

}

int convertCharToHex(char* args){
    char c;
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

