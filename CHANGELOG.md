# Changelog

Tutte le modifiche significative a questo progetto saranno documentate in questo file.

Il formato è basato su [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
e questo progetto aderisce al [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.0.1-alpha] - 2024-12-19

### Aggiunto
- Interfaccia utente principale per gestione logbook radioamatoriale
- Supporto database SQLite3 per archiviazione contatti
- Validazione automatica nominativi radioamatoriali
- Pre-valorizzazione intelligente valori RST basata su modulazione
  - RST a 2 cifre (59) per modi vocali (SSB, AM, FM, USB, LSB)
  - RST a 3 cifre (599) per modi digitali e CW (RTTY, CW, PSK31, FT8, ecc.)
- Conversione automatica nominativi in maiuscolo
- Validazione locatore Maidenhead (4 o 6 caratteri)
- Integrazione API QRZ.com per lookup automatico DXCC/locatore
- Integrazione API Clublog.org per lookup automatico DXCC/locatore
- Sistema di impostazioni avanzate con interfaccia a schede
- Funzionalità reset sicuro ai valori di fabbrica con conferma nominativo
- Supporto accessibilità completo (screen reader, navigazione tastiera)
- Configurazione iniziale guidata per operatore e credenziali API
- Gestione bande e modi da file JSON configurabili
- Tabella contatti con ordinamento e filtri
- Menu completo con scorciatoie da tastiera
- Aggiornamento automatico data/ora UTC

### Caratteristiche Tecniche
- Sviluppato in Qt6 con supporto multipiattaforma
- Architettura MVC con separazione logica/presentazione
- Database SQLite3 con schema ottimizzato
- Validazione robusta input utente
- Gestione errori completa
- Logging e debug integrati
- Build system qmake e CMake
- Documentazione completa

### Conformità Standard
- Standard RST radioamatoriali (R-S per fonia, R-S-T per digitale/CW)
- Formato locatore Maidenhead
- Convenzioni nominativi radioamatoriali internazionali
- Linee guida accessibilità Qt

### Note di Rilascio
Questa è la prima versione alpha di QT Logbook. Include tutte le funzionalità
base per la gestione di un logbook radioamatoriale moderno con supporto API
e validazione automatica. La versione è stabile per uso quotidiano ma potrebbe
contenere bug minori che saranno risolti nelle versioni successive.