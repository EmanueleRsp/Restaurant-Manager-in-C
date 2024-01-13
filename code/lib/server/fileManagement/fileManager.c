#include "fileManager.h"


/*----------------------------- FUNZIONI DI UTILITA' -----------------------------*/

// Verifica se un elemento è presente nell'array
int isInArray(int *array, int dim, int elem, int *index){

    int i;

    for(i = 0; i < dim; i++){
        if(array[i] == elem){
            if(index != NULL)
                *index = i;
            return 1;
        }
    }

    if(index != NULL)
        *index = -1;
    return 0;
}


void getNewCode(char codice[], int mese, int anno) {

    // VERBOSE
    printf("Generazione codice univoco...\n");

    do {

        int i;

        // Genera le prime tre lettere maiuscole
        for (i = 0; i < 3; ++i) {
            codice[i] = 'A' + rand() % 26;
        }

        // Genera le seconde tre cifre
        for (i = 3; i < 6; ++i) {
            codice[i] = '0' + rand() % 10;
        }

        // Termina la stringa con il carattere nullo
        codice[6] = '\0';

    }while(!isUnique(codice, mese, anno));
    
}


int isUnique(char codice[], int mese, int anno){

    char filePath[PATH_MAX];

    FILE *file;
    char line[LINE_LEN];

    // VERBOSE
    printf("Verifica univocità codice %s...\n", codice);

    // Recupero il nome del file da aprire
    sprintf(filePath, "files/reservations/res-%02d-%02d.txt", anno, mese);

    // Apro il file
    file = fopen(filePath, "r");

    // Se il file non esiste, il codice è univoco
    if(file == NULL){
        return 1;
    }

    // Recupero ciascuna riga del file
    while (fgets(line, sizeof(line), file)) {

        // Il codice della prenotazione è la prima colonna
        if(strcmp(strtok(line, "\t"), codice) == 0){
            fclose(file);
            printf("Codice non univoco!\n");
            return 0;
        }

    }
    fclose(file);
    printf("Codice univoco!\n");
    return 1;

}


int recordNumber(char *filePath){

    FILE *file = fopen(filePath, "r");
    int lines = 0;
    char line[LINE_LEN];

    printf("Conteggio righe del file %s...\n", filePath);

    if(file == NULL){
        printf("File non trovato, numero di righe: 0\n");
        return 0;
    }

    while (fgets(line, sizeof(line), file)) {
        lines++;
    }

    fclose(file);
    return lines;

}


int deleteRecord(char *filePath, int rows[], char *values[], int dim){

    FILE *file = fopen(filePath, "r");
    FILE *temp = fopen("temp.txt", "w");
    int lines = 0;
    int i;
    int totalLines = 0;
    char line[LINE_LEN];
    char lineCopy[LINE_LEN];


    // VERBOSE
    printf("Elimino le righe del file %s che contengono i valori:", filePath);
    for(i = 0; i < dim; i++){
        printf(" %s", values[i]);
    }
    printf(", nelle colonne di indice:");
    for(i = 0; i < dim; i++){
        printf(" %d", rows[i]);
    }
    printf("\n");

    if(file == NULL){
        printf("File non trovato, nessuna riga eliminata\n");
        return 0;
    }

    // Scannerizzo ciascuna riga del file
    while (fgets(line, sizeof(line), file)) {
        int match = dim;
        int i = 0;
        char *cell;
        
        strcpy(lineCopy, line);
        cell = strtok(line, "\t");
        
        // Scannerizzo ciascuna cella della riga
        while(cell != NULL){
            i++;
            int corrIndex;

            // Se l'i-esima cella è da analizzare...
            if(isInArray(rows, dim, i, &corrIndex)){

                // ...controllo se il valore corrispondente è contenuto nella riga
                if(strcmp(cell, values[corrIndex]) == 0){
                    match--;
                }
            }
            // Passo alla cella successiva
            cell = strtok(NULL, "\t");
        }

        totalLines++;

        // Se non tutte le celle da analizzare sono corrispondenti, salvo la riga
        if(match >  0){
            fputs(lineCopy, temp);
            lines++;
        }

        // VERBOSE
        else{
            printf("Riga n. %d eliminata\n", totalLines);
        }

    }

    fclose(file);
    fclose(temp);

    if(remove(filePath) == -1){
        perror("Errore nella rimozione del file");
        remove("temp.txt");
        return -1;
    }
    if(rename("temp.txt", filePath) == -1){
        perror("Errore nella rinomina del file temp.txt");
        return -1;
    }

    // VERBOSE
    printf("Numero di righe eliminate: %d\n", totalLines - lines);

    return lines;

}


/*----------------------------- GESTIONE DELLE PRENOTAZIONI -----------------------------*/


