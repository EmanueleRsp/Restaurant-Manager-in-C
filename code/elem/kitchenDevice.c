#include "../lib/kitchen/kitchenLib.h"


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
    
    char buffer[BUFFER_LEN];

    // Dichiarazione set di socket
    fd_set master;
    fd_set read_fds;
    int fdmax;

    //  Buffer e variabili per la gestione dei comandi da tastiera
    char command[COMMAND_LEN];
    int numComandeInAttesa = 0;


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
    if(send(serverSocket, "K", 2, 0) == -1){
        perror("Errore nell'invio dell'identificazione del device");
        exit(EXIT_FAILURE);
    }

    // Recupero del numero di comande in attesa
    memset(buffer, 0, sizeof(buffer));
    if(recv(serverSocket, (void *)buffer, BUFFER_LEN, 0) == -1){
        perror("Errore nella ricezione del numero di comande in attesa");
        exit(EXIT_FAILURE);
    }
    numComandeInAttesa = atoi(buffer);
    if(numComandeInAttesa < 0){
        perror("Errore nella ricezione del numero di comande in attesa");
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
        
        // Comande da prendere in carico
        printComandeInAttesa(numComandeInAttesa);

        // Mostro i comandi disponibili
        printCommands();
        printInputLine();

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

                    /* ------------------------ Comando "take" ------------------------ */ 
                    if(strcmp(command, "take") == 0){

                        if(numComandeInAttesa == 0){
                            printf("Non ci sono comande da prendere in carico.\n");
                            continue;
                        }

                        if(take(serverSocket) == -1)
                            continue;
                        
                        numComandeInAttesa--;

                    }

                    /* ------------------------ Comando "show" ------------------------ */ 
                    else if(strcmp(command, "show") == 0){
                        show();
                    }

                    /* ----------------------- Comando "ready" ----------------------- */ 
                    else if(strncmp(command, "ready", 5) == 0){
                        
                        int tavolo, comanda;

                        if(convalidaComandaUpdate(command + 6, &tavolo, &comanda) == -1)
                            continue;

                        ready(serverSocket, tavolo, comanda);
                    }

                    /* ------------------- Comando non riconosciuto ------------------- */ 
                    else{
                        printf("Comando non riconosciuto.\n");
                    }

                }
                
                // Se il socket del server è pronto
                else if(i == serverSocket){

                    int nbytes;

                    printf("\n\n");

                    // Ricevo la risposta del server
                    memset(buffer, 0, sizeof(buffer));
                    if((nbytes = recv(serverSocket, (void *)buffer, BUFFER_LEN, 0)) == -1){
                        perror("Errore nella ricezione della risposta del server");
                        exit(EXIT_FAILURE);
                    }

                    if(nbytes == 0){
                        printf("La chiusura del server è stata forzata...\n");
                        closeDevice(serverSocket);
                        break;
                    }

                    // Deserializzo la richiesta
                    server_message_t msg;
                    memset(&msg, 0, sizeof(msg));
                    msg.header = atoi(strtok(buffer, "\n"));

                    // Controllo il tipo di risposta
                    switch(msg.header){

                        case STOP:

                            printf("Il server è in chiusura...\n");
                            closeDevice(serverSocket);
                            
                            break;
                            
                        case INCREASE:
                        case DECREASE:

                            printf("Aggiornamento del numero di comande in attesa...\n");

                            // Aggiorno il numero di comande in attesa
                            if(msg.header == DECREASE && numComandeInAttesa > 0)
                                numComandeInAttesa--;
                            else if(msg.header == INCREASE)
                                numComandeInAttesa++;
                            else
                                printf("Errore: il numero di comande in attesa è già 0.\n");
                            break;

                        default:

                            printf("Richiesta dal server non riconosciuta\n");

                            break;
                    }

                }

                else
                    continue;

            }
        }

        printf("\n");
    }
    
}
