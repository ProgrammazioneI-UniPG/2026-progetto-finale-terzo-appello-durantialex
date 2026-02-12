[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/5fsIc7xe)
# Progetto-finale-2025-Cosestrane
Progetto finale Programmazione Procedurale UniPG Informatica

---
## Nome: Alex
---
## Cognome: Duranti
---
## Matricola: 391534
---
## Commenti/modifiche al progetto:
Il progetto implementa un gioco testuale in linguaggio **C** ispirato alla traccia d’esame *“Cosestrane”*.  
Il gioco è ambientato nella cittadina di **Occhinz**, collegata a una dimensione oscura chiamata **Soprasotto**.

I giocatori esplorano due mappe parallele (Mondo Reale e Soprasotto), combattono nemici e cercano di sconfiggere il **Demotorzone**, condizione necessaria per la vittoria.

---

## Struttura del progetto

Il progetto è composto **esclusivamente** dai tre file richiesti dalla traccia:

- `main.c`  
  Contiene:
  - la funzione **main()** per la gestione del menu principale.

- `gamelib.h`  
  Contiene:
  - dichiarazioni delle **funzioni pubbliche**
  - definizioni delle **strutture dati**
  - definizioni degli **enum**

- `gamelib.c`  
  Contiene tutta la logica del gioco:
  - impostazione del gioco
  - generazione e gestione della mappa
  - gestione dei giocatori
  - gestione dei nemici 
  - combattimento
  - turni di gioco
  - oggetti
    
  Tutte le funzioni non visibili all’esterno di gamelib.c sono dichiarate **static**.

---

## Compilazione

Il progetto è stato testato su **Linux Mint 22.2** con **GCC ≥ 13.3.0**.

Utilizzare **inizialmente** queste due comandi in **qualsiasi** ordine per la creazione dei file **.o**
```bash
gcc -c main.c -std=c11 -Wall
```
```bash
gcc -c gamelib.c -std=c11 -Wall
```
Utilizzare **successivamente** questo comando per la creazione del file di avvio del gioco
```bash
gcc -o gioco main.o gamelib.o
```
Infine scrivere:
```bash
./gioco
