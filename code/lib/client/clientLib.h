#ifndef CLIENT_H
#define CLIENT_H

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


/*--------------------------------- COSTANTI ----------------------------------*/


// Dimensione massima che può avere il cognome di un cliente
#define COGNOME_LEN 30

// Dimensione massima che può avere il comando inserito dall'utente
#define COMMAND_LEN 100

// Dimensione massima che può avere il buffer
#define BUFFER_LEN 1024

// Dimensione massima delle informazioni su un tavolo
#define INFO_TAVOLO_LEN 20

// Dimensione della data
#define DATE_LEN 9


/*--------------------------------- STRUTTURE ----------------------------------*/


// Enumeratore per specificare il tipo di richiesta da effettuare
typedef enum {
    FIND = 0,
    BOOK = 1
} operazione_t; 

// Struttura che rappresenta una richiesta
typedef struct {
    operazione_t op;
    char serializedBody[BUFFER_LEN - sizeof(operazione_t) - 1];
} request_t;


/*--------------------------------- FUNZIONI ----------------------------------*/


// FUNZIONI DI DISPLAY

/*
* Funzione che stampa il messaggio di benvenuto
* @return void
* @param void
*/
void start();


/*
* Funzione che chiude il client
* @return void
* @param int: socket del server
*/
void closeDevice(int);


/*
* Funzione che stampa le opzioni disponibili del client
* @return void
* @param void
*/
void printCommands();


// FUNZIONI PER LA CONNESSIONE AL SERVER

/*
* Funzione che richiede disponibilità dei tavoli al server
* @return int: numero di tavoli disponibili
* @param reservation_t: prenotazione da inviare
* @param int: socket del server
*/
int find(reservation_t, int);


/*
* Funzione che invia una prenotazione al server
* @return void
* @param int: opzione scelta
* @param int: socket del server
*/
void book(int, int);


#endif