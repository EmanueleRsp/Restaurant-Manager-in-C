#include "serverLib.h"


/*--------------------------------- FUNZIONI DI DISPLAY ----------------------------------*/


// Messaggio di avvio del server
void start(){
    printf("***************************** BENVENUTO *****************************\n");
}

// Stampa le opzioni dei comandi disponibili del server
void printCommands(){
    printf("\nDigita un comando:\n");
    printf("1) stat --> mostra lo stato di tutte le comande giornaliere\n");
    printf("2) stat <a|p|s> --> mostra comande di ogni pasto in corso in uno stato specifico\n");
    printf("3) stat T<numero_tavolo> --> mostra le comande del pasto in corso ad un tavolo\n");
    printf("4) stop --> arresta server e device connessi, se le comande sono in servizio\n");
    printf("Scelta: ");
    fflush(stdout);
}


/*--------------------------------- GESTIONE DELLE CONNESSIONI ----------------------------------*/

// Lista delle connessioni attive
connection_t *connectionList = NULL;

// Aggiunge una connessione alla lista delle connessioni attive (inserimento in testa)
void addConnection(connection_t *conn){
    
    // Inserimento in testa
    conn->next = connectionList;
    connectionList = conn;

    return;
}

// Rimuove una connessione dalla lista delle connessioni attive
void removeConnection(int socket){
    
    connection_t *conn = connectionList;
    connection_t *prev = NULL;

    while(conn != NULL){
        
        if(conn->socket == socket){

            switch (conn->dev) {
            case CLIENT:
                // Rimuovo eventuali proposte di prenotazione
                removeProposedTable(socket); 
                break;
            case KITCHEN:
                // Gestisco eventuali comande che aveva in carico
                resetComandeByKitchenDevice(socket);
                break;
            case TABLE:
                // Rimuovo il tavolo dalla lista dei tavoli attivi
                removeActiveTable(socket);
                break;
            default:
                break;
            }

            if(prev == NULL)
                connectionList = conn->next;
            else
                prev->next = conn->next;
            free(conn);
            return;
        }
        prev = conn;
        conn = conn->next;
    }

    return;
}

// Rimuove tutte le connessioni attive
int removeAllConnections(){

    connection_t *conn = connectionList;
    connection_t *next = NULL;

    char buffer[BUFFER_LEN];
    server_message_t msg;
    request_t req;

    while(conn != NULL){

        // Preparo il messaggio da inviare al dispositivo
        memset(&msg, 0, sizeof(server_message_t));
        memset(buffer, 0, BUFFER_LEN);
        msg.header = STOP;
        strcpy(msg.serializedBody, "Server in fase di arresto.");
        sprintf(buffer, "%d\n%s", msg.header, msg.serializedBody);

        // VERBOSE
        printf("Invio del messaggio di stop al device %d (tipo %d)...\n", conn->socket, conn->dev);

        // Invio il messaggio di stop
        if(send(conn->socket, (void *)buffer, BUFFER_LEN, 0) < 0){
            perror("Errore nell'invio del messaggio di stop");
            return conn->socket;
        }

        // Se è un table device in cui è in corso un pasto, richiederà il conto
        if(conn->dev == TABLE){

            ssize_t nbytes;

            // Recupero la richiesta
            memset(buffer, 0, BUFFER_LEN);
            if((nbytes = recv(conn->socket, (void *)buffer, BUFFER_LEN, 0)) < 0){
                perror("Errore nella ricezione della richiesta di conto");
                return conn->socket;
            }

            // Se il device si è chiuso ricevo 0 bytes, significa che era bloccato
            if(nbytes == 0){
                printf("Il table device %d non era in uso, si è chiuso senza dover richiedere il conto.\n", conn->socket);
                
                // Elimino la connessione dalla lista
                next = conn->next;
                free(conn);
                conn = next;
                continue;
            }

            printf("Richiesta di conto ricevuta dal tavolo %d\n", conn->socket);

            // Deserializzo la richiesta
            memset(&req, 0, sizeof(request_t));
            req.op = atoi(strtok(buffer, "\n"));
            if((req.op + TABLE) != CONTO){
                printf("Richiesta di conto non riconosciuta\n");
                reqErrorManager(conn->socket, "Richiesta di conto non riconosciuta");
                return conn->socket;
            }

            printf("Richiesta di conto riconosciuta\n");

            // Recupero il tavolo associato al socket
            active_table_device_t *table = getActiveTable(conn->socket);
            if(table != NULL)
                contoManager(conn->socket);
            else
                reqErrorManager(conn->socket, "Errore nel recupero del tavolo associato al socket");
            
        }

        // Se il device si è chiuso ricevo 0 bytes
        if(recv(conn->socket, (void *)buffer, BUFFER_LEN, 0) != 0){
            perror("Errore nella ricezione del messaggio di stop o nella chiusura del device");
            return conn->socket;
        }

        // VERBOSE
        printf("Messaggio di stop ricevuto dal device %d\n", conn->socket);

        // Elimino la connessione dalla lista
        next = conn->next;
        free(conn);
        conn = next;
    }

    connectionList = NULL;

    return 0;
}


device_t getDevice(int socket){
    
    connection_t *conn = connectionList;

    while(conn != NULL){
        if(conn->socket == socket)
            return conn->dev;
        conn = conn->next;
    }
    
    return NOTID;

}


/*--------------------------------- GESTIONE TABLE DEVICE ATTIVI ----------------------------------*/


// Lista dei table device attivi
active_table_device_t *tableDeviceList = NULL;


void addActiveTable(active_table_device_t *table){
    
    active_table_device_t *prev = NULL;
    active_table_device_t *next = tableDeviceList;

    while(next != NULL){
        if(table->tavoloAssegnato < next->tavoloAssegnato)
            break;
        prev = next;
        next = next->next;
    }

    if(prev == NULL){
        table->next = tableDeviceList;
        tableDeviceList = table;
    }
    else{
        table->next = prev->next;
        prev->next = table;
    }

    return;
}