table_t *getAvailableTable(int numPersone, char *data, int ora, int *tavoliDisponibili){

    char dtemp[DATE_LEN];
    int anno;
    int mese;
    int giorno;

    char filePath[PATH_MAX];
    
    int tabNum, i;
    int numeroPrenotazioni = 0;
    
    FILE *file;
    table_t *tableList = NULL;

    *tavoliDisponibili = 0;

    // VERBOSE
    printf("Recupero dei tavoli disponibili...\n");

    // Recupero il nome del file da aprire
    strcpy(dtemp, data);
    scomposeData(dtemp, &giorno, &mese, &anno);
    sprintf(filePath, "files/reservations/res-%02d-%02d.txt", anno, mese);

    // VERBOSE
    printf("File da aprire: %s\n", filePath);

    // Recupero il numero di tavoli
    tabNum = recordNumber("files/tableOrg.txt");
    if(tabNum < 1){
        return NULL;
    }

    // VERBOSE
    printf("Numero di tavoli del ristorante: %d\n", tabNum);

    // Recupero i codici dei tavoli prenotati per quell'orario
    int *tabCod = (int *) malloc(tabNum * sizeof(int));
    for(i = 0; i < tabNum; i++){
        tabCod[i] = -1;
    }
    if((numeroPrenotazioni = getBookedTableCode(filePath, data, ora, tabCod)) == -1){
        return NULL;
    }

    // VERBOSE
    printf("Numero di tavoli non disponibili: %d\n", numeroPrenotazioni);

    file = fopen("files/tableOrg.txt", "r");
    if(file == NULL){
        return NULL;
    }

    // Recupero le informazioni di ogni tavolo disponibile
    char line[LINE_LEN];
    while (fgets(line, sizeof(line), file)) {

        int cod, sala, posti;
        char *cella = strtok(line, "\t");
        
        // Il codice del tavolo è la prima colonna
        cod = atoi(cella);
        if(isInArray(tabCod, tabNum, cod, NULL)){
            continue;
        }

        // La sala è la seconda colonna
        cella = strtok(NULL, "\t");
        sala = atoi(cella);

        // Il numero di posti è la terza colonna
        cella = strtok(NULL, "\t");
        posti = atoi(cella);

        // Se il tavolo non ha posti sufficienti, lo salto
        if(posti < numPersone){
            continue;
        }

        // Se arrivo qui, il tavolo è disponibile
        table_t *newTable = (table_t *) malloc(sizeof(table_t));
        memset(newTable, 0, sizeof(table_t));

        newTable->codice = cod;
        newTable->sala = sala;
        strcpy(newTable->info, strtok(NULL, "\n"));

        newTable->next = tableList;
        tableList = newTable;

        // VERBOSE
        printf("Tavolo disponibile: T%d SALA%d %s\n", newTable->codice, newTable->sala, newTable->info);
    
        (*tavoliDisponibili)++;

    }

    // VERBOSE
    printf("Numero di tavoli disponibili: %d\n", *tavoliDisponibili);
    printf("Tavoli disponibili recuperati\n");

    free(tabCod);
    fclose(file);
    return tableList;

}


int getBookedTableCode(char *filePath, char data[], int ora, int tabCod[]){

    FILE *file = fopen(filePath, "r");
    int lines = 0;
    char line[LINE_LEN];

    // VERBOSE
    printf("Recupero dei tavoli prenotati per il %s alle %d...\n", data, ora);

    // Se il file non esiste, non ci sono prenotazioni
    if(file == NULL){
        return 0;
    }
    
    // VERBOSE
    printf("File aperto: %s\n", filePath);

    while (fgets(line, sizeof(line), file)) {

        char *cella = strtok(line, "\t");

        // Il codice del tavolo è la seconda colonna
        cella = strtok(NULL, "\t");
        tabCod[lines] = atoi(cella);

        // La data è la terza colonna
        cella = strtok(NULL, "\t");
        if(strcmp(data, cella) != 0){
            tabCod[lines] = -1;
            continue;
        }

        // L'ora è la quarta colonna
        cella = strtok(NULL, "\t");
        if(ora != atoi(cella)){
            tabCod[lines] = -1;
            continue;
        }

        printf("Tavolo prenotato: T%d\n", tabCod[lines]);
        lines++;

    }

    fclose(file);
    printf("Numero di tavoli prenotati: %d\n", lines);
    return lines;

}


int getTableRoom(int table){

    FILE *file = fopen("files/tableOrg.txt", "r");
    char line[LINE_LEN];

    // VERBOSE
    printf("Recupero della sala del tavolo %d...\n", table);

    // Se il file non esiste, non ci sono tavoli
    if(file == NULL){
        return -1;
    }

    // VERBOSE
    printf("File aperto: files/tableOrg.txt\n");

    while (fgets(line, sizeof(line), file)) {

        char *cella = strtok(line, "\t");

        // Il codice del tavolo è la prima colonna
        if(atoi(cella) != table){
            continue;
        }

        // La sala è la seconda colonna
        cella = strtok(NULL, "\t");
        printf("Sala del tavolo %d: %d\n", table, atoi(cella));
        fclose(file);
        return atoi(cella);

    }

    fclose(file);
    return -1;

}


