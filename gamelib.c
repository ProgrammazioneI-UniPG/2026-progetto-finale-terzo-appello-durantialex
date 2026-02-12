#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gamelib.h"

// Variabili globali statiche
static struct Giocatore* giocatori[4] = {NULL};
static int num_giocatori = 0;
static struct Zona_mondoreale* prima_zona_m = NULL;
static struct Zona_soprasotto* prima_zona_s = NULL;
static int mappa_chiusa = 0;
static char vincitori[3][50] = {""};
static char partecipanti[4][50];
static int vita_giocatori[4] = {1};// Array per tracciare lo stato di vita dei giocatori (1: vivo, 0: morto)
static int num_partecipanti_totali = 0;
static int vincitori_count = 0;
static int partita = 0; // Variabile per tracciare lo stato della partita

// --- FUNZIONI DI UTILITÀ ---

// Legge un intero da input con gestione degli errori
int leggi_int() {
    int valore;
    // Se scanf non legge un numero (ritorna 0), entra nel loop
    while (scanf("%d", &valore) != 1) {
        printf("Errore: Inserisci un numero valido: ");
        // Svuota il buffer di input (scarta le lettere)
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }
    // Una volta letto il numero, puliamo comunque il buffer dal '\n' residuo
    while (getchar() != '\n'); 
    return valore;
}

// Registra il nome del vincitore nella lista dei vincitori 
// Usata nella funzione gioca() quando un giocatore sconfigge il Demotorzone
static void registra_vincitore(const char* nome) {
    if (vincitori_count < 3) {
        strncpy(vincitori[vincitori_count], nome, 49);
        vincitori_count++; // Fondamentale per la funzione crediti()
    }
}

//Classificazione degli oggetti
//Utilizzata nella funzione raccogli_oggetto() e stampa_mini_mappa()
static const char* nome_oggetto(enum Tipo_oggetto obj) {
    switch(obj) {
        case nessun_oggetto:            return "Niente";
        case bicicletta:                return "Bicicletta (Scappare da combattimento!)";
        case maglietta_fuocoinferno:    return "Maglietta Hellfire Club (Più protezione!)";
        case bussola:                   return "Bussola (Trova il Demotorzone!)";
        case schitarrata_metallica:     return "Chitarra Elettrica (Più attacco!)";
        default:                        return "Oggetto Sconosciuto";
    }
}

// --- GESTIONE MAPPA ---
static void pulisci_mappa() {
    struct Zona_mondoreale* temp_m = prima_zona_m;
    while(temp_m) {
        struct Zona_mondoreale* next = temp_m->avanti;
        free(temp_m);
        temp_m = next;
    }
    struct Zona_soprasotto* temp_s = prima_zona_s;
    while(temp_s) {
        struct Zona_soprasotto* next = temp_s->avanti;
        free(temp_s);
        temp_s = next;
    }
    prima_zona_m = NULL; prima_zona_s = NULL;
}

static void genera_mappa() {
    pulisci_mappa(); // Libera eventuale memoria di mappe precedenti
    struct Zona_mondoreale* ultima_m = NULL;
    struct Zona_soprasotto* ultima_s = NULL;
    int demotorzone_inserito = 0;//flag per indicare se il demotorzone è già stato inserito

    for (int i = 0; i < 15; i++) {
        // Allocazione delle due zone corrispondenti
        struct Zona_mondoreale* n_m = malloc(sizeof(struct Zona_mondoreale));
        struct Zona_soprasotto* n_s = malloc(sizeof(struct Zona_soprasotto));
        
        // 1. Tipo zona: 10% di probabilità per ognuno dei 10 tipi
        enum Tipo_zona tipo_comune = (enum Tipo_zona)(rand() % 10);
        n_m->tipo = tipo_comune;
        n_s->tipo = tipo_comune;

        // 2. Configurazione MONDO REALE
        // Nemici: 20% Billi, 20% Democane, 60% Nessuno
        int r_nem_m = rand() % 100;
        if (r_nem_m < 20) n_m->nemico = billi;
        else if (r_nem_m < 40) n_m->nemico = democane;
        else n_m->nemico = nessun_nemico;

        // Oggetti: 40% di probabilità di trovare un oggetto casuale
        if (rand() % 100 < 40) n_m->oggetto = (enum Tipo_oggetto)(rand() % 4 + 1);
        else n_m->oggetto = nessun_oggetto;

        // 3. Configurazione SOPRASOTTO
        // Nemici: 30% Democane, 70% Nessuno (Il Demotorzone lo gestiamo dopo)
        int r_nem_s = rand() % 100;
        if (r_nem_s < 30) n_s->nemico = democane;
        else n_s->nemico = nessun_nemico;

        // Gestione obbligatoria Demotorzone nell'ultima zona se non ancora apparso
        if (i == 14 && !demotorzone_inserito) {
            n_s->nemico = demotorzone;
            demotorzone_inserito = 1;
        } else if (rand() % 100 < 10 && !demotorzone_inserito) {
            // 10% probabilità di trovarlo prima dell'ultima stanza
            n_s->nemico = demotorzone;
            demotorzone_inserito = 1;
        }

        // 4. COLLEGAMENTI (Lista Doppiamente Concatenata e Link Trasversale)
        n_m->link_soprasotto = n_s;
        n_s->link_mondoreale = n_m;
        n_m->avanti = NULL;
        n_s->avanti = NULL;

        if (prima_zona_m == NULL) {
            // Prima zona della lista
            prima_zona_m = n_m;
            prima_zona_s = n_s;
            n_m->indietro = NULL;
            n_s->indietro = NULL;
        } else {
            // Collegamento ai nodi precedenti
            ultima_m->avanti = n_m;
            n_m->indietro = ultima_m;
            ultima_s->avanti = n_s;
            n_s->indietro = ultima_s;
        }
        ultima_m = n_m;
        ultima_s = n_s;
    }
    printf("Mappa generata: 15 zone create in entrambe le dimensioni.\n");
}

