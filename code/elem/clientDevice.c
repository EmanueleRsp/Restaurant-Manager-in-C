#include "../lib/client/clientLib.h"


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
    char cognome[COGNOME_LEN];
    int numeroPersone;
    char data[DATE_LEN];
    int hh, dd, mm, yy;
    int opz;
    
    reservation_t p;
    memset(&p, 0, sizeof(p));
    int opzNum;

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
    if(send(serverSocket, "C", 2, 0) == -1){
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

    // Mostro la schermata di benvenuto
    start();

    while(1){
        
        // Mostro i comandi disponibili
        printCommands();

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

                    // Comando "esc"
                    if(strcmp(command, "esc") == 0){
                        printf("Chiusura del client in corso...\n");
                        closeDevice(serverSocket);
                    }

                    /* ------------------------ Comando "find" ------------------------ */ 
                    else if(strncmp(command, "find", 4) == 0){

                        strtok(command, " ");
                        
                        // Verificare che il token successivo (cognome) sia composto solo da lettere
                        if(convalidaCognome(strtok(NULL, " "), cognome) == 0)
                            continue;

                        // Verificare che il token successivo (numero di persone) sia un numero
                        if(convalidaNumeroPersone(strtok(NULL, " "), &numeroPersone) == 0)
                            continue;

                        // Verificare che il token successivo sia una data valida in formato GG-MM-AA
                        if(convalidaData(strtok(NULL, " "), data) == 0)
                            continue;                        

                        // Verificare che il token successivo sia un orario in formato HH
                        if(convalidaOra(strtok(NULL, " "), &hh) == 0)
                            continue;

                        // Verificare che non ci siano altri token
                        if(strtok(NULL, " ") != NULL){
                            printf("Errore comando find: troppi argomenti\n");
                            printf("Utilizzare: find <cognome> <numero_persone> <data> <ora>\n");
                            continue;
                        }

                        // Verificare che non si richieda un orario passato
                        data[DATE_LEN - 1] = '\0';
                        scomposeData(data, &dd, &mm, &yy);
                        if(convalidaOrarioPassato(dd, mm, yy, hh) == -1)
                            continue;

                        // Inizializzo la struttura prenotazione
                        strcpy(p.cognome, cognome);
                        p.numeroPersone = numeroPersone;
                        strcpy(p.data, data);
                        p.ora = hh;

                        // Gestisco l'operazione
                        opzNum = find(p, serverSocket);

                    }

                    /* ------------------------ Comando "book" ------------------------ */ 
                    else if(strncmp(command, "book", 4) == 0){

                        strtok(command, " ");

                        // Verificare che sia stata effettuata una ricerca
                        if(opzNum == -1){
                            printf("Errore comando book: nessuna ricerca effettuata\n");
                            printf("Utilizzare: find <cognome> <numero_persone> <data> <ora>\n");
                            continue;
                        }
                        if(opzNum == 0){
                            printf("Nessuna disponibilità per la richiesta effettuata.\n");
                            printf("Utilizza il comando \"find\" per effettuare una nuova ricerca!\n");
                            continue;
                        }

                        // Verificare che il token successivo (opz) sia composto da un numero
                        if(convalidaOpz(strtok(NULL, " "), &opz) == 0)
                            continue;

                        // Verificare che non ci siano altri token
                        if(strtok(NULL, " ") != NULL){
                            printf("Errore comando book: troppi argomenti\n");
                            printf("Utilizzare: book <opz>\n");
                            continue;
                        }

                        else if(opz == 0 || opz > opzNum){
                            printf("Errore comando book: opzione non valida\n");
                            continue;
                        }

                        // Invio i dati per la prenotazione e gestisco la risposta
                        book(opz, serverSocket);

                        opzNum = -1;
                    }

                    /* ------------------- Comando non riconosciuto ------------------- */ 
                    else{
                        printf("Comando non riconosciuto\n");
                    }

                }
                
                // Se il socket del server è pronto: sta tentando di chiudere la connessione
                else if(i == serverSocket){

                    char buffer[BUFFER_LEN];
                    memset(&buffer, 0, sizeof(buffer));

                    server_message_t msg;
                    memset(&msg, 0, sizeof(msg));

                    ssize_t byteRecv;

                    printf("\n\n");

                    // Recupero il messaggio di chiusura dal server
                    if((byteRecv = recv(serverSocket, (void *)buffer, BUFFER_LEN, 0)) == -1){
                        perror("Errore nella ricezione del messaggio");
                        exit(EXIT_FAILURE);
                    }

                    // Se il server ha chiuso la connessione
                    if(byteRecv == 0){
                        printf("Il server è stato chiuso forzatamente...\n");
                        closeDevice(serverSocket);
                    }

                    // Altrimenti...
                    printf("Il server sta eseguendo l'arresto...\n");

                    // Deserializzo il messaggio
                    msg.header = atoi(strtok(buffer, "\n"));
                    strncpy(msg.serializedBody, strtok(NULL, ""), 1019);
                    msg.serializedBody[1019 -1] = '\0';

                    if(msg.header != STOP){
                        printf("Errore nella chiusura del client\n");
                        exit(EXIT_FAILURE);
                    }

                    closeDevice(serverSocket);
                }

                else
                    continue;

            }
        }

        printf("\n");
    }
    
}
