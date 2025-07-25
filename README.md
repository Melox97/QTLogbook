# QT Logbook - Logbook Radioamatoriale

Un'applicazione multipiattaforma sviluppata in Qt6 per la gestione di un logbook radioamatoriale con database SQLite3.

## Caratteristiche

### Gestione Contatti
- **Data e Ora UTC**: Automaticamente derivata dal timestamp di sistema
- **Nominativo**: Campo per nominativo radioamatoriale (3-10 caratteri alfanumerici)
- **Banda**: Selezione da elenco predefinito (160m, 80m, 40m, 20m, 10m, 6m, 2m, 70cm, ecc.)
- **Modo**: Selezione da elenco predefinito (CW, SSB, FM, FT8, FT4, PSK31, RTTY, ecc.)
- **RST Inviato/Ricevuto**: Campi per rapporti RST a 3 cifre
- **DXCC**: Auto-determinato tramite API dal nominativo
- **Locatore**: Auto-determinato tramite API dal nominativo
- **Operatore**: Nominativo dell'operatore (configurato all'inizializzazione)

### Funzionalità
- **Gestione Contatti**: Aggiungi, modifica, elimina e cerca contatti radioamatoriali
- **Database SQLite3**: Archiviazione sicura e affidabile dei dati
- **Interfaccia Accessibile**: Supporto completo per `QTAccessibility`
- **UI Dinamica**: Interfaccia fluida e consistente con il sistema operativo
- **Configurazione Iniziale**: Dialog guidato per configurare operatore e credenziali API
- **Integrazione QRZ.com**: Lookup automatico dei dati del nominativo (opzionale)
- **Integrazione Clublog.org**: Lookup automatico dei dati del nominativo (opzionale)
- **Gestione Credenziali**: Configurazione sicura delle API con disabilitazione automatica
- **Lookup Automatico**: Ricerca automatica di DXCC e locatore
- **Fallback Locale**: Sistema di lookup locale se le API non sono disponibili
- **Validazione Nominativi**: Controllo formato dei nominativi radioamatoriali

### Interfaccia Utente
- **Design Nativo**: Interfaccia coerente con il sistema operativo
- **Accessibilità**: Implementazione completa di QTAccessibility
- **Responsive**: Layout dinamico e fluido
- **Validazione**: Controlli in tempo reale sui dati inseriti

### Database
- **SQLite3**: Database locale per prestazioni ottimali
- **Backup**: Possibilità di esportazione/importazione
- **Ricerca**: Funzionalità di ricerca avanzata nei contatti
- **Statistiche**: Visualizzazione statistiche del logbook

### API Integration
- **QRZ.com**: Integrazione completa con l'API di QRZ.com per lookup nominativi
- **Clublog.org**: Supporto per l'API di Clublog.org come alternativa
- **Lookup Automatico**: Determinazione automatica di DXCC e locatore
- **Fallback Locale**: Funzionamento anche senza connessione internet
- **Validazione**: Controllo della validità dei nominativi radioamatoriali

## Requisiti di Sistema

### Dipendenze
- Qt6 (Core, Widgets, Sql, Network)
- CMake 3.16+
- Compilatore C++17

### Piattaforme Supportate
- Windows 10/11
- macOS 10.15+
- Linux (Ubuntu 20.04+, Fedora 35+)

## Compilazione

### Prerequisiti
```bash
# Ubuntu/Debian
sudo apt install qt6-base-dev qt6-tools-dev cmake build-essential

# macOS (con Homebrew)
brew install qt6 cmake

# Windows
# Installare Qt6 dal sito ufficiale e Visual Studio
```

### Build
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Esecuzione
```bash
# Da directory build
./QTLogbook
```

## Struttura del Progetto

```
QTLogbook/
├── CMakeLists.txt          # Configurazione build
├── README.md               # Documentazione
├── src/                    # Codice sorgente
│   ├── main.cpp           # Entry point
│   ├── mainwindow.h/cpp   # Finestra principale
│   ├── contact.h/cpp      # Classe contatto
│   ├── database.h/cpp     # Gestione database
│   ├── apiservice.h/cpp   # Servizi API
│   └── logbookmodel.h/cpp # Modello dati tabella
├── data/                   # Dati di configurazione
│   ├── bands.json         # Definizione bande
│   └── modes.json         # Definizione modi
└── ui/                     # File interfaccia utente
    ├── mainwindow.ui      # UI principale
    └── setupdialog.ui     # Dialog configurazione
```

## Configurazione Iniziale

Al primo avvio, l'applicazione mostrerà un dialog di configurazione guidato con due sezioni:

### Dati Operatore (Obbligatorio)
- **Nominativo**: Il tuo nominativo radioamatoriale (obbligatorio)
- **Nome**: Il tuo nome (opzionale)
- **Cognome**: Il tuo cognome (opzionale)
- **Locatore**: Il tuo locatore Maidenhead (opzionale)

### Configurazione API (Opzionale)

Per abilitare il lookup automatico, puoi configurare le credenziali per uno o entrambi i servizi:

#### QRZ.com
- Abilita l'integrazione con la checkbox
- Username e password del tuo account QRZ.com
- Richiede abbonamento XML per funzionalità complete
- Pulsante "Test Connessione" per verificare le credenziali

#### Clublog.org
- Abilita l'integrazione con la checkbox
- API key ottenibile gratuitamente dal tuo account Clublog.org
- Vai su Settings → API Keys per generare una nuova chiave
- Pulsante "Test Connessione" per verificare l'API key

**Gestione Automatica**: Se una delle API non è abilitata o configurata, quella specifica funzionalità verrà automaticamente disabilitata per evitare errori. Il sistema utilizzerà il lookup locale come fallback.

## Utilizzo

### Aggiungere un Contatto
1. Inserire il nominativo della stazione contattata
2. Selezionare banda e modo
3. Inserire i rapporti RST
4. I campi DXCC e locatore si popolano automaticamente
5. Cliccare "Aggiungi Contatto"

### Ricerca Contatti
- Utilizzare la barra di ricerca per filtrare i contatti
- Ricerca per nominativo, banda, modo, DXCC
- Ordinamento per colonna cliccando sull'intestazione

### Esportazione Dati
- Menu File → Esporta
- Formati supportati: ADIF, CSV, JSON

## Accessibilità

L'applicazione implementa le linee guida di accessibilità:
- **Screen Reader**: Supporto completo per lettori di schermo
- **Navigazione Tastiera**: Tutti i controlli accessibili via tastiera
- **Alto Contrasto**: Supporto per temi ad alto contrasto
- **Zoom**: Interfaccia scalabile per ipovedenti

## Licenza

Questo progetto è rilasciato sotto licenza MIT. Vedere il file LICENSE per i dettagli.

## Contributi

I contributi sono benvenuti! Per favore:
1. Fork del repository
2. Creare un branch per la feature
3. Commit delle modifiche
4. Push del branch
5. Aprire una Pull Request

## Supporto

Per bug report e richieste di funzionalità, utilizzare il sistema di issue di GitHub.

## Roadmap

- [x] Integrazione con servizi QRZ.com e Clublog.org
- [ ] Cache locale per ridurre le chiamate API
- [ ] Supporto per contest logging
- [ ] Integrazione con HamQTH come provider aggiuntivo
- [ ] Sincronizzazione cloud
- [ ] App mobile companion
- [ ] Integrazione con software di logging esistenti (ADIF import/export)
- [ ] Supporto per QSL management
- [ ] Statistiche avanzate e grafici
- [ ] Backup automatico del database