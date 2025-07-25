#include "setupdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>

SetupDialog::SetupDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Configurazione Iniziale - QT Logbook");
    setModal(true);
    resize(500, 400);
    
    setupUI();
    
    // Connetti i segnali
    connect(m_enableQrzCheck, &QCheckBox::toggled, this, &SetupDialog::onQrzCredentialsChanged);
    connect(m_qrzUsernameEdit, &QLineEdit::textChanged, this, &SetupDialog::onQrzCredentialsChanged);
    connect(m_qrzPasswordEdit, &QLineEdit::textChanged, this, &SetupDialog::onQrzCredentialsChanged);
    connect(m_enableClublogCheck, &QCheckBox::toggled, this, &SetupDialog::onClublogCredentialsChanged);
    connect(m_clublogApiKeyEdit, &QLineEdit::textChanged, this, &SetupDialog::onClublogCredentialsChanged);
    
    connect(m_testQrzButton, &QPushButton::clicked, this, &SetupDialog::onTestQrzConnection);
    connect(m_testClublogButton, &QPushButton::clicked, this, &SetupDialog::onTestClublogConnection);
    connect(m_helpQrzButton, &QPushButton::clicked, this, &SetupDialog::onHelpQrz);
    connect(m_helpClublogButton, &QPushButton::clicked, this, &SetupDialog::onHelpClublog);
    
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SetupDialog::validateAndAccept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    // Aggiorna lo stato iniziale
    updateApiStatus();
}

void SetupDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Tab widget principale
    m_tabWidget = new QTabWidget();
    
    setupOperatorTab();
    setupApiTab();
    
    m_tabWidget->addTab(m_operatorTab, "Dati Operatore");
    m_tabWidget->addTab(m_apiTab, "Configurazione API");
    
    mainLayout->addWidget(m_tabWidget);
    
    // Nota informativa
    m_infoLabel = new QLabel(
        "<i>Nota: I dati dell'operatore sono obbligatori. "
        "Le configurazioni API sono opzionali ma raccomandate per lookup automatici.</i>"
    );
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setStyleSheet("color: #666; font-size: 10px; margin: 5px;");
    mainLayout->addWidget(m_infoLabel);
    
    // Pulsanti dialog
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(m_buttonBox);
}

void SetupDialog::setupOperatorTab()
{
    m_operatorTab = new QWidget();
    m_operatorLayout = new QGridLayout(m_operatorTab);
    
    // Nominativo (obbligatorio)
    m_callsignLabel = new QLabel("Nominativo *:");
    m_callsignEdit = new QLineEdit();
    m_callsignEdit->setPlaceholderText("Es: IZ0ABC");
    m_callsignEdit->setAccessibleName("Campo nominativo radioamatoriale obbligatorio");
    
    // Validatore per nominativo
    QRegularExpression callsignRegex("^[A-Z0-9]{1,10}$");
    m_callsignEdit->setValidator(new QRegularExpressionValidator(callsignRegex, this));
    
    m_operatorLayout->addWidget(m_callsignLabel, 0, 0);
    m_operatorLayout->addWidget(m_callsignEdit, 0, 1);
    
    // Nome
    m_firstNameLabel = new QLabel("Nome:");
    m_firstNameEdit = new QLineEdit();
    m_firstNameEdit->setPlaceholderText("Es: Mario");
    m_firstNameEdit->setAccessibleName("Campo nome dell'operatore");
    
    m_operatorLayout->addWidget(m_firstNameLabel, 1, 0);
    m_operatorLayout->addWidget(m_firstNameEdit, 1, 1);
    
    // Cognome
    m_lastNameLabel = new QLabel("Cognome:");
    m_lastNameEdit = new QLineEdit();
    m_lastNameEdit->setPlaceholderText("Es: Rossi");
    m_lastNameEdit->setAccessibleName("Campo cognome dell'operatore");
    
    m_operatorLayout->addWidget(m_lastNameLabel, 2, 0);
    m_operatorLayout->addWidget(m_lastNameEdit, 2, 1);
    
    // Locatore
    m_locatorLabel = new QLabel("Locatore:");
    m_locatorEdit = new QLineEdit();
    m_locatorEdit->setPlaceholderText("Es: JN45AB");
    m_locatorEdit->setAccessibleName("Campo locatore Maidenhead");
    
    // Validatore per locatore
    QRegularExpression locatorRegex("^[A-R]{2}[0-9]{2}[A-X]{2}$");
    m_locatorEdit->setValidator(new QRegularExpressionValidator(locatorRegex, this));
    
    m_operatorLayout->addWidget(m_locatorLabel, 3, 0);
    m_operatorLayout->addWidget(m_locatorEdit, 3, 1);
    
    // Nota sui campi obbligatori
    QLabel *requiredNote = new QLabel("<i>* Campo obbligatorio</i>");
    requiredNote->setStyleSheet("color: #666; font-size: 10px;");
    m_operatorLayout->addWidget(requiredNote, 4, 0, 1, 2);
    
    m_operatorLayout->setRowStretch(5, 1);
}