void removeActiveTable(int socket){
        
    active_table_device_t *table = tableDeviceList;
    active_table_device_t *prev = NULL;
    
    while(table != NULL){
        if(table->socket == socket){

            // Rimuovo le comande in attesa relative al tavolo
            removeComandeByTableDevice(socket);

            if(prev == NULL)
                tableDeviceList = table->next;
            else
                prev->next = table->next;
            free(table);
            return;
        }
        prev = table;
        table = table->next;
    }
    
    return;
}


void removeAllActiveTables(){

    active_table_device_t *table = tableDeviceList;
    active_table_device_t *next = NULL;

    while(table != NULL){
        next = table->next;
        free(table);
        table = next;
    }

    tableDeviceList = NULL;

    return;
}


active_table_device_t *getActiveTable(int socket){
    
    active_table_device_t *table = tableDeviceList;

    while(table != NULL){
        if(table->socket == socket)
            return table;
        table = table->next;
    }
    
    return NULL;
}


active_table_device_t *isTableActive(char data[], int orario, int tavolo){
    
    active_table_device_t *table = tableDeviceList;

    while(table != NULL){
        if(strcmp(table->data, data) == 0 && table->orario == orario && table->tavoloAssegnato == tavolo)
            return table;
        table = table->next;
    }
    
    return NULL;
}


/*--------------------------------- GESTIONE PROPOSTE ----------------------------------*/


// Lista delle proposte dei tavoli effettuate
active_client_device_t *proposedTableList = NULL;


void addProposedTable(active_client_device_t *table){
    
    // Inserimento in testa
    table->next = proposedTableList;
    proposedTableList = table;

    return;
}


void removeProposedTable(int socket){
        
    active_client_device_t *table = proposedTableList;
    active_client_device_t *prev = NULL;
    
    while(table != NULL){
        if(table->socket == socket){
            if(prev == NULL)
                proposedTableList = table->next;
            else
                prev->next = table->next;
            free(table);
            return;
        }
        prev = table;
        table = table->next;
    }
    
    return;
}


void removeAllProposedTables(){

    active_client_device_t *table = proposedTableList;
    active_client_device_t *next = NULL;

    while(table != NULL){
        next = table->next;
        free(table);
        table = next;
    }

    proposedTableList = NULL;

    return;
}


active_client_device_t *getProposedTable(int socket){
    
    active_client_device_t *table = proposedTableList;

    while(table != NULL){
        if(table->socket == socket)
            return table;
        table = table->next;
    }
    
    return NULL;
}


/* --------------------------------- GESTIONE ORDINE COMANDE ---------------------------------- */

// Puntatori alla testa e alla coda della lista delle comande effettuate
comanda_order_t *mostRecentComanda = NULL;
comanda_order_t *oldestComanda = NULL;


comanda_order_t *getOldestWaitingComandaOrder(){
    
    // Scorro dalla coda alla testa finchè non trovo una comanda che non è stata ancora presa in carico
    comanda_order_t *comanda = oldestComanda;
    while(comanda != NULL){
        if(comanda->kitchenSocket == -1)
            return comanda;
        comanda = comanda->prev;
    }

    return NULL;
}


comanda_t *getPreparatingComandaOrder(int kitchenSocket){

    comanda_order_t *comandaOrder = mostRecentComanda;
    comanda_t *comandaList = NULL;

    // Scorro la lista 
    while(comandaOrder != NULL){

        // Se la comanda è in preparazione dal kitchen device specificato la aggiungo alla lista
        if(comandaOrder->kitchenSocket == kitchenSocket){
            
            comandaOrder->comanda->next = comandaList;
            comandaList = comandaOrder->comanda;

        }
        comandaOrder = comandaOrder->next;
    }

    return comandaList;

}


void deleteComandaOrder(comanda_order_t *comanda){

    // Se è l'unico elemento
    if(comanda->prev == NULL && comanda->next == NULL){
        deallocateComandaDishList(comanda->comanda->piatti);
        free(comanda);
        mostRecentComanda = NULL;
        oldestComanda = NULL;
        return;
    }

    // Se è il primo elemento
    else if(comanda->prev == NULL){
        mostRecentComanda = comanda->next;
        mostRecentComanda->prev = NULL;
        deallocateComandaDishList(comanda->comanda->piatti);
        free(comanda);
        return;
    }

    // Se è l'ultimo elemento
    else if(comanda->next == NULL){
        oldestComanda = comanda->prev;
        oldestComanda->next = NULL;
        deallocateComandaDishList(comanda->comanda->piatti);
        free(comanda);
        return;
    }

    // Se è un elemento intermedio
    comanda->prev->next = comanda->next;
    comanda->next->prev = comanda->prev;
    deallocateComandaDishList(comanda->comanda->piatti);
    free(comanda);

    return;
}


comanda_order_t *getComandaOrderByComanda(char data[], int orario, int tavolo, int numero){

    comanda_order_t *comanda = mostRecentComanda;

    while(comanda != NULL){

        if(strcmp(comanda->comanda->data, data) == 0 && 
           comanda->comanda->ora == orario && 
           comanda->comanda->tavolo == tavolo && 
           comanda->comanda->numero == numero
          )
            return comanda;

        comanda = comanda->next;
    }

    return NULL;
}


void removeComandeByTableDevice(int socket){

    comanda_order_t *comanda = mostRecentComanda;
    comanda_order_t *next = NULL;

    while(comanda != NULL){
        next = comanda->next;

        if(comanda->tableSocket == socket){
            
            // Se era in attesa notifico che non c'è più
            if(comanda->kitchenSocket == -1)
                notifyKitchenDevices(DECREASE, -1);

            deleteComandaOrder(comanda);
        }

        comanda = next;
    }

    return;
}


