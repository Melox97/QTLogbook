# Istruzioni di Installazione - QT Logbook

Questo documento fornisce istruzioni dettagliate per compilare e installare QT Logbook su diverse piattaforme.

## Prerequisiti

### Tutti i Sistemi
- Qt6 (versione 6.2 o superiore)
- Compilatore C++17 compatibile
- CMake 3.16+ oppure qmake

### macOS
```bash
# Installazione con Homebrew (raccomandato)
brew install qt6

# Aggiungi Qt6 al PATH
echo 'export PATH="/opt/homebrew/opt/qt6/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

### Ubuntu/Debian
```bash
# Installa Qt6 e strumenti di sviluppo
sudo apt update
sudo apt install qt6-base-dev qt6-tools-dev cmake build-essential

# Per supporto SQL e Network
sudo apt install qt6-base-dev-tools libqt6sql6-dev libqt6network6-dev
```

### Fedora/CentOS/RHEL
```bash
# Fedora
sudo dnf install qt6-qtbase-devel qt6-qttools-devel cmake gcc-c++

# CentOS/RHEL (con EPEL)
sudo yum install epel-release
sudo yum install qt6-qtbase-devel qt6-qttools-devel cmake gcc-c++
```

### Windows
1. Scaricare Qt6 dal sito ufficiale: https://www.qt.io/download
2. Installare Visual Studio 2019/2022 o MinGW
3. Installare CMake: https://cmake.org/download/
4. Aggiungere Qt6 e CMake al PATH di sistema

## Metodi di Compilazione

### Metodo 1: Script Automatico (Raccomandato)

#### Con CMake
```bash
./build.sh
```

#### Con qmake (Alternativo)
```bash
./build_qmake.sh
```

### Metodo 2: Compilazione Manuale con CMake

```bash
# Crea directory di build
mkdir build
cd build

# Configura il progetto
cmake ..

# Compila
cmake --build . --config Release

# Su sistemi multi-core, usa:
cmake --build . --config Release --parallel
```

### Metodo 3: Compilazione Manuale con qmake

```bash
# Crea directory di build
mkdir build_qmake
cd build_qmake

# Genera Makefile
qmake6 ../QTLogbook.pro
# oppure: qmake ../QTLogbook.pro

# Compila
make -j$(nproc)  # Linux
make -j$(sysctl -n hw.ncpu)  # macOS
mingw32-make  # Windows con MinGW
```

## Risoluzione Problemi

### Qt6 non trovato
```bash
# Verifica installazione Qt6
qmake6 --version
# oppure
qmake --version

# Se non funziona, aggiungi Qt6 al PATH:
# macOS (Homebrew)
export PATH="/opt/homebrew/opt/qt6/bin:$PATH"

# Linux (installazione standard)
export PATH="/usr/lib/qt6/bin:$PATH"

# Windows
# Aggiungi C:\Qt\6.x.x\msvc2019_64\bin al PATH di sistema
```

### Errori di Compilazione

#### Errore: "QtCore/QString file not found"
```bash
# Assicurati che Qt6 sia correttamente installato
# Prova a usare qmake invece di CMake:
./build_qmake.sh
```

#### Errore: "No matching constructor"
```bash
# Questo è normale durante lo sviluppo
# Il progetto è strutturato per essere compilato con Qt6
# Assicurati di avere Qt6, non Qt5
```

#### Errori di Linking
```bash
# Assicurati che tutti i moduli Qt6 siano installati:
# Ubuntu/Debian
sudo apt install qt6-base-dev qt6-tools-dev libqt6sql6-dev libqt6network6-dev

# macOS
brew install qt6

# Fedora
sudo dnf install qt6-qtbase-devel qt6-qttools-devel
```

### Problemi Runtime

#### "cannot find libQt6Core.so.6"
```bash
# Linux: Aggiungi le librerie Qt6 al LD_LIBRARY_PATH
export LD_LIBRARY_PATH="/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH"

# Oppure installa qt6-base-dev
sudo apt install qt6-base-dev
```

#### macOS: "QTLogbook.app is damaged"
```bash
# Rimuovi la quarantena
xattr -dr com.apple.quarantine QTLogbook.app
```

## Esecuzione

### Dopo la Compilazione

#### Linux
```bash
cd build
./QTLogbook
```

#### macOS
```bash
cd build
open QTLogbook.app
# oppure
./QTLogbook.app/Contents/MacOS/QTLogbook
```

#### Windows
```bash
cd build
.\QTLogbook.exe
# oppure
.\Release\QTLogbook.exe
```

## Prima Configurazione

1. Al primo avvio, l'applicazione chiederà di configurare il nominativo dell'operatore
2. Inserire il proprio nominativo radioamatoriale (es: IZ0ABC)
3. **Configurazione API** (opzionale ma raccomandata):
   - **QRZ.com**: Inserire username e password del proprio account QRZ.com
   - **Clublog.org**: Inserire la propria API key di Clublog.org
   - Se non configurate, verrà utilizzato il lookup locale dei prefissi
4. Il database SQLite verrà creato automaticamente in:
   - Linux: `~/.local/share/Ham Radio Software/QTLogbook/logbook.db`
   - macOS: `~/Library/Application Support/Ham Radio Software/QTLogbook/logbook.db`
   - Windows: `%APPDATA%\Ham Radio Software\QTLogbook\logbook.db`

### Ottenere le Credenziali API

#### QRZ.com
1. Registrarsi su https://www.qrz.com
2. Accedere al proprio account
3. Le credenziali sono username e password del proprio account
4. Nota: È richiesto un abbonamento XML per l'accesso API completo

#### Clublog.org
1. Registrarsi su https://clublog.org
2. Accedere al proprio account
3. Andare su "Settings" → "API Keys"
4. Generare una nuova API key per l'applicazione
5. Copiare la chiave generata

## Funzionalità

- **Gestione Contatti**: Aggiunta, modifica, eliminazione contatti
- **Database SQLite**: Archiviazione locale sicura
- **Lookup Automatico**: DXCC e locatore determinati automaticamente
- **Accessibilità**: Supporto completo per screen reader e navigazione tastiera
- **Multipiattaforma**: Funziona su Windows, macOS e Linux
- **Interfaccia Nativa**: Si adatta al tema del sistema operativo

## Supporto

Per problemi di compilazione o utilizzo:
1. Verificare che tutti i prerequisiti siano installati
2. Controllare la versione di Qt6 (deve essere 6.2+)
3. Provare entrambi i metodi di compilazione (CMake e qmake)
4. Consultare i log di errore per dettagli specifici

## Sviluppo

Per contribuire al progetto:
1. Fork del repository
2. Creare un branch per la feature
3. Testare su almeno due piattaforme
4. Assicurarsi che l'accessibilità sia mantenuta
5. Inviare una Pull Request