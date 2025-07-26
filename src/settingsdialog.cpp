#include "settingsdialog.h"
#include "database.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QInputDialog>
#include <QtCore/QDateTime>
#include <QtCore/QRegularExpression>
#include <QtGui/QRegularExpressionValidator>
#include "adifhandler.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Impostazioni - QT Logbook");
    setModal(true);
    resize(500, 400);
    
    setupUI();
    
    // Connetti i segnali
    connect(m_enableQrzCheck, &QCheckBox::toggled, this, &SettingsDialog::onQrzCredentialsChanged);
    connect(m_qrzUsernameEdit, &QLineEdit::textChanged, this, &SettingsDialog::onQrzCredentialsChanged);
    connect(m_qrzPasswordEdit, &QLineEdit::textChanged, this, &SettingsDialog::onQrzCredentialsChanged);
    connect(m_enableClublogCheck, &QCheckBox::toggled, this, &SettingsDialog::onClublogCredentialsChanged);
    connect(m_clublogApiKeyEdit, &QLineEdit::textChanged, this, &SettingsDialog::onClublogCredentialsChanged);
    connect(m_testQrzButton, &QPushButton::clicked, this, &SettingsDialog::onTestQrzConnection);
    connect(m_helpQrzButton, &QPushButton::clicked, this, &SettingsDialog::onHelpQrz);
    connect(m_testClublogButton, &QPushButton::clicked, this, &SettingsDialog::onTestClublogConnection);
    connect(m_helpClublogButton, &QPushButton::clicked, this, &SettingsDialog::onHelpClublog);
    connect(m_resetButton, &QPushButton::clicked, this, &SettingsDialog::onResetToDefaults);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::validateAndAccept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    // Aggiorna lo stato iniziale
    updateApiStatus();
}

bool SettingsDialog::createBackupBeforeReset()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        "Salva Backup Database", 
        QString("backup_logbook_%1.adi").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "File ADIF (*.adi *.adif);;Tutti i file (*)");
    
    if (fileName.isEmpty()) {
        return false; // Utente ha annullato
    }
    
    Database *db = Database::instance();
    QList<Contact> contacts = db->getAllContacts();
    
    if (contacts.isEmpty()) {
        QMessageBox::information(this, "Backup", "Nessun contatto da salvare nel backup.");
        return true;
    }
    
    ADIFHandler adifHandler;
    ADIFHandler::ExportResult result = adifHandler.exportToFile(fileName, contacts, db->getOperatorCall());
    
    if (!result.success) {
        QMessageBox::critical(this, "Errore Backup", 
                             "Errore durante la creazione del backup:\n" + result.errorMessage);
        return false;
    }
    
    QMessageBox::information(this, "Backup Completato", 
                           QString("Backup creato con successo:\n\n"
                                  "File: %1\n"
                                  "Contatti salvati: %2")
                                  .arg(fileName)
                                  .arg(result.successfulExports));
    
    return true;
}

void SettingsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    m_tabWidget = new QTabWidget();
    m_tabWidget->setAccessibleName("<span lang=\"it\">Schede impostazioni</span>");
    
    setupOperatorTab();
    setupApiTab();
    setupAdvancedTab();
    
    m_tabWidget->addTab(m_operatorTab, "Operatore");
    m_tabWidget->addTab(m_apiTab, "API");
    m_tabWidget->addTab(m_advancedTab, "Avanzate");
    
    mainLayout->addWidget(m_tabWidget);
    
    // Pulsanti
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_buttonBox->button(QDialogButtonBox::Ok)->setText("Salva");
    m_buttonBox->button(QDialogButtonBox::Cancel)->setText("Annulla");
    mainLayout->addWidget(m_buttonBox);
}

