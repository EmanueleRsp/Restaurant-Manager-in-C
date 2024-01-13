#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <dirent.h>
#include <limits.h>

#include "../../utils/utils.h"


/*--------------------------------- COSTANTI ----------------------------------*/


// Dimensione massima che può avere la riga di un file
#define LINE_LEN 100


/*------------------------------ STRUTTURE DATI -------------------------------*/

// Rappresenta una prenotazione nel file
typedef struct reservationRow_t {
    bookingCode_t codice;
    int tavolo;
    char data[DATE_LEN];
    int ora;
    char cognome[COGNOME_LEN];
    int numeroPersone;
    reservation_state_t state;
    char timestamp[TIMESTAMP_LEN];
} reservationRow_t; 


/*--------------------------------- FUNZIONI ----------------------------------*/

// FUNZIONI DI UTILITA' --------------------

/*
* Verifica se un elemento è presente nell'array
* @return int: 1 se l'elemento è presente, 0 altrimenti
* @param int *: array in cui cercare l'elemento
* @param int: dimensione dell'array
* @param int: elemento da cercare
* @param int *: puntatore all'intero che conterrà l'indice dell'elemento se presente, NULL altrimenti
*/
int isInArray(int *, int, int, int *);

/*
* Genera un codice di prenotazione univoco per un certo mese
* @return void
* @param char[]: array di caratteri in cui inserire il codice generato
* @param int: mese per cui generare il codice
* @param int: anno 
*/
void getNewCode(char[], int, int);

/*
* Verifica se un codice è già presente nel file delle prenotazioni
* @return int: 1 se il codice è univoco, 0 altrimenti
* @param char[]: codice da verificare
* @param int: mese della prenotazione
* @param int: anno della prenotazione
*/
int isUnique(char[], int, int);

/*
* Funzione che conta il numero di righe di un file
* @return int: numero di righe del file
* @param char *: path del file
*/
int recordNumber(char *);

/*
* Elimina le righe di un determinato file che soddisfano i criteri specificati in determinate colonne
* @return int: numero di righe mantenute, -1 in caso di errore
* @param char *: path del file
* @param int []: array di interi contenente gli indici delle colonne da controllare
* @param char *[]: array di stringhe contenente i valori da cercare nelle colonne specificate
* @param int: numero di elementi negli array precedenti
*/
int deleteRecord(char *, int [], char *[], int);


// FUNZIONI PER LA GESTIONE DELLE PRENOTAZIONI --------------------

/*
* Recupera la lista dei tavoli disponibili
* @return table_t *: array di tavoli disponibili, NULL in caso di errore o se non ce ne sono
* @param int: numero di persone per cui è richiesta la prenotazione
* @param char *: data della prenotazione
* @param int: ora della prenotazione
* @param int *: puntatore a intero dove salvare il numero di tavoli disponibili
*/
table_t *getAvailableTable(int, char *, int, int *);

/*
* Recupera la lista dei codici dei tavoli prenotati in un certo orario
* @return int: il numero di tavoli disponibili, -1 in caso di errore
* @param char *: file path dove si trovano le prenotazioni
* @param char[]: data della prenotazione
* @param int: ora della prenotazione
* @param int[]: array di interi dove salvare i codici dei tavoli disponibili
*/
int getBookedTableCode(char *, char[], int, int[]);

/*
* Recupera il numero di sala di un tavolo
* @return int: numero della sala, -1 in caso di errore
* @param int: numero del tavolo
*/
int getTableRoom(int);

/*
* Salva una prenotazione nel file delle prenotazioni
* @return int: 0 in caso di successo, -1 in caso di errore
* @param char *: codice della prenotazione
* @param int: numero del tavolo prenotato
* @param char *: data della prenotazione
* @param int: ora della prenotazione
* @param char *: nome della persona che ha effettuato la prenotazione
* @param int: numero di persone per cui è richiesta la prenotazione
* @param char *: timestamp della prenotazione (se NULL viene inserito quello attuale)
* @param reservation_state_t: stato della prenotazione
*/
int setReservation(char[], int, char[], int, char[], int, char[], reservation_state_t);

/*
* Verifica se al codice inserito corrisponde una prenotazione valida, nel caso recupera il tavolo prenotato
* @return int: numero del tavolo, -1 in caso di errore
* @param char[]: data della prenotazione
* @param bookingCode_t: codice della prenotazione
* @param reservation_state_t: stato in cui portare la prenotazione
* @param char []: vettore in cui salvare un eventuale messaggio di errore
*/
int updateReservationState(char[], bookingCode_t, reservation_state_t, char[]);


// FUNZIONI PER LA GESTIONE DI PIATTI E COMANDE --------------------

