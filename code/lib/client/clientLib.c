#include "clientLib.h"


/*--------------------------------- FUNZIONI DI DISPLAY ----------------------------------*/


// Stampa il messaggio di benvenuto
void start(){
    printf("***************************** BENVENUTO *****************************\n");
}

// Chiude il client
void closeDevice(int socket){
    close(socket);
    printf("\n*********************** CHIUSURA CLIENT DEVICE **********************\n\n");
    exit(EXIT_SUCCESS);
}

// Stamp le opzioni disponibili
void printCommands(){
    printf("Digita un comando:\n");
    printf("1) find --> ricerca la disponibilità per una prenotazione\n");
    printf("2) book --> invia una prenotazione\n");
    printf("3) esc  --> termina il client\n");
    printf("Scelta: ");
    fflush(stdout);
}


/*--------------------------------- CONNESSIONE AL SERVER ----------------------------------*/

// Variabile globale che rappresenta la lista dei tavoli disponibili
table_t *tableList = NULL;

// Funzione che invia la richiesta per visualizzare le disponibilità dei tavoli al server
int find(reservation_t p, int serverSocket){

    // Variabili per la trasmissione dei dati
    char buffer[BUFFER_LEN];
    memset(&buffer, 0, sizeof(buffer));

    request_t req;
    memset(&req, 0, sizeof(req));

    response_t res;
    memset(&res, 0, sizeof(res));

    const operazione_t OP = FIND;

    // Se era presente una lista di tavoli disponibili, la dealloco
    if(tableList != NULL){
        deallocateTableList(tableList);
        tableList = NULL;
    }

    // Serializzo il body della richiesta e indico l'operazione da richiedere
    sprintf(req.serializedBody, "%s\t%d\t%s\t%d", 
        p.cognome,
        p.numeroPersone, 
        p.data,
        p.ora
    );
    req.op = OP;

    // Serializzo la richiesta
    sprintf(buffer, "%d\n%s", req.op, req.serializedBody);
    
    // Invio il messaggio
    if(send(serverSocket, (void *)buffer, BUFFER_LEN, 0) == -1){
        perror("Errore nell'invio del messaggio");
        return -1;
    }

    // Azzero il buffer
    memset(&buffer, 0, sizeof(buffer));

    // Ricevo il messaggio
    if(recv(serverSocket, (void *)buffer, BUFFER_LEN, 0) == -1){
        perror("Errore nella ricezione del messaggio");
        return -1;
    }

    // Deserializzo il messaggio ricevuto
    res.header = atoi(strtok(buffer, "\n"));
    strncpy(res.serializedBody, strtok(NULL, ""), 1019);
    res.serializedBody[1019 -1] = '\0';

    // Stampo il messaggio di errore se necessario
    if(res.header == -1){
        printf("Server: %s\n", res.serializedBody);
        return -1;
    }

    // Nessuna disponibilità
    else if(res.header == 0){
        printf("Nessun tavolo disponibile\n");
        return 0;
    }

    // Stampo la risposta
    else{

        int i = 0;

        // Deserializzo la lista dei tavoli
        deserializeTableList(&tableList, res.serializedBody);
        
        table_t *tmp = tableList;
        while(tmp != NULL){
            i++;
            printf("%d) T%d\tSALA%d\t%s\n", i, tmp->codice, tmp->sala, tmp->info);
            tmp = tmp->next;
        }

    }

    return res.header;

}


// Funzione che invia la richiesta per prenotare un tavolo al server
void book(int opzione, int serverSocket){
    
    // Variabili per la trasmissione dei dati
    char buffer[BUFFER_LEN];
    memset(&buffer, 0, sizeof(buffer));

    request_t req;
    memset(&req, 0, sizeof(req));

    response_t res;
    memset(&res, 0, sizeof(res));

    booking_t b;
    memset(&b, 0, sizeof(b));

    const operazione_t OP = BOOK;
    int i = 1;

    // Controllo che sia stata effettuata una ricerca
    if(tableList == NULL){
        printf("Prima di prenotare occorre verificare le disponibilità con il comando \"find\".\n");
        return;
    }

    // Controllo che l'opzione scelta sia valida recuperando il tavolo selezionato
    table_t *tmp = tableList;
    while (tmp != NULL && i < opzione){
        tmp = tmp->next;
        i++;
    }
    if(tmp == NULL){
        printf("Opzione non valida, seleziona un'opzione tra 1 e %d\n", i);
        return;
    }

    // Preparo la richiesta
    sprintf(req.serializedBody, "%d",
        tmp->codice
    );
    req.op = OP;
    
    // Serializzo la richiesta
    sprintf(buffer, "%d\n%s", req.op, req.serializedBody);
        
    // Invio il messaggio
    if(send(serverSocket, (void *)buffer, BUFFER_LEN, 0) == -1){
        perror("Errore nell'invio del messaggio");
        return;
    }
    
    // Azzero il buffer
    memset(buffer, 0, sizeof(buffer));
    
    // Ricevo il messaggio
    if(recv(serverSocket, (void *)buffer, BUFFER_LEN, 0) == -1){
        perror("Errore nella ricezione del messaggio");
        return;
    }

    // Deserializzo il messaggio ricevuto
    res.header = atoi(strtok(buffer, "\n"));
    strncpy(res.serializedBody, strtok(NULL, ""), 1019);
    res.serializedBody[1019 -1] = '\0';

    // A prescindere dal risultato, dealloco la lista dei tavoli disponibili
    deallocateTableList(tableList);
    tableList = NULL;

    // L'operazione non è andata a buon fine
    if(res.header == -1){
        printf("Errore dal server: %s\n", res.serializedBody);
        return;
    }

    // L'operazione è andata a buon fine
    strcpy(b.codicePrenotazione, strtok(res.serializedBody, "\t"));
    b.tavolo = atoi(strtok(NULL, "\t"));
    b.sala = atoi(strtok(NULL, "\t"));

    printf("PRENOTAZIONE EFFETTUATA:\n");
    printf("\tCodice prenotazione: %s\n", b.codicePrenotazione);
    printf("\tTavolo: T%d\n", b.tavolo);
    printf("\tSala: %d\n", b.sala);

    return;
}