void resetComandeByKitchenDevice(int socket){

    comanda_order_t *comanda = mostRecentComanda;
    comanda_order_t *next = NULL;

    while(comanda != NULL){
        next = comanda->next;

        if(comanda->kitchenSocket == socket){
            // Pongo la comanda in attesa
            comanda->kitchenSocket = -1;
            // Notifico i kitchen device
            notifyKitchenDevices(INCREASE, socket);
            // Notifico il table device
            notifyTableDevice(comanda->tableSocket, comanda->comanda->numero, ATTESA);
            // La setto nel file
            if(setComanda(
                comanda->comanda->data, 
                comanda->comanda->tavolo, 
                comanda->comanda->ora, 
                comanda->comanda->numero, 
                ATTESA, 
                comanda->comanda->dataRis
                ) == -1){
                printf("Errore nell'aggiornamento dello stato della comanda\n");
            }
        }

        comanda = next;
    }

    return;

}


void addComandaOrder(int socket, comanda_t *comandaDet){
    
    // Creo l'elemento
    comanda_order_t *comanda = (comanda_order_t *)malloc(sizeof(comanda_order_t));
    memset(comanda, 0, sizeof(comanda_order_t));
    
    comanda->tableSocket = socket;
    comanda->comanda = comandaDet;
    comanda->kitchenSocket = -1; 
    
    // Inserimento in testa
    comanda->prev = NULL;
    comanda->next = mostRecentComanda;
    if(mostRecentComanda != NULL)
        mostRecentComanda->prev = comanda;
    mostRecentComanda = comanda;

    // Se è il primo elemento inserito
    if(oldestComanda == NULL)
        oldestComanda = comanda;
    
    return;
}


int getWaitingComandeNum(){

    comanda_order_t *comanda = mostRecentComanda;
    int size = 0;

    while(comanda != NULL){
        if(comanda->kitchenSocket == -1)
            size++;
        comanda = comanda->next;
    }

    return size;
}


void removeWaitingComande(int socket){

    comanda_order_t *comanda = mostRecentComanda;
    comanda_order_t *next = NULL;

    while(comanda != NULL){
        next = comanda->next;

        if(comanda->kitchenSocket == socket){
            if(comanda->prev == NULL)
                mostRecentComanda = comanda->next;
            else
                comanda->prev->next = comanda->next;
            if(comanda->next == NULL)
                oldestComanda = comanda->prev;
            else
                comanda->next->prev = comanda->prev;

            deallocateComandaDishList(comanda->comanda->piatti);
            free(comanda);

            notifyKitchenDevices(DECREASE, -1);
        }

        comanda = next;
    }

    return;
}



/*--------------------------------- GESTIONE DEI COMANDI ----------------------------------*/



// Restituisce lo stato di tutte le comande giornaliere
void stat(){

    comanda_t *comandeList = NULL;

    // Recupero odierna
    char data[DATE_LEN];
    getTodayDate(data);

    // Recupero le comande
    comandeList = getDailyComanda(data, 0);

    // Se non ci sono comande, non stampo nulla
    if(comandeList == NULL)
        return;

    // Stampo le comande
    printComandeList(comandeList, 4);

    // Libero la memoria
    deallocateComandeList(comandeList);

    return;

}

// Restituisce le comande che presentano tale stato, per ogni tavolo in cui è in corso un pasto
void statStatus(comanda_state_t status){

    active_table_device_t *activeTable = tableDeviceList;
    comanda_t *comandeList = NULL;

    // Se non si richiedono le comande in attesa, posso sfruttare la lista
    if(status != SERVIZIO){
        comanda_order_t *comandaOrder = mostRecentComanda;
        
        while(comandaOrder != NULL){

            if((status == ATTESA && comandaOrder->kitchenSocket == -1) ||
               (status == PREPARAZIONE && comandaOrder->kitchenSocket != -1)){

                // Copio la comanda
                comanda_t *newComanda = (comanda_t *)malloc(sizeof(comanda_t));
                memset(newComanda, 0, sizeof(comanda_t));
                memcpy(newComanda, comandaOrder->comanda, sizeof(comanda_t));
                newComanda->next = NULL;

                // Inserimento ordinato
                sortedInsertComanda(&comandeList, newComanda);

            }

            comandaOrder = comandaOrder->next;
        }

        // Stampo le comande
        printComandeList(comandeList, 1);

        // Libero la memoria
        comanda_t *comanda = comandeList;
        comanda_t *next = NULL;
        while(comanda != NULL){
            next = comanda->next;
            free(comanda);
            comanda = next;
        }

    }

    // Se si richiedono le comande in servizio, devo ricorrere al file
    else{
        
        // Guardo i tavoli in cui è in corso un pasto
        while(activeTable != NULL){

            // Recupero le comande
            int tavoli[] = {activeTable->tavoloAssegnato};
            int orari[] = {activeTable->orario};
            comanda_state_t stati[] = {SERVIZIO};
            int dim[] = {1,1,0,1};
            comandeList = getComanda(activeTable->data, orari, tavoli, NULL, stati, NULL, dim, 0);

            // Stampo le comande
            printComandeList(comandeList, 1);

            // Libero la memoria
            deallocateComandeList(comandeList);

            activeTable = activeTable->next;

        }

    }

    return;

}

// Restituisce tutte le comande relative al pasto in corso per tale tavolo
void statTable(int table){

    active_table_device_t *activeTable = tableDeviceList;
    comanda_t *comandeList = NULL;
    
    // Guardo i tavoli in cui è in corso un pasto
    while(activeTable != NULL){
        
        if(activeTable->tavoloAssegnato != table){
            activeTable = activeTable->next;
            continue;
        }

        // Recupero le comande
        int tavoli[] = {activeTable->tavoloAssegnato};
        int orari[] = {activeTable->orario};
        comanda_state_t stato[] = {ATTESA, PREPARAZIONE, SERVIZIO};
        int dim[] = {1,1,0,3};
        comandeList = getComanda(activeTable->data, orari, tavoli, NULL, stato, NULL, dim, 0);

        // Stampo le comande
        printComandeList(comandeList, 2);

        // Libero la memoria
        deallocateComandeList(comandeList);

        activeTable = activeTable->next;

    }

    return;
    
}


/*--------------------------------- GESTIONE DELLE RICHIESTE ----------------------------------*/

// CLIENT DEVICE ----------------------------------