int setReservation(char codice[], int tavolo, char data[], int ora, char cognome[], int numPersone, char ts[], reservation_state_t conferma){

    char dtemp[DATE_LEN];
    int anno;
    int mese;
    int giorno;
    char timestamp[TIMESTAMP_LEN];

    char filePath[PATH_MAX];

    char line[LINE_LEN];
    
    FILE *file;

    // VERBOSE
    printf("Salvataggio della prenotazione...\n");

    // Recupero l'anno e il mese della data
    strcpy(dtemp, data);
    scomposeData(dtemp, &giorno, &mese, &anno);

    // Recupero il path del file
    sprintf(filePath, "files/reservations/res-%02d-%02d.txt", anno, mese);

    // VERBOSE
    printf("File da aprire: %s\n", filePath);

    // Recupero il timestamp se non indicato formattato YY-MM-DD HH:MM:SS
    if(ts == NULL)
        getTimestamp(timestamp);
    else
        strcpy(timestamp, ts);

    // Apro il file
    file = fopen(filePath, "a+"); // Apro il file in append/lettura
    if(file == NULL){
        printf("Errore nell'apertura del file\n");
        return -1;
    }

    // Verifico che il tavolo non sia occupato per quell'ora
    while(fgets(line, sizeof(line), file)){

        char *cella = strtok(line, "\t");

        // Il tavolo è la seconda colonna
        cella = strtok(NULL, "\t");
        if(atoi(cella) != tavolo){
            continue;
        }

        // La data è la terza colonna
        cella = strtok(NULL, "\t");
        if(strcmp(cella, data) != 0){
            continue;
        }

        // L'ora è la quarta colonna
        cella = strtok(NULL, "\t");
        if(atoi(cella) != ora){
            continue;
        }

        // VERBOSE
        printf("Il tavolo è già occupato per quell'ora...\n");
        fclose(file);
        return -1;

    }

    // Se arrivo qui il tavolo è libero, posso salvarlo    
    fprintf(file, "%s\t%d\t%s\t%d\t%s\t%d\t%s\t%d\n", 
        codice, 
        tavolo, 
        data, 
        ora, 
        cognome, 
        numPersone,
        timestamp,
        (int)conferma
    );

    fclose(file);
    
    printf("Prenotazione salvata:\n");
    printf("\tCodice: %s\n", codice);
    printf("\tTavolo: %d\n", tavolo);
    printf("\tData: %s\n", data);
    printf("\tOra: %d\n", ora);
    printf("\tCognome: %s\n", cognome);
    printf("\tNumero di persone: %d\n", numPersone);
    printf("\tTimestamp: %s\n", timestamp);
    printf("\tConferma: %d\n", conferma);

    return 0;

}