void SettingsDialog::setupOperatorTab()
{
    m_operatorTab = new QWidget();
    m_operatorLayout = new QGridLayout(m_operatorTab);
    
    // Nominativo
    m_callsignLabel = new QLabel("Nominativo *:");
    m_callsignEdit = new QLineEdit();
    m_callsignEdit->setPlaceholderText("Es: IZ0ABC");
    m_callsignEdit->setAccessibleName("<span lang=\"it\">Campo nominativo radioamatoriale obbligatorio</span>");
    
    // Rimuovo il validatore restrittivo per permettere l'inserimento di lettere
    // La validazione completa avverrà al momento del salvataggio
    
    // Connetti il segnale per convertire automaticamente in maiuscolo
    connect(m_callsignEdit, &QLineEdit::textEdited, this, &SettingsDialog::onCallsignTextEdited);
    
    m_operatorLayout->addWidget(m_callsignLabel, 0, 0);
    m_operatorLayout->addWidget(m_callsignEdit, 0, 1);
    
    // Nome
    m_firstNameLabel = new QLabel("Nome:");
    m_firstNameEdit = new QLineEdit();
    m_firstNameEdit->setPlaceholderText("Es: Mario");
    m_firstNameEdit->setAccessibleName("<span lang=\"it\">Campo nome dell'operatore</span>");
    
    m_operatorLayout->addWidget(m_firstNameLabel, 1, 0);
    m_operatorLayout->addWidget(m_firstNameEdit, 1, 1);
    
    // Cognome
    m_lastNameLabel = new QLabel("Cognome:");
    m_lastNameEdit = new QLineEdit();
    m_lastNameEdit->setPlaceholderText("Es: Rossi");
    m_lastNameEdit->setAccessibleName("<span lang=\"it\">Campo cognome dell'operatore</span>");
    
    m_operatorLayout->addWidget(m_lastNameLabel, 2, 0);
    m_operatorLayout->addWidget(m_lastNameEdit, 2, 1);
    
    // Locatore
    m_locatorLabel = new QLabel("Locatore:");
    m_locatorEdit = new QLineEdit();
    m_locatorEdit->setPlaceholderText("Es: JN45 o JN45AB");
    m_locatorEdit->setAccessibleName("<span lang=\"it\">Campo locatore Maidenhead</span>");
    
    // Validatore per locatore (4 o 6 caratteri)
    QRegularExpression locatorRegex("^[A-R]{2}[0-9]{2}([A-X]{2})?$");
    m_locatorEdit->setValidator(new QRegularExpressionValidator(locatorRegex, this));
    
    m_operatorLayout->addWidget(m_locatorLabel, 3, 0);
    m_operatorLayout->addWidget(m_locatorEdit, 3, 1);
    
    // Nota sui campi obbligatori
    QLabel *requiredNote = new QLabel("<i>* Campo obbligatorio</i>");
    requiredNote->setStyleSheet("color: #666; font-size: 10px;");
    m_operatorLayout->addWidget(requiredNote, 4, 0, 1, 2);
    
    m_operatorLayout->setRowStretch(5, 1);
}

