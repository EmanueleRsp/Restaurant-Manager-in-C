#ifndef VALIDATE_H
#define VALIDATE_H

#include <ctype.h>

#include "../utils.h"

/*
* Funzione che convalida il cognome
* @return int: 1 se il cognome è valido, 0 altrimenti
* @param char *: cognome da convalidare
* @param char **: puntatore alla stringa che conterrà il cognome se valido, '\0' altrimenti
*/
int convalidaCognome(char *, char[]);


/*
* Funzione che convalida il numero di persone
* @return int: 1 se il numero di persone è valido, 0 altrimenti
* @param char *: numero di persone da convalidare (stringa)
* @param int *: puntatore all'intero che conterrà il numero di persone se valido, 0 altrimenti
*/
int convalidaNumeroPersone(char *, int *);


/*
* Funzione che convalida la data
* @return int: 1 se la data è valida, 0 altrimenti
* @param char *: data da convalidare (stringa)
* @param char **: puntatore alla stringa che conterrà la data se valida, '\0' altrimenti
*/
int convalidaData(char *, char[]);


/*
* Funzione che convalida l'ora
* @return int: 1 se l'ora è valida, 0 altrimenti
* @param char *: ora da convalidare (stringa)
* @param int *: puntatore all'intero che conterrà l'ora se valida, 0 altrimenti
*/
int convalidaOra(char *, int *);

/*
* Funzione che verifica se l'ora inserita è passata
* @return int: 1 se l'ora è passata, 0 altrimenti
* @param int: giorno
* @param int: mese
* @param int: anno
* @param int: ora
*/
int convalidaOrarioPassato(int, int, int, int);

/*
* Funzione che convalida l'opzione del comando book
* @return int: 0 se l'opzione è valida, -1 altrimenti
* @param char *: opzione da convalidare (stringa)
* @param int *: puntatore all'intero che conterrà il numero dell'opzione selezionata se valido, 0 altrimenti
*/
int convalidaOpz(char *, int *);

/*
* Funzione che convalida la sintassi del codice di prenotazione
* @return int: 0 se la sintassi è valida, -1 altrimenti
* @param char *: codice da convalidare (stringa)
* @param char *[]: puntatore alla stringa che conterrà il codice se valido, '\0' altrimenti
*/
int convalidaSintassiCodice(char *, char []);

/*
* Convalida il numero del tavolo
* @return int: 1 se il numero è valido, 0 altrimenti
* @param char *: numero del tavolo
* @param int *: parametro in cui salvare il numero del tavolo se valido, 0 altrimenti 
*/
int convalidaTableCommand(char *, int *);

/*
* Convalida la comanda
* @return int: 0 se la comanda è valida, -1 altrimenti
* @param char *: input da convalidare
* @param comanda_dish_t *: puntatore alla testa della lista che conterrà la comanda, NULL altrimenti
*/
int convalidaComanda(char *, comanda_dish_t **);

/*
* Convalida il codice di un piatto
* @return int: 0 se il codice è valido, -1 altrimenti
* @param char *: codice da convalidare
* @param dishCode_t: conterrà il codice se valido, NULL altrimenti
*/
int validateDishCode(char *, dishCode_t);

/*
* Convalida la quantità di un piatto
* @return int: 0 se la quantità è valida, -1 altrimenti
* @param char *: quantità da convalidare
* @param int *: conterrà la quantità se valida, 0 altrimenti
* @param dishCode_t: codice del piatto
*/
int validateDishQuantity(char *, int *, dishCode_t);

/*
* Convalida la comanda per l'update
* @return int: 0 se la comanda è valida, -1 altrimenti
* @param char *: input da convalidare
* @param int *: conterrà il numero del tavolo se valido, 0 altrimenti
* @param int *: conterrà il numero della comanda se valido, 0 altrimenti
*/
int convalidaComandaUpdate(char *, int *, int *);

#endif