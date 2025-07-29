#include <QtWidgets/QApplication>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QMessageBox>
#include <QtCore/QTranslator>
#include <QtCore/QLocale>
#include <QtCore/QLibraryInfo>
#include <QtWidgets/QDialog>
#include <QtCore/QFile>
#include <QtWidgets/QStyle>
#include <QtWidgets/QStyleFactory>
#include <QtCore/QProcess>
#include <QtCore/QStringList>
#include <QtCore/QDebug>
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
    
    // Imposta il tema di base per una migliore integrazione
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // Inizializza il database prima di caricare il tema
    Database *db = Database::instance();
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dbPath);
    dbPath += "/logbook.db";
    
    if (!db->initialize(dbPath)) {
        QMessageBox::critical(nullptr, "Errore Database", 
                             "Impossibile inizializzare il database:\n" + db->lastError());
        return -1;
    }
    
    // Carica il tema in base alle impostazioni dell'utente
    QFile styleFile;
    QString stylePath;
    Database::ThemeMode themeMode = db->getThemeSettings();
    
    // Rileva il tema di sistema se è selezionato il tema automatico
    if (themeMode == Database::SystemTheme) {
        // Controlla se il sistema sta utilizzando un tema scuro
        bool isDarkMode = false;
        
#ifdef Q_OS_MACOS
        // Su macOS, controlla il valore di AppleInterfaceStyle
        QProcess process;
        process.start("defaults", QStringList() << "read" << "-g" << "AppleInterfaceStyle");
        process.waitForFinished();
        QString output = process.readAllStandardOutput().trimmed();
        isDarkMode = (output == "Dark");
#endif
        
        // Seleziona il tema appropriato in base al tema di sistema
        if (isDarkMode) {
            stylePath = ":/styles/dark_theme.qss";
            if (!QFile::exists(stylePath)) {
                stylePath = "styles/dark_theme.qss";
            }
        } else {
            stylePath = ":/styles/light_theme.qss";
            if (!QFile::exists(stylePath)) {
                stylePath = "styles/light_theme.qss";
            }
        }
    } else {
        // Usa il tema selezionato dall'utente
        switch (themeMode) {
            case Database::LightTheme:
                stylePath = ":/styles/light_theme.qss";
                if (!QFile::exists(stylePath)) {
                    stylePath = "styles/light_theme.qss";
                }
                break;
                
            case Database::DarkTheme:
                stylePath = ":/styles/dark_theme.qss";
                if (!QFile::exists(stylePath)) {
                    stylePath = "styles/dark_theme.qss";
                }
                break;
                
            case Database::HighContrastTheme:
                stylePath = ":/styles/high_contrast_theme.qss";
                if (!QFile::exists(stylePath)) {
                    stylePath = "styles/high_contrast_theme.qss";
                }
                break;
                
            default:
                // Fallback al tema moderno predefinito
                stylePath = ":/styles/modern_style.qss";
                if (!QFile::exists(stylePath)) {
                    stylePath = "styles/modern_style.qss";
                }
                break;
        }
    }
    
    // Applica il tema selezionato
    styleFile.setFileName(stylePath);
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        app.setStyleSheet(styleSheet);
        styleFile.close();
    } else {
        qWarning("Impossibile caricare il file di stile: %s", qPrintable(styleFile.errorString()));
        
        // Fallback al tema moderno predefinito
        QFile fallbackStyleFile(":/styles/modern_style.qss");
        if (fallbackStyleFile.open(QFile::ReadOnly)) {
            QString styleSheet = QLatin1String(fallbackStyleFile.readAll());
            app.setStyleSheet(styleSheet);
            fallbackStyleFile.close();
        } else {
            // Prova a caricare il file dalla directory locale
            QFile localStyleFile("styles/modern_style.qss");
            if (localStyleFile.open(QFile::ReadOnly)) {
                QString styleSheet = QLatin1String(localStyleFile.readAll());
                app.setStyleSheet(styleSheet);
                localStyleFile.close();
            } else {
                qWarning("Impossibile caricare il file di stile di fallback: %s", qPrintable(fallbackStyleFile.errorString()));
            }
        }
    }
    
    // Abilita accessibilità
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    
    // Configura localizzazione italiana per VoiceOver
    QLocale::setDefault(QLocale(QLocale::Italian, QLocale::Italy));
    
    // Imposta la lingua dell'applicazione per l'accessibilità
    QTranslator translator;
    QString locale = QLocale::system().name();
    
    // Forza l'uso dell'italiano se il sistema non è già configurato
    if (!locale.startsWith("it")) {
        locale = "it_IT";
    }
    
    // Carica le traduzioni Qt standard in italiano (se disponibili)
    QTranslator qtTranslator;
    if (qtTranslator.load("qt_" + locale, QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(&qtTranslator);
    }
    
    // Imposta attributi di accessibilità per VoiceOver
    app.setProperty("AA_MacDontSwapCtrlAndMeta", true);
    
    // Configura la lingua per i servizi di accessibilità
    QLocale italianLocale(QLocale::Italian, QLocale::Italy);
    QLocale::setDefault(italianLocale);
    
    // Imposta variabili d'ambiente per VoiceOver (macOS)
#ifdef Q_OS_MACOS
    qputenv("LANG", "it_IT.UTF-8");
    qputenv("LC_ALL", "it_IT.UTF-8");
#endif
    
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
    
    int result = app.exec();
    Database::destroy();
    return result;
}