void SettingsDialog::setupApiTab()
{
    m_apiTab = new QWidget();
    m_apiLayout = new QVBoxLayout(m_apiTab);
    
    // Gruppo QRZ.com
    m_qrzGroup = new QGroupBox("QRZ.com");
    m_qrzLayout = new QGridLayout(m_qrzGroup);
    
    m_enableQrzCheck = new QCheckBox("Abilita integrazione QRZ.com");
    m_enableQrzCheck->setAccessibleName("<span lang=\"it\">Checkbox per abilitare l'integrazione con QRZ.com</span>");
    m_qrzLayout->addWidget(m_enableQrzCheck, 0, 0, 1, 3);
    
    m_qrzUsernameLabel = new QLabel("Username:");
    m_qrzUsernameEdit = new QLineEdit();
    m_qrzUsernameEdit->setPlaceholderText("Il tuo username QRZ.com");
    m_qrzUsernameEdit->setAccessibleName("<span lang=\"it\">Campo username per QRZ.com</span>");
    
    m_qrzLayout->addWidget(m_qrzUsernameLabel, 1, 0);
    m_qrzLayout->addWidget(m_qrzUsernameEdit, 1, 1);
    
    m_qrzPasswordLabel = new QLabel("Password:");
    m_qrzPasswordEdit = new QLineEdit();
    m_qrzPasswordEdit->setEchoMode(QLineEdit::Password);
    m_qrzPasswordEdit->setPlaceholderText("La tua password QRZ.com");
    m_qrzPasswordEdit->setAccessibleName("<span lang=\"it\">Campo password per QRZ.com</span>");
    
    m_qrzLayout->addWidget(m_qrzPasswordLabel, 2, 0);
    m_qrzLayout->addWidget(m_qrzPasswordEdit, 2, 1);
    
    m_testQrzButton = new QPushButton("Test Connessione");
    m_testQrzButton->setAccessibleName("<span lang=\"it\">Pulsante per testare la connessione a QRZ.com</span>");
    m_helpQrzButton = new QPushButton("Aiuto");
    m_helpQrzButton->setAccessibleName("<span lang=\"it\">Pulsante per aprire l'aiuto su QRZ.com</span>");
    
    QHBoxLayout *qrzButtonLayout = new QHBoxLayout();
    qrzButtonLayout->addWidget(m_testQrzButton);
    qrzButtonLayout->addWidget(m_helpQrzButton);
    qrzButtonLayout->addStretch();
    
    m_qrzLayout->addLayout(qrzButtonLayout, 3, 0, 1, 2);
    
    m_qrzStatusLabel = new QLabel("Stato: Non configurato");
    m_qrzStatusLabel->setStyleSheet("color: #666; font-size: 10px;");
    m_qrzLayout->addWidget(m_qrzStatusLabel, 4, 0, 1, 2);
    
    m_apiLayout->addWidget(m_qrzGroup);
    
    // Gruppo Clublog.org
    m_clublogGroup = new QGroupBox("Clublog.org");
    m_clublogLayout = new QGridLayout(m_clublogGroup);
    
    m_enableClublogCheck = new QCheckBox("Abilita integrazione Clublog.org");
    m_enableClublogCheck->setAccessibleName("<span lang=\"it\">Checkbox per abilitare l'integrazione con Clublog.org</span>");
    m_clublogLayout->addWidget(m_enableClublogCheck, 0, 0, 1, 3);
    
    m_clublogApiKeyLabel = new QLabel("API Key:");
    m_clublogApiKeyEdit = new QLineEdit();
    m_clublogApiKeyEdit->setPlaceholderText("La tua API key di Clublog.org");
    m_clublogApiKeyEdit->setAccessibleName("<span lang=\"it\">Campo API key per Clublog.org</span>");
    
    m_clublogLayout->addWidget(m_clublogApiKeyLabel, 1, 0);
    m_clublogLayout->addWidget(m_clublogApiKeyEdit, 1, 1);
    
    m_testClublogButton = new QPushButton("Test Connessione");
    m_testClublogButton->setAccessibleName("<span lang=\"it\">Pulsante per testare la connessione a Clublog.org</span>");
    m_helpClublogButton = new QPushButton("Aiuto");
    m_helpClublogButton->setAccessibleName("<span lang=\"it\">Pulsante per aprire l'aiuto su Clublog.org</span>");
    
    QHBoxLayout *clublogButtonLayout = new QHBoxLayout();
    clublogButtonLayout->addWidget(m_testClublogButton);
    clublogButtonLayout->addWidget(m_helpClublogButton);
    clublogButtonLayout->addStretch();
    
    m_clublogLayout->addLayout(clublogButtonLayout, 2, 0, 1, 2);
    
    m_clublogStatusLabel = new QLabel("Stato: Non configurato");
    m_clublogStatusLabel->setStyleSheet("color: #666; font-size: 10px;");
    m_clublogLayout->addWidget(m_clublogStatusLabel, 3, 0, 1, 2);
    
    m_apiLayout->addWidget(m_clublogGroup);
    
    // Nota informativa
    QLabel *apiNote = new QLabel(
        "<b>Nota:</b> Le configurazioni API sono opzionali. Se non configurate, "
        "verrà utilizzato il lookup locale basato sui prefissi. "
        "Per ottenere dati più accurati, è consigliabile configurare almeno uno dei due servizi."
    );
    apiNote->setWordWrap(true);
    apiNote->setStyleSheet("color: #444; font-size: 11px; margin: 10px; padding: 10px; "
                          "background-color: #f0f0f0; border: 1px solid #ccc; border-radius: 5px;");
    m_apiLayout->addWidget(apiNote);
    
    m_apiLayout->addStretch();
}

