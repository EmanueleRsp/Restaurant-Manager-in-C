# Memorizzazione dei dati

- [Memorizzazione dei dati](#memorizzazione-dei-dati)
  - [1 tableOrg.txt](#1-tableorgtxt)
  - [2 menu-YY-MM-DD.txt](#2-menu-yy-mm-ddtxt)
  - [3 dis-YY-MM-DD.txt](#3-dis-yy-mm-ddtxt)
  - [4 com-YY-MM-DD.txt](#4-com-yy-mm-ddtxt)
  - [5 res-YY-MM.txt](#5-res-yy-mmtxt)


---

## 1 tableOrg.txt
Il file `tableOrg.txt` contiene la lista dei tavoli disponibili, per cui sono specificati _numero del tavolo_, _numero della sala_, _numero di coperti_ e _eventuali informazioni aggiuntive_ sulla loro locazione.
Il file è formattato in modo tabulare, con le colonne separate dal carattere `\t`, nel seguente modo:

| Tavolo | Sala | Coperti | Info |
|:---:|:---:|:---:|:---:|
|1|2|8|VERANDA|
|2|1|7|CAMINO|
|3|3|7|FINESTRA|
|4|2|7| - |
|5|2|2|VIP|
|6|2|6| - |
|7|3|6|INGRESSO|
|8|3|10| - |
|9|1|6|FINESTRA|
|10|3|6|VICINO BANCONE|


## 2 menu-YY-MM-DD.txt
Il file `menu/menu-YY-MM-DD.txt` contiene la lista dei piatti disponibili del menù, presenti a partire dal giorno `YY-MM-DD`, per cui sono specificati _id del piatto_, _nome_ e _prezzo_.
Il file è formattato in modo tabulare, con le colonne separate dal carattere `\t`, nel seguente modo:

| IdPiatto | Nome | Prezzo |
|:---:|:---:|:---:|
| A1 | Antipasto di terra | 7 |
| A2 | Antipasto di mare | 8 |
| P1 | Spaghetti alle vongole | 10 |
| P2 | Rigatoni all’amatriciana |6 |
| S1 | Frittura di calamari | 20 | 
| S2 | Arrosto misto | 15 |
| D1 | Crostata di mele | 5 | 
| D2 | Zuppa inglese | 5 |


## 3 dis-YY-MM-DD.txt
Il file `dishes/dis-YY-MM-DD.txt` contiene la lista piatti ordinati per prenotazioni in data `YY-MM-DD`, per cui sono specificati _numero del tavolo_, _ora della prenotazione_, _numero della comanda_, _id del piatto_ e _quantità_.
Il file è formattato in modo tabulare, con le colonne separate dal carattere `\t`, nel seguente modo:

| Tavolo | Ora | Comanda | IdPiatto | Quantità |
|:---:|:---:|:---:|:---:|:---:|
| 1 | 12 | 1 | A1 | 2 |
| 1 | 12 | 1 | P2 | 1 |
| 1 | 12 | 1 | A2 | 1 |


## 4 com-YY-MM-DD.txt
Il file `comande/com-YY-MM-DD.txt` contiene la lista delle comande effettuate da prenotazioni in data `YY-MM-DD`, per cui sono specificati _numero del tavolo_, _ora della prenotazione_, _numero della comanda_ e _stato della comanda_.
Il file è formattato in modo tabulare, con le colonne separate dal carattere `\t`, nel seguente modo:

| Tavolo | Ora | Comanda | Stato | Data invio |
|:---:|:---:|:---:|:---:|:---:|
|1|15|1|0| 00-00-00 |
|1|15|2|1| 00-00-00 |
|1|15|3|1| 00-00-00 |
|4|18|1|0| 00-00-00 |
|4|18|2|2| 00-00-01 |

I possibili valori dello stato sono 3:
- `0`: _In attesa_
- `1`: _In preparazione_
- `2`: _In servizio_


## 5 res-YY-MM.txt

Il file `reservations/res-YY-MM.txt` contiene la lista delle prenotazioni effettuate per il mese `MM` dell'anno `YY`. Per ogni prenotazione sono memorizzati _codice della prenotazione_, _numero del tavolo prenotato_, _data e ora della prenotazione_, _nome della prenotazione_, _numero di persone_, _data e ora in cui è stata effettuata la prenotazione_ e _presenza_.
Il file è formattato in modo tabulare, con le colonne separate dal carattere `\t`, nel seguente modo (_esempio per il mese di agosto 2023_):

| CodicePrenotazione | Tavolo | Data | Ora | NomePrenotazione | NumeroPersone | DataOraPrenotazione | Presenza |
|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:----:|
| ABC123 | 3 | 15-08-23 | 12 | Mario | 2 | 23-01-01 12:30:15 | 2 |
| XYZ789 | 7 | 10-08-23 | 14 | Anna | 4 | 23-02-08 10:30:25 | 2 |
| DEF456 | 12 | 05-08-23 | 18 | Luca | 3 | 23-03-03 12:30:35 | 0 |
| GHI789 | 20 | 20-08-23 | 20 | Sofia | 5 | 23-04-02 19:30:45 | 1 |
| LMN123 | 30 | 25-08-23 | 10 | Marta | 2 | 23-05-20 20:30:55 | 0 |
| OPQ456 | 5 | 12-08-23 | 15 | Carlo | 3 | 23-06-10 13:31:05 | 1 |
| RST789 | 9 | 01-08-23 | 17 | Chiara | 2 | 23-07-10 21:31:15 | 2 |
| UVW123 | 15 | 08-08-23 | 19 | Giulia | 4 | 23-08-05 10:31:25 | 1 |

Il campo di presenza può assumere 2 valori:
- `0`: _Prenotato_ (il cliente non è ancora arrivato)
- `1`: _Pasto in corso_ (il cliente ha sbloccato il table device)
- `2`: _Pasto terminato_ (il cliente ha richiesto il conto)