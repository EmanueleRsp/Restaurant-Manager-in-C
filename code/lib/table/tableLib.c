#include "tableLib.h"


/*--------------------------------- FUNZIONI DI DISPLAY ----------------------------------*/


// Stampa il messaggio di benvenuto
void start(){
    printf("\n***************************** BENVENUTO *****************************\n");
}

void printClosingMsg(){
    printf("\n******************** GRAZIE PER AVERCI SCELTO :) *******************\n\n");
}

// Stamp le opzioni disponibili
void printCommands(){
    printf("Digita un comando:\n");
    printf("\n");
    printf("1) help    --> mostra i dettagli dei comandi\n");
    printf("2) menu    --> mostra il menu dei piatti\n");
    printf("3) comanda --> invia una comanda\n");
    printf("4) conto   --> chiede il conto\n");
}

void printInputLine(){
    printf("Scelta: ");
    fflush(stdout);
}

// Stampa il messaggio della schermata di sblocco
void displayLock(){
    printf("Il device è attualmente bloccato.\n");
    printf("Inserisci il codice di prenotazione: ");
    fflush(stdout);
}

// Stampa i dettagli dei comandi
void displayCommandDetails(){
    
    printf("\n");
    
    printf("1) help\n");
    printf("\tMOSTRA I DETTAGLI DEI COMANDI (questa schermata).\n");
    printf("\n");

    printf("2) menu\n");
    printf("\tMOSTRA IL MENU DEI PIATTI DISPONIBILI.\n");
    printf("\tIl menù specifica codice, nome e prezzo di ciascuna portata disponibile.\n");
    printf("\tIl codice è composto da una lettera e un numero:\n");
    printf("\t\t- La lettera indica il tipo di portata\n");
    printf("\t\t  (A = antipasto, P = primo, S = secondo, D = dolce)\n");
    printf("\t\t- Il numero indica il numero progressivo della portata\n");
    printf("\t\t  nella relativa categoria, tra 1 e %d.\n", MAX_CATEGORY_INDEX);
    printf("\tEsempio:\n");
    printf("\t\tA2 - Antipasto misto\t12\n");
    printf("\n");
    
    printf("3) comanda {<piatto_1-quantità_1>...<piatto_n-quantità_n>}\n");
    printf("\tINVIA UNA COMANDA.\n");
    printf("\tLa comanda è composta da una o più portate,\n");
    printf("\tciascuna con la propria quantità (tra 1 e %d).\n", MAX_DISHES_PER_ORDER);
    printf("\tLe portate sono specificate nel formato <piatto-quantità>.\n");
    printf("\tPer indicare il piatto si specifica il relativo codice.\n");
    printf("\tEsempio:\n");
    printf("\t\tcomanda A1-2 A2-1 P1-1\n");
    printf("\n");
    
    printf("4) conto\n");
    printf("\tSI INVIA LA RICHIESTA DEL CONTO.\n");
    printf("\tSi riceve in risposta il conto dove, in ogni riga,\n");
    printf("\tè presente il codice del piatto, la quantità, e il subtotale.\n");
    printf("\tNOTA: Per la gentilezza che contraddistingue il nostro ristorante,\n");
    printf("\tse viene inviata una comanda, ma si richiede il conto prima che questa\n");
    printf("\tentri in preparazione, non verrà addebitata :).\n");
    printf("\tEsempio:\n");
    printf("\t\tA1 2 24\n");
    printf("\t\tP3 1 12\n");
    printf("\t\tS2 1 15\n");
    printf("\t\tTotale: 61\n");
    
    printf("\n");

}


/*--------------------------------- CONNESSIONE AL SERVER ----------------------------------*/

// GESTIONE COMANDE ------------------------------

// Lista delle comande inviate
comanda_t *comandeList = NULL;

// Stampa le comande inviate al server
void printComandeInviate(){
    if(comandeList == NULL){
        return;
    }

    printf("COMANDE RICHIESTE:\n");
    printComandeList(comandeList, 2);
    printf("------------------------\n");
}



// GESTIONE RICHIESTE AL SERVER ------------------------------