void SettingsDialog::setupAdvancedTab()
{
    m_advancedTab = new QWidget();
    m_advancedLayout = new QVBoxLayout(m_advancedTab);
    
    // Gruppo Reset
    m_resetGroup = new QGroupBox("Reset Configurazione");
    m_resetLayout = new QVBoxLayout(m_resetGroup);
    
    m_resetWarningLabel = new QLabel(
        "<b>Attenzione:</b> Il reset cancellerà tutte le impostazioni e riporterà "
        "l'applicazione ai valori di fabbrica. Questa operazione non può essere annullata.\n\n"
        "<b>Importante:</b> I contatti salvati nel logbook NON verranno eliminati."
    );
    m_resetWarningLabel->setWordWrap(true);
    m_resetWarningLabel->setStyleSheet("color: #d32f2f; font-size: 11px; margin: 10px; padding: 10px; "
                                      "background-color: #ffebee; border: 1px solid #f44336; border-radius: 5px;");
    m_resetLayout->addWidget(m_resetWarningLabel);
    
    m_resetButton = new QPushButton("Reset ai Valori di Fabbrica");
    m_resetButton->setAccessibleName("<span lang=\"it\">Pulsante per resettare tutte le impostazioni ai valori di fabbrica</span>");
    m_resetButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; padding: 8px; }"
                                "QPushButton:hover { background-color: #d32f2f; }"
                                "QPushButton:pressed { background-color: #b71c1c; }");
    m_resetLayout->addWidget(m_resetButton);
    
    m_advancedLayout->addWidget(m_resetGroup);
    m_advancedLayout->addStretch();
}

void SettingsDialog::onCallsignTextEdited(const QString &text)
{
    // Blocca temporaneamente i segnali per evitare ricorsione
    m_callsignEdit->blockSignals(true);
    
    // Converte automaticamente in maiuscolo
    QString upperText = text.toUpper();
    m_callsignEdit->setText(upperText);
    
    // Ripristina i segnali
    m_callsignEdit->blockSignals(false);
}

void SettingsDialog::onQrzCredentialsChanged()
{
    updateApiStatus();
}

void SettingsDialog::onClublogCredentialsChanged()
{
    updateApiStatus();
}

