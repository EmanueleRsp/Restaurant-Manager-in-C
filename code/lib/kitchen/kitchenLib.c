#include "kitchenLib.h"


/*--------------------------------- FUNZIONI DI DISPLAY ----------------------------------*/


// Stampa il messaggio di benvenuto
void start(){
    printf("\n***************************** BENVENUTO *****************************\n");
}

void printClosingMsg(){
    printf("\n**************************** ARRIVEDERCI ****************************\n\n");
}

// Stamp le opzioni disponibili
void printCommands(){
    printf("------------------------\n");
    printf("Digita un comando:\n");
    printf("1) take  --> accetta una comanda\n");
    printf("2) show  --> mostra le comande accettate\n");
    printf("3) ready --> imposta lo stato della comanda\n");
}

void printInputLine(){
    printf("Scelta: ");
    fflush(stdout);
}

// Stampa il numero di comande in attesa
void printComandeInAttesa(int numero){

    int i;

    if(numero == 0){
        printf("Nessuna comanda in attesa.\n");
        return;
    }

    printf("Comande in attesa (%d): ", numero);
    for(i = 0; i < numero; i++){
        printf("*");
    }
    printf("\n");
}

/*--------------------------------- GESTIONE COMANDI INPUT ----------------------------------*/

// Lista delle comande prese in carico
comanda_t *comandePreseInCaricoList = NULL;

// Verifica che la comanda sia stata presa in carico
comanda_t *getComanda(comanda_t *list, int tavolo, int comanda){

    comanda_t *tmp = list;

    while(tmp != NULL){

        if(tmp->tavolo == tavolo && tmp->numero == comanda)
            return tmp;

        tmp = tmp->next;
    }

    return NULL;
}

// Rimuove una comanda dalla lista
void removeComanda(comanda_t **list, int tavolo, int comanda){

    comanda_t *tmp = *list;
    comanda_t *prev = NULL;

    while(tmp != NULL){

        if(tmp->tavolo == tavolo && tmp->numero == comanda){

            if(prev == NULL){
                *list = tmp->next;
                free(tmp);
                return;
            }

            prev->next = tmp->next;
            free(tmp);
            return;
        }

        prev = tmp;
        tmp = tmp->next;
    }

}


// Gestisce il comando "take", prende in carico una comanda
int take(int socket){

    char buffer[BUFFER_LEN];
    memset(buffer, 0, sizeof(buffer));

    request_t request;
    memset(&request, 0, sizeof(request));

    response_t response;
    memset(&response, 0, sizeof(response));

    // Preparo la richiesta
    request.op = TAKE;
    strcpy(request.serializedBody, "take");
    
    // Serializzo la richiesta
    sprintf(buffer, "%d\n%s", request.op, request.serializedBody);

    // Invio la richiesta
    if(send(socket, (void *)buffer, BUFFER_LEN, 0) == -1){
        perror("Errore nell'invio della richiesta");
        return -1;
    }

    // Azzero il buffer
    memset(buffer, 0, sizeof(buffer));

    // Ricevo la risposta
    if(recv(socket, (void *)buffer, BUFFER_LEN, 0) == -1){
        perror("Errore nella ricezione della risposta");
        return -1;
    }

    // Deserializzo la risposta
    response.header = atoi(strtok(buffer, "\n"));
    strcpy(response.serializedBody, strtok(NULL, ""));
    response.serializedBody[strlen(response.serializedBody) - 1] = '\0';

    // Controllo se ci sono errori
    if(response.header == -1){
        printf("Server: %s\n", response.serializedBody);
        return -1;
    }
    
    // Prendo in carico la comanda
    comanda_t *comanda = NULL;

    // Deserializzo la comanda
    deserializeComandeList(&comanda, response.serializedBody);

    // Aggiungo la comanda alla fine della lista di quelle prese in carico (perché è l'ultima prelevata)
    comanda_t *tmp = comandePreseInCaricoList;
    if(tmp == NULL){
        comandePreseInCaricoList = comanda;
    }
    else{
        while(tmp->next != NULL)
            tmp = tmp->next;
        tmp->next = comanda;
    }

    // Stampo la comanda
    printComandeList(comanda, 1);

    return 0;

}

// Gestisce il comando "show", mostra le comande prese in carico
void show(){

    // Controllo se ci sono comande prese in carico
    if(comandePreseInCaricoList == NULL){
        printf("Non ci sono comande prese in carico.\n");
        return;
    }

    // Stampo le comande prese in carico
    printComandeList(comandePreseInCaricoList, 1);

}

// Gestisce il comando "set", imposta lo stato di una comanda
void ready(int socket, int tavolo, int numero){

    char buffer[BUFFER_LEN];
    memset(buffer, 0, sizeof(buffer));

    request_t request;
    memset(&request, 0, sizeof(request));

    response_t response;
    memset(&response, 0, sizeof(response));


    // Verifico che la comanda sia stata presa in carico
    comanda_t *comanda = getComanda(comandePreseInCaricoList, tavolo, numero);
    if(comanda == NULL){
        printf("La comanda non è stata presa in carico.\n");
        return;
    }

    // Preparo la richiesta
    request.op = READY;
    sprintf(request.serializedBody, "%s\t%d\t%d\t%d", comanda->data, comanda->ora, tavolo, numero);
    
    // Serializzo la richiesta
    sprintf(buffer, "%d\n%s", request.op, request.serializedBody);

    // Invio la richiesta
    if(send(socket, (void *)buffer, BUFFER_LEN, 0) == -1){
        perror("Errore nell'invio della richiesta");
        return;
    }

    // Azzero il buffer
    memset(buffer, 0, sizeof(buffer));

    // Ricevo la risposta
    if(recv(socket, (void *)buffer, BUFFER_LEN, 0) == -1){
        perror("Errore nella ricezione della risposta");
        return;
    }

    // Deserializzo la risposta
    response.header = atoi(strtok(buffer, "\n"));

    // Controllo se ci sono errori
    if(response.header == -1){
        strcpy(response.serializedBody, strtok(NULL, ""));
        response.serializedBody[strlen(response.serializedBody) - 1] = '\0';
        printf("Server: %s\n", response.serializedBody);
        return;
    }

    // Se la comanda che ho inviato non era più valida, aggiorno la lista di quelle prese in carico
    if(response.header == 1){

        printf("Il tavolo ha lasciato il locale, aggiorno le comande prese in carico...\n");
        comanda_t *comandaList = NULL;

        // Deserializzo la comanda
        strcpy(response.serializedBody, strtok(NULL, ""));
        response.serializedBody[strlen(response.serializedBody) - 1] = '\0';
        deserializeComandeList(&comandaList, response.serializedBody);

        // Resetto la lista di comande
        deallocateComandeList(comandePreseInCaricoList);
        comandePreseInCaricoList = comandaList;

    }
    
    // Successo
    else{
        
        // Rimuovo la comanda dalla lista di quelle prese in carico
        removeComanda(&comandePreseInCaricoList, tavolo, numero);
        
        printf("COMANDA IN SERVIZIO\n");

    }
    
    return;

}


/* ----------------------------------- OPERAZIONI SERVER ----------------------------------- */

// Chiude il device
void closeDevice(int socket){

    // Chiudo la connessione
    close(socket);

    // Stampo il messaggio di chiusura
    printClosingMsg();

    exit(EXIT_SUCCESS);
    
}

