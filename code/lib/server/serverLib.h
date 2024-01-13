#ifndef SERVER_H
#define SERVER_H

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
#include "fileManagement/fileManager.h"
#include "../utils/dataValidation/validate.h"


/*--------------------------------- COSTANTI ----------------------------------*/


// Dimensione della coda di richieste
#define QUEUE_LEN 10

// Dimensione massima del messaggio di errore
#define ERROR_MSG_LEN 200


/*--------------------------------- STRUTTURE ----------------------------------*/


// Enumeratore per specificare il tipo di richiesta da effettuare
typedef enum {
    FIND = 0,
    BOOK = 1,
    TAKE = 2,
    READY = 3,
    UNLOCK = 4,
    MENU = 5,
    COMANDA = 6,
    CONTO = 7
} operazione_t; 

// Struttura che rappresenta una richiesta
typedef struct {
    operazione_t op;
    char serializedBody[BUFFER_LEN - sizeof(operazione_t) - 1];
} request_t;

// Enumeratore per specificare il tipo di device
typedef enum {
    CLIENT = FIND,
    KITCHEN = TAKE,
    TABLE = UNLOCK,
    NOTID = -1
} device_t;

// Struttura che rappresenta una nuova connessione al server, permette la creazione di una coda
typedef struct connection_t{
    
    int socket;
    struct sockaddr_in address;
    int addressLength;

    device_t dev;

    struct connection_t *next;

} connection_t;

// Struttura che rappresenta un tavolo in cui è attualmente in corso un pasto
typedef struct active_table_device_t {

    int socket;             // Identifica il socket del table device associato al tavolo
    int tavoloAssegnato;    // Tavolo assegnato
    int orario;             // Orario di attivazione del table device
    char data[DATE_LEN];    // Data di attivazione del table device
    bookingCode_t codice;

    struct active_table_device_t *next;
} active_table_device_t;


// Struttura che memorizza i dati relativi alle richieste effettuate dai client
typedef struct active_client_device_t {

    int socket;
    reservation_t reservation;

    struct active_client_device_t *next;
} active_client_device_t;

// Elemento che memorizza l'andamento di una comanda
typedef struct comanda_order_t {
    int tableSocket;
    int kitchenSocket;
    comanda_t *comanda;
    struct comanda_order_t *next;
    struct comanda_order_t *prev;
} comanda_order_t;


/*--------------------------------- FUNZIONI ----------------------------------*/


// FUNZIONI DI DISPLAY ----------------------

/*
* Funzione che stampa il messaggio di avvio
* @return void
* @param void
*/
void start();

/*
* Stampa le opzioni dei comandi disponibili del server
* @return void
* @param void
*/
void printCommands();


// GESTIONE DELLE CONNESSIONI ------------------------------

/*
* Aggiunge una connessione alla lista delle connessioni attive (inserimento in testa)
* @return void
* @param connection_t *: puntatore alla connessione da aggiungere
*/
void addConnection(connection_t *);

/*
* Rimuove una connessione dalla lista delle connessioni attive
* @return void
* @param int: socket della connessione da rimuovere
*/
void removeConnection(int);

/*
* Rimuove tutte le connessioni dalla lista delle connessioni attive
* @return int: 0 se la rimozione è andata a buon fine, il socket in cui si è verificato l'errore altrimenti
*/
int removeAllConnections();

/*
* Recupera il tipo di device associato al socket
* @return device: tipo di device associato al socket
* @param int: socket
*/
device_t getDevice(int);


// GESTIONE DEI TABLE DEVICE ATTIVI ----------------------------

/*
* Aggiunge un tavolo alla lista dei tavoli attivi (inserimento ordinato per numero di tavolo)
* @return void
* @param active_table_device_t *: puntatore al tavolo da aggiungere
*/
void addActiveTable(active_table_device_t *);

/*
* Rimuove un tavolo dalla lista dei tavoli attivi
* @return void
* @param int: tavolo da rimuovere
*/
void removeActiveTable(int);

/*
* Rimuove tutti i tavoli dalla lista dei tavoli attivi, inviando il conto a ciascuno di essi
* @return void
*/
void removeAllActiveTables();

/*
* Recupera il tavolo attivo associato al table device
* @return active_table_device_t *: tavolo attivo associato al table device
* @param int: socket del table device
*/
active_table_device_t *getActiveTable(int);

/*
* Verifica se un tavolo è attivo
* @return active_table_device_t *: tavolo attivo, NULL altrimenti
* @param char[]: data
* @param int: tavolo
* @param int: mode (0 = socket, 1 = tavolo)
*/
active_table_device_t *isTableActive(char[], int, int);


// GESTIONE DELLE PROPOSTE ------------------------------

/*
* Aggiunge una proposta alla lista delle proposte (inserimento in testa)
* @return void
* @param active_client_device_t *: puntatore alla proposta da aggiungere
*/
void addProposedTable(active_client_device_t *);

/*
* Rimuove una proposta dalla lista delle proposte
* @return void
* @param int: socket della proposta da rimuovere
*/
void removeProposedTable(int);

