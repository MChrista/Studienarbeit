#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "mmu.h"

int main(){
    //init_paging();
    char str[100];
    do{
        memset(&str[0],0,sizeof(str));
        printf("Paging>");
        scanf("%s", str);
        printf( "\nYou entered: %s\n", str);
          // Testfälle
    }while(strcmp(str , "exit"));
}