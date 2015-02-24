#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include "mmu.h"


int zaehlen() {
	static int counter = 0;
	return ++counter;
}



int main(){
    //init_paging();
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
    printf("Zaehler: %d\n", zaehlen());
    printf("Zaehler: %d\n", zaehlen());	
    printf("Zaehler: %d\n", zaehlen());
 
    
    
    return 0;

}

/*
 * TUTORIAL Schachbrett
 * 
void printField(int *field);
void move(int *field, int *posX, int *posY, char zug);

int main() {

	int field[8][8] = { 0 }, posX=0, posY=0;
	char zug;

	// setzen der Spielfigur
	field[posY][posX] = 1;

	printf("\nBeenden mit x\n");

	do {
		printField(&field[0][0]);
		printf("\nZug [w hoch, a links, s runter, d rechts]: ");
		scanf("%c", &zug);
		move(&field[0][0], &posX, &posY, zug);		
	}while(zug != 'x');

	return 0;
}


// Ausgabe Spielfeld
void printField(int *field) {
	printf("\n");
	int i, j;
	// Schleife fuer Zeilen, Y-Achse
	for(i=0; i<8; i++) {
		// Schleife fuer Spalten, X-Achse
		for(j=0; j<8; j++) {
			printf("%d ", *(field+i*8+j));
		}
		printf("\n");
	}	
}

// Spielfigur bewegen
void move(int *field, int *posX, int *posY, char zug) {
	// alte Position loeschen
	*(field + *posY * 8 + *posX) = 0;
	
	// neue Position bestimmen
	switch(zug) {
		case 'w': (*posY)--; break;
		case 'a': (*posX)--; break;
		case 's': (*posY)++; break;
		case 'd': (*posX)++; break;
	}

	// Grenzueberschreitung pruefen
	if(*posX < 0) *posX = 7;
	if(*posX > 7) *posX = 0;
	if(*posY < 0) *posY = 7;
	if(*posY > 7) *posY = 0;

	// neue Position setzen
	*(field + *posY * 8 + *posX) = 1;
}*/