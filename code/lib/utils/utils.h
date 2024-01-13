#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


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

// Dimensione del codice di prenotazione
#define BOOK_CODE_LEN 7

// Dimensione del codice di un piatto
#define DISH_CODE_LEN 3

// Dimensione del nome di un piatto
#define DISH_NAME_LEN 50

// Dimensione massima del timestamp
#define TIMESTAMP_LEN 20

// Orario di apertura
#define OPENING_TIME 0

// Ultima ora di apertura della giornata
#define CLOSING_TIME 23

// Numero massimo di persone per tavolo
#define MAX_PEOPLE_PER_TABLE 100

// Numero massimo di portate per categoria
#define MAX_CATEGORY_INDEX 99

// Numero massimo di piatti dello stesso tipo ordinabili per comanda
#define MAX_DISHES_PER_ORDER 99

// Numero massimo di comande per tavolo
#define MAX_COMANDE_PER_TABLE 99999


/*--------------------------------- STRUTTURE ----------------------------------*/


// Struttura che rappresenta una risposta
typedef struct {
    int header;
    char serializedBody[BUFFER_LEN - sizeof(int) - 1];
} response_t;

typedef enum {
    STOP = 100,
    UPDATE = 101,
    INCREASE = 102,
    DECREASE = 103
} server_operation_t;

// Struttura che rappresenta una comunicazione dal server
typedef struct {
    server_operation_t header;
    char serializedBody[BUFFER_LEN - sizeof(int) - 1];
} server_message_t;


// STRUTTURE PER LA GESTIONE DELLE PRENOTAZIONI ---------------------------------

// Struttura che rappresenta una prenotazione
typedef struct {
    char cognome[COGNOME_LEN];
    int numeroPersone;
    char data[DATE_LEN];
    int ora;
} reservation_t;

// Struttura che rappresenta un tavolo
typedef struct table_t{
    int codice;
    int sala;
    char info[INFO_TAVOLO_LEN];
    struct table_t *next;
} table_t;

// Struttura che rappresenta un codice di prenotazione
typedef char bookingCode_t[BOOK_CODE_LEN]; 

// Struttura che rappresenta una prenotazione effettuata
typedef struct {
    bookingCode_t codicePrenotazione;
    int tavolo;
    int sala;
} booking_t;

typedef enum {
    PRENOTATO = 0,
    CORSO = 1,
    TERMINATA = 2
} reservation_state_t;


// STRUTTURE PER LA GESTIONE DELLE COMANDE -------------------------------------

// Struttura che rappresenta il codice di un piatto
typedef char dishCode_t[DISH_CODE_LEN]; 

// Enumeratore per specificare lo stato di una comanda
typedef enum {
    ATTESA = 0,
    PREPARAZIONE = 1,
    SERVIZIO = 2
} comanda_state_t;

// Struttura che rappresenta un piatto
typedef struct menu_dish_t{
    dishCode_t codice;
    char nome[DISH_NAME_LEN];
    int prezzo;
    struct menu_dish_t *next;
} menu_dish_t;

// Utilizzata per la trasmissione dei piatti ordinati
typedef struct comanda_dish_t {
    dishCode_t codice;
    int quantita;
    struct comanda_dish_t *next;
} comanda_dish_t;

// Struttura che rappresenta una comanda
typedef struct comanda_t {
    char data[DATE_LEN];
    int tavolo;
    int ora;
    int numero;
    char dataRis[DATE_LEN];
    comanda_state_t stato;
    comanda_dish_t *piatti;
    struct comanda_t *next;
} comanda_t;


// STRUTTURE PER LA GESTIONE DEL CONTO -------------------------------------

// Struttura che rappresenta una voce del conto
typedef struct subtotale_conto_t {
    dishCode_t codice;
    int quantita;
    int prezzo;
    struct subtotale_conto_t *next;
} subtotale_conto_t;


/*--------------------------------- FUNZIONI ----------------------------------*/

// UTILITA' -----------------------------------

/*
* Funzione che convalida la porta inserita, se non valida termina il programma
* @return void
* @param int: numero di argomenti passati al programma
* @param char *[]: array di stringhe contenente gli argomenti passati al programma
* @param int *: puntatore all'intero che conterrà la porta se valida
*/
void portValidation(int, char *[], int *);

/*
* Recupera la data odierna
* @return void
* @param char []: stringa che conterrà la data
*/
void getTodayDate(char []);

/*
* Recupera ora, giorno, mese e anno correnti
* @return void
* @param int *: puntatore all'intero che conterrà l'ora
* @param int *: puntatore all'intero che conterrà il giorno
* @param int *: puntatore all'intero che conterrà il mese
* @param int *: puntatore all'intero che conterrà l'anno
*/
void getCurrentDateTime(int *, int *, int *, int *);

/*
* Recupera il timestamp attuale
* @return void
* @param char []: puntatore alla stringa che conterrà il timestamp
*/
void getTimestamp(char []);

/*
* Scompone la data in giorno, mese e anno
* @return void
* @param char []: stringa contenente la data
* @param int *: puntatore all'intero che conterrà il primo campo
* @param int *: puntatore all'intero che conterrà il secondo campo
* @param int *: puntatore all'intero che conterrà il terzo campo
*/
void scomposeData(char[], int *, int *, int *);

/*
* Verifica se il primo codice precede il secondo
* @return int: 1 se il primo carattere è precedente, 0 altrimenti
* @param char: primo carattere
* @param char: secondo carattere
*/
int isCodePrev(dishCode_t, dishCode_t);

/*
* Verifica se la prima comanda precede la seconda
* @return int: 1 se la prima comanda è precedente, 0 altrimenti
* @param comanda_t *: prima comanda
* @param comanda_t *: seconda comanda
*/
int isComandaPrev(comanda_t *, comanda_t *);

