#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "gamelib.h"

int main() {
    srand((unsigned int)time(NULL));
    int scelta;

    do {
        printf("\n========= COSESTRANE - MENU =========\n");
        printf("1) Imposta gioco\n2) Gioca\n3) Termina gioco\n4) Crediti\n");
        printf("Scelta: ");
        while (scanf("%d", &scelta) != 1) {
            printf("Errore: Inserisci un numero valido: ");
            // Svuota il buffer di input (scarta le lettere)
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }
        // Una volta letto il numero, puliamo comunque il buffer dal '\n' residuo
        while (getchar() != '\n'); 

        switch (scelta) {
            case 1: imposta_gioco(); break;
            case 2: gioca(); break;
            case 3: termina_gioco(); break;
            case 4: crediti(); break;
            default: printf("Scelta non valida!\n");
        }
    } while (scelta != 3);

    return 0;
}