// Conta quante zone ci sono nella mappa del Mondo Reale e del Soprasotto
//Usata nella funzione inserisci_zona() e nel menu di gioco per mostrare la dimensione della mappa
static int conta_zone() {
    int conteggio = 0;
    struct Zona_mondoreale* temp = prima_zona_m;
    
    while (temp != NULL) {
        conteggio++;
        temp = temp->avanti;
    }
    return conteggio;
}

// Inserisce una nuova zona in una posizione specificata dall'utente
//Con nemici e oggetti scelti dall'utente
static void inserisci_zona() {
    if (prima_zona_m == NULL) {
        printf("Errore: genera prima la mappa base!\n");
        return;
    }

    int totali = conta_zone();
    int pos;

    printf("Attualmente ci sono %d zone.\n", totali);
    printf("In quale posizione vuoi inserire la nuova zona (1-%d)? ", totali + 1);
    pos = leggi_int();

    // Se l'utente inserisce un numero troppo alto, lo riportiamo al limite massimo + 1
    if (pos > totali + 1) pos = totali + 1;
    if (pos < 1) pos = 1;

    // 1. Creazione nuovi nodi
    struct Zona_mondoreale* n_m = malloc(sizeof(struct Zona_mondoreale));
    struct Zona_soprasotto* n_s = malloc(sizeof(struct Zona_soprasotto));

    // 2. Input Utente con vincoli
    printf("--- Configurazione Nuova Zona ---\n");
    n_m->tipo = n_s->tipo = (enum Tipo_zona)(rand() % 10);

    printf("Scegli nemico Mondo Reale(0:Nessuno, 1:Billi, 2:Democane): ");
    int nem; 
    nem = leggi_int();
    if(nem < 0 || nem > 2){
        printf("Valore impostato a 0 (Nessuno)\n"); nem = 0; // Vincolo obbligatorio
    }
    n_m->nemico = (enum Tipo_nemico)nem;

    printf("Scegli oggetto (0:Nessuno, 1:Bici, 2:Maglietta, 3:Bussola, 4:Chitarra): ");
    int obj; 
    obj = leggi_int();
    if(obj < 0 || obj > 4){
        printf("Valore impostato a 0 (Nessuno)\n"); obj = 0; // Vincolo obbligatorio
    } 
    n_m->oggetto = (enum Tipo_oggetto)obj;

    printf("Scegli nemico Soprasotto (0:Nessuno, 1:Democane, 2:Demotorzone): ");
    int nem_s;
    nem_s = leggi_int();
    if(nem_s < 0 || nem_s > 2){
        printf("Valore impostato a 0 (Nessuno)\n"); nem_s = 0; // Vincolo obbligatorio
    } 
    n_s->nemico = (enum Tipo_nemico)nem_s+1; // +1 perché Demotorzone è valorizzato come 3

    n_m->link_soprasotto = n_s;
    n_s->link_mondoreale = n_m;

    // 3. LOGICA DI INSERIMENTO NELLA LISTA
    struct Zona_mondoreale* curr_m = prima_zona_m;
    struct Zona_soprasotto* curr_s = prima_zona_s;
    int i = 1;

    // Scorriamo fino alla posizione desiderata o alla fine
    while (curr_m->avanti != NULL && i < pos) {
        curr_m = curr_m->avanti;
        curr_s = curr_s->avanti;
        i++;
    }

    if (pos <= 1) { // Inserimento in testa
        n_m->avanti = prima_zona_m;
        n_m->indietro = NULL;
        n_s->avanti = prima_zona_s;
        n_s->indietro = NULL;
        prima_zona_m->indietro = n_m;
        prima_zona_s->indietro = n_s;
        prima_zona_m = n_m;
        prima_zona_s = n_s;
    } else if (curr_m->avanti == NULL && pos > i) { // Inserimento in coda
        curr_m->avanti = n_m;
        n_m->indietro = curr_m;
        n_m->avanti = NULL;
        curr_s->avanti = n_s;
        n_s->indietro = curr_s;
        n_s->avanti = NULL;
    } else { // Inserimento in mezzo (prima di curr_m)
        struct Zona_mondoreale* prec_m = curr_m->indietro;
        struct Zona_soprasotto* prec_s = curr_s->indietro;

        n_m->avanti = curr_m;
        n_m->indietro = prec_m;
        prec_m->avanti = n_m;
        curr_m->indietro = n_m;

        n_s->avanti = curr_s;
        n_s->indietro = prec_s;
        prec_s->avanti = n_s;
        curr_s->indietro = n_s;
    }

    printf("Zona inserita con successo nella posizione %d!\n", pos);
}

static void cancella_zona() {
    if (prima_zona_m == NULL) {
        printf("Errore: La mappa è vuota!\n");
        return;
    }

    int totali = conta_zone();
    printf("Attualmente ci sono %d zone. Quale vuoi eliminare (1-%d)? ", totali, totali);
    int pos = leggi_int(); 

    if (pos < 1 || pos > totali) {
        printf("Posizione non valida.\n");
        return;
    }

    struct Zona_mondoreale* curr_m = prima_zona_m;
    struct Zona_soprasotto* curr_s = prima_zona_s;

    // 1. Raggiungiamo la zona specifica da eliminare
    for (int i = 1; i < pos; i++) {
        curr_m = curr_m->avanti;
        curr_s = curr_s->avanti;
    }

    // 2. CONTEGGIO BOSS (usando un puntatore temporaneo per non perdere curr_s!)
    int conteggio_boss_totali = 0;
    struct Zona_soprasotto* temp = prima_zona_s;
    while (temp != NULL) {
        if (temp->nemico == demotorzone) {
            conteggio_boss_totali++;
        }
        temp = temp->avanti;
    }

    // 3. CONTROLLI DI SICUREZZA
    if (curr_s->nemico == demotorzone) {
        if (conteggio_boss_totali == 1) {
            printf("[!] ERRORE: Non puoi cancellare l'UNICO Demotorzone presente!\n");
            return;
        } else {
            printf("[!] Attenzione: Stai eliminando uno dei Demotorzone. Continuare? (1:Si, 0:No): ");
            if (leggi_int() != 1) return;
        }
    }

    // 4. AGGIORNAMENTO PUNTATORI (Logica sfilamento nodo)
    // Mondo Reale
    if (curr_m->indietro != NULL) curr_m->indietro->avanti = curr_m->avanti;
    else prima_zona_m = curr_m->avanti;

    if (curr_m->avanti != NULL) curr_m->avanti->indietro = curr_m->indietro;

    // Soprasotto
    if (curr_s->indietro != NULL) curr_s->indietro->avanti = curr_s->avanti;
    else prima_zona_s = curr_s->avanti;

    if (curr_s->avanti != NULL) curr_s->avanti->indietro = curr_s->indietro;

    // 5. PROTEZIONE GIOCATORI: se qualcuno era qui, lo spostiamo alla testa
    for(int i = 0; i < 4; i++) {
        if (giocatori[i] != NULL) {
            if (giocatori[i]->pos_mondoreale == curr_m || giocatori[i]->pos_soprasotto == curr_s) {
                giocatori[i]->pos_mondoreale = prima_zona_m;
                giocatori[i]->pos_soprasotto = prima_zona_s;
            }
        }
    }

    // 6. LIBERAZIONE MEMORIA
    free(curr_m);
    free(curr_s);

    printf("Zona %d eliminata. Nuove zone totali: %d\n", pos, totali - 1);
}