int updateReservationState(char data[], bookingCode_t codice, reservation_state_t state, char errorStr[]){

    reservationRow_t reservation;

    char line[LINE_LEN];

    char filePath[PATH_MAX];
    int found = 0;
    
    FILE *file;

    // VERBOSE
    printf("Ricerca della prenotazione...\n");

    // Recupero il path del file
    char dataTemp[6];
    strncpy(dataTemp, data, 5);
    dataTemp[5] = '\0';
    sprintf(filePath, "files/reservations/res-%s.txt", dataTemp);
    
    // VERBOSE
    printf("File da aprire: %s\n", filePath);

    // Se il file non esiste, non ci sono proposte
    file = fopen(filePath, "r");
    if(file == NULL){
        strcpy(errorStr, "Non ci sono prenotazioni per questo mese");
        return -1;
    }

    while (fgets(line, sizeof(line), file)) {

        char *cella = strtok(line, "\t");

        // Il codice è la prima colonna
        if(strcmp(cella, codice) != 0){
            strcpy(errorStr, "Codice errato!");
            continue;
        }
        strcpy(reservation.codice, cella);
        reservation.codice[BOOK_CODE_LEN - 1] = '\0'; 

        // Il tavolo è la seconda colonna
        cella = strtok(NULL, "\t");
        reservation.tavolo = atoi(cella);

        // La data è la terza colonna
        cella = strtok(NULL, "\t");
        strcpy(reservation.data, cella);
        reservation.data[DATE_LEN - 1] = '\0'; 

        // L'ora è la quarta colonna
        cella = strtok(NULL, "\t");
        reservation.ora = atoi(cella);

        // Verifico se la prenotazione è ancora valida (se vogliamo iniziare il pasto)
        if(state == CORSO){
                    
            char currentDateHour[DATE_LEN + 3];
            int hh;
            char reservationDateHour[DATE_LEN + 3];
            int phh, pdd, pmm, pyy;

            // Recupero l'orario attuale
            getCurrentDateTime(&hh, NULL, NULL, NULL);
            sprintf(currentDateHour, "%s %d", data, hh);

            // Formatto l'orario della prenotazione
            scomposeData(reservation.data, &pdd, &pmm, &pyy);
            phh = reservation.ora;
            sprintf(reservationDateHour, "%02d-%02d-%02d %d", pyy, pmm, pdd, phh);

            // VERBOSE
            printf("Data e ora prenotazione: %s\n", reservationDateHour);
            printf("Data e ora attuali: %s\n", currentDateHour);

            // COnfronto gli orari
            switch (strcmp(currentDateHour, reservationDateHour)){
            case 1:
                strcpy(errorStr, "La prenotazione non è più valida");
                fclose(file);
                return -1;
            case -1:
                strcpy(errorStr, "La prenotazione non è ancora valida");
                fclose(file);
                return -1;
            default:
                printf("L'orario della prenotazione è valido\n");
                break;
            }

        }
        
        // Il cognome è la quinta colonna
        cella = strtok(NULL, "\t");
        strcpy(reservation.cognome, cella);

        // Il numero di persone è la sesta colonna
        cella = strtok(NULL, "\t");
        reservation.numeroPersone = atoi(cella);

        // Il timestamp è la settima colonna
        cella = strtok(NULL, "\t");
        strcpy(reservation.timestamp, cella);

        // L'ottava colonna è lo stato della prenotazione
        cella = strtok(NULL, "\t");
        reservation.state = atoi(cella);

        // Se lo stato della prenotazione è uguale o maggiore a quello a cui lo dobbiamo portare è errore
        if((int)reservation.state >= (int)state){
            strcpy(errorStr, "La prenotazione è già stata confermata");
            fclose(file);
            return -1;
        }

        // Se arrivo qui, ho trovato la prenotazione
        found = 1;
        break;

    }

    fclose(file);

    if(!found){
        strcpy(errorStr, "Codice errato!");
        return -1;
    }

    // VERBOSE
    printf("Prenotazione trovata!\n");

    // Stampo la reservation
    char *stateStr[3] = {"PRENOTATO", "PASTO IN CORSO", "PASTO TERMINATO"};
    printf("\tCodice: %s\n", reservation.codice);
    printf("\tTavolo: %d\n", reservation.tavolo);
    printf("\tData: %s\n", reservation.data);
    printf("\tOra: %d\n", reservation.ora);
    printf("\tCognome: %s\n", reservation.cognome);
    printf("\tNumero persone: %d\n", reservation.numeroPersone);
    printf("\tTimestamp: %s\n", reservation.timestamp);
    printf("\tStato attuale: %s\n", stateStr[reservation.state]);

    printf("Elimino i vecchi dati...\n");

    // Codice valido, devo confermare la prenotazione
    int row[] = {1};
    char *value[] = {reservation.codice};
    if(deleteRecord(filePath, row, value, 1) == -1){
        strcpy(errorStr, "Errore nell'inserimento della conferma");
        return -1;
    }

    // VERBOSE
    printf("Aggiorno il nuovo stato...\n");

    // Aggiorno il file
    if(setReservation(
        reservation.codice, 
        reservation.tavolo, 
        reservation.data, 
        reservation.ora, 
        reservation.cognome, 
        reservation.numeroPersone, 
        reservation.timestamp, 
        state
    ) == -1){
        strcpy(errorStr, "Errore nell'inserimento della conferma");
        return -1;
    }

    // VERBOSE
    printf("Stato della prenotazione aggiornato!\n");
    printf("Nuovo stato della prenotazione: %s\n", stateStr[state]);

    errorStr[0] = '\0';
    return reservation.tavolo;

}


/*----------------------------- GESTIONE DI PIATTI E COMANDE -----------------------------*/


