# Documentazione

Studente: Emanuele Respino.

Sviluppo di un'applicazione distribuita basata sul paradigma client-server che implementi un sistema di prenotazione tavoli e gestione delle comande di un ristorante.

---

## Indice

- [Documentazione](#documentazione)
  - [Indice](#indice)
  - [1. Visione d'insieme](#1-visione-dinsieme)
  - [2. Modalità di trasmissione dei dati](#2-modalità-di-trasmissione-dei-dati)
  - [3. Gestione della memoria](#3-gestione-della-memoria)
  - [4. Dettagli implementativi](#4-dettagli-implementativi)
    - [4.1 Client device](#41-client-device)
    - [4.2 Table device](#42-table-device)
    - [4.3 Kitchen device](#43-kitchen-device)
    - [4.4 Server](#44-server)
  - [5. Considerazioni](#5-considerazioni)
    - [5.1 Scalabilità orizzontale](#51-scalabilità-orizzontale)
    - [5.2 Scalabilità verticale](#52-scalabilità-verticale)
    - [5.3 Robustezza ai guasti](#53-robustezza-ai-guasti)
  - [6 Maggiori dettagli](#6-maggiori-dettagli)

---

## 1. Visione d'insieme
Il progetto è strutturato come un'applicazione **client-server**, in cui il _server_ **multiservizio** gestisce le richieste in arrivo da parte di tre tipologie di dispositivi (_client_, _table_ e _kitchen_), comunicando mediante scambio di messaggi sfruttando appositi protocolli e **strutture dati**. Il server è inoltre responsabile della gestione dei dati, i quali sono salvati su **file di testo**.

---

## 2. Modalità di trasmissione dei dati

La comunicazione tra i dispositivi e il server avviene tramite **socket TCP**, mediante scambio di dati in modalità _TEXT_ e sfruttando appositi protocolli e strutture dati.
Nello specifico, quando un dispositivo tenta di connettersi al server, invia un messaggio di inizializzazione (**handshake**) contenente il tipo di dispositivo che si è connesso (`K` - _kitchen device_, `T` - _table device_, `C` - _client device_), con cui il server può identificarlo.
Successivamente, il dispositivo può inviare richieste (sfruttando la struttura `request_t`) al server, il quale risponde con un messaggio contenente l'esito dell'operazione richiesta e, se necessario, dati aggiuntivi (tramite la struttura `response_t`). Inoltre, anche il server può notificare direttamente i dispositivi mediante l'invio di messaggi (struttura `server_message_t`). 
L'utilizzo di **strutture predefinite** consente di non dover trasmettere in anticipo la dimensione del messaggio, poiché conosciuto. Ognuna delle strutture dati sopra citate è composta da un campo che contiene _tipo/esito_ dell'operazione e un campo che contiene eventuali _dati_ necessari alla sua corretta elaborazione.
I sistemi sono **monoprocesso** e regolano la comunicazione tramite **I/O-Multiplexing**, in modo da gestire in maniera più semplice la comunicazione tra di essi, evitando l'hoveread dovuto alla creazione di nuovi processi e la complessità dell'utilizzo di thread.
L'_utilizzo di thread_ sarebbe potuta essere la scelta ottimale per quanto riguarda l'esecuzione del server, ma ipotizzando che il flusso di richieste per il ristorante non sia troppo elevato, si è preferito utilizzare un approccio più semplice (**server iterativo**), evitando di dover gestire l'accesso in mutua esclusione alle risorse, ma accedendovi in maniera seriale.

---

## 3. Gestione della memoria

Per mantenere i dati, è stato sfruttato un approccio ibrido tra **memoria dinamica** e **file persistenti** (_file di testo_).
Nello specifico, nei file sono mantenuti i dati relativi a _menù_, _organizzazione dei tavoli_, _prenotazioni_ e _comande_, mentre in memoria dinamica quelli riguardanti _dispositivi attualmente connessi_, _comande in attesa o in fase di elaborazione_ e _tavoli in cui è in corso un pasto_.
L'**utilizzo di file** consente la **persistenza** dei dati e garantisce maggiore **robustezza** ad eventuali guasti, permettendone nel caso un ripristino efficiente. Inoltre, per alcuni elementi (_menu_, _tavoli_ e _prenotazioni_), la persistenza risulta una proprietà **necessaria**, in quanto devono essere mantenuti anche oltre la chiusura del server.
Di contro, l'utilizzo di file comporta un overhead dovuto alla lettura/scrittura su disco. Per **ridurre il numero di accessi**, viene sfruttata anche la **memoria dinamica**, che mantiene in memoria i dati la cui **rilevanza è limitata** nel tempo: dati sui _dispositivi connessi_, _tavoli attualmente occupati_ e _richieste di prenotazione_ sono infatti deallocati al termine della loro utilità. Per quanto riguarda le _comande_, è stato deciso di mantenerle anche in memoria dinamica finché non sono servite, poiché in questa prima fase devono poter essere consultate più volte.
Per ridurre l'**overhead** dovuto all'accesso ai file, è stato optato per la **frammentazione** dei file in documenti più piccoli, _indicizzati per data_, ipotizzando di mantenere in memoria dati relativi alla ristorazione anche dopo il loro periodo di utilizzo, come a creare uno storico relativo all'attività del locale.

---

## 4. Dettagli implementativi

Tutti i dispositivi sono stati sviluppati con I/O Multiplexing, consentendo ad essi di essere sensibile sia a comandi in input, sia ai segnali ricevuti dagli altri dispositivi. Eventuali parametri di ogni operazione vengono convalidati sia dal device che dal server, il quale esegue eventuali controlli aggiuntivi e più approfonditi.

### 4.1 Client device
Tramite il client l’utente può richiedere una prenotazione.
- **Find**: Il client inoltra al server i dati di prenotazione. Questo recupera i dati sui tavoli disponibili e li invia in risposta, memorizzando in una lista la richiesta effettuata. Il client salva la risposta ricevuta.
- **Book**: Il client specifica l’opzione scelta dalla lista ricevuta tramite find, viene recuperato il tavolo relativo all’opzione selezionata e inviato al server, il quale tenta di inserire la prenotazione, verificando che non entri in conflitto con altre già registrate: due client, infatti, potrebbero tentare insieme la stessa prenotazione, ma solo la prima richiesta avrà successo. Una volta registrato, il server risponde con i dettagli dell’operazione svolta, incluso il codice di prenotazione generato, ed elimina dalla memoria le informazioni relative alla proposta.
- **Esc**: Il processo client termina, chiudendo la connessione.

### 4.2 Table device
Al cliente, giunto al ristorante, viene dato il `TD` con il quale effettuare il login: il device si sblocca inserendo il codice ricevuto in fase di prenotazione, che viene trasmesso al server e verificato. In caso di esito positivo, lo stato della prenotazione è aggiornato a in `CORSO`, viene memorizzata l’associazione device-prenotazione tramite un’apposita lista e il pasto può cominciare.
- **Menù**: Il `TD` contatta il server che risponde inviando il menù attualmente disponibile.
- **Comanda**: Il server controlla che i piatti siano effettivamente presenti nel menù. Superati i controlli, la comanda entra in `ATTESA` e inserita nel relativo file e in memoria, nella lista ordinata delle comande non in `SERVIZIO`. Viene poi notificata ai `KD` la presenza di una nuova comanda in attesa.
- **Conto**: Il cliente può in qualunque momento chiedere il conto. Il server provvederà ad addebitare i piatti relativi a comande che sono in `PREPARAZIONE` o in `SERVIZIO` consultando il relativo file, inviando la spesa al `TD`. Dopodiché, il server procede a rimuovere i dati in memoria dinamica relativi al pasto, inviando notifiche ai `TD` in caso ci fossero ancora comande in attesa. Infine, pone la prenotazione in stato `TERMINATO`.

### 4.3 Kitchen device
Alla connessione, il server invia il numero di comande in attesa.
- **Take**: Il server scorre dal fondo la lista delle comande non in servizio. Per ciascuna comanda nella lista sono memorizzati `TD` a cui appartiene e `KD` che l’ha presa in carico (-1 se in attesa). Alla prima ancora in attesa si ferma e imposta il `KD`, inoltra la modifica nel file (stato in `PREPARAZIONE`) e invia al `KD` che ha fatto la richiesta la comanda, mentre agli altri una notifica per indicare l’evento. Invia anche una notifica al `TD` per aggiornare lo stato della comanda.
- **Show**: Le comande in carico sono memorizzate dal `KD` al termine della TAKE.
- Ready: Il server verifica tramite la lista che la comanda sia sempre presente e che sia effettivamente in carico al `KD`. Dopodiché imposta lo stato della comanda in `SERVIZIO` nel file e la elimina dalla lista, notificando il `KD` l’update dello stato.

### 4.4 Server
Per quanto riguarda le azioni disponibili:
- **Stat**: A causa della frammentazione, nel caso delle comande giornaliere, occorre consultare i file relativi alla giornata corrente e precedente. Negli altri casi, per ogni pasto in corso, si accede al file della relativa giornata di prenotazione. Questo per gestire correttamente casi in cui il pasto si protrae oltre la mezzanotte.
- **Stop**: Il processo server termina, ma solo se tutte le comande sono in servizio. Prima di chiudere, disconnette tutti i device, inviando uno specifico messaggio e verificandone la chiusura. Nel caso di `TD` in cui è in corso il pasto viene inviato il conto.

---

## 5. Considerazioni

### 5.1 Scalabilità orizzontale
Nel caso della gestione di più ristoranti, il sistema **scalerebbe bene orizzontalmente**, nell'ottica di dedicare un server a ciascuno di questi.

### 5.2 Scalabilità verticale
L’I/O multiplexing ha lo svantaggio che l’aumento della frequenza delle operazioni richiede un maggiore impiego di risorse. Inoltre, il frequente accesso ai file, per mantenere il sistema robusto ai guasti, costituirebbe un altro problema. La soluzione migliore sarebbe convertire, almeno il server, a un **sistema basato su thread** (più complesso), e/o **limitare ulteriormente l’accesso ai file**, per esempio registrando le comande solo quando entrano in servizio, soluzione che però diminuirebbe la robustezza ai guasti.

### 5.3 Robustezza ai guasti
Come detto, la soluzione è progettata per essere **abbastanza robusta ad eventuali guasti**, cercando di gestire al meglio la chiusura improvvisa dei device connessi e salvando in memoria persistente lo stato delle comande, a discapito dell’efficienza. Questo, nel caso venisse implementato un apposito processo, potrebbe permettere un completo ripristino del sistema.

---

## 6 Maggiori dettagli
Per maggiori dettagli sulla suddivisione dei file di memoria persistente, sulla struttura dei file e sulle strutture dati utilizzate, consultare la relativa documentazione:
- [`docs/files organization.md`](/docs/files%20organization.md): descrive la suddivisione dei file di memoria persistente e la struttura dei file.
- [`docs/data transmission.md`](/docs/data%20transmission.md): descrive le strutture dati utilizzate per la trasmissione dei dati.

Per leggere le specifiche del progetto, consultare il file [`docs/Specifiche.pdf`](/docs/Specifiche.pdf).