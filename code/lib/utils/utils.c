#include "utils.h"


// UTILITA' -----------------------------------

void portValidation(int argc, char *argv[], int *PORT){
    
    if(argc != 2){
        printf("Utilizza: %s <port>\n", argv[0]);
        exit(1);
    }

    if(atoi(argv[1]) < 1024 || atoi(argv[1]) > 65535){
        printf("La porta deve essere tra 1024 e 65535\n");
        exit(1);
    }

    *PORT = atoi(argv[1]);
    return;

}


void getTodayDate(char date[]){

    time_t now;
    struct tm *local_time;

    time(&now);
    local_time = localtime(&now);

    // Formattazione della data in YY-MM-DD
    strftime(date, DATE_LEN, "%y-%m-%d", local_time);
    date[DATE_LEN - 1] = '\0';

    return;

}


void getCurrentDateTime(int *hh, int *dd, int *mm, int *yy){

    time_t now;
    struct tm *local_time;

    time(&now);
    local_time = localtime(&now);

    if(hh != NULL)
        *hh = local_time->tm_hour;
    if(dd != NULL)
        *dd = local_time->tm_mday;
    if(mm != NULL)    
        *mm = local_time->tm_mon + 1;               // Gennaio è 0
    if(yy != NULL)    
        *yy = local_time->tm_year + 1900 - 2000;    // 1900 è l'anno di partenza, 2000 è l'anno di partenza per il formato YY

    return;

}

void getTimestamp(char timestamp[]){

    time_t now;
    struct tm *local_time;

    time(&now);
    local_time = localtime(&now);

    // Formattazione della data in YYMMDDHHMMSS
    strftime(timestamp, TIMESTAMP_LEN, "%Y-%m-%d %H:%M:%S", local_time);

    return;

}


void scomposeData(char data[], int *dd, int *mm, int *yy){

    char tmp[3];

    // Giorno
    strncpy(tmp, data, 2);
    tmp[2] = '\0';
    if (dd != NULL)
        *dd = atoi(tmp);

    // Mese
    strncpy(tmp, data + 3, 2);
    tmp[2] = '\0';
    if(mm != NULL)
        *mm = atoi(tmp);

    // Anno
    strncpy(tmp, data + 6, 2);
    tmp[2] = '\0';
    if(yy != NULL)
        *yy = atoi(tmp);

    return;

}


int isCodePrev(dishCode_t primo, dishCode_t secondo){

    char ordine[] = {'A', 'P', 'S', 'D'};
    int i = 0;

    if(primo[0] == secondo[0]){
        if(strcmp(primo, secondo) < 0)
            return 1;
        else
            return 0;
    }

    while(ordine[i] != primo[0])
        i++;
    
    while(i < 4){
        if(ordine[i] == secondo[0]){
            return 1;
        }
        i++;
    }

    return 0;

}


int isComandaPrev(comanda_t *prima, comanda_t *seconda){

    if(prima->tavolo < seconda->tavolo)
        return 1;
    else if(prima->tavolo > seconda->tavolo)
        return 0;
    
    if(strcmp(prima->data, seconda->data) < 0)
        return 1;
    else if(strcmp(prima->data, seconda->data) > 0)
        return 0;

    if(prima->ora < seconda->ora)
        return 1;
    else if(prima->ora > seconda->ora)
        return 0;
    
    if(prima->numero < seconda->numero)
        return 1;
    else
        return 0;

}


void previousDay(char data[]){

    int yy, mm, dd;
    sscanf(data, "%d-%d-%d", &yy, &mm, &dd);

    // Creo la struttura tm
    struct tm tm_data = {0};
    tm_data.tm_year = yy - 1900;    // 1900 è l'anno di partenza
    tm_data.tm_mon = mm - 1;        // Gennaio è 0
    tm_data.tm_mday = dd;           // Giorno del mese
    
    // Converto la data in epoch time
    time_t epoch_time = mktime(&tm_data);
    epoch_time -= 86400;                    // Sottraggo un giorno (86400 secondi)
    tm_data = *localtime(&epoch_time);      // Converto l'epoch time nella nuova data

    // Formatto la data nel formato "YY-MM-DD"
    strftime(data, 9, "%y-%m-%d", &tm_data);

    return;

}


// GESTIONE DELLE STRUTTURE (TAVOLI) -----------------------------------


void deallocateTableList(table_t *list){

    table_t *tmp;
    while(list != NULL){
        tmp = list;
        list = list->next;
        free(tmp);
    }

}


void serializeTableList(table_t *list, char *buffer){

    table_t *tmp = list;
    int offset = 0;

    while(tmp != NULL){
        offset += sprintf(buffer + offset, "%d\t%d\t%s\n",
            tmp->codice,
            tmp->sala,
            tmp->info    
        );
        tmp = tmp->next;
    }

    return;
}