void findManager(int socket, char *serializedBody){

    reservation_t p;
    memset(&p, 0, sizeof(reservation_t));

    int dd, mm, yy;

    table_t *tableList = NULL;

    char buffer[BUFFER_LEN];
    memset(buffer, 0, BUFFER_LEN);
    
    response_t res;
    memset(&res, 0, sizeof(response_t));

    char dtemp[DATE_LEN];
    int tavoliDisponibili = 0;

    // VERBOSE
    printf("Richiesta find individuata\n");

    // Deserializzo il body della richiesta
    strncpy(p.cognome, strtok(serializedBody, "\t"), COGNOME_LEN);
    p.numeroPersone = atoi(strtok(NULL, "\t"));
    strncpy(p.data, strtok(NULL, "\t"), DATE_LEN);
    p.ora = atoi(strtok(NULL, "\t"));
    
    p.cognome[COGNOME_LEN -1] = '\0';
    p.data[DATE_LEN -1] = '\0';
    strcpy(dtemp, p.data);

    // Convalido i dati della prenotazione
    if(convalidaCognome(p.cognome, p.cognome) == 0){
        reqErrorManager(socket, "Errore nel formato del cognome\n");
        return;
    }
    if(p.numeroPersone < 1){
        reqErrorManager(socket, "Errore nel formato del numero di persone\n");
        return;
    }
    if(convalidaData(dtemp, p.data) == 0){
        reqErrorManager(socket, "Errore nel formato della data\n");
        return;
    }
    if(p.ora < 0 || p.ora > 23){
        reqErrorManager(socket, "Errore nel formato dell'ora\n");
        return;
    }   
    p.data[DATE_LEN - 1] = '\0';
    scomposeData(p.data, &dd, &mm, &yy);
    if(convalidaOrarioPassato(dd, mm, yy, p.ora) == -1){
        reqErrorManager(socket, "E' stato specificato un orario passato\n");
        return;
    }

    // VERBOSE
    printf("Dati della prenotazione convalidati:\n");
    printf("\tCognome: %s\n", p.cognome);
    printf("\tNumero persone: %d\n", p.numeroPersone);
    printf("\tData: %s\n", p.data);
    printf("\tOra: %d\n", p.ora);

    // Recupero la lista dei tavoli disponibili
    strcpy(dtemp, p.data);
    tableList = getAvailableTable(p.numeroPersone, dtemp, p.ora, &tavoliDisponibili);

    // Rimuovo eventuali proposte precedenti
    removeProposedTable(socket);

    // Salvo le nuove proposte di tavoli
    if(tavoliDisponibili != 0){

        active_client_device_t *table = (active_client_device_t *)malloc(sizeof(active_client_device_t));
        memset(table, 0, sizeof(active_client_device_t));
        
        table->socket = socket;
        strcpy(table->reservation.cognome, p.cognome);
        strcpy(table->reservation.data, p.data);
        table->reservation.ora = p.ora;
        table->reservation.numeroPersone = p.numeroPersone;
        table->next = NULL;

        addProposedTable(table);

    }

    // VERBOSE
    printf("Proposte di tavoli aggiornate\n");

    // Preparo la risposta
    res.header = tavoliDisponibili;

    // Se non ci sono tavoli disponibili
    if(tavoliDisponibili == 0){

        // Preparo la risposta
        sprintf(buffer, "%d\n%s", res.header, "Nessun tavolo disponibile");
    
    }

    // Se ci sono tavoli disponibili
    else{

        // Serializzo la lista sul buffer
        serializeTableList(tableList, res.serializedBody);

        // Preparo la risposta
        sprintf(buffer, "%d\n%s", res.header, res.serializedBody);

    }

    // VERBOSE
    printf("Risposta pronta\n");

    // Invio la risposta
    if(send(socket, (void *)buffer, BUFFER_LEN, 0) == 0){
        reqErrorManager(socket, "Errore nell'invio della risposta\n");
        return;
    };

    // VERBOSE
    printf("Risposta inviata\n");

    // Dealloco la lista dei tavoli
    deallocateTableList(tableList);

    // VERBOSE
    printf("Lista dei tavoli deallocata\n");

}


void bookManager(int socket, char *serializedBody){

    int tavolo = -1;

    char buffer[BUFFER_LEN];
    memset(buffer, 0, BUFFER_LEN);
    
    response_t res;
    memset(&res, 0, sizeof(response_t));

    booking_t b;
    memset(&b, 0, sizeof(booking_t));

    reservation_t p;
    memset(&p, 0, sizeof(reservation_t));

    active_client_device_t *proposal = NULL;
    memset(&proposal, 0, sizeof(active_client_device_t));

    int mese, anno;

    // VERBOSE
    printf("Richiesta book individuata\n");

    // Deserializzo il body della richiesta
    tavolo = atoi(serializedBody);

    // VERBOSE
    printf("Tavolo selezionato: %d\n", tavolo);

    // Recupero le informazioni della proposta
    proposal = getProposedTable(socket);
    if(proposal == NULL){
        reqErrorManager(socket, "Non sono state ancora effettuate proposte al client\n");
        return;
    }

    // Recuperare tavolo e sala
    b.tavolo = tavolo;
    if((b.sala = getTableRoom(tavolo)) == -1)

    // VERBOSE
    printf("Proposta trovata\n");
    printf("\tCognome: %s\n", proposal->reservation.cognome);
    printf("\tData: %s\n", proposal->reservation.data);
    printf("\tOra: %d\n", proposal->reservation.ora);
    printf("\tNumero persone: %d\n", proposal->reservation.numeroPersone);

    // VERBOSE
    printf("Opzione valida!\n");

    // Genero il codice della prenotazione
    scomposeData(proposal->reservation.data, &anno, &mese, NULL);
    getNewCode(b.codicePrenotazione, mese, anno);

    // VERBOSE
    printf("Codice prenotazione generato: %s\n", b.codicePrenotazione);

    // Salvo la prenotazione
    if(setReservation(
        b.codicePrenotazione, 
        b.tavolo, 
        proposal->reservation.data, 
        proposal->reservation.ora, 
        proposal->reservation.cognome, 
        proposal->reservation.numeroPersone, 
        NULL, 
        PRENOTATO
       ) == -1){
        reqErrorManager(socket, "Il tavolo è già occupato, prova ad aggiornare le opzioni.\n");
        return;
    }

    // Rimuovo le proposte fatte al client
    removeProposedTable(socket);

    // Preparo la risposta
    res.header = 0;
    sprintf(res.serializedBody, "%s\t%d\t%d", b.codicePrenotazione, b.tavolo, b.sala);

    // Serializzo la risposta
    sprintf(buffer, "%d\n%s", res.header, res.serializedBody);

    // VERBOSE
    printf("Risposta pronta\n");

    // Invio la risposta
    if(send(socket, (void *)buffer, BUFFER_LEN, 0) == 0){
        reqErrorManager(socket, "Errore nell'invio della risposta\n");
        return;
    };

    // VERBOSE
    printf("Risposta inviata\n");

}


