#include "mainwindow.h"
#include "settingsdialog.h"
#include <QMainWindow>
#include <QWidget>
#include <QOverload>
#include <QtCore/Qt>
#include "adifhandler.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTableView>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QTimer>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QAction>

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
    loadRSTData();
    
    // Chiama onModeChanged() per inizializzare i valori RST
    onModeChanged();
    
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
    
    // Imposta margini e spaziatura per un aspetto più moderno
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(15);
    
    // Titolo principale dell'applicazione (H1)
    QLabel *appTitleLabel = new QLabel("QT Logbook");
    appTitleLabel->setProperty("class", "h1");
    appTitleLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(appTitleLabel);
    
    // Titolo della sezione form (H2)
    QLabel *formTitleLabel = new QLabel("Inserimento Contatto");
    formTitleLabel->setProperty("class", "h2");
    m_mainLayout->addWidget(formTitleLabel);
    
    // Layout del form con spaziatura migliorata
    m_formLayout = new QGridLayout();
    m_formLayout->setHorizontalSpacing(12);
    m_formLayout->setVerticalSpacing(10);
    m_formLayout->setColumnStretch(1, 1); // La colonna dei campi si espande
    
    // Data e ora
    m_dateTimeLabel = new QLabel("Data/Ora UTC:");
    m_dateTimeLabel->setObjectName("fieldLabel");
    m_dateTimeEdit = new QLineEdit();
    m_dateTimeEdit->setReadOnly(true);
    m_dateTimeEdit->setAccessibleName("<span lang=\"it\">Campo data e ora UTC, sola lettura</span>");
    m_dateTimeEdit->setToolTip("Data e ora UTC del contatto (aggiornata automaticamente)");
    m_formLayout->addWidget(m_dateTimeLabel, 0, 0);
    m_formLayout->addWidget(m_dateTimeEdit, 0, 1);
    
    // Nominativo
    m_callsignLabel = new QLabel("Nominativo:");
    m_callsignLabel->setObjectName("fieldLabel");
    m_callsignEdit = new QLineEdit();
    m_callsignEdit->setPlaceholderText("Es: IZ0ABC");
    m_callsignEdit->setAccessibleName("<span lang=\"it\">Campo nominativo radioamatoriale</span>");
    m_callsignEdit->setAccessibleDescription("<span lang=\"it\">Inserire il nominativo della stazione contattata</span>");
    m_callsignEdit->setToolTip("Inserisci il nominativo della stazione contattata");
    m_callsignEdit->setObjectName("callsignEdit"); // Per stile specifico
    
    // Connetti il segnale per convertire automaticamente in maiuscolo
    connect(m_callsignEdit, &QLineEdit::textEdited, this, &MainWindow::onCallsignTextEdited);
    
    m_formLayout->addWidget(m_callsignLabel, 1, 0);
    m_formLayout->addWidget(m_callsignEdit, 1, 1);
    
    // Banda
    m_bandLabel = new QLabel("Banda:");
    m_bandLabel->setObjectName("fieldLabel");
    m_bandCombo = new QComboBox();
    m_bandCombo->setAccessibleName("<span lang=\"it\">Selezione banda di frequenza</span>");
    m_bandCombo->setToolTip("Seleziona la banda di frequenza utilizzata per il contatto");
    m_bandCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_bandCombo->setFixedWidth(80); // Larghezza fissa come per i campi RST
    m_bandCombo->setObjectName("dropdownField");
    m_formLayout->addWidget(m_bandLabel, 2, 0);
    m_formLayout->addWidget(m_bandCombo, 2, 1);
    
    // Modo
    m_modeLabel = new QLabel("Modo:");
    m_modeLabel->setObjectName("fieldLabel");
    m_modeCombo = new QComboBox();
    m_modeCombo->setAccessibleName("<span lang=\"it\">Selezione modo di trasmissione</span>");
    m_modeCombo->setToolTip("Seleziona il modo di trasmissione utilizzato per il contatto");
    m_modeCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    m_modeCombo->setFixedWidth(80); // Larghezza fissa come per i campi RST
    m_modeCombo->setObjectName("dropdownField");
    m_formLayout->addWidget(m_modeLabel, 3, 0);
    m_formLayout->addWidget(m_modeCombo, 3, 1);
    
    // RST inviato
    m_rstSentLabel = new QLabel("RST Inviato:");
    m_rstSentLabel->setObjectName("fieldLabel");
    m_rstSentCombo = new QComboBox();
    m_rstSentCombo->addItems({"599", "5NN", "5NNN"});
    m_rstSentCombo->setAccessibleName("<span lang=\"it\">Menu a tendina RST inviato</span>");
    m_rstSentCombo->setAccessibleDescription("<span lang=\"it\">Seleziona rapporto RST inviato per SSB/CW/DIGI</span>");
    m_rstSentCombo->setToolTip("Seleziona il rapporto RST inviato (599 per SSB, 5NN per CW, 5NNN per DIGI)");
    m_rstSentCombo->setFixedWidth(80);
    m_rstSentCombo->setObjectName("dropdownField");
    m_formLayout->addWidget(m_rstSentLabel, 4, 0);
    m_formLayout->addWidget(m_rstSentCombo, 4, 1);
    
    // RST ricevuto
    m_rstReceivedLabel = new QLabel("RST Ricevuto:");
    m_rstReceivedLabel->setObjectName("fieldLabel");
    m_rstReceivedCombo = new QComboBox();
    m_rstReceivedCombo->addItems({"599", "5NN", "5NNN"});
    m_rstReceivedCombo->setAccessibleName("<span lang=\"it\">Menu a tendina RST ricevuto</span>");
    m_rstReceivedCombo->setAccessibleDescription("<span lang=\"it\">Seleziona rapporto RST ricevuto per SSB/CW/DIGI</span>");
    m_rstReceivedCombo->setToolTip("Seleziona il rapporto RST ricevuto (599 per SSB, 5NN per CW, 5NNN per DIGI)");
    m_rstReceivedCombo->setFixedWidth(80);
    m_rstReceivedCombo->setObjectName("dropdownField");
    m_formLayout->addWidget(m_rstReceivedLabel, 5, 0);
    m_formLayout->addWidget(m_rstReceivedCombo, 5, 1);
    
    // DXCC
    m_dxccLabel = new QLabel("DXCC:");
    m_dxccLabel->setObjectName("fieldLabel");
    m_dxccEdit = new QLineEdit();
    m_dxccEdit->setReadOnly(true);
    m_dxccEdit->setAccessibleName("<span lang=\"it\">Campo DXCC, determinato automaticamente</span>");
    m_dxccEdit->setToolTip("Entità DXCC determinata automaticamente dal nominativo");
    m_dxccEdit->setObjectName("readOnlyField"); // Usa un nome oggetto per lo stile invece di hardcoded
    m_formLayout->addWidget(m_dxccLabel, 6, 0);
    m_formLayout->addWidget(m_dxccEdit, 6, 1);
    
    // Locatore
    m_locatorLabel = new QLabel("Locatore:");
    m_locatorLabel->setObjectName("fieldLabel");
    m_locatorEdit = new QLineEdit();
    m_locatorEdit->setReadOnly(true);
    m_locatorEdit->setAccessibleName("<span lang=\"it\">Campo locatore, determinato automaticamente</span>");
    m_locatorEdit->setToolTip("Locatore Maidenhead determinato automaticamente dal nominativo");
    m_locatorEdit->setObjectName("readOnlyField"); // Usa un nome oggetto per lo stile invece di hardcoded
    m_formLayout->addWidget(m_locatorLabel, 7, 0);
    m_formLayout->addWidget(m_locatorEdit, 7, 1);
    
    // Operatore
    m_operatorLabel = new QLabel("Operatore:");
    m_operatorLabel->setObjectName("fieldLabel");
    m_operatorEdit = new QLineEdit();
    m_operatorEdit->setReadOnly(true);
    m_operatorEdit->setAccessibleName("<span lang=\"it\">Campo operatore, configurato nelle impostazioni</span>");
    m_operatorEdit->setToolTip("Nominativo dell'operatore configurato nelle impostazioni");
    m_operatorEdit->setObjectName("readOnlyField"); // Usa un nome oggetto per lo stile invece di hardcoded
    m_formLayout->addWidget(m_operatorLabel, 8, 0);
    m_formLayout->addWidget(m_operatorEdit, 8, 1);
    
    m_mainLayout->addLayout(m_formLayout);
    
    // Pulsanti con stile migliorato
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->setSpacing(10);
    m_buttonLayout->setContentsMargins(0, 15, 0, 15); // Aggiungi spazio sopra e sotto i pulsanti
    
    m_addButton = new QPushButton("Aggiungi Contatto");
    m_addButton->setAccessibleName("<span lang=\"it\">Pulsante per aggiungere il contatto al logbook</span>");
    m_addButton->setToolTip("Aggiungi questo contatto al logbook");
    m_addButton->setMinimumWidth(150);
    m_addButton->setCursor(Qt::PointingHandCursor);
    
    m_clearButton = new QPushButton("Pulisci Form");
    m_clearButton->setAccessibleName("<span lang=\"it\">Pulsante per pulire tutti i campi del form</span>");
    m_clearButton->setToolTip("Pulisci tutti i campi del form");
    m_clearButton->setMinimumWidth(120);
    m_clearButton->setCursor(Qt::PointingHandCursor);
    
    m_buttonLayout->addWidget(m_addButton);
    m_buttonLayout->addWidget(m_clearButton);
    m_buttonLayout->addStretch();
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    // Titolo della sezione tabella (H2)
    QLabel *tableTitleLabel = new QLabel("Contatti Registrati");
    tableTitleLabel->setProperty("class", "h2");
    m_mainLayout->addWidget(tableTitleLabel);
    
    // Tabella contatti con stile migliorato
    m_contactsModel = new LogbookModel(this);
    m_contactsTable = new QTableView();
    m_contactsTable->setModel(m_contactsModel);
    m_contactsTable->setAlternatingRowColors(true);
    m_contactsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_contactsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_contactsTable->horizontalHeader()->setStretchLastSection(true);
    m_contactsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_contactsTable->verticalHeader()->setVisible(false); // Nascondi l'intestazione verticale
    m_contactsTable->setShowGrid(true);
    m_contactsTable->setGridStyle(Qt::SolidLine);
    m_contactsTable->setFrameShape(QFrame::NoFrame); // Rimuovi il bordo esterno
    m_contactsTable->setAccessibleName("<span lang=\"it\">Tabella dei contatti del logbook</span>");
    m_contactsTable->setAccessibleDescription("<span lang=\"it\">Tabella che mostra tutti i contatti registrati nel logbook</span>");
    
    m_mainLayout->addWidget(m_contactsTable);
    
    // Imposta il titolo della finestra e dimensioni ottimali
    setWindowTitle("QT Logbook - Logbook Radioamatoriale");
    resize(900, 700); // Dimensioni leggermente maggiori per una migliore visualizzazione
    
    // Imposta lo stile per i widget principali
    m_centralWidget->setObjectName("centralWidget");
    m_contactsTable->setObjectName("contactsTable");
    m_addButton->setObjectName("addButton");
    m_clearButton->setObjectName("clearButton");
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
    // Load bands from JSON file
    QJsonDocument bandsData = loadJsonFile(":/data/bands.json");
    if (!bandsData.isNull()) {
        QJsonArray bands = bandsData.object()["bands"].toArray();
        for (const QJsonValue &value : bands) {
            QJsonObject band = value.toObject();
            m_bandCombo->addItem(band["name"].toString());
        }
    } else {
        // Fallback if file is not available
        QStringList defaultBands = {"160m", "80m", "40m", "30m", "20m", "17m", "15m", "12m", "10m", "6m", "2m", "70cm"};
        m_bandCombo->addItems(defaultBands);
    }

    // Load modes from JSON file
    m_modesData = loadJsonFile(":/data/modes.json");
    if (!m_modesData.isNull()) {
        QJsonArray modes = m_modesData.object()["modes"].toArray();
        for (const QJsonValue &value : modes) {
            QJsonObject mode = value.toObject();
            m_modeCombo->addItem(mode["name"].toString());
        }
    } else {
        // Fallback if file is not available
        QStringList defaultModes = {"CW", "SSB", "FM", "FT8", "FT4", "PSK31", "RTTY", "JT65", "JT9"};
        m_modeCombo->addItems(defaultModes);
    }
}