void SettingsDialog::updateApiStatus()
{
    // Aggiorna stato QRZ.com
    bool qrzEnabled = m_enableQrzCheck->isChecked();
    m_qrzUsernameEdit->setEnabled(qrzEnabled);
    m_qrzPasswordEdit->setEnabled(qrzEnabled);
    m_qrzUsernameLabel->setEnabled(qrzEnabled);
    m_qrzPasswordLabel->setEnabled(qrzEnabled);
    m_testQrzButton->setEnabled(qrzEnabled);
    
    if (qrzEnabled) {
        bool hasCredentials = !m_qrzUsernameEdit->text().isEmpty() && !m_qrzPasswordEdit->text().isEmpty();
        if (hasCredentials) {
            m_qrzStatusLabel->setText("Stato: Configurato");
            m_qrzStatusLabel->setStyleSheet("color: green; font-size: 10px;");
        } else {
            m_qrzStatusLabel->setText("Stato: Credenziali incomplete");
            m_qrzStatusLabel->setStyleSheet("color: orange; font-size: 10px;");
        }
    } else {
        m_qrzStatusLabel->setText("Stato: Disabilitato");
        m_qrzStatusLabel->setStyleSheet("color: #666; font-size: 10px;");
    }
    
    // Aggiorna stato Clublog.org
    bool clublogEnabled = m_enableClublogCheck->isChecked();
    m_clublogApiKeyEdit->setEnabled(clublogEnabled);
    m_clublogApiKeyLabel->setEnabled(clublogEnabled);
    m_testClublogButton->setEnabled(clublogEnabled);
    
    if (clublogEnabled) {
        bool hasApiKey = !m_clublogApiKeyEdit->text().isEmpty();
        if (hasApiKey) {
            m_clublogStatusLabel->setText("Stato: Configurato");
            m_clublogStatusLabel->setStyleSheet("color: green; font-size: 10px;");
        } else {
            m_clublogStatusLabel->setText("Stato: API Key mancante");
            m_clublogStatusLabel->setStyleSheet("color: orange; font-size: 10px;");
        }
    } else {
        m_clublogStatusLabel->setText("Stato: Disabilitato");
        m_clublogStatusLabel->setStyleSheet("color: #666; font-size: 10px;");
    }
}

void SettingsDialog::onTestQrzConnection()
{
    if (m_qrzUsernameEdit->text().isEmpty() || m_qrzPasswordEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Test QRZ.com", 
                           "Inserire username e password prima di testare la connessione.");
        return;
    }
    
    QMessageBox::information(this, "Test QRZ.com", 
                           "Funzionalità di test non ancora implementata. "
                           "Le credenziali verranno verificate al primo utilizzo.");
}

void SettingsDialog::onTestClublogConnection()
{
    if (m_clublogApiKeyEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Test Clublog.org", 
                           "Inserire l'API key prima di testare la connessione.");
        return;
    }
    
    QMessageBox::information(this, "Test Clublog.org", 
                           "Funzionalità di test non ancora implementata. "
                           "L'API key verrà verificata al primo utilizzo.");
}

void SettingsDialog::onHelpQrz()
{
    QMessageBox::information(this, "Aiuto QRZ.com", 
                           "<h3>Come ottenere le credenziali QRZ.com:</h3>"
                           "<ol>"
                           "<li>Registrati su <a href='https://www.qrz.com'>www.qrz.com</a></li>"
                           "<li>Accedi al tuo account</li>"
                           "<li>Usa le stesse credenziali (username e password) qui</li>"
                           "<li><b>Nota:</b> Per l'accesso API completo è richiesto un abbonamento XML</li>"
                           "</ol>"
                           "<p>Senza abbonamento, alcune funzionalità potrebbero essere limitate.</p>");
}

void SettingsDialog::onHelpClublog()
{
    QMessageBox::information(this, "Aiuto Clublog.org", 
                           "<h3>Come ottenere l'API key di Clublog.org:</h3>"
                           "<ol>"
                           "<li>Registrati su <a href='https://clublog.org'>clublog.org</a></li>"
                           "<li>Accedi al tuo account</li>"
                           "<li>Vai su 'Settings' → 'API Keys'</li>"
                           "<li>Genera una nuova API key per questa applicazione</li>"
                           "<li>Copia e incolla la chiave qui</li>"
                           "</ol>"
                           "<p>Il servizio Clublog.org è gratuito per utenti registrati.</p>");
}

void SettingsDialog::onResetToDefaults()
{
    if (confirmReset()) {
        resetAllSettings();
        QMessageBox::information(this, "Reset Completato", 
                               "Le impostazioni sono state resettate ai valori di fabbrica. "
                               "L'applicazione verrà riavviata per applicare le modifiche.");
        accept();
    }
}