// Tenta di sbloccare il device
int unlock(char code[], int socket){

    char buffer[BUFFER_LEN];
    memset(buffer, 0, BUFFER_LEN);

    request_t req;
    memset(&req, 0, sizeof(request_t));

    response_t res;
    memset(&res, 0, sizeof(response_t));

    const operazione_t OP = UNLOCK;


    // Costruisco la richiesta
    req.op = OP;
    sprintf(req.serializedBody, "%s", code);

    // Serializzo la richiesta
    sprintf(buffer, "%d\n%s", req.op, req.serializedBody);

    // Invio il codice di sblocco al server
    if(send(socket, (void *)buffer, BUFFER_LEN, 0) == -1){
        perror("Errore nell'invio del codice di sblocco");
        return -1;
    }

    // Azzero il buffer
    memset(buffer, 0, BUFFER_LEN);

    // Ricevo la risposta dal server
    if(recv(socket, (void *)buffer, BUFFER_LEN, 0) == -1){
        perror("Errore nella ricezione della risposta dal server");
        return -1;
    }

    // Deserializzo il messaggio ricevuto (interessa solo l'header)
    res.header = atoi(strtok(buffer, "\n"));

    // L'operazione non è andata a buon fine
    if(res.header == -1){

        // Deserializzo il body
        sprintf(res.serializedBody, "%s", strtok(NULL, "\n"));

        printf("Server: %s\n", res.serializedBody);
        return -1;
    }

    // L'operazione è andata a buon fine
    printf("Device sbloccato con successo!\n");
    start();
    return 0;

}

// Invia la richiesta per recuperare il menu
void getMenu(int serverSocket){

    // Variabili per la trasmissione dei dati
    char buffer[BUFFER_LEN];
    memset(&buffer, 0, sizeof(buffer));

    request_t req;
    memset(&req, 0, sizeof(req));

    response_t res;
    memset(&res, 0, sizeof(res));

    const operazione_t OP = MENU;

    // Serializzo il body della richiesta e specifico l'operazione da richiedere
    strcpy(req.serializedBody, "menu");
    req.op = OP;

    // Serializzo la richiesta
    sprintf(buffer, "%d\n%s", req.op, req.serializedBody);
    
    // Invio il messaggio
    if(send(serverSocket, (void *)buffer, BUFFER_LEN, 0) == -1){
        perror("Errore nell'invio del messaggio");
        return;
    }

    // Azzero il buffer
    memset(&buffer, 0, sizeof(buffer));

    // Ricevo il messaggio
    if(recv(serverSocket, (void *)buffer, BUFFER_LEN, 0) == -1){
        perror("Errore nella ricezione del messaggio");
        return;
    }


    // Deserializzo il messaggio ricevuto
    res.header = atoi(strtok(buffer, "\n"));
    strncpy(res.serializedBody, strtok(NULL, ""), 1019);
    res.serializedBody[1019 -1] = '\0';

    // Stampo il messaggio di errore se necessario
    if(res.header == -1){
        printf("Server: %s\n", res.serializedBody);
        return;
    }

    // Stampo la risposta
    else{

        menu_dish_t *menuList = NULL;

        // Deserializzo la lista dei piatti
        deserializeMenu(&menuList, res.serializedBody);

        // Stampo il menu
        printf("\n");
        printMenu(menuList);
        printf("\n");

        // Libero la memoria
        deallocateMenu(menuList);

    }

    return;
    
}