//Randomizer per l'ordine di turno dei giocatori
static void mescola_giocatori(int* ordine, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = ordine[i];
        ordine[i] = ordine[j];
        ordine[j] = temp;
    }
}

// Gestione della morte del giocatore con liberazione memoria
static void gestisci_morte(int id_giocatore) {
    // Faccio un controllo di sicurezza fondamentale
    if (id_giocatore < 0 || id_giocatore >= 4 || giocatori[id_giocatore] == NULL) {
        return;
    }

    // Stampo prima di liberare la memoria
    printf("\n--- DISASTRO: %s e' stato trascinato nel Soprasotto per sempre! ---\n", giocatori[id_giocatore]->nome);
    
    // Libera la memoria
    free(giocatori[id_giocatore]);
    
    // Segna il giocatore come morto
    vita_giocatori[id_giocatore] = 0;

    // Imposta a NULL (così i futuri controlli if(giocatori[id]) funzioneranno)
    giocatori[id_giocatore] = NULL;
}

// --- GESTIONE COMBATTIMENTI ---
static void combatti(int id) {
    if (giocatori[id] == NULL) return; // Sicurezza iniziale
    struct Giocatore *g = giocatori[id];
    enum Tipo_nemico nemico_presente;

    // Determiniamo il nemico in base al mondo attuale
    if (g->mondo == 0) nemico_presente = g->pos_mondoreale->nemico;
    else nemico_presente = g->pos_soprasotto->nemico;

    if (nemico_presente == nessun_nemico) {
        printf("Non c'e' nessuno qui con cui combattere.\n");
        return;
    }

    printf("\n--- COMBATTIMENTO CONTRO UN %s ---\n", 
           (nemico_presente == billi) ? "BILLI" : 
           (nemico_presente == democane) ? "DEMOCANE" : "DEMOTORZONE");

    int dado_attacco = (rand() % 20) + 1;
    int potenza_attacco = g->attacco_psichico + dado_attacco;
    int difficolta_nemico = (nemico_presente == demotorzone) ? 25 : 15;

    if (potenza_attacco >= difficolta_nemico) {
        printf("Colpo critico! Hai sconfitto il nemico.\n");
        
        // Se è il Demotorzone, vittoria gestita tramite flag o stato
        if (nemico_presente == demotorzone) {
            printf("IL DEMOTORZONE E' CADUTO!\n");
            // Nota: La vittoria la controlliamo nel ciclo principale di gioca()
            // basandoci sul fatto che il nemico in quella zona diventerà 'nessun_nemico'
        }
        // Il nemico viene rimosso dalla zona
        if (g->mondo == 0) g->pos_mondoreale->nemico = nessun_nemico;
        else g->pos_soprasotto->nemico = nessun_nemico;
        
    } else {
        printf("Il nemico ti sovrasta! La tua difesa psichica vacilla.\n");
        g->difesa_psichica -= (rand() % 5) + 1;// Danno casuale tra 1 e 5 alla difesa psichica del giocatore

        if (g->difesa_psichica <= 0) { //Se la difesa psichica scende a 0 o meno, il giocatore muore
            gestisci_morte(id); // Qui il puntatore giocatori[id] diventa NULL
            return; // USCITA IMMEDIATA: il giocatore non esiste più!
        } else {
            printf("Sei ferito, ma resisti! Difesa attuale: %d\n", g->difesa_psichica);
        }
    }
}

// --- LOGICA DI GIOCO ---
// Cambiamento di mondo (Reale <-> Soprasotto)
static void cambia_mondo(int id) {
    struct Giocatore *g = giocatori[id];
    if (g->mondo == 0) { // Passa al Soprasotto
        g->pos_soprasotto = g->pos_mondoreale->link_soprasotto;
        g->mondo = 1;
        printf("Sei entrato nel Soprasotto!\n");
    } else {
        if ((rand() % 20 + 1) < g->fortuna) { // Controllo fortuna per riuscire a tornare dal Soprasotto
            g->pos_mondoreale = g->pos_soprasotto->link_mondoreale;
            g->mondo = 0;
            printf("Tornato nel Mondo Reale!\n");
        } else printf("Fuga fallita!\n");
    }
}

static void raccogli_oggetto(int id) {
    struct Giocatore *g = giocatori[id];
    if (g->mondo == 1) return;
    for (int i = 0; i < 3; i++) {
            // Controllo slot libero nello zaino
        if (g->zaino[i] == nessun_oggetto && g->pos_mondoreale->oggetto != nessun_oggetto) {
            g->zaino[i] = g->pos_mondoreale->oggetto;
            g->pos_mondoreale->oggetto = nessun_oggetto;
            printf("Oggetto raccolto nello slot %d\n", i+1);
            return;
        }
        else if( i == 2 && g->zaino[i] != nessun_oggetto){
            printf("Zaino pieno! Non puoi raccogliere l'oggetto.\n");
            printf("Libera uno slot prima di raccogliere nuovi oggetti.\n");
            return;
        }
    }
}