// KITCHEN DEVICE ----------------------------------

void takeManager(int kitchenSocket){

    char buffer[BUFFER_LEN];
    memset(buffer, 0, BUFFER_LEN);

    response_t res;
    memset(&res, 0, sizeof(response_t));

    // VERBOSE
    printf("Richiesta take individuata\n");

    // Recupero la comanda in attesa da più tempo
    comanda_order_t *comandaOrder = NULL;
    if((comandaOrder = getOldestWaitingComandaOrder()) == NULL){
        reqErrorManager(kitchenSocket, "Non ci sono comande in attesa\n");
        return;
    }

    // VERBOSE
    printf("Comanda trovata, è relativa al socket %d\n", comandaOrder->tableSocket);

    // VERBOSE
    printf("Dati del tavolo trovati\n");
    printf("\tTavolo assegnato: %d\n", comandaOrder->comanda->tavolo);
    printf("\tOrario: %d\n", comandaOrder->comanda->ora);
    printf("\tData: %s\n", comandaOrder->comanda->data);

    // VERBOSE
    printf("Comanda in attesa:\n");
    printComandeList(comandaOrder->comanda, 0);

    // Preparo la risposta
    res.header = 0;
    serializeComandeList(comandaOrder->comanda, res.serializedBody);

    // Serializzo la risposta
    sprintf(buffer, "%d\n%s", res.header, res.serializedBody);
    
    // VERBOSE
    printf("Risposta pronta per il kitchen device...\n");

    // Invio la risposta
    if(send(kitchenSocket, (void *)buffer, BUFFER_LEN, 0) == 0){
        reqErrorManager(kitchenSocket, "Errore nell'invio della risposta\n");
        return;
    };

    // VERBOSE
    printf("Risposta inviata!\n");

    // Aggiorno lo stato della comanda
    if(setComanda(
        comandaOrder->comanda->data,
        comandaOrder->comanda->tavolo, 
        comandaOrder->comanda->ora, 
        comandaOrder->comanda->numero, 
        PREPARAZIONE, 
        comandaOrder->comanda->dataRis
        ) == -1){
        reqErrorManager(kitchenSocket, "Errore nell'aggiornamento dello stato della comanda\n");
        return;
    }
    
    // Specifico il kitchen device che se ne è preso carico
    comandaOrder->kitchenSocket = kitchenSocket;

    // Notifico il table device
    notifyTableDevice(comandaOrder->tableSocket, comandaOrder->comanda->numero, PREPARAZIONE);

    // Notifico gli altri kitchen device
    notifyKitchenDevices(DECREASE, kitchenSocket);

    return;
}


void readyManager(int socket, char *serializedBody){

    char buffer[BUFFER_LEN];
    memset(buffer, 0, BUFFER_LEN);

    response_t res;
    memset(&res, 0, sizeof(response_t));

    int ora, tavolo, numComanda;
    char data[DATE_LEN];
    char tmp[DATE_LEN];

    comanda_order_t *comandaOrder = NULL;

    // VERBOSE
    printf("Richiesta ready individuata\n");

    // Deserializzo il body della richiesta
    sscanf(serializedBody, "%s\t%d\t%d\t%d", data, &ora, &tavolo, &numComanda);

    // Convalido i dati
    if(tavolo < 1){
        reqErrorManager(socket, "Valore del tavolo non valido\n");
        return;
    }
    if(numComanda < 1){
        reqErrorManager(socket, "Valore della comanda non valido\n");
        return;
    }
    if (ora < 0 || ora > 23){
        reqErrorManager(socket, "Valore dell'ora non valido\n");
        return;
    }
    sprintf(tmp, "%c%c-%c%c-%c%c", data[6], data[7], data[3], data[4], data[0], data[1]);
    if(convalidaData(tmp, tmp) == 0){
        reqErrorManager(socket, "Errore nel formato della data\n");
        return;
    }

    // VERBOSE
    printf("Dati deserializzati:\n");
    printf("\tData: %s\n", data);
    printf("\tOra: %d\n", ora);
    printf("\tTavolo: %d\n", tavolo);
    printf("\tComanda: %d\n", numComanda);

    // Verifico che la comanda sia in preparazione
    if((comandaOrder = getComandaOrderByComanda(data, ora, tavolo, numComanda)) == NULL || comandaOrder->kitchenSocket != socket){

        printf("Il tavolo ha lasciato il ristorante o la comanda non è in preparazione, procedo a segnalarlo.\n");
        
        // Recupero le comande in preparazione dal kitchen device
        comanda_t *comandaList = getPreparatingComandaOrder(socket);

        // VERBOSE
        if(comandaList != NULL){
            printf("Comande in preparazione per il kitchen device %d:\n", socket);
            printComandeList(comandaList, 0);
        }
        else
            printf("Non ci sono comande in preparazione per il kitchen device %d.\n", socket);


        // Preparo la risposta
        res.header = 1;
        serializeComandeList(comandaList, res.serializedBody);
        sprintf(buffer, "%d\n%s", res.header, res.serializedBody);

        // Invio la risposta
        if(send(socket, (void *)buffer, BUFFER_LEN, 0) == 0){
            reqErrorManager(socket, "Errore nell'invio della risposta\n");
            return;
        };

        // VERBOSE
        printf("Risposta inviata\n");

        // Azzero tutti i puntatori della lista
        comanda_t *comanda = comandaList;
        comanda_t *next = NULL;
        while(comanda != NULL){
            next = comanda->next;
            comanda->next = NULL;
            comanda = next;
        }


        return;

    }

    // VERBOSE
    printf("Dati del tavolo trovati:\n");
    printf("\tTavolo assegnato: %d\n", comandaOrder->comanda->tavolo);
    printf("\tOrario: %d\n", comandaOrder->comanda->ora);
    printf("\tData: %s\n", comandaOrder->comanda->data);

    // Aggiorno lo stato della comanda
    if(setComanda(
        comandaOrder->comanda->data, 
        comandaOrder->comanda->tavolo, 
        comandaOrder->comanda->ora, 
        comandaOrder->comanda->numero, 
        SERVIZIO, 
        comandaOrder->comanda->dataRis
        ) == -1){
        reqErrorManager(socket, "Errore nell'aggiornamento dello stato della comanda\n");
        return;
    }

    // VERBOSE
    printf("Stato della comanda aggiornato\n");

    // Preparo la risposta
    res.header = 0;
    res.serializedBody[0] = '\0';

    // Serializzo la risposta
    sprintf(buffer, "%d\n%s", res.header, res.serializedBody);
    
    // VERBOSE
    printf("Risposta pronta: %d\n", res.header);

    // Invio la risposta
    if(send(socket, (void *)buffer, BUFFER_LEN, 0) == 0){
        reqErrorManager(socket, "Errore nell'invio della risposta\n");
        return;
    };

    // VERBOSE
    printf("Risposta inviata\n");

    // Notifico il table device
    notifyTableDevice(comandaOrder->tableSocket, comandaOrder->comanda->numero, SERVIZIO);

    // Dealloco la comanda
    deleteComandaOrder(comandaOrder);

    return;

}