void deserializeTableList(table_t **list, char *buffer){

    char *line = strtok(buffer, "\n");
    while(line != NULL){

        table_t *newTable = (table_t *) malloc(sizeof(table_t));
        memset(newTable, 0, sizeof(table_t));

        sscanf(line, "%d\t%d\t%[^\n]", &newTable->codice, &newTable->sala, newTable->info);

        newTable->next = *list;
        *list = newTable;

        line = strtok(NULL, "\n");
    }

    return;
}


// GESTIONE DELLE STRUTTURE (PIATTI) -----------------------------------


void deallocateMenu(menu_dish_t *list){

    menu_dish_t *tmp;
    while(list != NULL){
        tmp = list;
        list = list->next;
        free(tmp);
    }

}


void serializeMenu(menu_dish_t *list, char *buffer){

    menu_dish_t *tmp = list;
    int offset = 0;

    while(tmp != NULL){
        offset += sprintf(buffer + offset, "%s\t%s\t%d\n",
            tmp->codice,
            tmp->nome,
            tmp->prezzo  
        );
        tmp = tmp->next;
    }

    return;
}


void deserializeMenu(menu_dish_t **list, char *buffer){

    char *line = strtok(buffer, "\n");
    while(line != NULL){

        menu_dish_t *newTable = (menu_dish_t *) malloc(sizeof(menu_dish_t));
        memset(newTable, 0, sizeof(menu_dish_t));

        sscanf(line, "%s\t%[^\t]\t%d",
            newTable->codice,
            newTable->nome,
            &newTable->prezzo
        );

        newTable->next = *list;
        *list = newTable;

        line = strtok(NULL, "\n");
    }

    return;
}


void printMenu(menu_dish_t *list){
    
    menu_dish_t *tmp = list;
    int maxlen = 0;

    while(tmp != NULL){
        int nomeLen = strlen(tmp->nome);
        if(nomeLen > maxlen)
            maxlen = nomeLen;
        tmp = tmp->next;
    }

    tmp = list;
    while(tmp != NULL){
            
        int nomeLen = strlen(tmp->nome);
            
        printf("%s - %s", tmp->codice, tmp->nome);
        while(nomeLen < maxlen + 2){
            printf(" ");
            nomeLen++;
        }
        printf("\t%d\n", tmp->prezzo);

        tmp = tmp->next;
    }

    return;
    
}


// GESTIONE DELLE STRUTTURE (COMANDE) -----------------------------------


void serializeComandaDishList(comanda_dish_t *list, char *buffer){

    comanda_dish_t *tmp = list;
    int offset = 0;

    while(tmp != NULL){
        offset += sprintf(buffer + offset, "%s\t%d\n",
            tmp->codice,
            tmp->quantita
        );
        tmp = tmp->next;
    }

    return;

}


void serializeComandeList(comanda_t *list, char *buffer){

    comanda_t *tmp = list;
    int offset = 0;

    while(tmp != NULL){
        offset += sprintf(buffer + offset, "%s\t%d\t%d\t%d\t%d\n",
            tmp->data,
            tmp->ora,
            tmp->tavolo,
            tmp->numero,
            tmp->stato
        );
        serializeComandaDishList(tmp->piatti, buffer + offset);
        offset += strlen(buffer + offset);
        offset += sprintf(buffer + offset, "\n");
        tmp = tmp->next;
    }

    return;

}


void deserializeComandaDishList(comanda_dish_t **list, char *buffer){

    char *line = strtok(buffer, "\n");
    while(line != NULL){

        comanda_dish_t *newDish = (comanda_dish_t *) malloc(sizeof(comanda_dish_t));
        memset(newDish, 0, sizeof(comanda_dish_t));

        sscanf(line, "%s\t%d\n",
            newDish->codice,
            &newDish->quantita
        );

        // Inserimento ordinato
        sortedInsertDish(list, newDish);

        line = strtok(NULL, "\n");
    }

    return;

}


void deserializeComandeList(comanda_t **list, char *buffer){

    char *line = strtok(buffer, "\n");
    while(line != NULL){

        comanda_t *newComanda = (comanda_t *) malloc(sizeof(comanda_t));
        memset(newComanda, 0, sizeof(comanda_t));

        sscanf(line, "%s\t%d\t%d\t%d\t%d\n",
            newComanda->data,
            &newComanda->ora,
            &newComanda->tavolo,
            &newComanda->numero,
            (int *)&newComanda->stato
        );

        deserializeComandaDishList(&newComanda->piatti, strtok(NULL, ""));

        // Inserimento ordinato
        sortedInsertComanda(list, newComanda);

        line = strtok(NULL, "\n");
    }

    return;

}