// --- FUNZIONI DI STAMPA E UTILITÀ ---
static void stampa_mappa() {
    int scelta_mondo;
    printf("\nQuale mappa vuoi visualizzare?\n1) Mondo Reale\n2) Soprasotto\nScelta: ");
    scelta_mondo = leggi_int();

    if (scelta_mondo == 1) {
        struct Zona_mondoreale* curr = prima_zona_m;
        if (!curr) { printf("Mappa non ancora generata!\n"); return; }
        
        printf("\n--- MAPPA MONDO REALE (Occhinz) ---\n");
        int contatore = 1;
        while (curr != NULL) {
            printf("[%d] Tipo: %d | Nemico: %d | Oggetto: %d\n", 
                    contatore++, curr->tipo, curr->nemico, curr->oggetto);
            curr = curr->avanti;
        }
    } else if (scelta_mondo == 2) {
        struct Zona_soprasotto* curr = prima_zona_s;
        if (!curr) { printf("Mappa non ancora generata!\n"); return; }

        printf("\n--- MAPPA SOPRASOTTO ---\n");
        int contatore = 1;
        while (curr != NULL) {
            printf("[%d] Tipo: %d | Nemico: %d\n", 
                    contatore++, curr->tipo, curr->nemico);
            curr = curr->avanti;
        }
    } else {
        //controllo se l'input non è valido, minore di 1 o superiore a 2
        printf("Scelta non valida.\n");
    }
}

// Mini-mappa contestuale alla posizione attuale del giocatore
// Mostra nemici e oggetti presenti nella zona
// Usata nel menu di gioco per ispezionare la zona attuale e riperire informazioni utili
static void stampa_mini_mappa(int id) {
    if (giocatori[id] == NULL) return;
    struct Giocatore *g = giocatori[id];

    printf("\n--- [ ISPEZIONE ZONA ] ---\n");

    if (g->mondo == 0) { // MONDO REALE
        // 1. Controllo Nemici
        if (g->pos_mondoreale->nemico != nessun_nemico) {
            printf("ATTENZIONE: Un %s ti sta davanti!\n", 
                    (g->pos_mondoreale->nemico == billi) ? "Billi" : "Democane");
        } else {
            printf("Nemici: Non sembrano esserci pericoli immediati.\n");
        }

        // 2. Controllo Oggetti
        if (g->pos_mondoreale->oggetto != nessun_oggetto) {
            printf("OGGETTI: Noti qualcosa a terra... sembra %s.\n", nome_oggetto(g->pos_mondoreale->oggetto));
        } else {
            printf("OGGETTI: Non vedi nulla di utile qui.\n");
        }
    } 
    else { // SOPRASOTTO
        // Nel Soprasotto i nemici sono diversi e mancano gli oggetti
        if (g->pos_soprasotto->nemico != nessun_nemico) {
            printf("PERICOLO: Un'ombra massiccia si muove nella nebbia... e' un %s!\n", 
                    (g->pos_soprasotto->nemico == demotorzone) ? "DEMOTORZONE (BOSS)" : "Democane");
        } else {
            printf("Per ora sei da solo, non sembrano esserci nemici.\n");
        }
    }
}

static int mappa_valida() {
    int zone_totali = conta_zone();
    int conteggio_boss = 0;

    // 1. Controllo numero minimo di zone
    if (zone_totali < 15) {
        printf("[!] Errore: La mappa ha solo %d zone. Ne servono almeno 15!\n", zone_totali);
        return 0;
    }

    // 2. Controllo presenza Demotorzone (uno e uno solo)
    struct Zona_soprasotto* curr_s = prima_zona_s;
    while (curr_s != NULL) {
        if (curr_s->nemico == demotorzone) {
            conteggio_boss++;
        }
        curr_s = curr_s->avanti;
    }

    if (conteggio_boss == 0) {
        printf("[!] Errore: Il Demotorzone non e' presente nella mappa!\n");
        return 0;
    } else if (conteggio_boss > 1) {
        printf("[!] Errore: Ci sono %d Demotorzone. Deve essercene esattamente UNO!\n", conteggio_boss);
        return 0;
    }

    // Se arriva qui, la mappa è perfetta
    return 1;
}

// Movimento del giocatore avanti con controlli sui nemici
static void avanza(int id) {
    if (giocatori[id] == NULL) return;
    struct Giocatore *g = giocatori[id];
    //printf("DEBUG: Pos Reale: %p, Pos Sopra: %p\n", (void*)g->pos_mondoreale, (void*)g->pos_soprasotto);

    // SICUREZZA: Se per qualche motivo uno dei due puntatori è NULL, fermati subito
    if (g->pos_mondoreale == NULL || g->pos_soprasotto == NULL) {
        printf("Errore critico: Posizione del giocatore corrotta!\n");
        return;
    }

    // Determina il nemico basandoti sul mondo attuale
    enum Tipo_nemico nemico_presente;
    if (g->mondo == 0) {
        nemico_presente = g->pos_mondoreale->nemico;
    } else {
        nemico_presente = g->pos_soprasotto->nemico;
    }

    int controllo = 1;

    do{
        if (nemico_presente != nessun_nemico) {
            printf("Un nemico ti sbarra la strada! Devi combattere.\n");
            return;
        }
        if(g->mondo == 0){
            if( nemico_presente!= nessun_nemico && g->pos_mondoreale->oggetto != nessun_oggetto){
                printf("Devi sconfiggere il nemico prima di avanzare e raccogliere l'oggetto: %s.\n", nome_oggetto(g->pos_mondoreale->oggetto));
                printf("Digita 4 per raccogliere l'oggetto e poi 6 per combattere.\n");
            }
            else if(nemico_presente!= nessun_nemico && g->pos_mondoreale->oggetto == nessun_oggetto){
                printf("Devi sconfiggere il nemico prima di avanzare.\n");
                printf("Digita 6 nel menu principale per combattere.\n");
            }
        }
        else if(g->mondo == 1){
            if( nemico_presente!= nessun_nemico){
                printf("Devi sconfiggere il nemico prima di avanzare.\n");
                printf("Digita 6 nel menu principale per combattere.\n");
            }
        }
        // Spostamento sincronizzato
        // Controlliamo che ENTRAMBI abbiano una zona successiva
        if (g->pos_mondoreale->avanti != NULL && g->pos_soprasotto->avanti != NULL) {
            g->pos_mondoreale = g->pos_mondoreale->avanti;
            g->pos_soprasotto = g->pos_soprasotto->avanti;
            
            if (g->mondo == 0 && nemico_presente == nessun_nemico) {
                printf("Avanzi nel Mondo Reale.\n");
                controllo = 0;
            } 
            else if(g->mondo == 1 && nemico_presente == nessun_nemico) {
                printf("Avanzi nel Soprasotto.\n");
                controllo = 0;
            }
            
        } else {
            printf("Fine della mappa! Non ci sono piu' zone avanti.\n");
            controllo = 0;
        }
    }while(controllo);
    
}

