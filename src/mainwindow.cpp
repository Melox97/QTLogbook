#include "mainwindow.h"
#include "settingsdialog.h"
#include "adifhandler.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableView>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QFileDialog>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QFile>
#include <QtCore/QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_database(Database::instance())
    , m_apiService(new ApiService(this))
    , m_dateTimeTimer(new QTimer(this))
{
    setupUI();
    setupMenuBar();
    setupStatusBar();
    loadBandsAndModes();
    
    // Connetti i segnali
    connect(m_addButton, &QPushButton::clicked, this, &MainWindow::onAddContact);
    connect(m_clearButton, &QPushButton::clicked, this, &MainWindow::onClearForm);
    connect(m_callsignEdit, &QLineEdit::textChanged, this, &MainWindow::onCallsignChanged);
    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onModeChanged);
    connect(m_apiService, &ApiService::callsignLookupFinished, this, &MainWindow::onCallsignLookupFinished);
    connect(m_apiService, &ApiService::callsignLookupError, this, &MainWindow::onCallsignLookupError);
    
    // Configure API service with saved credentials
    configureApiService();
    connect(m_dateTimeTimer, &QTimer::timeout, this, &MainWindow::updateDateTime);
    
    // Avvia il timer per aggiornare data/ora ogni minuto invece di ogni secondo
    // per ridurre l'interferenza con VoiceOver
    m_dateTimeTimer->start(60000); // 60 secondi invece di 1 secondo
    updateDateTime();
    
    // Verifica se l'operatore è configurato
    if (m_database->getOperatorCall().isEmpty()) {
        QTimer::singleShot(100, this, &MainWindow::onSettings);
    }
    
    // Carica i contatti esistenti
    updateContactsTable();
}