comanda_t *getComanda(char data[], int ore[], int tavoli[], int comande[], comanda_state_t stati[], char dataRistorazione[], int dim[], int verbose){

    char filePath[PATH_MAX];

    FILE *file;
    char line[LINE_LEN];

    comanda_t *comandaList = NULL;

    // VERBOSE
    if(verbose){
        printf("Recupero delle comande...\n");

        // Stampo i parametri di ricerca
        printf("Parametri di ricerca:\n");
        printf("\tData: %s\n", data);
        printf("\tOre: ");
        if(ore == NULL){
            printf("Tutte\n");
        }
        else{
            int i;
            for(i = 0; i < dim[1]; i++){
                printf("%d ", ore[i]);
            }
            printf("\n");
        }
        printf("\tTavoli: ");
        if(tavoli == NULL){
            printf("Tutti\n");
        }
        else{
            int i;
            for(i = 0; i < dim[0]; i++){
                printf("%d ", tavoli[i]);
            }
            printf("\n");
        }
        printf("\tComande: ");
        if(comande == NULL){
            printf("Tutte\n");
        }
        else{
            int i;
            for(i = 0; i < dim[2]; i++){
                printf("%d ", comande[i]);
            }
            printf("\n");
        }
        printf("\tStati: ");
        if(stati == NULL){
            printf("Tutti\n");
        }
        else{
            int i;
            for(i = 0; i < dim[3]; i++){
                printf("%d ", stati[i]);
            }
            printf("\n");
        }
        printf("\tData di ristorazione: ");
        if(dataRistorazione == NULL){
            printf("Tutte\n");
        }
        else{
            printf("%s\n", dataRistorazione);
        }
    }

    // Recupero il file
    sprintf(filePath, "files/comande/com-%s.txt", data);    

    // Apro il file
    file = fopen(filePath, "r");

    // Se il file non esiste, non sono ancora state effettuate comande
    if(file == NULL){
        if(verbose)
            printf("Non sono ancora state effettuate comande in data %s\n", data);
        return NULL;
    }

    // VERBOSE
    if(verbose)
        printf("File aperto: %s\n", filePath);

    // Recupero ciascuna riga del file
    while (fgets(line, sizeof(line), file)) {

        int tavolo, orario, numero;
        comanda_state_t stato;
        char risData[DATE_LEN];

        // Il tavolo è il primo campo
        char *cell = strtok(line, "\t");
        if(tavoli != NULL && !isInArray(tavoli, dim[0], atoi(cell), NULL))
            continue;
        tavolo = atoi(cell);

        // L'ora è il secondo campo
        cell = strtok(NULL, "\t");
        if(ore != NULL && !isInArray(ore, dim[1], atoi(cell), NULL))
            continue;
        orario = atoi(cell);

        // Il numero di comanda è il terzo campo
        cell = strtok(NULL, "\t");
        if(comande != NULL && !isInArray(comande, dim[2], atoi(cell), NULL))
            continue;
        numero = atoi(cell);

        // Lo stato è il quarto campo
        cell = strtok(NULL, "\t");
        if(stati != NULL && !isInArray((int *)stati, dim[3], atoi(cell), NULL))
            continue;
        stato = atoi(cell);

        // La data di ristorazione è il quinto campo
        cell = strtok(NULL, "\t");
        if(dataRistorazione != NULL && strncmp(dataRistorazione, cell, 8) != 0)
            continue;
        strcpy(risData, cell);
        risData[DATE_LEN - 1] = '\0';

        // VERBOSE
        if(verbose)
            printf("Comanda recuperata: %s %d %d %d %d %s\n", data, tavolo, orario, numero, stato, risData);

        // Recupero la lista delle piatti ordinati nella comanda
        comanda_t *comanda = (comanda_t *) malloc(sizeof(comanda_t));
        memset(comanda, 0, sizeof(comanda_t));

        strcpy(comanda->data, data);
        comanda->tavolo = tavolo;
        comanda->ora = orario;
        comanda->numero = numero;
        comanda->stato = stato;
        strcpy(comanda->dataRis, risData);

        comanda->piatti = getComandaDishes(comanda, verbose);

        // Se la lista è vuota è errore
        if(comanda->piatti == NULL){
            printf("Errore nel recupero della lista dei piatti della comanda %d del tavolo %d delle %d\n", comanda->numero, comanda->tavolo, comanda->ora);
            free(comanda);
            continue;
        }

        // Aggiungo la comanda alla lista in coda
        sortedInsertComanda(&comandaList, comanda);

        // VERBOSE
        if(verbose)
            printf("Lista piatti recuperata\n");

    }

    fclose(file);
    return comandaList;

}


int setComanda(char data[], int tavolo, int ora, int comanda, comanda_state_t stato, char dataRistorazione[]){

    char filePath[PATH_MAX];
    
    FILE *file;

    // VERBOSE
    printf("Salvataggio della comanda...\n");

    // Recupero il path del file
    sprintf(filePath, "files/comande/com-%s.txt", data);

    // VERBOSE
    printf("File da aprire: %s\n", filePath);

    // Se la comanda è già stata salvata, occorre eliminarla prima di aggiornarne lo stato
    if(comanda != 0){

        int rows[] = {1, 2, 3};
        char *values[3];

        char tavoloStr[10];
        char oraStr[3];
        char comandaStr[10];

        sprintf(tavoloStr, "%d", tavolo);
        sprintf(oraStr, "%d", ora);
        sprintf(comandaStr, "%d", comanda);

        values[0] = tavoloStr;
        values[1] = oraStr;
        values[2] = comandaStr;

        if(deleteRecord(filePath, rows, values, 3) == -1){
            printf("Non è stata trovata alcuna comanda corrispondente\n");
            return -1;
        }

        // VERBOSE
        printf("Upload stato comanda in corso...\n");

    }

    // Se invece è una nuova comanda da inserire, recupero il nuovo numero di comanda e aggiorno il file delle statistiche
    else{
 
        if((comanda = getNextComandaNumber(filePath, tavolo, ora)) == -1){
            printf("Errore nel recupero del numero di comanda\n");
            return -1;
        }
        
    }

    if(dataRistorazione == NULL){
        getTodayDate(dataRistorazione);
    }

    // Salvo la nuova comanda
    file = fopen(filePath, "a"); // Apro il file in append
    if(file == NULL){
        return -1;
    }

    fprintf(file, "%d\t%d\t%d\t%d\t%s\n", 
        tavolo, 
        ora, 
        comanda, 
        stato,
        dataRistorazione
    );

    fclose(file);

    // VERBOSE
    printf("Comanda salvata:\n");
    printf("\tData: %s\n", data);
    printf("\tTavolo: %d\n", tavolo);
    printf("\tOra: %d\n", ora);
    printf("\tNumero: %d\n", comanda);
    printf("\tStato: %d\n", (int)stato);
    printf("\tData di ristorazione: %s\n", dataRistorazione);

    return comanda;

}


