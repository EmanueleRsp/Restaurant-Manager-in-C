#ifndef KITCHEN_H
#define KITCHEN_H

#include <stdint.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <ctype.h>

#include "../utils/utils.h"
#include "../utils/dataValidation/validate.h"


/*--------------------------------- STRUTTURE ----------------------------------*/


// Enumeratore per specificare il tipo di richiesta da effettuare
typedef enum {
    TAKE = 0,
    READY = 1
} operazione_t; 

// Struttura che rappresenta una richiesta
typedef struct {
    operazione_t op;
    char serializedBody[BUFFER_LEN - sizeof(operazione_t) - 1];
} request_t;


/*--------------------------------- FUNZIONI ----------------------------------*/


// FUNZIONI DI DISPLAY ----------------------------------

/*
* Funzione che stampa il messaggio di benvenuto
* @return void
* @param void
*/
void start();

/*
* Stampa messaggio di fine pasto
* @return void
* @param void
*/
void printClosingMsg();


/*
* Funzione che stampa le opzioni disponibili del client
* @return void
* @param void
*/
void printCommands();

/*
* Funzione che stampa la linea per l'input dell'utente
* @return void
* @param void
*/
void printInputLine();


// GESTIONE LISTA COMANDE PRESE IN CARICO --------------------------

/*
* Funzione che verifica che la comanda sia stata presa in carico
* @return comanda_t*: puntatore alla lista
* @param int: tavolo
* @param int: comanda
*/
comanda_t *getComanda(comanda_t *, int, int);

/*
* Funzione che rimuove una comanda dalla lista
* @return void
* @param comanda_t**: puntatore alla lista
* @param int: tavolo
* @param int: comanda
*/
void removeComanda(comanda_t **, int, int);

/* 
* Stampa il numero di comande in attesa
* @return void
* @param int: numero di comande in attesa
*/
void printComandeInAttesa(int);


// GESTIONE COMANDI INPUT ----------------------------------

/*
* Funzione che prende in carico una comanda
* @return int: 0 se la comanda è stata presa in carico, -1 altrimenti
* @param int: socket del server
*/
int take(int);

/*
* Funzione che mostra le comande prese in carico
* @return void
* @param void
*/
void show();

/*
* Funzione che imposta lo stato di una comanda
* @return void
* @param int: socket del server
* @param int: tavolo
* @param int: comanda
*/
void ready(int, int, int);


// OPERAZIONI DEL SERVER ----------------------------------

/*
* Funzione che chiude il device
* @return void
* @param int: socket del server
*/
void closeDevice(int);

#endif