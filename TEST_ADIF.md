# Test delle Funzionalità ADIF

## Panoramica
Questo documento descrive come testare le funzionalità di importazione ed esportazione ADIF implementate nel QT Logbook.

## File di Test Disponibili

### 1. test_import.adi
Contiene 3 contatti di esempio:
- IZ0ABC (20M SSB)
- DL1DEF (40M CW) 
- G0GHI (80M FT8)

### 2. test_duplicates.adi
Contiene 4 contatti, di cui 2 sono duplicati dei contatti nel primo file:
- IZ0ABC (20M SSB) - **DUPLICATO**
- F1NEW (40M CW) - Nuovo
- DL1DEF (40M CW) - **DUPLICATO**
- EA1NEW (15M FT8) - Nuovo

## Procedura di Test

### Test 1: Importazione Base
1. Avviare QT Logbook
2. Menu File → Importa ADIF...
3. Selezionare `test_import.adi`
4. Verificare che vengano importati 3 contatti
5. Controllare che i contatti appaiano nella tabella principale

### Test 2: Gestione Duplicati
1. Con i contatti del Test 1 già nel database
2. Menu File → Importa ADIF...
3. Selezionare `test_duplicates.adi`
4. **Risultato atteso:**
   - Record totali: 4
   - Contatti importati: 2 (F1NEW, EA1NEW)
   - Duplicati trovati: 2 (IZ0ABC, DL1DEF)
5. Salvare il report duplicati come `DupeImport.adi`
6. Verificare che il file contenga i 2 contatti duplicati

### Test 3: Esportazione
1. Menu File → Esporta ADIF...
2. Salvare come `export_test.adi`
3. Verificare che il file contenga tutti i contatti del logbook
4. Controllare la conformità del formato ADIF

## Criteri di Rilevamento Duplicati
Un contatto è considerato duplicato se ha:
- Stesso callsign
- Stessa data (YYYYMMDD)
- Stessa banda

## Formato ADIF Supportato
- Versione: ADIF 3.1.4
- Campi supportati: CALL, QSO_DATE, TIME_ON, BAND, MODE, RST_SENT, RST_RCVD, DXCC, GRIDSQUARE, OPERATOR
- Encoding: UTF-8
- Formato data: YYYYMMDD
- Formato ora: HHMMSS

## Note
- I duplicati non vengono aggiunti al database
- Il report duplicati viene generato in formato ADIF standard
- L'operatore corrente viene impostato automaticamente se non specificato
- La validazione del formato ADIF è rigorosa