MainWindow::~MainWindow()
{
    // Ferma il timer quando l'applicazione viene chiusa
    if (m_dateTimeTimer && m_dateTimeTimer->isActive()) {
        m_dateTimeTimer->stop();
    }
}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    
    // Layout del form
    m_formLayout = new QGridLayout();
    
    // Data e ora
    m_dateTimeLabel = new QLabel("Data/Ora UTC:");
    m_dateTimeEdit = new QLineEdit();
    m_dateTimeEdit->setReadOnly(true);
    m_dateTimeEdit->setAccessibleName("<span lang=\"it\">Campo data e ora UTC, sola lettura</span>");
    m_formLayout->addWidget(m_dateTimeLabel, 0, 0);
    m_formLayout->addWidget(m_dateTimeEdit, 0, 1);
    
    // Nominativo
    m_callsignLabel = new QLabel("Nominativo:");
    m_callsignEdit = new QLineEdit();
    m_callsignEdit->setPlaceholderText("Es: IZ0ABC");
    m_callsignEdit->setAccessibleName("<span lang=\"it\">Campo nominativo radioamatoriale</span>");
    m_callsignEdit->setAccessibleDescription("<span lang=\"it\">Inserire il nominativo della stazione contattata</span>");
    
    // Connetti il segnale per convertire automaticamente in maiuscolo
    connect(m_callsignEdit, &QLineEdit::textEdited, this, &MainWindow::onCallsignTextEdited);
    
    m_formLayout->addWidget(m_callsignLabel, 1, 0);
    m_formLayout->addWidget(m_callsignEdit, 1, 1);
    
    // Banda
    m_bandLabel = new QLabel("Banda:");
    m_bandCombo = new QComboBox();
    m_bandCombo->setAccessibleName("<span lang=\"it\">Selezione banda di frequenza</span>");
    m_formLayout->addWidget(m_bandLabel, 2, 0);
    m_formLayout->addWidget(m_bandCombo, 2, 1);
    
    // Modo
    m_modeLabel = new QLabel("Modo:");
    m_modeCombo = new QComboBox();
    m_modeCombo->setAccessibleName("<span lang=\"it\">Selezione modo di trasmissione</span>");
    m_formLayout->addWidget(m_modeLabel, 3, 0);
    m_formLayout->addWidget(m_modeCombo, 3, 1);
    
    // RST inviato
    m_rstSentLabel = new QLabel("RST Inviato:");
    m_rstSentEdit = new QLineEdit();
    m_rstSentEdit->setPlaceholderText("599");
    m_rstSentEdit->setMaxLength(3);
    m_rstSentEdit->setAccessibleName("<span lang=\"it\">Campo RST inviato</span>");
    m_rstSentEdit->setAccessibleDescription("<span lang=\"it\">Rapporto RST inviato, 3 cifre</span>");
    m_formLayout->addWidget(m_rstSentLabel, 4, 0);
    m_formLayout->addWidget(m_rstSentEdit, 4, 1);
    
    // RST ricevuto
    m_rstReceivedLabel = new QLabel("RST Ricevuto:");
    m_rstReceivedEdit = new QLineEdit();
    m_rstReceivedEdit->setPlaceholderText("599");
    m_rstReceivedEdit->setMaxLength(3);
    m_rstReceivedEdit->setAccessibleName("<span lang=\"it\">Campo RST ricevuto</span>");
    m_rstReceivedEdit->setAccessibleDescription("<span lang=\"it\">Rapporto RST ricevuto, 3 cifre</span>");
    m_formLayout->addWidget(m_rstReceivedLabel, 5, 0);
    m_formLayout->addWidget(m_rstReceivedEdit, 5, 1);
    
    // DXCC
    m_dxccLabel = new QLabel("DXCC:");
    m_dxccEdit = new QLineEdit();
    m_dxccEdit->setReadOnly(true);
    m_dxccEdit->setAccessibleName("<span lang=\"it\">Campo DXCC, determinato automaticamente</span>");
    m_formLayout->addWidget(m_dxccLabel, 6, 0);
    m_formLayout->addWidget(m_dxccEdit, 6, 1);
    
    // Locatore
    m_locatorLabel = new QLabel("Locatore:");
    m_locatorEdit = new QLineEdit();
    m_locatorEdit->setReadOnly(true);
    m_locatorEdit->setAccessibleName("<span lang=\"it\">Campo locatore, determinato automaticamente</span>");
    m_formLayout->addWidget(m_locatorLabel, 7, 0);
    m_formLayout->addWidget(m_locatorEdit, 7, 1);
    
    // Operatore
    m_operatorLabel = new QLabel("Operatore:");
    m_operatorEdit = new QLineEdit();
    m_operatorEdit->setReadOnly(true);
    m_operatorEdit->setAccessibleName("<span lang=\"it\">Campo operatore, configurato nelle impostazioni</span>");
    m_formLayout->addWidget(m_operatorLabel, 8, 0);
    m_formLayout->addWidget(m_operatorEdit, 8, 1);
    
    m_mainLayout->addLayout(m_formLayout);
    
    // Pulsanti
    m_buttonLayout = new QHBoxLayout();
    m_addButton = new QPushButton("Aggiungi Contatto");
    m_addButton->setAccessibleName("<span lang=\"it\">Pulsante per aggiungere il contatto al logbook</span>");
    m_clearButton = new QPushButton("Pulisci Form");
    m_clearButton->setAccessibleName("<span lang=\"it\">Pulsante per pulire tutti i campi del form</span>");
    
    m_buttonLayout->addWidget(m_addButton);
    m_buttonLayout->addWidget(m_clearButton);
    m_buttonLayout->addStretch();
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    // Tabella contatti
    m_contactsModel = new LogbookModel(this);
    m_contactsTable = new QTableView();
    m_contactsTable->setModel(m_contactsModel);
    m_contactsTable->setAlternatingRowColors(true);
    m_contactsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_contactsTable->horizontalHeader()->setStretchLastSection(true);
    m_contactsTable->setAccessibleName("<span lang=\"it\">Tabella dei contatti del logbook</span>");
    
    m_mainLayout->addWidget(m_contactsTable);
    
    // Imposta il titolo della finestra
    setWindowTitle("QT Logbook - Logbook Radioamatoriale");
    resize(800, 600);
}

