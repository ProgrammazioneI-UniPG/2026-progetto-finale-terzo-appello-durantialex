// Definizione dei tipi necessari alla libreria
#ifndef GAMELIB_H
#define GAMELIB_H

//Definizione del tipo enumerativo per le zone del gioco
enum Tipo_zona {    bosco, 
                    scuola, 
                    laboratorio, 
                    caverna, 
                    strada, 
                    giardino, 
                    supermercato, 
                    centrale_elettrica, 
                    deposito_abbandonato, 
                    stazione_polizia };
                    
//Definizione del tipo enumerativo per i nemici del gioco
enum Tipo_nemico {  nessun_nemico, 
                    billi, 
                    democane, 
                    demotorzone };

//Definizione del tipo enumerativo per gli oggetti del gioco presente nello zaino del giocatore
enum Tipo_oggetto { nessun_oggetto, 
                    bicicletta, 
                    maglietta_fuocoinferno, 
                    bussola, 
                    schitarrata_metallica };

// Strutture Zone (Liste doppiamente concatenate)
struct Zona_mondoreale {
    enum Tipo_zona tipo;
    enum Tipo_nemico nemico;
    enum Tipo_oggetto oggetto;
    struct Zona_mondoreale *avanti;
    struct Zona_mondoreale *indietro;
    struct Zona_soprasotto *link_soprasotto;
};

struct Zona_soprasotto {
    enum Tipo_zona tipo;
    enum Tipo_nemico nemico;
    struct Zona_soprasotto *avanti;
    struct Zona_soprasotto *indietro;
    struct Zona_mondoreale *link_mondoreale;
};

// Struttura Giocatore
struct Giocatore {
    char nome[50];
    int mondo; // 0: Reale, 1: Soprasotto
    struct Zona_mondoreale *pos_mondoreale;//puntatore alla mappa del Mondo Reale
    struct Zona_soprasotto *pos_soprasotto;//puntatore alla mappa del Soprasotto
    int attacco_psichico;
    int difesa_psichica;
    int fortuna;
    enum Tipo_oggetto zaino[3];
};

// Funzioni pubbliche
void imposta_gioco();
void gioca();
void termina_gioco();
void crediti();

#endif
