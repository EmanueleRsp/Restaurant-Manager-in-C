#include "../lib/table/tableLib.h"


int main(int argc, char *argv[]){

    // Gestione del segnale SIGINT
    // signal(SIGINT, SIG_IGN);

    // Dichiarazione costanti per la gestione del socket del server
    const int DOMAIN = AF_INET;
    const int TYPE = SOCK_STREAM;
    const in_addr_t IP = INADDR_ANY;
    
    // Dichiarazione variabili per la gestione del socket del server
    int PORT;
    int serverSocket;
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    // Dichiarazione set di socket
    fd_set master;
    fd_set read_fds;
    int fdmax;

    //  Buffer e variabili per la gestione dei comandi da tastiera
    char command[COMMAND_LEN];
    int unlocked = 0;

    bookingCode_t code;

    /*---------------------------------------------------------------*/


    // Inizializazione della porta del server
    portValidation(argc, argv, &PORT);


    // Creazione del socket del server
    serverSocket = socket(DOMAIN, TYPE, 0);
    if(serverSocket == -1){
        perror("Errore nella creazione del socket del server");
        exit(EXIT_FAILURE);
    }

    // Inizializzazione dell'indirizzo del server
    serverAddress.sin_family = DOMAIN;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = IP;

    // Connessione al server
    if(connect(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1){
        perror("Errore nella connessione al server");
        exit(EXIT_FAILURE);
    }

    // Identificazione del device al server
    if(send(serverSocket, "T", 2, 0) == -1){
        perror("Errore nell'invio dell'identificazione del device");
        exit(EXIT_FAILURE);
    }

    // Reset dei set di socket
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    // Aggiunta del socket del server e dello standard input al set di socket
    FD_SET(serverSocket, &master);
    FD_SET(STDIN_FILENO, &master);
    fdmax = serverSocket;

    while(1){
        
        // Mostro i comandi disponibili se il client è stato sbloccato
        if (unlocked == 1){
            printComandeInviate();
            printCommands();
            printInputLine();
        }

        // Mostro il comando per sbloccare il client se è ancora bloccato
        else
            displayLock();

        // Copio il file descriptor master
        read_fds = master;

        // Effettuo la select
        if(select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1){
            perror("Errore nella select");
            exit(EXIT_FAILURE);
        }

        // Controllo i socket pronti
        int i;
        for (i = 0; i <= fdmax; i++){
            if(FD_ISSET(i, &read_fds)){

                // Se lo stdin è pronto
                if(i == 0){

                    // Leggo il comando da tastiera
                    fgets(command, COMMAND_LEN, stdin);
                    command[strlen(command) - 1] = '\0';

                    /* ---------------------------- Unlock ---------------------------- */
                    if(unlocked == 0){

                        // Controllo se il comando inserito è un codice di prenotazione
                        if(convalidaSintassiCodice(command, code) == -1){
                            continue;
                        }

                        // Invio il codice di prenotazione al server e aggiorno il flag di sblocco
                        if(unlock(code, serverSocket) != -1){
                            unlocked = 1;
                            continue;
                        }

                    }


                    /* ------------------------ Comando "help" ------------------------ */ 
                    else if(strcmp(command, "help") == 0){
                        displayCommandDetails();
                    }

                    /* ------------------------ Comando "menu" ------------------------ */ 
                    else if(strcmp(command, "menu") == 0){
                        getMenu(serverSocket);
                    }

                    /* ----------------------- Comando "comanda" ----------------------- */ 
                    else if(strncmp(command, "comanda", 7) == 0){
                        
                        comanda_dish_t *dishList = NULL;

                        // Controllo se il comando inserito è corretto
                        if(convalidaComanda(command + strlen("comanda "), &dishList) == -1){
                            printf("\n");
                            continue;
                        }

                        // Invio la comanda al server
                        comanda(dishList, serverSocket);

                    }

                    /* ------------------------ Comando "conto" ------------------------ */ 
                    else if(strcmp(command, "conto") == 0){
                        if(conto(serverSocket) == 0)
                            unlocked = 0;
                        printClosingMsg();
                    }

                    /* ------------------- Comando non riconosciuto ------------------- */ 
                    else{
                        printf("Comando non riconosciuto, digitare \"help\" per maggiori informazioni.\n\n");
                    }

                }
                
                // Se il socket del server è pronto
                else if(i == serverSocket){

                    char buffer[BUFFER_LEN];
                    memset(buffer, 0, sizeof(buffer));
                    
                    int comanda;
                    comanda_state_t state;

                    int nbytes;

                    printf("\n");

                    // Ricevo la risposta del server
                    if((nbytes = recv(serverSocket, (void *)buffer, BUFFER_LEN, 0)) == -1){
                        perror("Errore nella ricezione della risposta del server");
                        exit(EXIT_FAILURE);
                    }

                    if(nbytes == 0){
                        printf("La chiusura del server è stata forzata...\n");
                        
                        // Chiudo la connessione
                        close(serverSocket);
                        // Stampo il messaggio di chiusura
                        printClosingMsg();

                        exit(EXIT_FAILURE);
                    }

                    // Deserializzo la richiesta
                    server_message_t msg;
                    memset(&msg, 0, sizeof(msg));
                    msg.header = atoi(strtok(buffer, "\n"));

                    // Controllo il tipo di risposta
                    switch(msg.header){
                        case STOP:
                            printf("\nIl server è in chiusura...\n");
                            if(unlocked == 1)
                                conto(serverSocket);
                            closeDevice(serverSocket);
                            break;
                        case UPDATE:
                            strncpy(msg.serializedBody, strtok(NULL, ""), BUFFER_LEN - sizeof(operazione_t) - 2);
                            msg.serializedBody[BUFFER_LEN - sizeof(operazione_t) - 2] = '\0';
                            sscanf(msg.serializedBody, "%d\t%d", &comanda, (int *)&state);
                            updateComandaStatus(comanda, state);
                            break;
                        default:
                            printf("\nRichiesta dal server non riconosiuta\n");
                            break;
                    }

                    printf("\n");

                }

                else
                    continue;

            }
        }

    }
    
}