void MainWindow::setupMenuBar()
{
    // Menu File
    QMenu *fileMenu = menuBar()->addMenu("&File");
    
    // Azioni ADIF
    m_importADIFAction = new QAction("&Importa ADIF...", this);
    m_importADIFAction->setShortcut(QKeySequence("Ctrl+I"));
    connect(m_importADIFAction, &QAction::triggered, this, &MainWindow::onImportADIF);
    fileMenu->addAction(m_importADIFAction);
    
    m_exportADIFAction = new QAction("&Esporta ADIF...", this);
    m_exportADIFAction->setShortcut(QKeySequence("Ctrl+E"));
    connect(m_exportADIFAction, &QAction::triggered, this, &MainWindow::onExportADIF);
    fileMenu->addAction(m_exportADIFAction);
    
    fileMenu->addSeparator();
    
    m_exitAction = new QAction("&Esci", this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    connect(m_exitAction, &QAction::triggered, qApp, &QApplication::quit);
    fileMenu->addAction(m_exitAction);
    
    // Menu Strumenti
    QMenu *toolsMenu = menuBar()->addMenu("&Strumenti");
    
    m_settingsAction = new QAction("&Impostazioni", this);
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::onSettings);
    toolsMenu->addAction(m_settingsAction);
    
    // Menu Aiuto
    QMenu *helpMenu = menuBar()->addMenu("&Aiuto");
    
    m_aboutAction = new QAction("&Informazioni", this);
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
    helpMenu->addAction(m_aboutAction);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("Pronto");
}

void MainWindow::loadBandsAndModes()
{
    // Carica le bande dal file JSON
    QFile bandsFile(":/data/bands.json");
    if (bandsFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(bandsFile.readAll());
        QJsonArray bands = doc.object()["bands"].toArray();
        
        for (const QJsonValue &value : bands) {
            QJsonObject band = value.toObject();
            m_bandCombo->addItem(band["name"].toString());
        }
    } else {
        // Fallback se il file non è disponibile
        QStringList defaultBands = {"160m", "80m", "40m", "30m", "20m", "17m", "15m", "12m", "10m", "6m", "2m", "70cm"};
        m_bandCombo->addItems(defaultBands);
    }
    
    // Carica i modi dal file JSON
    QFile modesFile(":/data/modes.json");
    if (modesFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(modesFile.readAll());
        QJsonArray modes = doc.object()["modes"].toArray();
        
        for (const QJsonValue &value : modes) {
            QJsonObject mode = value.toObject();
            m_modeCombo->addItem(mode["name"].toString());
        }
    } else {
        // Fallback se il file non è disponibile
        QStringList defaultModes = {"CW", "SSB", "FM", "FT8", "FT4", "PSK31", "RTTY", "JT65", "JT9"};
        m_modeCombo->addItems(defaultModes);
    }
}

void MainWindow::onAddContact()
{
    if (!validateForm()) {
        return;
    }
    
    Contact contact(
        m_callsignEdit->text().toUpper(),
        m_bandCombo->currentText(),
        m_modeCombo->currentText(),
        m_rstSentEdit->text(),
        m_rstReceivedEdit->text(),
        m_database->getOperatorCall()
    );
    
    contact.setDxcc(m_dxccEdit->text());
    contact.setLocator(m_locatorEdit->text());
    
    if (m_database->addContact(contact)) {
        statusBar()->showMessage("Contatto aggiunto con successo", 3000);
        clearForm();
        updateContactsTable();
    } else {
        QMessageBox::critical(this, "Errore", 
                             "Errore durante l'aggiunta del contatto:\n" + m_database->lastError());
    }
}

void MainWindow::onClearForm()
{
    clearForm();
}