/*
* Funzione che recupera la lista delle comande che soddisfano i criteri specificati
* @return comanda_t *: array di comande, NULL in caso di errore
* @param char []: data del pasto
* @param int []: array di ore di inizio pasto (NULL se non si vuole filtrare per ora)
* @param int []: array di numeri di tavolo (NULL se non si vuole filtrare per tavolo)
* @param int []: array di numeri di comanda (NULL se non si vuole filtrare per comanda)
* @param comanda_state_t[]: array di stati delle comande da recuperare (NULL se non si vuole filtrare per stato)
* @oaram char[]: data del giorno di ristorazione (NULL se non si vuole filtrare per giorno di ristorazione)
* @param int[]: vettore con il numero di elementi negli array precedenti
* @param int: modalità verbose (1 se si vuole stampare a video il processo, 0 altrimenti)
*/
comanda_t *getComanda(char[], int[], int[], int[], comanda_state_t[], char[], int[], int);

/*
* Inserisce una comanda nel file delle comande, se già presente la aggiorna
* @return int: numero della comanda, -1 in caso di errore
* @param char []: data della conferma della prenotazione
* @param int: numero del tavolo prenotato
* @param int: ora della conferma della prenotazione
* @param int: numero della comanda (0 se nuova)
* @param comanda_state_t: stato della comanda
* @param char []: data del giorno di ristorazione
*/
int setComanda(char [], int, int, int, comanda_state_t, char []);

/*
* Recupera la lista dei piatti ordinati per una comanda
* @return comanda_dish_t *: array di piatti ordinati, NULL in caso di errore
* @param comanda_t *: puntatore alla comanda
* @param int: modalità verbose (1 se si vuole stampare a video il processo, 0 altrimenti)
*/
comanda_dish_t *getComandaDishes(comanda_t *, int);

/*
* Inserisce un piatto nel file dei piatti ordinati
* @return int: 0 in caso di successo, -1 in caso di errore
* @param char []: data della conferma della prenotazione
* @param int: numero del tavolo prenotato
* @param int: ora della conferma della prenotazione
* @param int: numero della comanda
* @param char []: codice del piatto
* @param int: quantità del piatto
*/
int setDish(char [], int, int, int, char [], int);

/*
* Recupera la comanda in attesa da più tempo relativa ad un pasto in corso
* @return int: 0 se la comanda è stata recuperata, -1 altrimenti
* @param comanda_t *: puntatore alla comanda da aggiornare
* @param char[]: data di inizio pasto
* @param int: ora di inizio pasto
* @param int: numero del tavolo
*/
int getFirstWaitingComanda(comanda_t *, char[], int, int);

/*
* Trova il numero della prossima comanda da inserire per un pasto
* @return int: numero della comanda, -1 in caso di errore
* @param char []: path del file
* @param int: numero del tavolo prenotato
* @param int: ora della conferma della prenotazione
*/
int getNextComandaNumber(char [], int, int);

/*
* Recupera le comande giornaliere
* @return comanda_t *: array di comande, NULL in caso di errore o se non ce ne sono
* @param char[]: data del giorno di ristorazione
* @param int: modalità verbose (1 se si vuole stampare a video il processo, 0 altrimenti)
*/
comanda_t *getDailyComanda(char [], int);

// FUNZIONI PER LA GESTIONE DEL MENU --------------------

/*
* Recupera i piatti del menu
* @return int: numero di piatti del menu, -1 in caso di errore
* @param menu_dish_t **: puntatore a puntatore di piatti
* @param char[]: data per cui si richiede il menù
*/
int getMenu(menu_dish_t **, char date[]);

/*
* Verifica se un piatto è presente nel menu
* @return int: 0 se il piatto è presente, -1 altrimenti
* @param dishCode_t: codice del piatto
*/
int isDishInMenu(dishCode_t, menu_dish_t *);


// FUNZIONI PER LA GESTIONE DEL CONTO --------------------

/*
* Calcola il conto di un tavolo
* @return subtotale_conto_t: lista contentente le voci del conto, NULL in caso non siano statti ordinati piatti
* @param char[]: data inizio pasto
* @param int: ora inizio pasto
* @param int: numero del tavolo
*/
subtotale_conto_t *getBill(char[], int, int);

/*
* Aggiunge un piatto al conto di un tavolo
* @return subtotale_conto_t: lista contentente le voci del conto aggiornate
* @param subtotale_conto_t: lista contentente le voci del conto attuali
* @param dishCode_t: codice del piatto
* @param int: quantità del piatto
*/
subtotale_conto_t *addDishToBill(subtotale_conto_t *, dishCode_t, int);


#endif