bool SettingsDialog::confirmReset()
{
    // Prima richiesta di conferma con opzione database
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Conferma Reset");
    msgBox.setText("Sei sicuro di voler resettare tutte le impostazioni ai valori di fabbrica?");
    msgBox.setDetailedText("Questa operazione cancellerà:\n"
                          "• Dati dell'operatore\n"
                          "• Credenziali API\n"
                          "• Tutte le configurazioni personalizzate\n\n"
                          "ATTENZIONE: Puoi anche scegliere di cancellare tutti i contatti del database.");
    
    QPushButton *resetSettingsBtn = msgBox.addButton("Solo Impostazioni", QMessageBox::AcceptRole);
    QPushButton *resetAllBtn = msgBox.addButton("Impostazioni + Database", QMessageBox::DestructiveRole);
    QPushButton *cancelBtn = msgBox.addButton("Annulla", QMessageBox::RejectRole);
    
    msgBox.setDefaultButton(cancelBtn);
    msgBox.exec();
    
    if (msgBox.clickedButton() == cancelBtn) {
        return false;
    }
    
    bool resetDatabase = (msgBox.clickedButton() == resetAllBtn);
    
    // Se si vuole resettare il database, chiedi backup
    if (resetDatabase) {
        Database *db = Database::instance();
        int contactCount = db->getTotalContacts();
        
        if (contactCount > 0) {
            int backupResult = QMessageBox::question(this, "Backup Database",
                QString("Il database contiene %1 contatti.\n\n"
                       "Vuoi creare un backup in formato ADIF prima di cancellare tutto?")
                       .arg(contactCount),
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                QMessageBox::Yes);
            
            if (backupResult == QMessageBox::Cancel) {
                return false;
            }
            
            if (backupResult == QMessageBox::Yes) {
                if (!createBackupBeforeReset()) {
                    return false;
                }
            }
        }
    }
    
    // Salva la scelta per il reset del database
    m_resetDatabase = resetDatabase;
    
    int result = QMessageBox::Yes; // Procedi con la conferma identità
    
    if (result != QMessageBox::Yes) {
        return false;
    }
    
    // Seconda conferma con inserimento nominativo
    Database *db = Database::instance();
    QString currentCallsign = db->getOperatorCall();
    
    if (currentCallsign.isEmpty()) {
        QMessageBox::warning(this, "Errore", "Impossibile determinare il nominativo dell'operatore corrente.");
        return false;
    }
    
    bool ok;
    QString confirmCallsign = QInputDialog::getText(this, "Conferma Identità",
                                                   QString("Per confermare il reset, inserisci il tuo nominativo corrente:\n(%1)")
                                                   .arg(currentCallsign),
                                                   QLineEdit::Normal, "", &ok);
    
    if (!ok || confirmCallsign.isEmpty()) {
        return false;
    }
    
    if (confirmCallsign.toUpper() != currentCallsign.toUpper()) {
        QMessageBox::critical(this, "Errore Conferma", 
                             "Il nominativo inserito non corrisponde a quello dell'operatore corrente. "
                             "Reset annullato per sicurezza.");
        return false;
    }
    
    return true;
}

void SettingsDialog::resetAllSettings()
{
    Database *db = Database::instance();
    
    // Reset delle impostazioni nel database
    db->clearAllSettings();
    
    // Reset del database se richiesto
    if (m_resetDatabase) {
        db->clearAllContacts();
    }
    
    // Reset dei campi del dialog
    m_callsignEdit->clear();
    m_firstNameEdit->clear();
    m_lastNameEdit->clear();
    m_locatorEdit->clear();
    
    m_enableQrzCheck->setChecked(false);
    m_qrzUsernameEdit->clear();
    m_qrzPasswordEdit->clear();
    
    m_enableClublogCheck->setChecked(false);
    m_clublogApiKeyEdit->clear();
    
    updateApiStatus();
}

