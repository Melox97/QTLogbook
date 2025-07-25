#include <QApplication>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QTranslator>
#include <QLocale>
#include <QDialog>
#include "mainwindow.h"
#include "database.h"
#include "setupdialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Imposta informazioni applicazione
    app.setApplicationName("QT Logbook");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Ham Radio Software");
    app.setOrganizationDomain("hamradio.local");
    
    // Abilita accessibilità
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.setAttribute(Qt::AA_EnableHighDpiScaling);
    
    // Inizializza il database
    Database *db = Database::instance();
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dbPath);
    dbPath += "/logbook.db";
    
    if (!db->initialize(dbPath)) {
        QMessageBox::critical(nullptr, "Errore Database", 
                             "Impossibile inizializzare il database:\n" + db->lastError());
        return -1;
    }
    
    // Verifica se è necessario configurare l'operatore
    if (db->getOperatorCall().isEmpty()) {
        SetupDialog setupDialog;
        setupDialog.setWindowTitle("Configurazione Iniziale - QT Logbook");
        
        if (setupDialog.exec() == QDialog::Accepted) {
            // Salva i dati dell'operatore
            SetupDialog::OperatorData operatorData = setupDialog.getOperatorData();
            if (!db->setOperatorData(operatorData.callsign, operatorData.firstName,
                                   operatorData.lastName, operatorData.locator)) {
                QMessageBox::warning(nullptr, "Avviso", 
                                    "Impossibile salvare i dati dell'operatore. Puoi configurarli successivamente nelle impostazioni.");
            }
            
            // Salva le credenziali API
            SetupDialog::ApiCredentials apiCredentials = setupDialog.getApiCredentials();
            if (!db->setApiCredentials(apiCredentials.qrzUsername, apiCredentials.qrzPassword,
                                     apiCredentials.clublogApiKey, apiCredentials.enableQrz,
                                     apiCredentials.enableClublog)) {
                QMessageBox::warning(nullptr, "Avviso", 
                                    "Impossibile salvare le credenziali API. Puoi configurarle successivamente nelle impostazioni.");
            }
        } else {
            // L'utente ha annullato la configurazione
            return 0;
        }
    }
    
    // Crea e mostra la finestra principale
    MainWindow window;
    window.show();
    
    return app.exec();
}