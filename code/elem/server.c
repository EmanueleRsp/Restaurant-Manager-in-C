#include "../lib/server/serverLib.h"

int main(int argc, char *argv[]){

    // Gestione del segnale SIGINT
    // signal(SIGINT, SIGN_IGN);

    /*---------------------------- VARIABILI ----------------------------*/

    // Dichiarazione costanti per la gestione del socket del server
    const int DOMAIN = AF_INET;
    const int TYPE = SOCK_STREAM;
    const in_addr_t IP = INADDR_ANY;
    
    // Dichiarazione variabili per la gestione del socket del server
    int PORT;
    int serverSocket;
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    // Set di file descriptor per la gestione delle richieste dei client
    fd_set master;
    fd_set readSet;
    int fdmax;

    // Variabili per la gestione delle richieste dei client
    int clientSocket;
    struct sockaddr_in clientAddress;
    memset(&clientAddress, 0, sizeof(clientAddress));
    socklen_t clientAddressLength = sizeof(clientAddress);

    // Buffer per gestire i comandi da tastiera
    char command[COMMAND_LEN];

    /*---------------------------------------------------------------*/


    // Inizializzazione della porta del server
    portValidation(argc, argv, &PORT);

    // Creazione del socket
    serverSocket = socket(DOMAIN, TYPE, 0);
    if(serverSocket == -1){
        perror("Errore nella creazione del socket");
        exit(1);
    }
        
    // Inizializzazione indirizzo e porta del server
    serverAddress.sin_family = DOMAIN;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = IP;

    // Binding del socket
    if(bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1){
        perror("Errore nel binding del socket");
        exit(1);
    }

    // Listening del socket
    if(listen(serverSocket, QUEUE_LEN) == -1){
        perror("Errore nel listening del socket");
        exit(1);
    }

    // Inizializzazione dei set di file descriptor
    FD_ZERO(&master);
    FD_ZERO(&readSet);

    // Aggiunta del socket del server e dello stdin al set di file descriptor
    FD_SET(serverSocket, &master);
    FD_SET(STDIN_FILENO, &master);
    fdmax = serverSocket;

    // Inizializzazione del generatore di numeri casuali
    srand(time(NULL));

    // Schermata di benvenuto
    start();

    // Gestione dei file descriptor mediante I/O multiplexing
    while(1){

        printCommands();
        
        readSet = master;
    
        // Effettuo la select
        if(select(fdmax + 1, &readSet, NULL, NULL, NULL) == -1){
            perror("Errore nella select");
            exit(1);
        }
    
        int i;
        for(i = 0; i <= fdmax; i++){
    
            if(FD_ISSET(i, &readSet)){
                
                /*---------------- GESTIONE STANDARD INPUT ----------------*/
                if(i == 0){

                    // Leggo il comando da tastiera
                    fgets(command, COMMAND_LEN, stdin);
                    command[strlen(command) - 1] = '\0';

                    // Comando "stop"
                    if(strcmp(command, "stop") == 0){

                        // Si tenta di chiudere il server
                        closeDevice(&master, serverSocket);
                        continue;

                    }

                    // Comando "stat" senza parametri
                    if(strcmp(command, "stat") == 0){
                        stat();
                    }

                    // Comando "stat" con parametri
                    else if(strncmp(command, "stat", 4) == 0){

                        char *token;

                        strtok(command, " ");
                        token = strtok(NULL, " ");

                        // Comando "stat" senza parametri
                        if(token == NULL){
                            printf("Comando non valido\n");
                            continue;
                        }

                        // Comando "stat status"
                        else if(strcmp(token, "a") == 0 || strcmp(token, "s") == 0 || strcmp(token, "p") == 0){
                            
                            comanda_state_t state;

                            if(strtok(NULL, " ") != NULL){
                                printf("Troppi argomenti, utilizzare: stat <a|s|p>\n");
                                continue;
                            }

                            // Converto il carattere 
                            if(strcmp(token, "a") == 0){
                                state = ATTESA;
                            }
                            else if(strcmp(token, "s") == 0){
                                state = SERVIZIO;
                            }
                            else{
                                state = PREPARAZIONE;
                            }
                            
                            statStatus(state);
                        }

                        // Comando "stat table"
                        else if(strncmp(token, "T", 1) == 0){
                            
                            int table;

                            if(convalidaTableCommand(token + 1, &table) == 0){
                                continue;
                            }

                            // Non devono essere presenti altri argomenti
                            if(strtok(NULL, " ") != NULL){
                                printf("Troppi argomenti, utilizzare: stat T<numero_tavolo>\n");
                                return 0;
                            }

                            statTable(table);
                        }

                        else{
                            printf("Numero e/o tipo di argomenti incorretto per il comando stat\n");
                        }

                    }

                    // Comando non riconosciuto
                    else{
                        printf("Comando non riconosciuto\n");
                    }


                }

                /*---------------- GESTIONE NUOVE CONNESSIONI ----------------*/
                else if(i == serverSocket){

                    printf("\n\n");

                    // VERBOSE
                    printf("Nuova connessione in ingresso\n");
    
                    // Gestione della richiesta di connessione di un nuovo client    
                    clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &clientAddressLength);
                    if(clientSocket == -1){
                        perror("Errore nell'accettazione della connessione");
                        continue;
                    }

                    // VERBOSE
                    printf("Nuova connessione accettata\n");
                    
                    // Eseguo la recv per identificare il tipo di dispositivo in connessione
                    char buffer[2];
                    memset(buffer, 0, sizeof(buffer));
                    if(recv(clientSocket, (void *)buffer, BUFFER_LEN, 0) == -1){
                        perror("Errore nell'identificazione del dispositivo");
                        close(clientSocket);
                        continue;
                    }

                    // VERBOSE
                    printf("Identificazione del dispositivo (%s) in corso...\n", buffer);

                    // Identifico il dispositivo
                    device_t dev;
                    if(strcmp(buffer, "C") == 0){
                        dev = CLIENT;
                    } else if(strcmp(buffer, "K") == 0){
                        dev = KITCHEN;
                    } else if(strcmp(buffer, "T") == 0){
                        dev = TABLE;
                    } else{
                        printf("Dispositivo non riconosciuto\n");
                        close(clientSocket);
                        continue;
                    }

                    // VERBOSE
                    printf("Dispositivo identificato: %d\n", dev);

                    // Se è un kitchen device, devo inviare in risposta il numero di comande in attesa
                    if(dev == KITCHEN){
                        
                        // Recupero il numero di comande in attesa
                        int numComande;
                        if((numComande = getWaitingComandeNum()) == -1){
                            perror("Errore nel recupero del numero di comande in attesa");
                            close(clientSocket);
                            continue;
                        }
                        printf("Numero di comande in attesa: %d\n", numComande);

                        // Bufferizzazione del numero di comande
                        memset(buffer, 0, sizeof(buffer));
                        sprintf(buffer, "%d", numComande);

                        // Invio del numero di comande in attesa
                        if(send(clientSocket, (void *)buffer, BUFFER_LEN, 0) == -1){
                            perror("Errore nell'invio del numero di comande in attesa");
                            close(clientSocket);
                            continue;
                        }
                    }

                    // Creo un nuovo cliente
                    connection_t *c = (connection_t *)malloc(sizeof(connection_t));
                    memset(c, 0, sizeof(*c));

                    c->socket = clientSocket;
                    c->dev = dev;
                    c->next = NULL;
                    c->address = clientAddress;
                    c->addressLength = clientAddressLength;

                    // VERBOSE stampa tutti i campi della struct connection_t
                    printf("Nuova connessione:\n");
                    printf("\tsocket: %d\n", c->socket);
                    printf("\tdev: %d\n", c->dev);
                    printf("\taddress: %s\n", inet_ntoa(c->address.sin_addr));
                    printf("\taddressLength: %d\n", c->addressLength);

                    // Aggiungo il cliente alla lista dei clienti
                    addConnection(c);

                    // VERBOSE
                    printf("Nuova connessione salvata\n");

                    // Aggiunta del socket del client al set di file descriptor
                    FD_SET(clientSocket, &master);
                    if(clientSocket > fdmax){
                        fdmax = clientSocket;
                    }

                    // VERBOSE
                    printf("Nuovo socket inserito per %s\n", inet_ntoa(clientAddress.sin_addr));
    
                }
                
                /*-------------------- GESTIONE RICHIESTE DEI DEVICE --------------------*/
                else {

                    printf("\n\n");
                    
                    // Variabili per la ricezione della richiesta
                    char buffer[BUFFER_LEN];
                    memset(buffer, 0, sizeof(buffer));
                    device_t dev;
                    ssize_t bytes;

                    // Recupero il tipo di client che ha effettuato la richiesta
                    dev = getDevice(i);
                    if(dev == NOTID){
                        perror("Errore nella ricezione della richiesta");
                        close(i);
                        FD_CLR(i, &master);
                        continue;
                    }

                    // VERBOSE
                    printf("Richiesta ricevuta dal socket %d (tipo %d)\n", i, dev);

                    // Ricevo il messaggio
                    if((bytes = recv(i, (void *)buffer, BUFFER_LEN, 0)) == -1){
                        perror("Errore nella ricezione della richiesta");
                        removeConnection(i);
                        close(i);
                        FD_CLR(i, &master);
                        continue;
                    }

                    // Se il client ha chiuso la connessione
                    if(bytes == 0){
                        printf("Il client ha chiuso la connessione\n");
                        removeConnection(i);
                        close(i);
                        FD_CLR(i, &master);
                        continue;
                    }

                    // Deserializzo la richiesta
                    request_t req;
                    req.op = atoi(strtok(buffer, "\n"));
                    strncpy(req.serializedBody, strtok(NULL, ""), BUFFER_LEN - sizeof(operazione_t) - 2);
                    req.serializedBody[BUFFER_LEN - sizeof(operazione_t) - 2] = '\0';

                    // VERBOSE
                    printf("Tipo di operazione richiesta: %d\n", req.op);

                    // Identificazione della richiesta
                    switch (dev){
                        case CLIENT:
                            switch (req.op + dev){
                                case FIND:
                                    findManager(i, req.serializedBody);
                                    break;
                                case BOOK:
                                    bookManager(i, req.serializedBody);
                                    break;
                                default:
                                    reqErrorManager(i, "Errore nel tipo di richiesta");
                                    break;
                            }
                            break;
                        
                        case KITCHEN:
                            switch (req.op + dev){
                                case TAKE:
                                    takeManager(i);
                                    break;
                                case READY:
                                    readyManager(i, req.serializedBody);
                                    break;
                                default:
                                    reqErrorManager(i, "Errore nel tipo di richiesta");
                                    break;
                            }
                            break;
                        
                        case TABLE:
                            switch (req.op + dev){                                
                                case UNLOCK:
                                    unlockManager(i, req.serializedBody);
                                    break;
                                case MENU:
                                    menuManager(i);
                                    break;
                                case COMANDA:
                                    comandaManager(i, req.serializedBody);
                                    break;
                                case CONTO:
                                    contoManager(i);
                                    break;
                                default:
                                    reqErrorManager(i, "Errore nel tipo di richiesta");
                                    break;
                            }
                            break;
                        
                        default:
                            reqErrorManager(i, "Errore nel tipo di device");
                            break;
                    }
    
                }
    
            }

        }
    
    }   

}