bool SettingsDialog::validateOperatorData() const
{
    if (m_callsignEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(const_cast<SettingsDialog*>(this), "Errore Validazione", 
                           "Il nominativo dell'operatore è obbligatorio.");
        return false;
    }
    
    // Validazione formato nominativo
    QString callsign = m_callsignEdit->text().trimmed().toUpper();
    QRegularExpression regex("^[A-Z0-9]{1,3}[0-9][A-Z0-9]{0,3}[A-Z]$");
    if (!regex.match(callsign).hasMatch() || callsign.length() < 3 || callsign.length() > 10) {
        QMessageBox::warning(const_cast<SettingsDialog*>(this), "Errore Validazione", 
                           "Il formato del nominativo non è valido. "
                           "Deve essere un nominativo radioamatoriale valido (es: IZ0ABC).");
        return false;
    }
    
    // Validazione locatore se presente
    QString locator = m_locatorEdit->text().trimmed().toUpper();
    if (!locator.isEmpty()) {
        QRegularExpression locatorRegex("^[A-R]{2}[0-9]{2}([A-X]{2})?$");
        if (!locatorRegex.match(locator).hasMatch()) {
            QMessageBox::warning(const_cast<SettingsDialog*>(this), "Errore Validazione", 
                               "Il formato del locatore non è valido. "
                               "Deve essere nel formato Maidenhead a 4 o 6 caratteri (es: JN45 o JN45AB).");
            return false;
        }
    }
    
    return true;
}

bool SettingsDialog::validateApiCredentials() const
{
    // Validazione QRZ.com se abilitato
    if (m_enableQrzCheck->isChecked()) {
        if (m_qrzUsernameEdit->text().trimmed().isEmpty() || m_qrzPasswordEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(const_cast<SettingsDialog*>(this), "Errore Validazione", 
                               "Se QRZ.com è abilitato, username e password sono obbligatori.");
            return false;
        }
    }
    
    // Validazione Clublog.org se abilitato
    if (m_enableClublogCheck->isChecked()) {
        if (m_clublogApiKeyEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(const_cast<SettingsDialog*>(this), "Errore Validazione", 
                               "Se Clublog.org è abilitato, l'API key è obbligatoria.");
            return false;
        }
    }
    
    return true;
}

void SettingsDialog::validateAndAccept()
{
    if (!validateOperatorData() || !validateApiCredentials()) {
        return;
    }
    
    accept();
}

SettingsDialog::OperatorData SettingsDialog::getOperatorData() const
{
    OperatorData data;
    data.callsign = m_callsignEdit->text().trimmed().toUpper();
    data.firstName = m_firstNameEdit->text().trimmed();
    data.lastName = m_lastNameEdit->text().trimmed();
    data.locator = m_locatorEdit->text().trimmed().toUpper();
    return data;
}

SettingsDialog::ApiCredentials SettingsDialog::getApiCredentials() const
{
    ApiCredentials credentials;
    credentials.enableQrz = m_enableQrzCheck->isChecked();
    credentials.enableClublog = m_enableClublogCheck->isChecked();
    
    if (credentials.enableQrz) {
        credentials.qrzUsername = m_qrzUsernameEdit->text().trimmed();
        credentials.qrzPassword = m_qrzPasswordEdit->text().trimmed();
    }
    
    if (credentials.enableClublog) {
        credentials.clublogApiKey = m_clublogApiKeyEdit->text().trimmed();
    }
    
    return credentials;
}

void SettingsDialog::setOperatorData(const OperatorData &data)
{
    m_callsignEdit->setText(data.callsign);
    m_firstNameEdit->setText(data.firstName);
    m_lastNameEdit->setText(data.lastName);
    m_locatorEdit->setText(data.locator);
}

void SettingsDialog::setApiCredentials(const ApiCredentials &credentials)
{
    m_enableQrzCheck->setChecked(credentials.enableQrz);
    m_enableClublogCheck->setChecked(credentials.enableClublog);
    
    m_qrzUsernameEdit->setText(credentials.qrzUsername);
    m_qrzPasswordEdit->setText(credentials.qrzPassword);
    m_clublogApiKeyEdit->setText(credentials.clublogApiKey);
    
    updateApiStatus();
}