comanda_dish_t *getComandaDishes(comanda_t *comanda, int verbose){

    char filePath[PATH_MAX];

    FILE *file;
    char line[LINE_LEN];

    comanda_dish_t *dishList = NULL;

    // VERBOSE
    if(verbose)
        printf("Recupero dei piatti della comanda %d del tavolo %d delle %d...\n", comanda->numero, comanda->tavolo, comanda->ora);

    // Recupero la data della comanda
    sprintf(filePath, "files/dishes/dis-%s.txt", comanda->data);   

    // Apro il file
    file = fopen(filePath, "r");

    // Se il file non esiste, è un errore
    if(file == NULL){
        return NULL;
    }

    // VERBOSE
    if(verbose)
        printf("File aperto: %s\n", filePath);

    // Scannerizzo le righe del file
    while (fgets(line, sizeof(line), file)) {

        // Il numero di tavolo è il primo campo
        char *cell = strtok(line, "\t");
        if(atoi(cell) != comanda->tavolo)
            continue;

        // L'ora è il secondo campo
        cell = strtok(NULL, "\t");
        if(atoi(cell) != comanda->ora)
            continue;

        // Il numero della comanda è il terzo campo
        cell = strtok(NULL, "\t");
        if(atoi(cell) != comanda->numero)
            continue;
        
        // Se la comanda è stata trovata, recupero i piatti
        comanda_dish_t *dish = (comanda_dish_t *) malloc(sizeof(comanda_dish_t));
        memset(dish, 0, sizeof(comanda_dish_t));

        // Il codice del piatto è il quarto campo
        cell = strtok(NULL, "\t");
        strcpy(dish->codice, cell);

        // La quantità è il quinto campo
        cell = strtok(NULL, "\t");
        dish->quantita = atoi(cell);

        // VERBOSE
        if(verbose)
            printf("Piatto recuperato: %s %d\n", dish->codice, dish->quantita);

        // Inserisco il piatto nella lista ordinata
        sortedInsertDish(&dishList, dish);

    }

    fclose(file);
    return dishList;

}


int setDish(char data[], int tavolo, int ora, int comanda, char piatto[], int quantita){

    char filePath[PATH_MAX];
    
    FILE *file;

    // VERBOSE
    printf("Salvataggio del piatto...\n");

    // Recupero il path del file
    sprintf(filePath, "files/dishes/dis-%s.txt", data);

    // VERBOSE
    printf("File da aprire: %s\n", filePath);

    // Salvo il nuovo piatto
    file = fopen(filePath, "a"); // Apro il file in append
    if(file == NULL){
        return -1;
    }

    fprintf(file, "%d\t%d\t%d\t%s\t%d\n", 
        tavolo, 
        ora, 
        comanda, 
        piatto, 
        quantita
    );

    fclose(file);

    // VERBOSE 
    printf("Piatto salvato: %d %d %d %s %d\n", tavolo, ora, comanda, piatto, quantita);

    return 0;


}


