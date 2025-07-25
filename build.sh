#!/bin/bash

# Script di build per QT Logbook
# Supporta macOS, Linux e Windows (con MSYS2/MinGW)

set -e

echo "=== QT Logbook Build Script ==="
echo

# Verifica se Qt6 è installato
if ! command -v qmake6 &> /dev/null && ! command -v qmake &> /dev/null; then
    echo "Errore: Qt6 non trovato nel PATH"
    echo "Installare Qt6 e assicurarsi che sia nel PATH"
    echo
    echo "macOS: brew install qt6"
    echo "Ubuntu: sudo apt install qt6-base-dev qt6-tools-dev"
    echo "Windows: Scaricare da https://www.qt.io/download"
    exit 1
fi

# Verifica se CMake è installato
if ! command -v cmake &> /dev/null; then
    echo "Errore: CMake non trovato nel PATH"
    echo "Installare CMake:"
    echo "macOS: brew install cmake"
    echo "Ubuntu: sudo apt install cmake"
    echo "Windows: Scaricare da https://cmake.org/download/"
    exit 1
fi

# Crea directory di build
echo "Creazione directory di build..."
if [ -d "build" ]; then
    echo "Directory build esistente, pulizia..."
    rm -rf build
fi
mkdir build
cd build

echo "Configurazione del progetto con CMake..."
# Configura il progetto
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    echo "Rilevato macOS"
    cmake -DCMAKE_BUILD_TYPE=Release ..
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    echo "Rilevato Linux"
    cmake -DCMAKE_BUILD_TYPE=Release ..
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
    # Windows
    echo "Rilevato Windows"
    cmake -DCMAKE_BUILD_TYPE=Release -G "MinGW Makefiles" ..
else
    echo "Sistema operativo non riconosciuto, tentativo configurazione generica..."
    cmake -DCMAKE_BUILD_TYPE=Release ..
fi

echo "Compilazione del progetto..."
# Compila il progetto
cmake --build . --config Release

echo
echo "=== Build completato con successo! ==="
echo
echo "Eseguibile creato in: $(pwd)"
echo

if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "Per eseguire l'applicazione:"
    echo "  ./QTLogbook.app/Contents/MacOS/QTLogbook"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "Per eseguire l'applicazione:"
    echo "  ./QTLogbook"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
    echo "Per eseguire l'applicazione:"
    echo "  ./QTLogbook.exe"
else
    echo "Per eseguire l'applicazione, cercare l'eseguibile nella directory corrente"
fi

echo
echo "Nota: Al primo avvio sarà necessario configurare il nominativo dell'operatore."