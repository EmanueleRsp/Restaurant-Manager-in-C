#include "validate.h"


// Convalida il cognome
int convalidaCognome(char *token, char cognome[]){
    int i = 0;

    // Controllo che il token non sia NULL
    if(token == NULL){
        printf("Comando find incorretto, utilizzare: find <cognome> <numero_persone> <data> <ora>\n");
        *cognome = '\0';
        return 0;
    }

    // Controllo che il token non superi la lunghezza massima
    if(strlen(token) > COGNOME_LEN){
        printf("Errore: argomento <cognome> deve essere lungo al massimo %d caratteri\n", COGNOME_LEN);
        *cognome = '\0';
        return 0;
    }

    // Controllo che il token sia composto da sole lettere
    for(i = 0; i < strlen(token); i++){
        if(!isalpha(token[i])){
            printf("Errore: argomento <cognome> deve essere composto da sole lettere, senza spazi o caratteri speciali\n");
            *cognome = '\0';
            return 0;
        }
    }

    strcpy(cognome, token);
    cognome[COGNOME_LEN - 1] = '\0';

    return 1;
}


// Convalida il numero di persone
int convalidaNumeroPersone(char *token, int *numeroPersone){
    int i;

    // Controllo che il token non sia NULL
    if(token == NULL){
        printf("Comando find incorretto, utilizzare: find <cognome> <numero_persone> <data> <ora>\n");
        *numeroPersone = 0;
        return 0;
    }

    // Controllo che il token sia composto da sole cifre
    for(i = 0; i < strlen(token); i++){
        if(!isdigit(token[i])){
            printf("Errore: argomento <numero_persone> deve essere un numero\n");
            *numeroPersone = 0;
            return 0;
        }
    }
    *numeroPersone = atoi(token);

    // Controllo che il numero di persone sia compreso tra 1 e MAX_PEOPLE_PER_TABLE
    if(*numeroPersone < 1 || *numeroPersone > MAX_PEOPLE_PER_TABLE){
        printf("Errore: argomento <numero_persone> deve essere compreso tra 1 e %d\n", MAX_PEOPLE_PER_TABLE);
        *numeroPersone = 0;
        return 0;
    }

    return 1;
}