int getFirstWaitingComanda(comanda_t *comanda, char data[], int ora, int tavolo){

    char filePath[PATH_MAX];

    FILE *file;
    char line[LINE_LEN];

    // VERBOSE
    printf("Recupero della prima comanda in attesa del tavolo %d...\n", tavolo);

    // Recupero il file
    sprintf(filePath, "files/comande/com-%s.txt", data); 

    // Inserisco la data nella comanda
    strcpy(comanda->data, data);

    // Apro il file
    file = fopen(filePath, "r");

    // Se il file non esiste, non sono ancora state effettuate comande
    if(file == NULL){
        printf("Errore nell'apertura del file %s\n", filePath);
        return -1;
    }

    // VERBOSE
    printf("File aperto: %s\n", filePath);

    // Recupero ciascuna riga del file
    while (fgets(line, sizeof(line), file)) {
        
        // Il tavolo è il primo campo
        char *cell = strtok(line, "\t");
        comanda->tavolo = atoi(cell);
        if(comanda->tavolo != tavolo)
            continue;

        // L'ora è il secondo campo
        cell = strtok(NULL, "\t");
        comanda->ora = atoi(cell);
        if(comanda->ora != ora)
            continue;

        // Il numero di comanda è il terzo campo
        cell = strtok(NULL, "\t");
        comanda->numero = atoi(cell);

        // Lo stato è il quarto campo
        cell = strtok(NULL, "\t");
        comanda->stato = atoi(cell);
        if(comanda->stato != ATTESA)
            continue;

        // La data di ristorazione è il quinto campo
        cell = strtok(NULL, "\t");
        strncpy(comanda->dataRis, cell, DATE_LEN - 1);
        comanda->dataRis[DATE_LEN - 1] = '\0';

        // Aggiungo in append, quindi la prima trovata è la più vecchia

        // VERBOSE
        printf("Comanda in attesa recuperata: %s %d %d %d %d\n", comanda->data, comanda->tavolo, comanda->ora, comanda->numero, comanda->stato);

        // Recupero la lista dei piatti ordinati nella comanda
        comanda->piatti = getComandaDishes(comanda, 1);

        // VERBOSE
        printf("Lista piatti recuperata\n");

        fclose(file);
        return 0;

    }

    fclose(file);

    // Se arrivo qui non ci sono comande in attesa
    printf("Errore: Non ci sono comande in attesa per il pasto in corso al tavolo %d\n", tavolo);
    deallocateComandeList(comanda);
    return -1;

}


int getNextComandaNumber(char filePath[], int tavolo, int ora){

    FILE *file;
    char line[LINE_LEN];
    int comanda = 1;
    int t, o, c, s;

    // VERBOSE
    printf("Recupero del numero di comanda...\n");

    // Apro il file
    file = fopen(filePath, "r");

    // Se il file non esiste, la comanda è la prima
    if(file == NULL){
        printf("File non trovato\n");
        return comanda;
    }

    // VERBOSE
    printf("File aperto: %s\n", filePath);

    // Leggo il file
    while(fgets(line, LINE_LEN, file) != NULL){
        sscanf(line, "%d\t%d\t%d\t%d", &t, &o, &c, &s);
        printf("Trovata comanda: T(%d) ORA(%d) %d %d\n", t, o, c, s);
        if(t == tavolo && o == ora){
            comanda++;
        }
    }

    // VERBOSE
    printf("Prossimo numero di comanda: %d\n", comanda);

    fclose(file);
    return comanda;

}


comanda_t *getDailyComanda(char dataRistorazione[], int verbose){

    // Si può ipotizzare che le comande effettuate in un giorno siano quelle
    // relative a prenotazioni della stessa data di ristorazione e/o al più
    // quelle del giorno precedente

    char filePath[PATH_MAX];
    sprintf(filePath, "files/comande/com-%s.txt", dataRistorazione);

    char dataPrecedente[DATE_LEN];
    memset(dataPrecedente, 0, DATE_LEN);

    // Recupero la data del giorno precedente
    strcpy(dataPrecedente, dataRistorazione);
    previousDay(dataPrecedente);

    // Recupero le comande dei file
    int dim[] = {0,0,0,0};    
    comanda_t *comandaList = getComanda(dataPrecedente, NULL, NULL, NULL, NULL, dataRistorazione, dim, verbose);
    comanda_t *secondComandaList = getComanda(dataRistorazione, NULL, NULL, NULL, NULL, dataRistorazione, dim, verbose);

    // Concateno le due liste
    comanda_t *temp = comandaList;
    while(temp != NULL && temp->next != NULL)
        temp = temp->next;
    if(temp == NULL)
        comandaList = secondComandaList;
    else
        temp->next = secondComandaList;
    
    return comandaList;

}


/*----------------------------- GESTIONE DEL MENU -----------------------------*/