static void indietreggia(int id) {
    if (giocatori[id] == NULL) return;
    struct Giocatore *g = giocatori[id];

   // Determina il nemico basandoti sul mondo attuale
    enum Tipo_nemico nemico_presente;
    if (g->mondo == 0) {
        nemico_presente = g->pos_mondoreale->nemico;
    } else {
        nemico_presente = g->pos_soprasotto->nemico;
    }

    int controllo = 1;

    do{
        if (nemico_presente != nessun_nemico) {
            printf("Un nemico ti sbarra la strada! Devi combattere.\n");
            return;
        }
        if(g->mondo == 0){
            if( nemico_presente!= nessun_nemico && g->pos_mondoreale->oggetto != nessun_oggetto){
                printf("Devi sconfiggere il nemico prima di indietreggiare e raccogliere l'oggetto: %s.\n", nome_oggetto(g->pos_mondoreale->oggetto));
                printf("Digita 4 per raccogliere l'oggetto e poi 6 per combattere.\n");
            }
            else if(nemico_presente!= nessun_nemico && g->pos_mondoreale->oggetto == nessun_oggetto){
                printf("Devi sconfiggere il nemico prima di indietreggiare.\n");
                printf("Digita 6 nel menu principale per combattere.\n");
            }
        }
        else if(g->mondo == 1){
            if( nemico_presente!= nessun_nemico){
                printf("Devi sconfiggere il nemico prima di indietreggiare.\n");
                printf("Digita 6 nel menu principale per combattere.\n");
            }
        }

        // Controllo sincronizzato per tornare indietro
        if (g->pos_mondoreale->indietro != NULL && g->pos_soprasotto->indietro != NULL) {
            g->pos_mondoreale = g->pos_mondoreale->indietro;
            g->pos_soprasotto = g->pos_soprasotto->indietro;
            printf("Torni nella zona precedente.\n");
            controllo = 0;
        } else {
            printf("Sei all'inizio della mappa, non puoi tornare indietro!\n");
            controllo = 0;
        }
    }while(controllo);
    
}

static int calcola_indice_posizione(int id) {
    struct Giocatore *g = giocatori[id];
    int contatore = 1;
    
    if (g->mondo == 0) { // Mondo Reale
        struct Zona_mondoreale *temp = prima_zona_m;
        while (temp != NULL && temp != g->pos_mondoreale) {
            temp = temp->avanti;
            contatore++;
        }
    } else { // Soprasotto
        struct Zona_soprasotto *temp = prima_zona_s;
        while (temp != NULL && temp != g->pos_soprasotto) {
            temp = temp->avanti;
            contatore++;
        }
    }
    return contatore;
}

static void stampa_posizione_gps(int id) {
    if (giocatori[id] == NULL) return;
    
    int indice = calcola_indice_posizione(id);
    int totali = conta_zone();
    const char* nome_mondo = (giocatori[id]->mondo == 0) ? "MONDO REALE" : "SOPRASOTTO";
    
    printf("Dimensione attuale: %s\n", nome_mondo);
    printf("Posizione: [");
    
    // Creiamo una barra visiva: es. [#####.....]
    for (int i = 1; i <= totali; i++) {
        if (i < indice) printf("#");
        else if (i == indice) printf("P");
        else printf(".");
    }
    printf("] (%d / %d)\n", indice, totali);
    
    // Info sulla zona attuale
    if (giocatori[id]->mondo == 0) {
        printf("Tipo zona: %d | Oggetto presente: %s\n", 
                giocatori[id]->pos_mondoreale->tipo, 
                nome_oggetto(giocatori[id]->pos_mondoreale->oggetto));
    }
}

static void stampa_status(int id){

    printf("\nStatus di %s:\nAttacco Psichico: %d\nDifesa Psichica: %d\nFortuna: %d\n", 
        giocatori[id]->nome, 
        giocatori[id]->attacco_psichico,
        giocatori[id]->difesa_psichica,
        giocatori[id]->fortuna);
}

static void usa_bicicletta(int id) {
    if (giocatori[id] == NULL) return;
    struct Giocatore *g = giocatori[id];

    // Controlliamo se c'è effettivamente un nemico da scacciare
    enum Tipo_nemico nemico_presente;
    if (g->mondo == 0) nemico_presente = g->pos_mondoreale->nemico;
    else nemico_presente = g->pos_soprasotto->nemico;

    if (nemico_presente == nessun_nemico) {
        printf("\nPedali un po' nei paraggi, ma non c'e' nessuno da cui scappare.\n");
        return;
    }

    // Se il nemico è il Demotorzone, la fuga è impossibile!
    if (nemico_presente == demotorzone) {
        printf("\nIl Demotorzone e' troppo veloce! Non puoi scappare con una semplice bici!\n");
        return;
    }

    // ESECUZIONE FUGA: Rimuoviamo il nemico dalla zona attuale
    printf("\n--- FUGA IN BICICLETTA! ---\n");
    printf("Sbatti la bici contro il nemico, spazzandolo via!\n");
    
    if (g->mondo == 0) g->pos_mondoreale->nemico = nessun_nemico;
    else g->pos_soprasotto->nemico = nessun_nemico;

    printf("La strada ora e' libera.\n");
    printf("---------------------------\n");
}