void SetupDialog::setupApiTab()
{
    m_apiTab = new QWidget();
    m_apiLayout = new QVBoxLayout(m_apiTab);
    
    // Gruppo QRZ.com
    m_qrzGroup = new QGroupBox("QRZ.com");
    m_qrzLayout = new QGridLayout(m_qrzGroup);
    
    m_enableQrzCheck = new QCheckBox("Abilita integrazione QRZ.com");
    m_enableQrzCheck->setAccessibleName("Checkbox per abilitare l'integrazione con QRZ.com");
    m_qrzLayout->addWidget(m_enableQrzCheck, 0, 0, 1, 3);
    
    m_qrzUsernameLabel = new QLabel("Username:");
    m_qrzUsernameEdit = new QLineEdit();
    m_qrzUsernameEdit->setPlaceholderText("Il tuo username QRZ.com");
    m_qrzUsernameEdit->setAccessibleName("Campo username per QRZ.com");
    
    m_qrzLayout->addWidget(m_qrzUsernameLabel, 1, 0);
    m_qrzLayout->addWidget(m_qrzUsernameEdit, 1, 1);
    
    m_qrzPasswordLabel = new QLabel("Password:");
    m_qrzPasswordEdit = new QLineEdit();
    m_qrzPasswordEdit->setEchoMode(QLineEdit::Password);
    m_qrzPasswordEdit->setPlaceholderText("La tua password QRZ.com");
    m_qrzPasswordEdit->setAccessibleName("Campo password per QRZ.com");
    
    m_qrzLayout->addWidget(m_qrzPasswordLabel, 2, 0);
    m_qrzLayout->addWidget(m_qrzPasswordEdit, 2, 1);
    
    m_testQrzButton = new QPushButton("Test Connessione");
    m_testQrzButton->setAccessibleName("Pulsante per testare la connessione a QRZ.com");
    m_helpQrzButton = new QPushButton("Aiuto");
    m_helpQrzButton->setAccessibleName("Pulsante per aprire l'aiuto su QRZ.com");
    
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
    m_enableClublogCheck->setAccessibleName("Checkbox per abilitare l'integrazione con Clublog.org");
    m_clublogLayout->addWidget(m_enableClublogCheck, 0, 0, 1, 3);
    
    m_clublogApiKeyLabel = new QLabel("API Key:");
    m_clublogApiKeyEdit = new QLineEdit();
    m_clublogApiKeyEdit->setPlaceholderText("La tua API key di Clublog.org");
    m_clublogApiKeyEdit->setAccessibleName("Campo API key per Clublog.org");
    
    m_clublogLayout->addWidget(m_clublogApiKeyLabel, 1, 0);
    m_clublogLayout->addWidget(m_clublogApiKeyEdit, 1, 1);
    
    m_testClublogButton = new QPushButton("Test Connessione");
    m_testClublogButton->setAccessibleName("Pulsante per testare la connessione a Clublog.org");
    m_helpClublogButton = new QPushButton("Aiuto");
    m_helpClublogButton->setAccessibleName("Pulsante per aprire l'aiuto su Clublog.org");
    
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

void SetupDialog::onQrzCredentialsChanged()
{
    updateApiStatus();
}

void SetupDialog::onClublogCredentialsChanged()
{
    updateApiStatus();
}

void SetupDialog::updateApiStatus()
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

void SetupDialog::onTestQrzConnection()
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

void SetupDialog::onTestClublogConnection()
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

void SetupDialog::onHelpQrz()
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

void SetupDialog::onHelpClublog()
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

bool SetupDialog::validateOperatorData() const
{
    if (m_callsignEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(const_cast<SetupDialog*>(this), "Errore Validazione", 
                           "Il nominativo dell'operatore è obbligatorio.");
        return false;
    }
    
    // Validazione formato nominativo
    QString callsign = m_callsignEdit->text().trimmed().toUpper();
    QRegularExpression regex("^[A-Z0-9]{1,3}[0-9][A-Z0-9]{0,3}[A-Z]$");
    if (!regex.match(callsign).hasMatch() || callsign.length() < 3 || callsign.length() > 10) {
        QMessageBox::warning(const_cast<SetupDialog*>(this), "Errore Validazione", 
                           "Il formato del nominativo non è valido. "
                           "Deve essere un nominativo radioamatoriale valido (es: IZ0ABC).");
        return false;
    }
    
    // Validazione locatore se presente
    QString locator = m_locatorEdit->text().trimmed().toUpper();
    if (!locator.isEmpty()) {
        QRegularExpression locatorRegex("^[A-R]{2}[0-9]{2}([A-X]{2})?$");
        if (!locatorRegex.match(locator).hasMatch()) {
            QMessageBox::warning(const_cast<SetupDialog*>(this), "Errore Validazione", 
                               "Il formato del locatore non è valido. "
                               "Deve essere nel formato Maidenhead (es: JN45AB).");
            return false;
        }
    }
    
    return true;
}

bool SetupDialog::validateApiCredentials() const
{
    // Validazione QRZ.com se abilitato
    if (m_enableQrzCheck->isChecked()) {
        if (m_qrzUsernameEdit->text().trimmed().isEmpty() || m_qrzPasswordEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(const_cast<SetupDialog*>(this), "Errore Validazione", 
                               "Se QRZ.com è abilitato, username e password sono obbligatori.");
            return false;
        }
    }
    
    // Validazione Clublog.org se abilitato
    if (m_enableClublogCheck->isChecked()) {
        if (m_clublogApiKeyEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(const_cast<SetupDialog*>(this), "Errore Validazione", 
                               "Se Clublog.org è abilitato, l'API key è obbligatoria.");
            return false;
        }
    }
    
    return true;
}

void SetupDialog::validateAndAccept()
{
    if (!validateOperatorData() || !validateApiCredentials()) {
        return;
    }
    
    accept();
}

SetupDialog::OperatorData SetupDialog::getOperatorData() const
{
    OperatorData data;
    data.callsign = m_callsignEdit->text().trimmed().toUpper();
    data.firstName = m_firstNameEdit->text().trimmed();
    data.lastName = m_lastNameEdit->text().trimmed();
    data.locator = m_locatorEdit->text().trimmed().toUpper();
    return data;
}

SetupDialog::ApiCredentials SetupDialog::getApiCredentials() const
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

void SetupDialog::setOperatorData(const OperatorData &data)
{
    m_callsignEdit->setText(data.callsign);
    m_firstNameEdit->setText(data.firstName);
    m_lastNameEdit->setText(data.lastName);
    m_locatorEdit->setText(data.locator);
}

void SetupDialog::setApiCredentials(const ApiCredentials &credentials)
{
    m_enableQrzCheck->setChecked(credentials.enableQrz);
    m_enableClublogCheck->setChecked(credentials.enableClublog);
    
    m_qrzUsernameEdit->setText(credentials.qrzUsername);
    m_qrzPasswordEdit->setText(credentials.qrzPassword);
    m_clublogApiKeyEdit->setText(credentials.clublogApiKey);
    
    updateApiStatus();
}