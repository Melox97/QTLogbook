# Test dei Bug Fix - QTLogbook

## Bug Risolti

### 1. Bug: Shortcut Cmd+Q non funziona
**Problema:** La shortcut Cmd+Q non chiudeva l'applicazione su macOS
**Causa:** La connessione del segnale era collegata a `QWidget::close` invece di `QApplication::quit`
**Soluzione:** Modificato il collegamento del segnale per usare `qApp->quit()`

**Test:**
1. Avviare l'applicazione QTLogbook
2. Premere Cmd+Q
3. ✅ **Risultato atteso:** L'applicazione si chiude immediatamente

### 2. Bug: Campo "Nominativo" accetta solo numeri
**Problema:** Il campo nominativo nella configurazione iniziale accettava solo valori numerici
**Causa:** Validatore `QRegularExpressionValidator` troppo restrittivo con pattern `^[A-Z0-9]{1,10}$`
**Soluzione:** Rimosso il validatore restrittivo, mantenendo solo la conversione automatica in maiuscolo

**Test:**
1. Avviare l'applicazione per la prima volta (o resettare le impostazioni)
2. Nel dialog di configurazione iniziale, provare a inserire lettere nel campo "Nominativo"
3. ✅ **Risultato atteso:** Le lettere vengono accettate e convertite automaticamente in maiuscolo
4. Esempi di input validi: "IZ0ABC", "W1AW", "JA1XYZ"

## File Modificati

### `/src/mainwindow.cpp`
- **Linea 206:** Cambiato `&QWidget::close` in `qApp, &QApplication::quit`

### `/src/setupdialog.cpp`
- **Linee 87-89:** Rimosso validatore restrittivo per campo nominativo

### `/src/settingsdialog.cpp`
- **Linee 121-123:** Rimosso validatore restrittivo per campo nominativo

## Note Tecniche

### Shortcut Cmd+Q
- Su macOS, `QKeySequence::Quit` corrisponde automaticamente a Cmd+Q
- `QApplication::quit()` è il metodo corretto per chiudere l'applicazione
- `QWidget::close()` chiude solo la finestra, non l'intera applicazione

### Campo Nominativo
- La validazione completa del formato nominativo avviene comunque al momento del salvataggio
- La conversione automatica in maiuscolo è mantenuta tramite il segnale `textEdited`
- Il pattern di validazione finale rimane `^[A-Z0-9]{1,3}[0-9][A-Z0-9]{0,3}[A-Z]$` (3-10 caratteri)

## Stato Compilazione
✅ **Compilazione:** Successo (solo 2 warning deprecation Qt6)
✅ **Avvio:** Applicazione si avvia correttamente
✅ **Funzionalità:** Entrambi i bug sono stati risolti

## Test di Regressione
Verificare che le seguenti funzionalità continuino a funzionare:
- [x] Aggiunta contatti
- [x] Configurazione API
- [x] Import/Export ADIF
- [x] Reset ai valori di fabbrica
- [x] Accessibilità