static void usa_maglietta(int id) {
    if (giocatori[id] == NULL) return;
    struct Giocatore *g = giocatori[id];

    // Effetto: Aumento della difesa psichica
    int bonus = (rand() % 6) + 5; // Bonus tra 5 e 10
    g->difesa_psichica += bonus;
    printf("Indossi la Maglietta Hellfire Club! Difesa Psichica aumentata a %d.\n", g->difesa_psichica);
}

static void usa_chitarra(int id) {
    if (giocatori[id] == NULL) return;
    struct Giocatore *g = giocatori[id];

    // Effetto: Aumento dell'attacco psichico
    g->attacco_psichico += (rand() % 6) + 5; // Aumenta di 5-10
    printf("Hai suonato una potente schitarrata! Attacco Psichico aumentato a %d.\n", g->attacco_psichico);
}

static void usa_bussola(int id) {
    if (giocatori[id] == NULL) return;
    struct Giocatore *g = giocatori[id];

    // La bussola funziona sempre, ma il Boss è nel Soprasotto
    struct Zona_soprasotto *temp = prima_zona_s;
    int pos_boss = -1;
    int contatore = 0;

    // 1. Cerchiamo dove si trova il Demotorzone nella mappa
    while (temp != NULL) {
        if (temp->nemico == demotorzone) {
            pos_boss = contatore;
            break;
        }
        temp = temp->avanti;
        contatore++;
    }

    if (pos_boss == -1) {
        printf("La bussola gira a vuoto... Il Demotorzone non sembra essere in questa dimensione.\n");
        return;
    }

    // 2. Calcoliamo la posizione attuale del giocatore
    // Contiamo quante zone ci sono dall'inizio alla posizione attuale
    int pos_attuale = 0;
    struct Zona_soprasotto *check = prima_zona_s;
    while (check != g->pos_soprasotto && check != NULL) {
        pos_attuale++;
        check = check->avanti;
    }

    // 3. Confronto e indicazione
    printf("\n[ BUSSOLA ]\n");
    if (pos_attuale < pos_boss) {
        printf("L'ago punta con decisione verso AVANTI.\n");
        printf("Il Demotorzone dista %d zone da qui.\n", pos_boss - pos_attuale);
    } else if (pos_attuale > pos_boss) {
        printf("L'ago ruota e punta all'INDIETRO.\n");
        printf("Hai superato il nido! Il nemico dista %d zone.\n", pos_attuale - pos_boss);
    } else {
        printf("L'ago impazzisce e gira su se stesso... IL DEMOTORZONE E' QUI!\n");
    }
}
// --- FUNZIONI PUBBLICHE ---

void imposta_gioco() {
    pulisci_mappa();

    int undici_usato = 0;
    mappa_chiusa = 1;
    int scelta_mappa;
    mappa_chiusa = 0; // Reset flag

    do {
        printf("\n--- GESTIONE MAPPA ---\n");
        printf("1) Genera mappa (15 zone)\n");
        printf("2) Modifica zona\n");
        printf("3) Cancela zona\n");
        printf("4) Stampa mappa completa\n");
        printf("5) Chiudi mappa e torna al menu\n");
        printf("Scelta: ");
        scelta_mappa = leggi_int();

        switch (scelta_mappa) {
            case 1: genera_mappa(); break;
            case 2: inserisci_zona(); break;
            case 3: cancella_zona(); break;
            case 4: stampa_mappa(); break;
            case 5: 
                // Controllo requisiti minimi prima di chiudere
                if (mappa_valida()) {
                    mappa_chiusa = 1;
                    printf("Mappa validata e chiusa con successo. Puoi iniziare la partita.\n");
                } else {
                    printf("Sistemazione mappa necessaria prima di poter chiudere.\n");
                    // mappa_chiusa rimane 0, quindi il ciclo do-while non finisce
                }
                break;
            default: printf("Opzione non valida.\n");
        }
    } while (scelta_mappa != 5 || !mappa_chiusa);

    printf("\n--- CREAZIONE GIOCATORI ---\n");
    printf("Quanti giocatori (1-4)? ");
    num_giocatori = leggi_int();
    if (num_giocatori < 1 || num_giocatori > 4) {
        printf("Numero di giocatori non valido!\n");
        num_giocatori = 0;
        return;
    }

    for(int i=0; i<num_giocatori; i++) {
        giocatori[i] = malloc(sizeof(struct Giocatore));
        printf("Nome: "); scanf("%s", giocatori[i]->nome);
        
        strncpy(partecipanti[i], giocatori[i]->nome, 49); // Salvataggio per i crediti
        num_partecipanti_totali++;

        giocatori[i]->attacco_psichico = rand()%20+1;
        giocatori[i]->difesa_psichica = rand()%20+1;
        giocatori[i]->fortuna = rand()%20+1;
        giocatori[i]->pos_mondoreale = prima_zona_m;
        giocatori[i]->pos_soprasotto = prima_zona_s;
        giocatori[i]->mondo = 0; // Inizia nel Mondo Reale
        for(int j=0; j<3; j++){
            giocatori[i]->zaino[j] = nessun_oggetto;
        } 
        printf("Giocatore %d: %s creato con Attacco Psichico: %d, Difesa Psichica: %d, Fortuna: %d\n", 
                i+1, giocatori[i]->nome, giocatori[i]->attacco_psichico, 
                giocatori[i]->difesa_psichica, giocatori[i]->fortuna);
        int scelta_stat;
        int continua=1;
        do{
        if(undici_usato == 0){
            printf("Vuoi cambiare statistiche?\n 1:Attacco Psichico\n 2:Difesa Psichica\n 3:Nessuno\n 4:UndiciVirgolaCinque\n \nScelta: ");}
            else{
                printf("Vuoi cambiare statistiche?\n 1:Attacco Psichico\n 2:Difesa Psichica\n 3:Nessuno\nScelta: ");
            }
            scelta_stat = leggi_int();
            switch (scelta_stat)
            {
            case 1:
                if(scelta_stat ==1 && giocatori[i]->difesa_psichica -3 >=0){
                giocatori[i]->attacco_psichico = giocatori[i]->attacco_psichico + 3;
                giocatori[i]->difesa_psichica = giocatori[i]->difesa_psichica - 3;
                printf("Nuove statistiche - Attacco Psichico: %d, Difesa Psichica: %d\n", giocatori[i]->attacco_psichico, giocatori[i]->difesa_psichica);
                continua = 0; //esce dal ciclo
                }
                else{
                    printf("Non hai abbastanza difesa psichica per aumentare l'attacco psichico.\n");
                    printf("Continua con le modifiche.\n");
                }
                break;
            case 2:
                if(scelta_stat ==2 && giocatori[i]->attacco_psichico -3 >=0){
                giocatori[i]->difesa_psichica = giocatori[i]->difesa_psichica + 3;
                giocatori[i]->attacco_psichico = giocatori[i]->attacco_psichico - 3;
                printf("Nuove statistiche - Attacco Psichico: %d, Difesa Psichica: %d\n", giocatori[i]->attacco_psichico, giocatori[i]->difesa_psichica);
                }
                else{
                    printf("Non hai abbastanza attacco psichico per aumentare la difesa psichica.\n");
                    printf("Continua con le modifiche.\n");
                }
                break;
            case 3:
                printf("Nessuna modifica alle statistiche.\n");
                continua = 0; //esce dal ciclo
                break;
            case 4:
                if(scelta_stat ==4 && undici_usato == 0 && (giocatori[i]->fortuna -7) >=0){
                    giocatori[i]->attacco_psichico = giocatori[i]->attacco_psichico + 4;
                    giocatori[i]->difesa_psichica = giocatori[i]->difesa_psichica + 4;
                    giocatori[i]->fortuna = giocatori[i]->fortuna - 7;
                    printf("Nuove statistiche - Attacco Psichico: %d, Difesa Psichica: %d, Fortuna: %d\n", giocatori[i]->attacco_psichico, giocatori[i]->difesa_psichica, giocatori[i]->fortuna);
                    strncpy(giocatori[i]->nome, "UndiciVirgolaCinque", 49);
                    giocatori[i]->nome[49] = '\0';
                    undici_usato = 1;
                    continua = 0; //esce dal ciclo
                }
                else{
                    printf("Non hai abbastanza difesa psichica per aumentare l'attacco psichico.\n");
                    printf("Continua con le modifiche.\n");
                }
                break;
            default:
                printf("Scelta non valida.\n");
                //ripete il ciclo
            }

        }while(continua);
        
    }

    partita = 0;
    
}