// TABLE DEVICE ----------------------------------

void unlockManager(int socket, char *serializedBody){

    char error[ERROR_MSG_LEN];
    int tavolo = -1;
    int orario = -1;
    int giorno, mese, anno;
    
    char buffer[BUFFER_LEN];
    memset(buffer, 0, BUFFER_LEN);
    
    response_t res;
    memset(&res, 0, sizeof(response_t));

    bookingCode_t codPrenotazione;
    memset(&codPrenotazione, 0, sizeof(bookingCode_t));

    // VERBOSE
    printf("Richiesta unlock individuata\n");

    // Deserializzo il body della richiesta
    sscanf(serializedBody, "%s", codPrenotazione);

    // VERBOSE
    printf("Codice deserializzato: %s\n", codPrenotazione);


    // Verifico che il codice sia sintatticamente valido
    if(convalidaSintassiCodice(codPrenotazione, codPrenotazione) == -1){
        reqErrorManager(socket, "Codice piatto sintatticamente non valido\n");
        return;
    }

    // VERBOSE
    printf("Sintassi del codice valida\n");

    // Tento di aggiornare lo stato della prenotazione
    char today[DATE_LEN];
    getTodayDate(today);
    if((tavolo = updateReservationState(today, codPrenotazione, CORSO, error)) == -1){
        reqErrorManager(socket, error);
        return;
    }

    // VERBOSE
    printf("Codice valido!\n");

    // Recupero l'orario
    getCurrentDateTime(&orario, &giorno, &mese, &anno);

    // Alloco la struttura dati del tavolo
    active_table_device_t *table = (active_table_device_t *)malloc(sizeof(active_table_device_t));
    memset(table, 0, sizeof(active_table_device_t));
    table->socket = socket;
    table->tavoloAssegnato = tavolo;
    table->orario = orario;
    strcpy(table->codice, codPrenotazione);
    sprintf(table->data, "%02d-%02d-%02d", anno, mese, giorno);
    addActiveTable(table);

    // VERBOSE
    printf("Tavolo sbloccato: il table device %d è assegnato a T%d dall'ora %02d (%02d-%02d-%02d)\n", socket, tavolo, orario, giorno, mese, anno);

    // Preparo la risposta
    res.header = 0;
    res.serializedBody[0] = '\0';

    // Serializzo la risposta
    sprintf(buffer, "%d\n%s", res.header, res.serializedBody);

    // VERBOSE
    printf("Risposta pronta\n");

    // Invio la risposta
    if(send(socket, (void *)buffer, BUFFER_LEN, 0) == 0){
        reqErrorManager(socket, "Errore nell'invio della risposta\n");
        return;
    };

    // VERBOSE
    printf("Risposta inviata\n");

}


void menuManager(int socket){

    char buffer[BUFFER_LEN];
    memset(buffer, 0, BUFFER_LEN);
    
    response_t res;
    memset(&res, 0, sizeof(response_t));

    menu_dish_t *menu = NULL;
    int numPortate = -1;
    active_table_device_t *table = NULL;
    
    // VERBOSE
    printf("Richiesta menu individuata\n");
    
    if((table = getActiveTable(socket)) == NULL){
        reqErrorManager(socket, "Errore nel recupero del tavolo associato\n");
        return;
    }

    if((numPortate = getMenu(&menu, table->data)) == -1){
        reqErrorManager(socket, "Errore nel recupero del menu proposto\n");
        return;
    }

    // Serializzo il menu
    serializeMenu(menu, res.serializedBody);

    // VERBOSE
    printf("Menu (%d portate) serializzato\n", numPortate);    

    // Preparo la risposta
    res.header = numPortate;

    // Serializzo la risposta
    sprintf(buffer, "%d\n%s", res.header, res.serializedBody);

    // VERBOSE
    printf("Risposta pronta\n");

    // Invio la risposta
    if(send(socket, (void *)buffer, BUFFER_LEN, 0) == 0){
        reqErrorManager(socket, "Errore nell'invio della risposta\n");
        return;
    };

    // VERBOSE
    printf("Risposta inviata\n");
    
    // Deallocare il menu
    deallocateMenu(menu);

    return;

}