void MainWindow::onCallsignTextEdited(const QString &text)
{
    // Blocca temporaneamente i segnali per evitare ricorsione
    m_callsignEdit->blockSignals(true);
    
    // Converte automaticamente in maiuscolo
    QString upperText = text.toUpper();
    m_callsignEdit->setText(upperText);
    
    // Ripristina i segnali
    m_callsignEdit->blockSignals(false);
    
    // Trigger del lookup se il nominativo è abbastanza lungo
    if (upperText.length() >= 3) {
        m_apiService->lookupCallsign(upperText);
    } else {
        m_dxccEdit->clear();
        m_locatorEdit->clear();
    }
}

void MainWindow::onCallsignChanged()
{
    QString callsign = m_callsignEdit->text().toUpper();
    if (callsign.length() >= 3) {
        m_apiService->lookupCallsign(callsign);
    } else {
        m_dxccEdit->clear();
        m_locatorEdit->clear();
    }
}

void MainWindow::onCallsignLookupFinished(const QString &callsign, const QString &dxcc, const QString &locator)
{
    if (callsign.toUpper() == m_callsignEdit->text().toUpper()) {
        m_dxccEdit->setText(dxcc);
        m_locatorEdit->setText(locator);
    }
}

void MainWindow::onCallsignLookupError(const QString &callsign, const QString &error)
{
    Q_UNUSED(callsign)
    qDebug() << "Errore lookup nominativo:" << error;
}

void MainWindow::onModeChanged()
{
    QString currentMode = m_modeCombo->currentText();
    
    // Pre-valorizza RST in base al modo di modulazione
    // 59 per modi vocali (SSB, AM, FM, USB, LSB)
    // 599 per modi digitali e CW (RTTY, CW, PSK31, FT8, ecc.)
    if (currentMode == "SSB" || currentMode == "AM" || currentMode == "FM" || 
        currentMode == "USB" || currentMode == "LSB") {
        // Modi vocali: RST a 2 cifre (59)
        m_rstSentEdit->setText("59");
        m_rstReceivedEdit->setText("59");
        m_rstSentEdit->setPlaceholderText("59");
        m_rstReceivedEdit->setPlaceholderText("59");
    } else {
        // Modi digitali e CW: RST a 3 cifre (599)
        m_rstSentEdit->setText("599");
        m_rstReceivedEdit->setText("599");
        m_rstSentEdit->setPlaceholderText("599");
        m_rstReceivedEdit->setPlaceholderText("599");
    }
}

void MainWindow::updateDateTime()
{
    QDateTime utc = QDateTime::currentDateTimeUtc();
    m_dateTimeEdit->setText(utc.toString("yyyy-MM-dd hh:mm:ss"));
}

void MainWindow::onAbout()
{
    QMessageBox::about(this, "Informazioni su QT Logbook",
                      "<h3>QT Logbook v1.0.0</h3>"
                      "<p>Logbook radioamatoriale multipiattaforma</p>"
                      "<p>Sviluppato con Qt6 e SQLite3</p>"
                      "<p>Supporta accessibilità completa</p>"
                      "<p>&copy; 2024 Ham Radio Software</p>");
}