// Convalida la data in formato GG-MM-AA
int convalidaData(char *token, char data[]){
    char *subtoken;
    int i;
    int counter = 0;
    int giorno, mese, anno;
    int giorniPerMese[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

    // Controlla che il token non sia NULL
    if(token == NULL){
        printf("Comando find incorretto, utilizzare: find <cognome> <numero_persone> <data> <ora>\n");
        data[0] = '\0';
        return 0;
    }

    // Controlla il numero di caratteri '-' presenti  e che non siano presenti troppi caratteri
    for(i = 0; i < strlen(token); i++){
        if(token[i] == '-')
            counter++;
    }
    if(counter != 2 || strlen(token) > DATE_LEN - 1){
        printf("Errore: argomento <data> deve essere in formato GG-MM-AA\n");
        data[0] = '\0';
        return 0;
    }

    // Copio il token
    strncpy(data, token, DATE_LEN);
    data[DATE_LEN - 1] = '\0';

    // Controlla che sia un giorno valido
    subtoken = strtok(token, "-");
    if(subtoken == NULL){
        printf("Errore: argomento <data> deve essere in formato GG-MM-AA\n");
        data[0] = '\0';
        return 0;
    }
    if(strlen(subtoken) != 2 || atoi(subtoken) < 1 || atoi(subtoken) > 31){
        printf("Errore: inserire un giorno valido nel formato GG\n");
        data[0] = '\0';
        return 0;
    }
    giorno = atoi(subtoken);

    
    // Controlla che sia un mese valido
    subtoken = strtok(NULL, "-");
    if(subtoken == NULL){
        printf("Errore: argomento <data> deve essere in formato GG-MM-AA\n");
        data[0] = '\0';
        return 0;
    }
    if(strlen(subtoken) != 2 || atoi(subtoken) < 1 || atoi(subtoken) > 12){
        printf("Errore: inserire un mese valido nel formato MM\n");
        data[0] = '\0';
        return 0;
    }
    mese = atoi(subtoken);

    
    // Controlla che sia un anno valido
    subtoken = strtok(NULL, "-");
    if(subtoken == NULL){
        printf("Errore: argomento <data> deve essere in formato GG-MM-AA\n");
        data[0] = '\0';
        return 0;
    }
    if(strlen(subtoken) != 2 || atoi(subtoken) < 0 || atoi(subtoken) > 99){
        printf("Errore: inserire un anno valido nel formato AA\n");
        data[0] = '\0';
        return 0;
    }
    anno = atoi(subtoken);


    // Verificare che non ci siano altri subtoken
    if(strtok(NULL, " ") != NULL){
        printf("Errore: argomento <data> deve essere in formato GG-MM-AA\n");
        data[0] = '\0';
        return 0;
    }


    // Controllo per gli anni bisestili
    if(((anno % 4 == 0) && (anno % 100 != 0)) || (anno % 400 == 0)){
        giorniPerMese[1] = 29;
    }


    // Controllo che il giorno sia valido per il mese
    if(giorno > giorniPerMese[mese - 1]){
        printf("Errore: inserire un giorno valido per il mese inserito\n");
        data = '\0';
        return 0;
    }

    // Aggiungo uno spazio alla fine dell'ultimo subtoken per permettere eventuali controlli successivi
    subtoken[strlen(subtoken)] = ' ';
    
    return 1;
}


// Funzione che convalida l'ora
int convalidaOra(char *token, int *ora){
    int i;

    // Controllo che il token non sia NULL
    if(token == NULL){
        printf("Comando find incorretto, utilizzare: find <cognome> <numero_persone> <data> <ora>\n");
        *ora = 0;
        return 0;
    }

    // Controllo che il token sia composto da 2 cifre
    if(strlen(token) != 2){
        printf("Errore: argomento <ora> deve essere in formato HH\n");
        *ora = 0;
        return 0;
    }

    // Controllo che il token sia composto da sole cifre
    for(i = 0; i < strlen(token); i++){
        if(!isdigit(token[i])){
            printf("Errore: argomento <ora> deve essere un numero\n");
            *ora = 0;
            return 0;
        }
    }
    *ora = atoi(token);

    // Controllo che l'ora sia valida
    if(*ora < OPENING_TIME || *ora > CLOSING_TIME){
        printf("Errore: inserire un'ora valida\n");
        *ora = 0;
        return 0;
    }

    return 1;
}


// Funzione che verifica se l'orario non è passato
int convalidaOrarioPassato(int dd, int mm, int yy, int hh){

    int currentDay, currentMonth, currentYear, currentHour;

    // Ottengo l'orario attuale
    getCurrentDateTime(&currentHour, &currentDay, &currentMonth, &currentYear);

    // Controllo che l'orario non sia passato
    if(yy < currentYear ||
      (yy == currentYear && mm < currentMonth) || 
      (yy == currentYear && mm == currentMonth && dd < currentDay) || 
      (yy == currentYear && mm == currentMonth && dd == currentDay && hh < currentHour)){
        printf("Errore: inserire orario attuale o futuro\n");
        return -1;
    }

    return 0;

}


// Funzione che convalida l'opzione del comando book
int convalidaOpz(char *token, int *opz){
    int i = 0;

    // Controllo che il token non sia NULL
    if(token == NULL){
        printf("Comando book incorretto, utilizzare: book <opz>\n");
        *opz = 0;
        return 0;
    }

    // Controllo che il token sia composto da sole cifre
    for(i = 0; i < strlen(token); i++){
        if(!isdigit(token[i])){
            printf("Errore: argomento <opz> deve essere un numero intero positivo\n");
            *opz = 0;
            return 0;
        }
    }
    *opz = atoi(token);

    // Controllo che l'opzione sia un valore positivo
    if(*opz < 1){
        printf("Errore: argomento <opz> deve essere un valore positivo\n");
        *opz = 0;
        return 0;
    }

    return 1;
}


// Funzione che convalida la sintassi del codice di prenotazione
int convalidaSintassiCodice(char *input, char codice[]){

    if(strlen(input) != 6){
        printf("Errore: il codice di prenotazione deve essere composto da 6 caratteri\n");
        codice[0] = '\0';
        return -1;
    }

    // Verifico che i primi tre caratteri siano lettere maiuscole
    if(!isupper(input[0]) || !isupper(input[1]) || !isupper(input[2])){
        printf("Errore: i primi tre caratteri del codice di prenotazione devono essere lettere maiuscole\n");
        codice[0] = '\0';
        return -1;
    }

    // Verifico che gli ultimi tre caratteri siano numeri
    if(!isdigit(input[3]) || !isdigit(input[4]) || !isdigit(input[5])){
        printf("Errore: gli ultimi tre caratteri del codice di prenotazione devono essere numeri\n");
        codice[0] = '\0';
        return -1;
    }

    strcpy(codice, input);
    return 0;

}


// Convalida il numero del tavolo
int convalidaTableCommand(char *token, int *table){

    int isNumber = 1;
    int i;

    // Controllo che il numero del tavolo sia un numero
    for(i = 0; i < strlen(token); i++){
        
        if(!isdigit(token[i])){
            printf("L'argomento <numero_tavolo> deve essere un numero\n");
            isNumber = 0;
            break;
        }

    }

    if(!isNumber)
        return 0;

    // Il numero del tavolo deve essere maggiore di 0
    if(atoi(token) == 0){
        printf("L'argomento <numero_tavolo> deve essere maggiore di 0 (sintassi: stat T<numero_tavolo>)\n");
        return 0;
    }

    *table = atoi(token);
    return 1;

}

// Convalida l'input della comanda
int convalidaComanda(char *input, comanda_dish_t **list){

    char *token;

    if(input == NULL || strlen(input) == 0){
        printf("Errore: inserire almeno un piatto, per maggiori info digita \"help\"\n");
        return -1;
    }
    token = strtok(input, "-");

    // Verifico che ogni piatto sia valido
    do{

        dishCode_t code;
        int quantity;
        int found = 0;

        // Controllo che il codice del piatto sia valido
        if(validateDishCode(token, code) == -1){
            deallocateComandaDishList(*list);
            return -1;
        }

        token = strtok(NULL, " ");

        // Controllo che la quantità sia un numero tra 1 e MAX_DISHES_PER_ORDER
        if(validateDishQuantity(token, &quantity, code) == -1){
            deallocateComandaDishList(*list);
            return -1;
        }

        // Se il piatto è già stato inserito aggiungo la quantità
        comanda_dish_t *tmp = *list;
        while(tmp != NULL){
            if(strcmp(tmp->codice, code) == 0){
                tmp->quantita += quantity;
                if(tmp->quantita > MAX_DISHES_PER_ORDER){
                    printf("Errore: non è possibile ordinare più di %d piatti per comanda\n", MAX_DISHES_PER_ORDER);
                    deallocateComandaDishList(*list);
                    return -1;
                }
                found = 1;
                break;
            }
            tmp = tmp->next;
        }
        if(found)
            continue;

        // Creo il nuovo elemento se non è già stato inserito
        comanda_dish_t *newDish = (comanda_dish_t *)malloc(sizeof(comanda_dish_t));
        strcpy(newDish->codice, code);
        newDish->quantita = quantity;

        // Lo inserisco nella lista ordinata
        sortedInsertDish(list, newDish);


    }while((token = strtok(NULL, "-")) != NULL);

    return 0;


}

// Funzione che convalida il codice di un piatto
int validateDishCode(char *input, dishCode_t code){

    int i;
    int index;

    if(input == NULL || strlen(input) == 0){
        printf("Errore: sintassi della comanda errata, per maggiori info digita \"help\".\n");
        return -1;
    }

    // Controllo che il codice non sia troppo lungo
    if(strlen(input) > DISH_CODE_LEN - 1){
        printf("Errore: codice del piatto errato, per maggiori info digita \"help\".\n");
        return -1;
    }

    // Il primo carattere indica la categoria del piatto
    if(input[0] != 'A' && input[0] != 'P' && input[0] != 'S' && input[0] != 'D'){
        printf("Errore: categoria del piatto errata, per maggiori info digita \"help\".\n");
        return -1;
    }

    // Controllo che i restanti caratteri siano numeri
    for(i = 1; i < strlen(input); i++){
        if(!isdigit(input[i])){
            printf("Errore: indice del piatto errato, per maggiori info digita \"help\".\n");
            return -1;
        }
    }
    index = atoi(input + 1);

    // Controllo che l'indice del piatto sia valido
    if(index < 1 || index > MAX_CATEGORY_INDEX){
        printf("Errore: indice del piatto errato, per maggiori info digita \"help\".\n");
        return -1;
    }


    strcpy(code, input);
    return 0;

}

// Funzione che convalida la quantità di un piatto
int validateDishQuantity(char *input, int *quantity, dishCode_t code){

    int i;

    if(input == NULL){
        printf("Errore: quantità del piatto %s non specificata, per maggiori info digita \"help\".\n", code);
        return -1;
    }

    // Controllo che la quantità sia un numero
    for(i = 0; i < strlen(input); i++){
        if(!isdigit(input[i])){
            printf("Errore: quantità del piatto %s non è un numero, per maggiori info digita \"help\".\n", code);
            return -1;
        }
    }

    *quantity = atoi(input);

    // Controllo che la quantità sia un numero tra 1 e MAX_DISHES_PER_ORDER
    if(*quantity < 1 || *quantity > MAX_DISHES_PER_ORDER){
        printf("Errore: quantità del piatto %s errata, per maggiori info digita \"help\".\n", code);
        return -1;
    }

    return 0;

}

// Funzione che convalida la comanda per l'update
int convalidaComandaUpdate(char *input, int *tavolo, int *comanda){

    char *token;

    // Controllo che l'input non sia NULL
    if(input == NULL){
        printf("Errore: inserire un input valido\n");
        return -1;
    }

    // Il primo token deve essere 'com<numero_comanda>'
    token = strtok(input, "-");
    if(token == NULL || strncmp(token, "com", 3) != 0){
        printf("Errore sintassi: ready com<numero_comanda>-T<numero_tavolo> \n");
        return -1;
    }
    if (atoi(token + 3) == 0){
        printf("Errore sintassi: ready com<numero_comanda>-T<numero_tavolo> \n");
        return -1;
    }
    *comanda = atoi(token + 3);
    
    // Il secondo token deve essere 'T<numero_tavolo>'
    token = strtok(NULL, "");
    if(token == NULL || strncmp(token, "T", 1) != 0){
        printf("Errore sintassi: ready com<numero_comanda>-T<numero_tavolo> \n");
        return -1;
    }
    if (atoi(token + 1) == 0){
        printf("Errore sintassi: ready com<numero_comanda>-T<numero_tavolo> \n");
        return -1;
    }
    *tavolo = atoi(token + 1);

    // Il numero di tavolo deve essere un valore positivo
    if(*tavolo < 1){
        printf("Errore: argomento <numero_tavolo> deve essere un valore positivo\n");
        return -1;
    }
    if(*comanda < 1 || *comanda > MAX_COMANDE_PER_TABLE){
        printf("Errore: è stato superato il limite di comande inviabili per tavolo (%d)\n", MAX_COMANDE_PER_TABLE);
        return -1;
    }
    
    return 0;


}