/*
* Rimuove tutte le proposte dalla lista delle proposte
* @return void
* @param void
*/
void removeAllProposedTables();

/*
* Recupera la proposta associata al socket
* @return active_client_device_t *: proposta associata al socket
* @param int: socket
*/
active_client_device_t *getProposedTable(int);


// GESTIONE ORDINE DELLE COMANDE ------------------------------

/*
* Recupera la comanda più vecchia in attesa
* @return comanda_order_t *: puntatore alla comanda più vecchia in attesa
* @param void
*/
comanda_order_t *getOldestWaitingComandaOrder();

/*
* Recupera le comande in preparazione da un kitchen device
* @return comanda_t *: puntatore alla comanda in preparazione
* @param int: socket del kitchen device
*/
comanda_t *getPreparatingComandaOrder(int);

/*
* Rimuove una comanda dall'ordine delle comande
* @return void
* @param comanda_order_t *: puntatore alla comanda da rimuovere
*/
void deleteComandaOrder(comanda_order_t *);

/*
* Recupera le informazioni su una comanda
* @return comanda_order_t *: puntatore alla comanda
* @param char[]: data
* @param int: tavolo
* @param int: ora
* @param int: numero della comanda
*/
comanda_order_t *getComandaOrderByComanda(char [], int, int, int);

/*
* Rimuove le comande in attesa dalla coda, relative ad un tavolo
* @return void
* @param int: socket
*/
void removeComandeByTableDevice(int);

/*
* Ripone in attesa le comande che erano in preparazione da un kitchen device
* @return void
* @param int: socket
*/
void resetComandeByKitchenDevice(int);

/*
* Aggiunge una comanda all'ordine delle comande (inserimento in testa)
* @return void
* @param int: socket
* @param comanda_t *: puntatore alla comanda da aggiungere
*/
void addComandaOrder(int, comanda_t *);

/*
* Recupera il numero di comande in attesa presenti nella coda
* @return int: numero di comande in attesa
* @param void
*/
int getWaitingComandeNum();



// GESTIONE DEI COMANDI ---------------------------------- 

/*
* Restituisce lo stato di tutte le comande giornaliere
* @return void
*/
void stat();

/*
* Restituisce tutte le comande in tale stato per ogni tavolo in cui è in corso un pasto
* @return void
* @param comanda_state_t: stato delle comande da restituire
*/
void statStatus(comanda_state_t);

/*
* Restituisce tutte le comande relative al pasto in corso per tale tavolo
* @return void
* @param table: tavolo di cui restituire le comande
*/
void statTable(int);


// GESTIONE DELLE RICHIESTE ------------------------------

/*
* Gestisce la richiesta "find" del client device
* @return void
* @param int: socket
* @param char *: corpo della richiesta
*/
void findManager(int, char *);

/*
* Gestisce la richiesta "book" del client device
* @return void
* @param int: socket
* @param char *: corpo della richiesta
*/
void bookManager(int, char *);

/*
* Gestisce la richiesta "take" del kitchen device
* @return void
* @param int: socket
*/
void takeManager(int);

/*
* Gestisce la richiesta "ready" del kitchen device
* @return void
* @param int: socket
* @param char *: corpo della richiesta
*/
void readyManager(int, char *);

/*
* Gestisce la richiesta "unlock" del table device
* @return void
* @param int: socket
* @param char *: corpo della richiesta
*/
void unlockManager(int, char *);

/*
* Gestisce la richiesta "menu" del table device
* @return void
* @param int: socket
*/
void menuManager(int);

/*
* Gestisce la richiesta "comanda" del table device
* @return void
* @param int: socket
* @param char *: corpo della richiesta
*/
void comandaManager(int, char *);

/*
* Gestisce la richiesta "conto" del table device
* @return void
* @param int: socket
*/
void contoManager(int);

/*
* Calcola il subtotale per ogni voce del conto
* @return void
* @param subtotale_conto_t *: lista contentente le voci del conto da aggiornare
* @param menu_dish_t *: puntatore al menu
*/
void computeSubtotals(subtotale_conto_t *, menu_dish_t *);

/*
* Gestisce una richiesta errata
* @return void
* @param int: socket
* @param char *: messaggio di errore
*/
void reqErrorManager(int, char *);


// GESTIONE DELLE NOTIFICHE ------------------------------

/*
* Notifica tutti i kitchen devices (tranne chi ha richiesto la take) sull'aggiornamento delle comande in attesa
* @return void
* @param server_operation_t: messaggio da inviare (INCREASE/DECREASE)
* @param int: socket del kitchen device che ha generato la modifica
*/
void notifyKitchenDevices(server_operation_t, int);

/*
* Notifica il table device sullo stato della comanda
* @return void
* @param int: socket
* @param int: comanda
* @param comanda_state_t: stato della comanda
*/
void notifyTableDevice(int, int, comanda_state_t);



// CHIUSURA SERVER ----------------------------------------

/*
* Tenta di arrestare il server
* @return void
* @param fd_set *: puntatore al set dei descrittori
* @param int: socket del server
*/
void closeDevice(fd_set *, int);

#endif