void MainWindow::loadRSTData()
{
    m_rstData = loadJsonFile(":/data/rst.json");
}

QJsonDocument MainWindow::loadJsonFile(const QString &filePath)
{
    QFile loadFile(filePath);
    if (!loadFile.open(QIODevice::ReadOnly))
    {
        qWarning("Couldn't open json file: %s", qUtf8Printable(filePath));
        return QJsonDocument();
    }

    QByteArray allData = loadFile.readAll();
    loadFile.close();

    QJsonDocument jsonDoc(QJsonDocument::fromJson(allData));
    if (jsonDoc.isNull())
    {
        qWarning("Failed to create JSON document from file: %s", qUtf8Printable(filePath));
    }
    return jsonDoc;
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
        m_rstSentCombo->currentText(),
        m_rstReceivedCombo->currentText(),
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
    // Clear existing items
    m_rstSentCombo->clear();
    m_rstReceivedCombo->clear();

    // Determine RST values based on mode category
    QString category = "";

    if (!m_modesData.isNull()) {
        QJsonArray modes = m_modesData.object()["modes"].toArray();
        for (const QJsonValue &value : modes) {
            QJsonObject mode = value.toObject();
            if (mode["name"].toString() == currentMode) {
                category = mode["category"].toString();
                break;
            }
        }
    }

    QStringList rstValues;
    if (!m_rstData.isNull()) {
        QJsonObject rstCategories = m_rstData.object();
        
        // Mappatura specifica per modo
        if (currentMode == "CW") {
            // CW usa valori a 3 cifre
            QJsonObject cwRST = rstCategories["CW"].toObject();
            for (const QString &key : cwRST.keys()) {
                rstValues << key;
            }
        } else if (currentMode == "FT8" || currentMode == "FT4" || currentMode == "MSK144" || currentMode == "JT65" || currentMode == "JT9") {
            // Modi FT8/FT4 e derivati usano valori dB (FT)
            QJsonObject ftRST = rstCategories["FT"].toObject();
            for (const QString &key : ftRST.keys()) {
                rstValues << key;
            }
        } else if (category == "Voice" || currentMode == "AM" || currentMode == "FM" || 
                   currentMode == "SSB" || currentMode == "USB" || currentMode == "LSB") {
            // Modi vocali usano valori a 2 cifre (PHONE)
            QJsonObject phoneRST = rstCategories["PHONE"].toObject();
            for (const QString &key : phoneRST.keys()) {
                rstValues << key;
            }
        } else if (category == "Digital" && currentMode != "CW") {
            // Modi digitali (eccetto CW e FT) usano valori DIGI
            QJsonObject digiRST = rstCategories["DIGI"].toObject();
            for (const QString &key : digiRST.keys()) {
                rstValues << key;
            }
        }
    }

    if (rstValues.isEmpty()) {
        // Fallback if no specific RST values are found
        rstValues << "599" << "5NN" << "5NNN";
    }

    // Ordina i valori RST in modo logico
    rstValues.sort();

    m_rstSentCombo->addItems(rstValues);
    m_rstReceivedCombo->addItems(rstValues);
    
    // Imposta il valore predefinito più comune
    if (!rstValues.isEmpty()) {
        QString defaultValue;
        
        // Determina il valore predefinito in base agli standard radioamatoriali
        if (currentMode == "FT8" || currentMode == "FT4" || currentMode == "MSK144" || currentMode == "JT65" || currentMode == "JT9") {
            // Per FT8/FT4 e derivati, usa +00 (standard)
            defaultValue = "+00";
        } else if (currentMode == "CW") {
            // Per CW, usa 599 (standard)
            defaultValue = "599";
        } else if (category == "Voice" || currentMode == "AM" || currentMode == "FM" || 
                   currentMode == "SSB" || currentMode == "USB" || currentMode == "LSB") {
            // Per modi vocali, usa 59 (standard radioamatoriale)
            defaultValue = "59";
        } else if (category == "Digital") {
            // Per altri modi digitali, usa 599 (standard)
            defaultValue = "599";
        } else {
            // Fallback per modi non riconosciuti
            defaultValue = "59";
        }
        
        int index = rstValues.indexOf(defaultValue);
        if (index >= 0) {
            m_rstSentCombo->setCurrentIndex(index);
            m_rstReceivedCombo->setCurrentIndex(index);
        } else {
            // Se il valore predefinito non è disponibile, usa il primo valore
            m_rstSentCombo->setCurrentIndex(0);
            m_rstReceivedCombo->setCurrentIndex(0);
        }
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
        m_rstSentCombo->currentText(),
        m_rstReceivedCombo->currentText(),
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