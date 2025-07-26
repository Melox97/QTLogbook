# Analisi Problema VoiceOver Persistente - QTLogbook

## 🚨 Stato Attuale del Problema

**PROBLEMA CONFERMATO**: Nonostante le multiple implementazioni di fix per VoiceOver, il problema della pronuncia inglese degli elementi italiani **PERSISTE**.

### Modifiche Già Implementate (Senza Successo)

#### 1. ✅ Configurazione Locale Italiana
- `QLocale::setDefault(QLocale::Italian, QLocale::Italy)`
- Variabili ambiente `LANG=it_IT.UTF-8`, `LC_ALL=it_IT.UTF-8`
- Caricamento traduzioni Qt standard italiane
- **Risultato**: Compilato e applicato, ma problema persiste

#### 2. ✅ Attributi HTML lang="it"
- Aggiunto `<span lang="it">...</span>` a tutti i `setAccessibleName`
- Modificati `mainwindow.cpp`, `setupdialog.cpp`, `settingsdialog.cpp`
- **Risultato**: Compilato e applicato, ma problema persiste

#### 3. ✅ Ottimizzazioni Timer
- Ridotto timer da 1s a 60s (98.3% riduzione aggiornamenti)
- Gestione intelligente focus con pause/resume
- **Risultato**: Compilato e applicato, ma problema persiste

## 🔍 Analisi Cause Possibili

### Ipotesi 1: Limitazioni Qt su macOS
**Problema**: Qt potrebbe non esporre correttamente gli attributi di lingua a VoiceOver
- VoiceOver su macOS usa NSAccessibility nativo
- Qt potrebbe non tradurre correttamente `lang="it"` in attributi NSAccessibility
- Gli attributi HTML potrebbero essere ignorati da VoiceOver

### Ipotesi 2: Configurazione VoiceOver Sistema
**Problema**: VoiceOver potrebbe essere configurato per ignorare attributi di lingua
- Impostazioni VoiceOver potrebbero forzare inglese
- Voce italiana potrebbe non essere installata/configurata
- Priorità lingua sistema potrebbe sovrascrivere attributi app

### Ipotesi 3: Implementazione Accessibilità Qt Incompleta
**Problema**: Qt6 potrebbe avere bug/limitazioni nell'accessibilità macOS
- `setAccessibleName` potrebbe non supportare markup HTML
- Attributi di lingua potrebbero richiedere implementazione nativa
- Bridge Qt-NSAccessibility potrebbe perdere informazioni lingua

### Ipotesi 4: Timing e Inizializzazione
**Problema**: Configurazione lingua potrebbe avvenire troppo tardi
- VoiceOver potrebbe "bloccare" la lingua al primo accesso
- Configurazione locale potrebbe non propagarsi a VoiceOver
- Riavvio applicazione potrebbe essere necessario

## 🧪 Test Diagnostici Necessari

### Test 1: Verifica Configurazione VoiceOver Sistema
```bash
# Verificare voce italiana installata
say -v ? | grep -i ital

# Testare pronuncia italiana diretta
say -v "Alice (italiano)" "Campo nominativo radioamatoriale"

# Verificare impostazioni VoiceOver
defaults read com.apple.VoiceOver4
```

### Test 2: Test Applicazione Nativa macOS
Creare app nativa Objective-C/Swift con:
```objc
// Objective-C
[button setAccessibilityLabel:@"Campo nominativo radioamatoriale"];
[button setAccessibilityLanguage:@"it-IT"];

// Swift
button.accessibilityLabel = "Campo nominativo radioamatoriale"
button.accessibilityLanguage = "it-IT"
```

### Test 3: Verifica Attributi Qt Runtime
```cpp
// Debug output per verificare attributi effettivi
qDebug() << "Accessible name:" << m_callsignEdit->accessibleName();
qDebug() << "Locale:" << QLocale::system().name();
qDebug() << "App locale:" << QLocale().name();
```

### Test 4: Test Altre Applicazioni Qt
- Testare altre app Qt6 italiane con VoiceOver
- Verificare se problema è specifico di QTLogbook
- Confrontare comportamento con app native

## 🛠️ Soluzioni Alternative da Provare