void MainWindow::onSettings()
{
    SettingsDialog settingsDialog(this);
    
    // Carica i dati correnti
    Database::OperatorData operatorData = m_database->getOperatorData();
    SettingsDialog::OperatorData dialogOperatorData;
    dialogOperatorData.callsign = operatorData.callsign;
    dialogOperatorData.firstName = operatorData.firstName;
    dialogOperatorData.lastName = operatorData.lastName;
    dialogOperatorData.locator = operatorData.locator;
    settingsDialog.setOperatorData(dialogOperatorData);
    
    Database::ApiCredentials apiCredentials = m_database->getApiCredentials();
    SettingsDialog::ApiCredentials dialogApiCredentials;
    dialogApiCredentials.enableQrz = apiCredentials.enableQrz;
    dialogApiCredentials.enableClublog = apiCredentials.enableClublog;
    dialogApiCredentials.qrzUsername = apiCredentials.qrzUsername;
    dialogApiCredentials.qrzPassword = apiCredentials.qrzPassword;
    dialogApiCredentials.clublogApiKey = apiCredentials.clublogApiKey;
    settingsDialog.setApiCredentials(dialogApiCredentials);
    
    if (settingsDialog.exec() == QDialog::Accepted) {
        // Salva i dati dell'operatore
        SettingsDialog::OperatorData newOperatorData = settingsDialog.getOperatorData();
        if (!m_database->setOperatorData(newOperatorData.callsign, newOperatorData.firstName,
                                       newOperatorData.lastName, newOperatorData.locator)) {
            QMessageBox::warning(this, "Avviso", 
                                "Impossibile salvare i dati dell'operatore. Errore: " + m_database->lastError());
        }
        
        // Salva le credenziali API
        SettingsDialog::ApiCredentials newApiCredentials = settingsDialog.getApiCredentials();
        if (!m_database->setApiCredentials(newApiCredentials.qrzUsername, newApiCredentials.qrzPassword,
                                         newApiCredentials.clublogApiKey, newApiCredentials.enableQrz,
                                         newApiCredentials.enableClublog)) {
            QMessageBox::warning(this, "Avviso", 
                                "Impossibile salvare le credenziali API. Errore: " + m_database->lastError());
        }
        
        // Ricarica la configurazione API
        configureApiService();
        
        // Aggiorna l'interfaccia
        updateContactsTable();
        
        statusBar()->showMessage("Impostazioni salvate con successo", 3000);
    }
}

void MainWindow::clearForm()
{
    m_callsignEdit->clear();
    m_dxccEdit->clear();
    m_locatorEdit->clear();
    
    // Imposta i valori RST in base al modo corrente
    onModeChanged();
    
    m_callsignEdit->setFocus();
}

bool MainWindow::validateForm()
{
    Contact contact(
        m_callsignEdit->text(),
        m_bandCombo->currentText(),
        m_modeCombo->currentText(),
        m_rstSentEdit->text(),
        m_rstReceivedEdit->text(),
        m_database->getOperatorCall()
    );
    
    if (!contact.isValid()) {
        showValidationError(contact.validationError());
        return false;
    }
    
    return true;
}

void MainWindow::showValidationError(const QString &message)
{
    QMessageBox::warning(this, "Errore di Validazione", message);
}

void MainWindow::updateContactsTable()
{
    QList<Contact> contacts = m_database->getAllContacts();
    m_contactsModel->setContacts(contacts);
    
    // Aggiorna la status bar
    int totalContacts = contacts.size();
    statusBar()->showMessage(QString("Contatti totali: %1").arg(totalContacts));
    
    // Aggiorna il campo operatore
    m_operatorEdit->setText(m_database->getOperatorCall());
}