void comandaManager(int socket, char *serializedBody){

    char buffer[BUFFER_LEN];
    memset(buffer, 0, BUFFER_LEN);
    
    response_t res;
    memset(&res, 0, sizeof(response_t));

    comanda_dish_t *dishList = NULL;
    comanda_dish_t *dishListTmp;

    int numeroComanda;

    active_table_device_t *table = NULL;

    menu_dish_t *menu = NULL;
    int numPortate = -1;

    char data[DATE_LEN];
    
    // VERBOSE
    printf("Richiesta comanda individuata\n");
    
    // Recupero il tavolo associato
    table = getActiveTable(socket);
    if(table == NULL){
        reqErrorManager(socket, "Errore nella ricerca del tavolo\n");
        return;
    }
    
    // Recupero il menu proposto
    if((numPortate = getMenu(&menu, table->data)) == -1){
        reqErrorManager(socket, "Errore nel recupero del menu proposto\n");
        return;
    }

    // Verifico che la comanda sia sintatticamente valida
    deserializeComandaDishList(&dishList, serializedBody);

    // VERBOSE
    printf("Comanda deserializzata:\n");
    printComandaDishList(dishList);


    // Verifico che i piatti specificati siano presenti nel menu
    dishListTmp = dishList;
    while(dishListTmp != NULL){
        if(isDishInMenu(dishListTmp->codice, menu) == -1){
            char error[ERROR_MSG_LEN];
            sprintf(error, "Il piatto %s non è presente nel menu\n", dishListTmp->codice);
            reqErrorManager(socket, error);
            deallocateComandaDishList(dishList);
            return;
        }
        dishListTmp = dishListTmp->next;
    }

    // VERBOSE
    printf("I piatti sono tutti validi!\n");

    // Inserisco la dishList nel file
    getTodayDate(data);
    if((numeroComanda = setComanda(table->data, table->tavoloAssegnato, table->orario, 0, ATTESA, data)) == -1){
        reqErrorManager(socket, "Errore nell'inserimento della comanda\n");
        return;
    }

    // VERBOSE
    printf("Comanda %d inserita nel database!\n", numeroComanda);


    // Inserisco i piatti della comanda nel file
    dishListTmp = dishList;
    while(dishListTmp != NULL){
        if(setDish(table->data, table->tavoloAssegnato, table->orario, numeroComanda, dishListTmp->codice, dishListTmp->quantita) == -1){
            reqErrorManager(socket, "Errore nell'inserimento dei piatti della comanda\n");
            return;
        }
        dishListTmp = dishListTmp->next;
    }

    // VERBOSE
    printf("Piatti della comanda %d inseriti nel database!\n", numeroComanda);

    // Preparo la risposta
    res.header = numeroComanda;
    res.serializedBody[0] = '\0';

    // Serializzo la risposta
    sprintf(buffer, "%d\n%s", res.header, res.serializedBody);

    // VERBOSE
    printf("Risposta pronta: %s\n", buffer);

    // Invio la risposta
    if(send(socket, (void *)buffer, BUFFER_LEN, 0) == 0){
        reqErrorManager(socket, "Errore nell'invio della risposta\n");
        return;
    };

    // VERBOSE
    printf("Risposta inviata\n");

    // Aggiungo la comanda all'ordine delle comande effettuate
    comanda_t *comanda = (comanda_t *)malloc(sizeof(comanda_t));
    memset(comanda, 0, sizeof(comanda_t));
    strcpy(comanda->data, table->data);
    strcpy(comanda->dataRis, data);
    comanda->numero = numeroComanda;
    comanda->stato = ATTESA;
    comanda->ora = table->orario;
    comanda->tavolo = table->tavoloAssegnato;
    comanda->piatti = dishList;

    addComandaOrder(socket, comanda);
    
    // VERBOSE
    printf("Comanda %d aggiunta all'ordine delle comande effettuate\n", numeroComanda);
    printf("Numero di comande in attesa: %d\n", getWaitingComandeNum());

    // Notifico i kitchen device
    printf("PROCEDO A NOTIFICARE I KITCHEN DEVICE\n");
    notifyKitchenDevices(INCREASE, socket);

    // Deallocare il menu
    deallocateMenu(menu);

    return;
}


void contoManager(int socket){

    char buffer[BUFFER_LEN];
    memset(buffer, 0, BUFFER_LEN);
    
    response_t res;
    memset(&res, 0, sizeof(response_t));

    active_table_device_t *table = NULL;
    subtotale_conto_t *conto = NULL;

    // VERBOSE
    printf("Richiesta conto individuata\n");

    // Recupero i dati 
    table = getActiveTable(socket);
    if(table == NULL){
        reqErrorManager(socket, "Errore nella ricerca del tavolo\n");
        return;
    }

    // Recupero i piatti ordinati
    conto = getBill(table->data, table->orario, table->tavoloAssegnato);

    // Non sono stati ordinati piatti
    if(conto == NULL){
        printf("Non sono stati ordinati piatti\n");

        // Preparo la risposta
        res.header = 0;
        res.serializedBody[0] = '\0';
    }
    
    // Sono stati ordinati piatti
    else{
        
        menu_dish_t *menu = NULL;
        int numPortate = -1;

        // VERBOSE
        printf("Recuperate le voci del conto, calcolo il subtotale...\n");

        if((numPortate = getMenu(&menu, table->data)) == -1){
            reqErrorManager(socket, "Errore nel recupero del menu\n");
            return;
        }

        // Calcolo il subtotale per ogni voce del conto
        computeSubtotals(conto, menu);

        // VERBOSE
        printf("Conto calcolato\n");
        printConto(conto);

        // Preparo la risposta
        res.header = 1;
        serializeConto(conto, res.serializedBody);

    }

    // VERBOSE
    printf("Conto serializzato\n");

    // Serializzo la risposta
    sprintf(buffer, "%d\n%s", res.header, res.serializedBody);

    // VERBOSE
    printf("Risposta pronta\n");

    // Invio la risposta
    if(send(socket, (void *)buffer, BUFFER_LEN, 0) == 0){
        reqErrorManager(socket, "Errore nell'invio della risposta\n");
        return;
    };

    // VERBOSE
    printf("Risposta inviata\n");

    // Aggiorno la prenotazione nel file
    char error[ERROR_MSG_LEN];
    if(updateReservationState(table->data, table->codice, TERMINATA, error) == -1)
        printf("Errore durante l'aggiornamento dello stato della comanda:\n\t%s\n", error);

    // Dealloco il conto
    deallocateConto(conto);

    // Rimuovo il tavolo dalla lista dei tavoli attivi
    removeActiveTable(socket);

    // Rimuovo eventuali comande ancora in attesa
    removeWaitingComande(socket);

    // VERBOSE
    printf("Tavolo rimosso e dati deallocati\n");

    return;


}