int getMenu(menu_dish_t **menu, char date[]){

    char line[LINE_LEN];
    int i = 0;

    char dateFile[DATE_LEN] = "";

    DIR *directory;
    struct dirent *dirFile;

    FILE *file;
    char filePath[PATH_MAX];

    // VERBOSE
    printf("Recupero del menù...\n");

    // VERBOSE
    printf("Data: %s\n", date);

    // Apro la directory dei menù
    directory = opendir("files/menu");
    if (directory == NULL) {
        printf("Impossibile aprire la directory.");
        return -1;
    }

    // VERBOSE
    printf("Apro la directory dei menù...\n");

    // Leggo i file della directory
    while ((dirFile = readdir(directory)) != NULL) {
        
        // Tento di trovare il file "menu-YY-MM-DD.txt" più recente
        if( strncmp(dirFile->d_name, "menu-", 5) == 0 &&
            strncmp(dirFile->d_name + strlen("menu-YY-MM-DD"), ".txt", 4) == 0 &&
            strncmp(dirFile->d_name + 5, date, 8) <= 0 &&
            strncmp(dirFile->d_name + 5, dateFile, 8) > 0){
            
            strncpy(dateFile, dirFile->d_name + 5, DATE_LEN - 1);

        }
    }
    closedir(directory);

    // VERBOSE
    if(strcmp(dateFile, "") == 0){
        printf("Non è stato trovato un file di menù.\n");
        return -1;
    }
    printf("Trovato file di menù: %s\n", dateFile);

    // Apro il file
    sprintf(filePath, "files/menu/menu-%s.txt", dateFile);
    file = fopen(filePath, "r");
    if (file == NULL) {
        printf("Impossibile aprire il file.");
        return -1;
    }

    // VERBOSE
    printf("Apro il file di menù...\n");

    // Leggo il file
    while (fgets(line, sizeof(line), file)) {

        char *cella;

        i++;
        
        // Alloco la memoria per il nuovo piatto
        menu_dish_t *newDish = (menu_dish_t *) malloc(sizeof(menu_dish_t));
        memset(newDish, 0, sizeof(menu_dish_t));

        // Recupero il codice del piatto
        cella = strtok(line, "\t");
        strcpy(newDish->codice, cella);

        // Recupero il nome del piatto
        cella = strtok(NULL, "\t");
        strcpy(newDish->nome, cella);

        // Recupero il prezzo del piatto
        cella = strtok(NULL, "\t");
        newDish->prezzo = atoi(cella);

        // Aggiungo il piatto alla lista
        newDish->next = *menu;
        *menu = newDish;

    }

    fclose(file);
    printf("Menù recuperato\n");
    return i;

}


int isDishInMenu(dishCode_t codice, menu_dish_t *menu){
    
    menu_dish_t *dish = menu;
    while(dish != NULL){
        if(strcmp(dish->codice, codice) == 0){
            printf("Piatto disponibile: %s - %s\n", dish->codice, dish->nome);
            return 1;
        }
        dish = dish->next;
    }

    return -1;

}


/*----------------------------- GESTIONE DEL CONTO -----------------------------*/


subtotale_conto_t *getBill(char data[], int orario, int tavolo){
    
    subtotale_conto_t *conto = NULL;
    comanda_t *comandaList = NULL;

    // VERBOSE
    printf("Calcolo il conto del tavolo %d...\n", tavolo);
    printf("\tData: %s\n", data);
    printf("\tOrario: %d\n", orario);

    // Recupero le comande del tavolo (tralascio quelle in attesa)
    int oraList[] = {orario};
    int tavoloList[] = {tavolo};
    comanda_state_t stato[] = {PREPARAZIONE, SERVIZIO};
    int dim[] = {1, 1, 0, 2};
    comandaList = getComanda(data, oraList, tavoloList, NULL, stato, NULL, dim, 1);
    
    // VERBOSE
    printf("Comande recuperate:\n");
    printComandeList(comandaList, 2);

    // Se non ci sono comande, non c'è conto
    if(comandaList == NULL){
        printf("Non sono state richieste comande dal tavolo %d\n", tavolo);
        return NULL;
    }

    // VERBOSE
    printf("Calcolo il conto...\n");

    // Scannerizzo ogni piatto di ogni comanda recuperata
    while(comandaList != NULL){

        comanda_dish_t *dishList = comandaList->piatti;

        // Scannerizzo ogni piatto della comanda
        while(dishList != NULL){

            // Aggiungo il piatto al conto
            conto = addDishToBill(conto, dishList->codice, dishList->quantita);

            dishList = dishList->next;
        }

        comandaList = comandaList->next;
    }


    return conto;    

}


subtotale_conto_t *addDishToBill(subtotale_conto_t *conto, dishCode_t codice, int quantita){

    subtotale_conto_t *subtotale = conto;

    // VERBOSE
    printf("Aggiungo il piatto %s al conto...\n", codice);

    // Cerco il piatto nel conto
    while(subtotale != NULL){
        if(strcmp(subtotale->codice, codice) == 0){
            break;
        }
        subtotale = subtotale->next;
    }

    // Se il piatto non è nel conto, lo aggiungo
    if(subtotale == NULL){

        subtotale = (subtotale_conto_t *) malloc(sizeof(subtotale_conto_t));
        
        strcpy(subtotale->codice, codice);
        subtotale->quantita = 0;
        subtotale->prezzo = 0;

        subtotale->next = conto;
        conto = subtotale;
    }

    // Aggiorno la quantità
    subtotale->quantita += quantita;

    // VERBOSE
    printf("Quantità attuale: %d\n", subtotale->quantita);

    return conto;

}