void MainWindow::configureApiService()
{
    Database::ApiCredentials credentials = m_database->getApiCredentials();
    
    // Configure QRZ.com if enabled
    if (credentials.enableQrz && !credentials.qrzUsername.isEmpty() && !credentials.qrzPassword.isEmpty()) {
        m_apiService->setQrzCredentials(credentials.qrzUsername, credentials.qrzPassword);
    }
    
    // Configure Clublog.org if enabled
    if (credentials.enableClublog && !credentials.clublogApiKey.isEmpty()) {
        m_apiService->setClublogApiKey(credentials.clublogApiKey);
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ActivationChange) {
        if (isActiveWindow()) {
            resumeTimerForAccessibility();
        } else {
            pauseTimerForAccessibility();
        }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::focusInEvent(QFocusEvent *event)
{
    resumeTimerForAccessibility();
    QMainWindow::focusInEvent(event);
}

void MainWindow::focusOutEvent(QFocusEvent *event)
{
    pauseTimerForAccessibility();
    QMainWindow::focusOutEvent(event);
}

void MainWindow::pauseTimerForAccessibility()
{
    // Ferma il timer quando l'applicazione non ha il focus
    // per ridurre l'interferenza con VoiceOver
    if (m_dateTimeTimer && m_dateTimeTimer->isActive()) {
        m_dateTimeTimer->stop();
    }
}

void MainWindow::resumeTimerForAccessibility()
{
    // Riavvia il timer quando l'applicazione riacquista il focus
    if (m_dateTimeTimer && !m_dateTimeTimer->isActive()) {
        m_dateTimeTimer->start(60000); // 60 secondi
        updateDateTime(); // Aggiorna immediatamente
    }
}

void MainWindow::onImportADIF()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "Importa file ADIF", "", "File ADIF (*.adi *.adif);;Tutti i file (*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    ADIFHandler adifHandler;
    
    // Verifica che sia un file ADIF valido
    if (!adifHandler.isValidADIFFile(fileName)) {
        QMessageBox::warning(this, "Errore", 
                           "Il file selezionato non sembra essere un file ADIF valido.");
        return;
    }
    
    // Ottieni tutti i contatti esistenti per il controllo duplicati
    QList<Contact> existingContacts = m_database->getAllContacts();
    
    // Importa i contatti
    ADIFHandler::ImportResult result = adifHandler.importFromFile(fileName, existingContacts);
    
    if (!result.success) {
        QMessageBox::critical(this, "Errore Importazione", 
                             "Errore durante l'importazione:\n" + result.errorMessage);
        return;
    }
    
    // Aggiungi i contatti importati al database
    int importedCount = 0;
    for (Contact &contact : result.importedContacts) {
        // Imposta l'operatore corrente se non specificato
        if (contact.operatorCall().isEmpty()) {
            contact.setOperatorCall(m_database->getOperatorCall());
        }
        
        if (m_database->addContact(contact)) {
            importedCount++;
        }
    }
    
    // Genera report duplicati se necessario
    if (result.duplicatesFound > 0) {
        QString duplicateReportPath = QFileDialog::getSaveFileName(this,
            "Salva report duplicati", "DupeImport.adi", "File ADIF (*.adi *.adif)");
        
        if (!duplicateReportPath.isEmpty()) {
            adifHandler.generateDuplicateReport(duplicateReportPath, result.duplicateContacts, 
                                               m_database->getOperatorCall());
        }
    }
    
    // Aggiorna la tabella
    updateContactsTable();
    
    // Mostra risultato
    QString message = QString("Importazione completata:\n\n")
                     + QString("Record totali: %1\n").arg(result.totalRecords)
                     + QString("Contatti importati: %1\n").arg(importedCount)
                     + QString("Duplicati trovati: %1").arg(result.duplicatesFound);
    
    if (result.duplicatesFound > 0) {
        message += QString("\n\nI duplicati sono stati salvati nel report DupeImport.adi");
    }
    
    QMessageBox::information(this, "Importazione ADIF", message);
}

void MainWindow::onExportADIF()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "Esporta file ADIF", "logbook.adi", "File ADIF (*.adi *.adif);;Tutti i file (*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Ottieni tutti i contatti
    QList<Contact> contacts = m_database->getAllContacts();
    
    if (contacts.isEmpty()) {
        QMessageBox::information(this, "Esportazione ADIF", 
                               "Nessun contatto da esportare.");
        return;
    }
    
    ADIFHandler adifHandler;
    ADIFHandler::ExportResult result = adifHandler.exportToFile(fileName, contacts, 
                                                               m_database->getOperatorCall());
    
    if (!result.success) {
        QMessageBox::critical(this, "Errore Esportazione", 
                             "Errore durante l'esportazione:\n" + result.errorMessage);
        return;
    }
    
    QString message = QString("Esportazione completata:\n\n")
                     + QString("Contatti totali: %1\n").arg(result.totalRecords)
                     + QString("Contatti esportati: %1\n").arg(result.successfulExports)
                     + QString("\nFile salvato: %1").arg(fileName);
    
    QMessageBox::information(this, "Esportazione ADIF", message);
}