void computeSubtotals(subtotale_conto_t *conto, menu_dish_t *menu){
    
    subtotale_conto_t *contoTmp = NULL;
    menu_dish_t *menuTmp = NULL;

    // Per ogni voce del conto
    contoTmp = conto;
    while(contoTmp != NULL){

        // Per ogni piatto del menu
        menuTmp = menu;
        while(menuTmp != NULL){

            // Se il piatto è presente nel menu
            if(strcmp(contoTmp->codice, menuTmp->codice) == 0){

                // Calcolo il subtotale
                contoTmp->prezzo = menuTmp->prezzo * contoTmp->quantita;
                break;
            }

            menuTmp = menuTmp->next;
        }

        contoTmp = contoTmp->next;
    }
    
    // Deallocare il menu
    deallocateMenu(menu);

    return;

}


// GESTIONE RISPOSTA AD ERRORI ----------------------------------

void reqErrorManager(int socket, char *errMesg){
    
    char buffer[BUFFER_LEN];
    memset(buffer, 0, BUFFER_LEN);
    response_t res;

    printf("Errore:\n%s\n", errMesg);
    
    // Preparo la risposta
    res.header = -1;
    sprintf(res.serializedBody, "%s", errMesg);
    sprintf(buffer, "%d\n%s", res.header, res.serializedBody);
    
    // Invio la risposta
    if(send(socket, (void *)buffer, BUFFER_LEN, 0) == 0){
        printf("Errore nell'invio della risposta\n");
        return;
    };
    
    return;
}


/*--------------------------------- GESTIONE DELLE NOTIFICHE ----------------------------------*/


// KITCHEN DEVICE ----------------------------------

void notifyKitchenDevices(server_operation_t op, int socket){

    char buffer[BUFFER_LEN];
    memset(buffer, 0, BUFFER_LEN);

    server_message_t msg;
    memset(&msg, 0, sizeof(server_message_t));
    
    connection_t *connection = connectionList;

    // VERBOSE
    printf("Notifico i kitchen device...\n");

    // Preparo il messaggio da inviare ai dispositivi
    msg.header = op;
    strcpy(msg.serializedBody, "Notifica dal server.");

    // Serializzo il messaggio
    sprintf(buffer, "%d\n%s", msg.header, msg.serializedBody);

    // Invio il messaggio a tutti i kitchen device
    while(connection != NULL){

        // Esento eventualmente il kitchen device che ha inviato la richiesta
        if(connection->dev != KITCHEN || connection->socket == socket){
            connection = connection->next;
            continue;
        }

        if(send(connection->socket, (void *)buffer, BUFFER_LEN, 0) < 0){
            perror("Errore nell'invio del messaggio di notifica");
            return;
        }
        printf("Notifica inviata al kitchen device %d\n", connection->socket);

        connection = connection->next;
    }

    printf("Notifiche inviate!\n");
    return;
}


// TABLE DEVICE ----------------------------------

void notifyTableDevice(int socket, int comanda, comanda_state_t stato){

    char buffer[BUFFER_LEN];
    memset(buffer, 0, BUFFER_LEN);

    server_message_t msg;
    memset(&msg, 0, sizeof(server_message_t));

    // VERBOSE
    printf("Notifico il table device %d...\n", socket);

    // Preparo il messaggio da inviare ai dispositivi
    msg.header = UPDATE;
    sprintf(msg.serializedBody, "%d\t%d", comanda, (int)stato);

    // Serializzo il messaggio
    sprintf(buffer, "%d\n%s", msg.header, msg.serializedBody);
    
    // Invio il messaggio al table device
    if(send(socket, (void *)buffer, BUFFER_LEN, 0) < 0){
        perror("Errore nell'invio del messaggio di notifica");
        return;
    }

    // VERBOSE
    printf("Notifica inviata al table device %d\n", socket);    

    return;
}




/*--------------------------------- CHIUSURA SERVER ----------------------------------*/


void closeDevice(fd_set *master, int serverSocket){

    int socket;

    printf("Procedo con la chiusura del server...\n");

    // Controllo che non ci siano comande in attesa o in preprarazione
    if(mostRecentComanda != NULL){
        printf("Ci sono ancora comande in attesa o in preparazione:\n");
        printComandeList(mostRecentComanda->comanda, 0);
        return;
    }

    printf("Tutte le comande sono in servizio.\n");
    printf("Procedo alla chiusura delle connessioni...\n");

    // Invio il segnale di chiusura a tutti i device
    if((socket = removeAllConnections()) > 0){
        printf("Errore nella chiusura del device %d\n", socket);
        return;
    }

    printf("Connessioni chiuse!\n");
    printf("Dealloco le risorse... ");

    // Eliminare tutte le info sui table device
    removeAllActiveTables();

    // Eliminare tutte le proposal fatte ai client
    removeAllProposedTables();

    printf("Risorse deallocate!\n");

    // Chiudo il socket
    close(serverSocket);

    printf("Socket chiuso.\n");
    printf("Alla prossima :)\n");

    printf("\n*********************** CHIUSURA DEL SERVER ***********************\n\n");
    exit(EXIT_SUCCESS);
}

