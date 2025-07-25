#!/bin/bash

# Script di build alternativo per QT Logbook usando qmake
# Questo script dovrebbe funzionare meglio sui sistemi con Qt6 installato

set -e

echo "=== QT Logbook Build Script (qmake) ==="
echo

# Verifica se qmake è disponibile
QMAKE_CMD=""
if command -v qmake6 &> /dev/null; then
    QMAKE_CMD="qmake6"
elif command -v qmake &> /dev/null; then
    QMAKE_CMD="qmake"
else
    echo "Errore: qmake non trovato nel PATH"
    echo "Installare Qt6 e assicurarsi che qmake sia nel PATH"
    echo
    echo "macOS: brew install qt6 && export PATH=/opt/homebrew/opt/qt6/bin:\$PATH"
    echo "Ubuntu: sudo apt install qt6-base-dev qt6-tools-dev"
    echo "Windows: Scaricare da https://www.qt.io/download"
    exit 1
fi

echo "Usando qmake: $QMAKE_CMD"
echo

# Verifica la versione di Qt
echo "Versione Qt:"
$QMAKE_CMD --version
echo

# Crea directory di build
echo "Preparazione directory di build..."
if [ -d "build_qmake" ]; then
    echo "Directory build_qmake esistente, pulizia..."
    rm -rf build_qmake
fi
mkdir build_qmake
cd build_qmake

echo "Generazione Makefile con qmake..."
# Genera il Makefile
$QMAKE_CMD ../QTLogbook.pro

echo "Compilazione del progetto..."
# Compila il progetto
if [[ "$OSTYPE" == "darwin"* ]]; then
    # macOS
    make -j$(sysctl -n hw.ncpu)
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    # Linux
    make -j$(nproc)
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
    # Windows
    if command -v mingw32-make &> /dev/null; then
        mingw32-make -j$(nproc)
    elif command -v make &> /dev/null; then
        make -j$(nproc)
    else
        echo "Errore: make non trovato"
        exit 1
    fi
else
    echo "Sistema operativo non riconosciuto, tentativo make generico..."
    make
fi

echo
echo "=== Build completato con successo! ==="
echo
echo "Eseguibile creato in: $(pwd)"
echo

if [[ "$OSTYPE" == "darwin"* ]]; then
    if [ -d "QTLogbook.app" ]; then
        echo "Per eseguire l'applicazione:"
        echo "  open QTLogbook.app"
        echo "  oppure: ./QTLogbook.app/Contents/MacOS/QTLogbook"
    else
        echo "Per eseguire l'applicazione:"
        echo "  ./QTLogbook"
    fi
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "Per eseguire l'applicazione:"
    echo "  ./QTLogbook"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
    echo "Per eseguire l'applicazione:"
    if [ -f "release/QTLogbook.exe" ]; then
        echo "  ./release/QTLogbook.exe"
    elif [ -f "QTLogbook.exe" ]; then
        echo "  ./QTLogbook.exe"
    else
        echo "  Cercare QTLogbook.exe nella directory corrente"
    fi
else
    echo "Per eseguire l'applicazione, cercare l'eseguibile nella directory corrente"
fi

echo
echo "Nota: Al primo avvio sarà necessario configurare il nominativo dell'operatore."
echo "Nota: Se si verificano errori di runtime, assicurarsi che le librerie Qt6 siano nel PATH."