void deallocateComandeList(comanda_t *comandeList){
    
    comanda_t *comanda = comandeList;

    while(comanda != NULL){
        
        // Libero la memoria allocata per la lista dei piatti
        deallocateComandaDishList(comanda->piatti);

        // Dealloco la comanda
        free(comanda);

        comanda = comanda->next;
    }

    return;
}


void deallocateComandaDishList(comanda_dish_t *list){

    comanda_dish_t *tmp;
    while(list != NULL){
        tmp = list;
        list = list->next;
        free(tmp);
    }
    
    return;
}


void printComandeList(comanda_t *comandeList, int mode){
    
    const char *stato[] = {"in attesa", "in preparazione", "in servizio"};
    comanda_t *comanda = comandeList;

    while(comanda != NULL){
        
        // Stampo info sulla comanda
        printf("com%d", comanda->numero);
        if(mode == 0 || mode == 1 || mode == 4){
            printf(" T%d", comanda->tavolo);
        }
        if(mode == 4)
            printf(" %02d:00", comanda->ora);
        if(mode == 0 || mode == 2 || mode == 4){
            printf(" <%s>", stato[comanda->stato]);
        }

        printf("\n");

        // Stampo i piatti associati
        printComandaDishList(comanda->piatti);

        comanda = comanda->next;
    }

    return;    
}


void printComandaDishList(comanda_dish_t *dishList){

    comanda_dish_t *dish = dishList;

    while(dish != NULL){
        printf("%s %d\n", dish->codice, dish->quantita);
        dish = dish->next;
    }

}

void sortedInsertComanda(comanda_t **comandaList, comanda_t *comanda){

    comanda_t *current;

    // Se la lista è vuota o la comanda va inserita in testa
    if(*comandaList == NULL || isComandaPrev(comanda, *comandaList)){
        comanda->next = *comandaList;
        *comandaList = comanda;
    }

    // Scorro la lista fino a trovare il posto giusto
    else{
        current = *comandaList;
        while(current->next != NULL && !isComandaPrev(comanda, current->next)){
            current = current->next;
        }
        comanda->next = current->next;
        current->next = comanda;
    }

    return;

}

void sortedInsertDish(comanda_dish_t **dishList, comanda_dish_t *dish){

    // Se la lista è vuota o il piatto va inserito in testa
    if(*dishList == NULL || isCodePrev(dish->codice, (*dishList)->codice)){
        dish->next = *dishList;
        *dishList = dish;
    }

    // Scorro la lista fino a trovare il posto giusto
    else{
        comanda_dish_t *current = *dishList;
        while(current->next != NULL && !isCodePrev(dish->codice, current->next->codice)){
            current = current->next;
        }
        dish->next = current->next;
        current->next = dish;
    }

    return;

}


// GESTIONE DELLE LISTE (CONTO) -----------------------------------


void serializeConto(subtotale_conto_t *list, char *buffer){

    subtotale_conto_t *tmp = list;
    int offset = 0;

    while(tmp != NULL){
        offset += sprintf(buffer + offset, "%s\t%d\t%d\n",
            tmp->codice,
            tmp->quantita,
            tmp->prezzo
        );
        tmp = tmp->next;
    }

    return;

}


void deserializeConto(subtotale_conto_t **list, char *buffer){

    char *line = strtok(buffer, "\n");
    while(line != NULL){

        subtotale_conto_t *newDish = (subtotale_conto_t *) malloc(sizeof(subtotale_conto_t));
        memset(newDish, 0, sizeof(subtotale_conto_t));

        sscanf(line, "%s\t%d\t%d\n",
            newDish->codice,
            &newDish->quantita,
            &newDish->prezzo
        );

        // Inserimento ordinato
        sortedInsertConto(list, newDish);

        line = strtok(NULL, "\n");
    }

    return;

}


void deallocateConto(subtotale_conto_t *list){

    subtotale_conto_t *tmp;
    while(list != NULL){
        tmp = list;
        list = list->next;
        free(tmp);
    }

}


void printConto(subtotale_conto_t *list){

    subtotale_conto_t *tmp = list;
    int totale = 0;

    while(tmp != NULL){
        printf("%s %d %d\n", tmp->codice, tmp->quantita, tmp->prezzo);
        totale += tmp->prezzo;
        tmp = tmp->next;
    }
    printf("Totale: %d\n", totale);

    return;

}


void sortedInsertConto(subtotale_conto_t **dishList, subtotale_conto_t *dish){

    // Se la lista è vuota o il piatto va inserito in testa
    if(*dishList == NULL || isCodePrev(dish->codice, (*dishList)->codice)){
        dish->next = *dishList;
        *dishList = dish;
    }

    // Scorro la lista fino a trovare il posto giusto
    else{
        subtotale_conto_t *current = *dishList;
        while(current->next != NULL && !isCodePrev(dish->codice, current->next->codice)){
            current = current->next;
        }
        dish->next = current->next;
        current->next = dish;
    }

    return;


}