void gioca() {
    if(!mappa_chiusa){
        printf("Errore: la mappa non è chiusa! Imposta il gioco prima di giocare.\n");
        return;
    }
    if(num_giocatori == 0){
        printf("Errore: nessun giocatore creato! Imposta il gioco prima di giocare.\n");
        return;
    }
    if(partita>0){
        printf("Errore: la partita e' gia' stata completata! Reimposta il gioco per una nuova partita.\n");
        return;
        exit(1);
    }

    int vittoria = 0;
    while(!vittoria) {
        int ordine[4], vivi = 0;
        // Creiamo la lista dei giocatori ancora in gioco
        for(int i=0; i<num_giocatori; i++) {
            if(giocatori[i] != NULL) {
                ordine[vivi++] = i;
            }
        }

        if(vivi == 0) {
            printf("\n--- TUTTI I GIOCATORI SONO STATI SCONFITTI. GAME OVER ---\n");
            break;
        }
        
        mescola_giocatori(ordine, vivi);

        for(int i=0; i<vivi && !vittoria; i++) {
            int id = ordine[i];
            
            // Verifichiamo che il giocatore non sia morto durante i turni precedenti dello stesso round
            if (giocatori[id] == NULL) continue;

            // Determiniamo il nemico nel mondo ATTUALE del giocatore
            enum Tipo_nemico nemico_qui;
            if (giocatori[id]->mondo == 0) {
                nemico_qui = giocatori[id]->pos_mondoreale->nemico;
            } else {
                nemico_qui = giocatori[id]->pos_soprasotto->nemico;
            }

            int turno_concluso = 0; 

            while (!turno_concluso && giocatori[id] != NULL) {

                // Ispezione zona attuale
                stampa_mini_mappa(id); // Funzione che descrive la zona

                printf("\n--- TURNO DI %s Posizione(%d/%d) ---\n", giocatori[id]->nome, calcola_indice_posizione(id), conta_zone());
                printf("1: Avanza\n2: Indietro\n3: Cambia Mondo\n4: Zaino (Usa/Raccogli)\n5: Status\n6: Combatti\n7: Mappa Completa\n8: Passa Turno\nScelta: ");
                
                int m; 
                m = leggi_int();

                switch(m) {
                    case 1:
                        if(nemico_qui != nessun_nemico){
                            printf("Devi sconfiggere il nemico prima di avanzare.\n");
                            printf("Digita 6 nel menu principale per combattere.\n");
                            break;
                        }
                        else{
                        avanza(id);
                        turno_concluso = 1; 
                        break;
                        }

                    case 2:
                        if(nemico_qui != nessun_nemico){
                            printf("Devi sconfiggere il nemico prima di tornare indietro.\n");
                            printf("Digita 6 nel menu principale per combattere.\n");
                            break;
                        }
                        else{
                        indietreggia(id);
                        turno_concluso = 1;
                        break;
                        }

                    case 3:
                        cambia_mondo(id);
                        turno_concluso = 1;
                        break;

                    case 4:
                        if(giocatori[id]->mondo == 0) {
                            printf("1: Raccogli oggetto in zona\n2: Usa oggetto dallo zaino\nScelta: ");
                            int s_zaino; 
                            s_zaino = leggi_int();
                            if(s_zaino == 1) raccogli_oggetto(id);
                            else {
                                printf("Quale slot (1-3)? \n");
                                for(int i=0; i<3; i++) {
                                    if(giocatori[id]->zaino[i] != nessun_oggetto) {
                                        printf("%d: %s\n", i+1, nome_oggetto(giocatori[id]->zaino[i]));
                                    } else {
                                        printf("%d: [Vuoto]\n", i+1);
                                    }
                                }
                                int slot; 
                                slot = leggi_int();
                                if(slot >= 1 && slot <= 3) {
                                    if (giocatori[id]->zaino[slot-1] == bicicletta) {
                                        // Salviamo se c'era un nemico prima di usare l'oggetto per sapere se consumarlo
                                        enum Tipo_nemico n_prima;
                                        if (giocatori[id]->mondo == 0) n_prima = giocatori[id]->pos_mondoreale->nemico;
                                        else n_prima = giocatori[id]->pos_soprasotto->nemico;

                                        usa_bicicletta(id);

                                        // Se abbiamo effettivamente scacciato qualcuno (non il boss), consumiamo la bici
                                        if (n_prima != nessun_nemico && n_prima != demotorzone) {
                                            giocatori[id]->zaino[slot-1] = nessun_oggetto;
                                            printf("Scacciando il nemico, la bici si e' rotta!\n");
                                        }
                                    }
                                    else if(giocatori[id]->zaino[slot-1] == bussola){
                                        usa_bussola(id);
                                        //monouso
                                        giocatori[id]->zaino[slot-1] = nessun_oggetto;
                                        printf("La bussola si e' rotta dopo l'uso!\n");
                                    } 
                                    else if (giocatori[id]->zaino[slot-1] == schitarrata_metallica) {
                                        usa_chitarra(id);
                                        //monouso
                                        giocatori[id]->zaino[slot-1] = nessun_oggetto;
                                        printf("Le corde della chitarra si sono spezzate per l'intensita'!\n");
                                    }
                                    else if(giocatori[id]->zaino[slot-1] == maglietta_fuocoinferno) {
                                        usa_maglietta(id);
                                        //monouso
                                        giocatori[id]->zaino[slot-1] = nessun_oggetto;
                                        printf("La maglietta ha preso fuoco dopo l'uso!\n");
                                    }
                                    else printf("Oggetto non presente o non usabile.\n");
                                }
                                else {
                                    printf("Slot non valido.\n");
                                }
                            }
                        } else {
                            printf("Nel Soprasotto puoi solo usare la bussola. Quale slot (1-3)? ");
                            int slot; 
                            slot = leggi_int();
                            if(slot >= 1 && slot <= 3 && giocatori[id]->zaino[slot-1] == bussola) usa_bussola(id);
                            else printf("Non puoi farlo qui.\n");
                        }
                        break;

                    case 5:
                        stampa_status(id);
                        stampa_posizione_gps(id);
                        break;
                    case 6: 
                        // Salviamo il tipo di nemico PRIMA del combattimento
                        enum Tipo_nemico nemico_da_battere = giocatori[id]->pos_soprasotto->nemico;
                        int nel_soprasotto = giocatori[id]->mondo;
                        combatti(id);

                        if(nemico_da_battere != nessun_nemico){
                            // Se il giocatore è ancora vivo dopo lo scontro
                            if (giocatori[id] != NULL) { 
                                // Se era nel soprasotto e il nemico era il boss, e ORA non c'è più (perché ucciso)
                                if (nel_soprasotto == 1 && 
                                    nemico_da_battere == demotorzone && 
                                    giocatori[id]->pos_soprasotto->nemico == nessun_nemico) {
                                    
                                    vittoria = 1;
                                    partita++; // Segnaliamo che la partita è stata completata
                                    printf("\nVITTORIA!!\n");
                                    printf("%s ha liberato Hawkins dal Demotorzone!\n", giocatori[id]->nome);
                                    registra_vincitore(giocatori[id]->nome);
                                }
                            }
                            turno_concluso = 1;
                        }
                        break;
                    case 7:
                        stampa_mappa();
                        break;

                    case 8:
                        turno_concluso = 1;
                        break;

                    default:
                        printf("Scelta non valida.\n");
                }

                // CONTROLLO CRITICO: Se il giocatore è morto durante l'azione, usciamo dal while
                if (giocatori[id] == NULL) {
                    turno_concluso = 1;
                }
            }
            if (vittoria) break;
        }
    }
}