// Invia la comanda al server
void comanda(comanda_dish_t *dishList, int socket){

    // Variabili per la trasmissione dei dati
    char buffer[BUFFER_LEN];
    memset(&buffer, 0, sizeof(buffer));

    request_t req;
    memset(&req, 0, sizeof(req));

    response_t res;
    memset(&res, 0, sizeof(res));

    comanda_t *comanda = NULL;

    const operazione_t OP = COMANDA;

    // Serializzo il body della richiesta e specifico l'operazione da richiedere
    serializeComandaDishList(dishList, req.serializedBody);
    req.op = OP;

    // Serializzo la richiesta
    sprintf(buffer, "%d\n%s", req.op, req.serializedBody);
    
    // Invio il messaggio
    if(send(socket, (void *)buffer, BUFFER_LEN, 0) == -1){
        perror("Errore nell'invio del messaggio");
        deallocateComandaDishList(dishList);
        return;
    }

    // Azzero il buffer
    memset(&buffer, 0, sizeof(buffer));

    // Ricevo il messaggio
    if(recv(socket, (void *)buffer, BUFFER_LEN, 0) == -1){
        perror("Errore nella ricezione del messaggio");
        deallocateComandaDishList(dishList);
        return;
    }

    // Deserializzo il messaggio ricevuto (solo l'header è rilevante se non ci sono errori)
    res.header = atoi(strtok(buffer, "\n"));

    // Stampo il messaggio di errore se necessario
    if(res.header == -1){
        strncpy(res.serializedBody, strtok(NULL, ""), 1019);
        res.serializedBody[1019 -1] = '\0';
        printf("Server: %s\n", res.serializedBody);
        deallocateComandaDishList(dishList);
        return;
    }

    // Stampo la risposta
    printf("COMANDA RICEVUTA\n\n");

    // Salvo la comanda inviata
    comanda = (comanda_t *)malloc(sizeof(comanda_t));
    memset(comanda, 0, sizeof(comanda_t));

    // Campi rilevanti
    comanda->piatti = dishList;
    comanda->numero = res.header;
    comanda->stato = ATTESA;
    
    // Aggiungo la comanda alla lista
    sortedInsertComanda(&comandeList, comanda);

    return;


}


// Invia la richiesta del conto
int conto(int socket){

    // Variabili per la trasmissione dei dati
    char buffer[BUFFER_LEN];
    memset(&buffer, 0, sizeof(buffer));

    request_t req;
    memset(&req, 0, sizeof(req));

    response_t res;
    memset(&res, 0, sizeof(res));

    const operazione_t OP = CONTO;

    subtotale_conto_t *conto = NULL;

    // Serializzo il body della richiesta e specifico l'operazione da richiedere
    strcpy(req.serializedBody, "conto");
    req.op = OP;

    // Serializzo la richiesta
    sprintf(buffer, "%d\n%s", req.op, req.serializedBody);
    
    // Invio il messaggio
    if(send(socket, (void *)buffer, BUFFER_LEN, 0) == -1){
        perror("Errore nell'invio del messaggio");
        return -1;
    }

    // Azzero il buffer
    memset(&buffer, 0, sizeof(buffer));

    // Ricevo il messaggio
    if(recv(socket, (void *)buffer, BUFFER_LEN, 0) == -1){
        perror("Errore nella ricezione del messaggio");
        return -1;
    }

    // Deserializzo il messaggio ricevuto
    res.header = atoi(strtok(buffer, "\n"));

    // Stampo il messaggio di errore se necessario
    if(res.header == -1){
        printf("Server: %s\n", res.serializedBody);
        return -1;
    }

    // Se il conto è vuoto
    else if(res.header == 0){
        printf("Il conto è vuoto, non sono state effettuate ordinazioni.\n");
        return 0;
    }

    // Deserializzo il corpo del messaggio
    strncpy(res.serializedBody, strtok(NULL, ""), 1019);
    res.serializedBody[1019 -1] = '\0';

    // Deserializzo il conto
    deserializeConto(&conto, res.serializedBody);

    // Stampo il conto
    printConto(conto);

    // Libero la memoria
    deallocateConto(conto);
    deallocateComandeList(comandeList);
    comandeList = NULL;
    
    return 0;

}


/* ----------------------------------- OPERAZIONI SERVER ----------------------------------- */

// Aggiorna lo stato di una comanda, dato il suo numero
void updateComandaStatus(int numero, comanda_state_t stato){

    comanda_t *comanda = comandeList;
    
    printf("\nAggiornamento stato della comanda %d...\n", numero);
    while(comanda != NULL){
        if(comanda->numero == numero){
            comanda->stato = stato;
            return;
        }
        comanda = comanda->next;
    }
    printf("La comanda %d non è stata trovata.\n", numero);
    return;

}


// Chiude il device
void closeDevice(int socket){

    // Dealloco la lista delle comande del pasto
    deallocateComandeList(comandeList);
    comandeList = NULL;

    // Chiudo la connessione
    close(socket);

    // Stampo il messaggio di chiusura
    printClosingMsg();

    exit(EXIT_SUCCESS);
}