### Soluzione A: Implementazione NSAccessibility Nativa
```cpp
#ifdef Q_OS_MACOS
#include <Cocoa/Cocoa.h>

void setNativeAccessibilityLanguage(QWidget* widget, const QString& language) {
    NSView* nsView = reinterpret_cast<NSView*>(widget->winId());
    if (nsView) {
        [nsView setAccessibilityLanguage:@"it-IT"];
    }
}
#endif
```

### Soluzione B: Plugin Accessibilità Personalizzato
```cpp
// Implementare QAccessibleInterface personalizzato
class ItalianAccessibleInterface : public QAccessibleInterface {
public:
    QString text(QAccessible::Text t) const override {
        QString text = baseInterface->text(t);
        // Aggiungere markup lingua specifico
        return QString("<speak xml:lang=\"it-IT\">%1</speak>").arg(text);
    }
};
```

### Soluzione C: Configurazione Avanzata VoiceOver
```cpp
// Forzare configurazione VoiceOver via AppleScript
QProcess::execute("osascript", QStringList() 
    << "-e" << "tell application \"VoiceOver Utility\" to set voice to \"Alice (italiano)\"");
```

### Soluzione D: Riavvio Servizi Accessibilità
```bash
# Script per riavviare servizi accessibilità
sudo launchctl unload /System/Library/LaunchDaemons/com.apple.accessibility.AXVisualSupportAgent.plist
sudo launchctl load /System/Library/LaunchDaemons/com.apple.accessibility.AXVisualSupportAgent.plist
```

## 📋 Piano di Azione Immediato

### Fase 1: Diagnosi Approfondita (Priorità ALTA)
1. **Verificare configurazione VoiceOver sistema**
   - Controllare voci italiane installate
   - Testare pronuncia italiana diretta con `say`
   - Verificare impostazioni VoiceOver

2. **Test comparativo con app native**
   - Creare semplice app Objective-C con attributi lingua
   - Confrontare comportamento VoiceOver
   - Identificare differenze implementazione

3. **Debug runtime Qt**
   - Aggiungere logging attributi accessibilità
   - Verificare propagazione configurazione locale
   - Controllare timing inizializzazione

### Fase 2: Implementazione Soluzioni Native (Se Fase 1 conferma problema Qt)
1. **Implementare bridge NSAccessibility nativo**
2. **Creare plugin accessibilità personalizzato**
3. **Aggiungere configurazione VoiceOver programmatica**

### Fase 3: Workaround Utente (Se soluzioni tecniche falliscono)
1. **Documentare configurazione manuale VoiceOver**
2. **Creare script configurazione automatica**
3. **Fornire istruzioni dettagliate utente finale**

## 🎯 Criteri di Successo

### ✅ Successo Completo
- VoiceOver pronuncia TUTTI gli elementi in italiano
- Configurazione automatica senza intervento utente
- Funzionamento su tutti i sistemi macOS supportati

### ⚠️ Successo Parziale
- VoiceOver pronuncia in italiano dopo configurazione manuale
- Script automatico per configurazione sistema
- Documentazione chiara per utenti

### ❌ Fallimento
- Impossibilità di ottenere pronuncia italiana
- Necessità configurazione complessa per ogni utilizzo
- Incompatibilità con VoiceOver standard

## 📝 Note per Sviluppo Futuro

### Considerazioni Architetturali
- **Qt Limitations**: Considerare migrazione a framework nativo per accessibilità
- **Platform Specific**: Implementare codice specifico macOS per accessibilità
- **User Experience**: Prioritizzare semplicità configurazione utente

### Documentazione Necessaria
- **Guida configurazione VoiceOver** per utenti finali
- **Troubleshooting accessibilità** per problemi comuni
- **Best practices** per sviluppo accessibile su macOS

---

**STATO**: 🔴 Problema ATTIVO e PERSISTENTE
**PRIORITÀ**: 🚨 CRITICA (accessibilità essenziale)
**PROSSIMO STEP**: Diagnosi approfondita configurazione sistema
**RESPONSABILE**: Sviluppatore + Tester con VoiceOver
**DEADLINE**: Risoluzione entro prossima release