void crediti() {
    printf("\nCreato da studente: Duranti Alex \nMatricola: 391534\n");

    printf("\n--- GLI EROI DI HAWKINS (Vincitori) ---\n");
    if (vincitori_count == 0) {
        printf("Nessuno ha ancora sconfitto il Demotorzone...\n");
    } else {
        for (int i = 0; i < vincitori_count; i++) {
            printf("%d° POSTO: %s\n", i + 1, vincitori[i]);
        }
    }

    printf("--- I PARTECIPANTI DEL TURNO ---\n");
    if (num_partecipanti_totali == 0) {
        printf("Nessun giocatore ha ancora partecipato.\n");
    } else {
        for (int i = 0; i < num_partecipanti_totali; i++) {
            // Salta se il nome è vuoto (protezione extra)
            if (strlen(partecipanti[i]) == 0) continue;

            int ha_vinto = 0;
            for (int j = 0; j < vincitori_count; j++) {
                if (strcmp(partecipanti[i], vincitori[j]) == 0) {
                    ha_vinto = 1;
                    break;
                }
            }

            if (ha_vinto) {
                printf("- %s [TRIONFATORE]\n", partecipanti[i]);
            } 
            else if(!ha_vinto) {
                if(vita_giocatori[i] == 1) {
                    printf("- %s [SOPRAVVISSUTO]\n", partecipanti[i]);
                } else {
                    printf("- %s [DISPERSO]\n", partecipanti[i]);
                }
            }
        }
    }
    printf("\nGrazie per aver giocato a 'Cosestrane'!\n");
}

void termina_gioco() {
    pulisci_mappa();
    for(int i=0; i<4; i++) { if(giocatori[i]) free(giocatori[i]); }
    printf("Memoria liberata. Arrivederci!\n");
}
