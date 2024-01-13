#ifndef TABLE_H
#define TABLE_H

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
    UNLOCK = 0,
    MENU = 1,
    COMANDA = 2,
    CONTO = 3
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

/*
* Stampa il messaggio della schermata di sblocco
* @return void
* @param void
*/
void displayLock();

/*
* Stampa i dettagli dei comandi
* @return void
* @param void
*/
void displayCommandDetails();


// GESTIONE DELLE COMANDE ----------------------------------

/*
* Stampa le comande inviate al server per il pasto in corso
* @return void
* @param void
*/
void printComandeInviate();

// FUNZIONI PER LA CONNESSIONE AL SERVER ----------------------------------

/*
* Tenta di sbloccare il device
* @return int: 0 se lo sblocco è andato a buon fine, -1 altrimenti
* @param char[]: codice di prenotazione inserito
* @param int: socket del server
*/
int unlock(char[], int);

/*
* Funzione che invia la richiesta per recuperare il menu
* @return void
* menu: struttura che conterrà il menu
* @param int: socket del server
*/
void getMenu(int);

/*
* Funzione che invia la comanda
* @return void
* @param comanda_dish_t *: puntatore alla lista di piatti da inviare
* @param int: socket del server
*/
void comanda(comanda_dish_t *, int);

/*
* Funzione che invia la richiesta del conto
* @return int: 0 se il conto è stato richiesto correttamente, -1 altrimenti
* @param int: socket del server
*/
int conto(int);


// OPERAZIONI DEL SERVER ----------------------------------

/*
* Funzione che chiude il client
* @return void
* @param int: socket del server
*/
void closeDevice(int);

/*
* Aggiorna lo stato di un comanda inviata
* @return void
* @param int: numero della comanda
* @param comanda_state_t: nuovo stato della comanda
*/
void updateComandaStatus(int, comanda_state_t);

#endif