/*
* Recupera il giorno precedente
* @return void
* @param char []: stringa contenente la data, in cui verrà inserita la data precedente 
*/
void previousDay(char []);

// GESTIONE DELLE STRUTTURE (TAVOLI) -----------------------------------

/*
* Libera la memoria allocata per la lista dei tavoli
* @return void
* @param table_t *: puntatore alla testa della lista dei tavoli
*/
void deallocateTableList(table_t *);

/*
* Serializza la lista dei tavoli
* @return void
* @param table_t *: puntatore alla testa della lista dei tavoli
* @param char *: buffer in cui inserire la lista serializzata
*/
void serializeTableList(table_t *, char *);

/*
* Deserializza la lista dei tavoli
* @return void
* @param table_t **: puntatore alla testa della lista dei tavoli, passato per riferimento
* @param char *: buffer contenente la lista serializzata
*/
void deserializeTableList(table_t **, char *);


// GESTIONE DELLE LISTE (MENU) -----------------------------------


/*
* Serializza la lista dei piatti del menu
* @return void
* @param menu_dish_t *: puntatore alla testa della lista dei piatti
* @param char *: buffer in cui inserire la lista serializzata
*/
void serializeMenu(menu_dish_t *, char *);

/*
* Deserializza la lista dei piatti del menu
* @return void
* @param menu_dish_t **: puntatore alla testa della lista dei piatti, passato per riferimento
* @param char *: buffer contenente la lista serializzata
*/
void deserializeMenu(menu_dish_t **, char *);

/*
* Mosttra a video la lista dei piatti disponibili
* @return void
* @param menu_dish_t *: puntatore alla testa della lista dei piatti
*/
void printMenu(menu_dish_t *);

/*
* Libera la memoria allocata per la lista dei piatti
* @return void
* @param menu_dish_t *: puntatore alla testa della lista dei piatti
*/
void deallocateMenu(menu_dish_t *);


// GESTIONE DELLE LISTE (COMANDE) -----------------------------------

/*
* Serializza la lista dei piatti della comanda
* @return void
* @param menu_dish_t *: puntatore alla testa della lista dei piatti
* @param char *: buffer in cui inserire la lista serializzata
*/
void serializeComandaDishList(comanda_dish_t *, char *);

/*
* Serializza la lista delle comande
* @return void
* @param comanda_t *: puntatore alla testa della lista delle comande
* @param char *: buffer in cui inserire la lista serializzata
*/
void serializeComandeList(comanda_t *, char *);

/*
* Deserializza la lista dei piatti della comanda
* @return void
* @param menu_dish_t **: puntatore alla testa della lista dei piatti, passato per riferimento
* @param char *: buffer contenente la lista serializzata
*/
void deserializeComandaDishList(comanda_dish_t **, char *);

/*
* Deserializza la lista delle comande
* @return void
* @param comanda_t **: puntatore alla testa della lista delle comande, passato per riferimento
* @param char *: buffer contenente la lista serializzata
*/
void deserializeComandeList(comanda_t **, char *);

/*
* Liberazione della memoria allocata per la lista delle comande
* @return void
* @param comanda_t *: lista delle comande
*/
void deallocateComandeList(comanda_t *);

/*
* Libera la memoria allocata per la lista dei piatti della comanda
* @return void
* @param comanda_dish_t *: puntatore alla testa della lista dei piatti
*/
void deallocateComandaDishList(comanda_dish_t *);

/*
* Stampa la lista delle comande recuperate
* @return void
* @param comanda_t *: lista delle comande
* @param int: modalità (0 = stampa completa, 1 = tavolo associato, 2 = stato associato, 3 = nulla, 4 = stampa completa con orario)
*/
void printComandeList(comanda_t *, int);

/*
* Stampa la lista dei piatti ordinati con la comanda
* @return void
* @param comanda_dish_t *: lista dei piatti ordinati
*/
void printComandaDishList(comanda_dish_t *);

/*
* Aggiungi una comanda in lista ordinata
* @return void
* @param comanda_t **: puntatore alla testa della lista
* @param comanda_t *: puntatore alla comanda da aggiungere
*/
void sortedInsertComanda(comanda_t **, comanda_t *);

/*
* Inserisce un piatto in ordine nella lista
* @return void
* @param comanda_dish_t **: puntatore alla testa della lista
* @oaram comanda_dish_t *: puntatore al piatto da inserire
*/
void sortedInsertDish(comanda_dish_t **, comanda_dish_t *);


// GESTIONE DELLE LISTE (CONTI) -----------------------------------

/*
* Serializza la lista dei conti
* @return void
* @param conto_t *: puntatore alla testa della lista delle voci del conto
* @param char *: buffer in cui inserire la lista serializzata
*/
void serializeConto(subtotale_conto_t *, char *);

/*
* Deserializza la lista dei conti
* @return void
* @param conto_t **: puntatore alla testa delle voci del conto, passato per riferimento
* @param char *: buffer contenente la lista serializzata
*/
void deserializeConto(subtotale_conto_t **, char *);

/*
* Dealloca le voci del conto
* @return void
* @param conto_t *: puntatore alla testa delle voci del conto
*/
void deallocateConto(subtotale_conto_t *);

/*
* Stampa a video la lista delle voci del conto
* @return void
* @param conto_t *: puntatore alla testa delle voci del conto
*/
void printConto(subtotale_conto_t *);

/*
* Inserisce una voce del conto in ordine nella lista
* @return void
* @param conto_t **: puntatore alla testa della lista
* @oaram conto_t *: puntatore alla voce del conto da inserire
*/
void sortedInsertConto(subtotale_conto_t **, subtotale_conto_t *);

#endif