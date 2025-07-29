#include "database.h"
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QScopedPointer>

Database* Database::m_instance = nullptr;

Database::Database()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
}

Database::~Database()
{
    close();
}

Database* Database::instance()
{
    if (!m_instance) {
        m_instance = new Database();
    }
    return m_instance;
}

void Database::destroy()
{
    if (m_instance) {
        delete m_instance;
        m_instance = nullptr;
    }
}

bool Database::initialize(const QString &dbPath)
{
    QString path = dbPath;
    if (path.isEmpty()) {
        QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(dataPath);
        path = dataPath + "/logbook.db";
    }
    
    m_db.setDatabaseName(path);
    
    if (!m_db.open()) {
        m_lastError = "Impossibile aprire il database: " + m_db.lastError().text();
        return false;
    }
    
    return createTables();
}

bool Database::isOpen() const
{
    return m_db.isOpen();
}

void Database::close()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

bool Database::createTables()
{
    if (!createContactsTable()) {
        return false;
    }
    
    if (!createSettingsTable()) {
        return false;
    }
    
    return true;
}

bool Database::createContactsTable()
{
    QSqlQuery query(m_db);
    
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS contacts (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            datetime TEXT NOT NULL,
            callsign TEXT NOT NULL,
            band TEXT NOT NULL,
            mode TEXT NOT NULL,
            rst_sent TEXT NOT NULL,
            rst_received TEXT NOT NULL,
            dxcc TEXT,
            locator TEXT,
            operator_call TEXT NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!query.exec(sql)) {
        m_lastError = "Errore creazione tabella contacts: " + query.lastError().text();
        return false;
    }
    
    // Crea indici per migliorare le performance
    query.exec("CREATE INDEX IF NOT EXISTS idx_callsign ON contacts(callsign)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_datetime ON contacts(datetime)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_band ON contacts(band)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_mode ON contacts(mode)");
    
    return true;
}

bool Database::createSettingsTable()
{
    QSqlQuery query(m_db);
    
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS settings (
            key TEXT PRIMARY KEY,
            value TEXT
        )
    )";
    
    if (!query.exec(sql)) {
        m_lastError = "Errore creazione tabella settings: " + query.lastError().text();
        return false;
    }
    
    return true;
}

bool Database::addContact(Contact &contact)
{
    QSqlQuery query(m_db);
    
    QString sql = R"(
        INSERT INTO contacts 
        (datetime, callsign, band, mode, rst_sent, rst_received, dxcc, locator, operator_call)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";
    
    query.prepare(sql);
    query.addBindValue(contact.dateTime().toString(Qt::ISODate));
    query.addBindValue(contact.callsign());
    query.addBindValue(contact.band());
    query.addBindValue(contact.mode());
    query.addBindValue(contact.rstSent());
    query.addBindValue(contact.rstReceived());
    query.addBindValue(contact.dxcc());
    query.addBindValue(contact.locator());
    query.addBindValue(contact.operatorCall());
    
    if (!query.exec()) {
        m_lastError = "Errore inserimento contatto: " + query.lastError().text();
        return false;
    }
    
    // Imposta l'ID del contatto appena inserito
    contact.setId(query.lastInsertId().toInt());
    
    return true;
}

bool Database::updateContact(const Contact &contact)
{
    QSqlQuery query(m_db);
    
    QString sql = R"(
        UPDATE contacts SET
        datetime = ?, callsign = ?, band = ?, mode = ?,
        rst_sent = ?, rst_received = ?, dxcc = ?, locator = ?, operator_call = ?
        WHERE id = ?
    )";
    
    query.prepare(sql);
    query.addBindValue(contact.dateTime().toString(Qt::ISODate));
    query.addBindValue(contact.callsign());
    query.addBindValue(contact.band());
    query.addBindValue(contact.mode());
    query.addBindValue(contact.rstSent());
    query.addBindValue(contact.rstReceived());
    query.addBindValue(contact.dxcc());
    query.addBindValue(contact.locator());
    query.addBindValue(contact.operatorCall());
    query.addBindValue(contact.id());
    
    if (!query.exec()) {
        m_lastError = "Errore aggiornamento contatto: " + query.lastError().text();
        return false;
    }
    
    return true;
}

bool Database::deleteContact(int contactId)
{
    QSqlQuery query(m_db);
    
    query.prepare("DELETE FROM contacts WHERE id = ?");
    query.addBindValue(contactId);
    
    if (!query.exec()) {
        m_lastError = "Errore eliminazione contatto: " + query.lastError().text();
        return false;
    }
    
    return true;
}

Contact Database::getContact(int contactId) const
{
    QSqlQuery query(m_db);
    
    query.prepare("SELECT * FROM contacts WHERE id = ?");
    query.addBindValue(contactId);
    
    if (query.exec() && query.next()) {
        Contact contact;
        contact.setId(query.value("id").toInt());
        contact.setDateTime(QDateTime::fromString(query.value("datetime").toString(), Qt::ISODate));
        contact.setCallsign(query.value("callsign").toString());
        contact.setBand(query.value("band").toString());
        contact.setMode(query.value("mode").toString());
        contact.setRstSent(query.value("rst_sent").toString());
        contact.setRstReceived(query.value("rst_received").toString());
        contact.setDxcc(query.value("dxcc").toString());
        contact.setLocator(query.value("locator").toString());
        contact.setOperatorCall(query.value("operator_call").toString());
        return contact;
    }
    
    return Contact();
}

QList<Contact> Database::getAllContacts() const
{
    QList<Contact> contacts;
    QSqlQuery query("SELECT * FROM contacts ORDER BY datetime DESC", m_db);
    
    while (query.next()) {
        Contact contact;
        contact.setId(query.value("id").toInt());
        contact.setDateTime(QDateTime::fromString(query.value("datetime").toString(), Qt::ISODate));
        contact.setCallsign(query.value("callsign").toString());
        contact.setBand(query.value("band").toString());
        contact.setMode(query.value("mode").toString());
        contact.setRstSent(query.value("rst_sent").toString());
        contact.setRstReceived(query.value("rst_received").toString());
        contact.setDxcc(query.value("dxcc").toString());
        contact.setLocator(query.value("locator").toString());
        contact.setOperatorCall(query.value("operator_call").toString());
        contacts.append(contact);
    }
    
    return contacts;
}

QList<Contact> Database::searchContacts(const QString &searchTerm) const
{
    QList<Contact> contacts;
    QSqlQuery query(m_db);
    
    QString sql = R"(
        SELECT * FROM contacts 
        WHERE callsign LIKE ? OR band LIKE ? OR mode LIKE ? OR dxcc LIKE ?
        ORDER BY datetime DESC
    )";
    
    query.prepare(sql);
    QString term = "%" + searchTerm + "%";
    query.addBindValue(term);
    query.addBindValue(term);
    query.addBindValue(term);
    query.addBindValue(term);
    
    if (query.exec()) {
        while (query.next()) {
            Contact contact;
            contact.setId(query.value("id").toInt());
            contact.setDateTime(QDateTime::fromString(query.value("datetime").toString(), Qt::ISODate));
            contact.setCallsign(query.value("callsign").toString());
            contact.setBand(query.value("band").toString());
            contact.setMode(query.value("mode").toString());
            contact.setRstSent(query.value("rst_sent").toString());
            contact.setRstReceived(query.value("rst_received").toString());
            contact.setDxcc(query.value("dxcc").toString());
            contact.setLocator(query.value("locator").toString());
            contact.setOperatorCall(query.value("operator_call").toString());
            contacts.append(contact);
        }
    }
    
    return contacts;
}

bool Database::setOperatorCall(const QString &operatorCall)
{
    QSqlQuery query(m_db);
    
    query.prepare("INSERT OR REPLACE INTO settings (key, value) VALUES ('operator_call', ?)");
    query.addBindValue(operatorCall);
    
    if (!query.exec()) {
        m_lastError = "Errore impostazione operatore: " + query.lastError().text();
        return false;
    }
    
    return true;
}

QString Database::getOperatorCall() const
{
    QSqlQuery query(m_db);
    query.prepare("SELECT value FROM settings WHERE key = 'operator_call'");
    
    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    
    return QString();
}

bool Database::setOperatorData(const QString &callsign, const QString &firstName, 
                              const QString &lastName, const QString &locator)
{
    QSqlQuery query(m_db);
    
    // Inizia transazione
    if (!m_db.transaction()) {
        return false;
    }
    
    // Salva tutti i dati dell'operatore
    QStringList keys = {"operator_call", "operator_firstname", "operator_lastname", "operator_locator"};
    QStringList values = {callsign, firstName, lastName, locator};
    
    for (int i = 0; i < keys.size(); ++i) {
        query.prepare("INSERT OR REPLACE INTO settings (key, value) VALUES (?, ?)");
        query.addBindValue(keys[i]);
        query.addBindValue(values[i]);
        
        if (!query.exec()) {
            m_db.rollback();
            return false;
        }
    }
    
    return m_db.commit();
}

Database::OperatorData Database::getOperatorData() const
{
    OperatorData data;
    QSqlQuery query(m_db);
    
    QStringList keys = {"operator_call", "operator_firstname", "operator_lastname", "operator_locator"};
    QString *fields[] = {&data.callsign, &data.firstName, &data.lastName, &data.locator};
    
    for (int i = 0; i < keys.size(); ++i) {
        query.prepare("SELECT value FROM settings WHERE key = ?");
        query.addBindValue(keys[i]);
        
        if (query.exec() && query.next()) {
            *fields[i] = query.value(0).toString();
        }
    }
    
    return data;
}

bool Database::setApiCredentials(const QString &qrzUsername, const QString &qrzPassword,
                                const QString &clublogApiKey, bool enableQrz, bool enableClublog)
{
    QSqlQuery query(m_db);
    
    // Inizia transazione
    if (!m_db.transaction()) {
        return false;
    }
    
    // Salva le credenziali API
    QStringList keys = {"qrz_username", "qrz_password", "clublog_apikey", "enable_qrz", "enable_clublog"};
    QStringList values = {qrzUsername, qrzPassword, clublogApiKey, 
                         enableQrz ? "1" : "0", enableClublog ? "1" : "0"};
    
    for (int i = 0; i < keys.size(); ++i) {
        query.prepare("INSERT OR REPLACE INTO settings (key, value) VALUES (?, ?)");
        query.addBindValue(keys[i]);
        query.addBindValue(values[i]);
        
        if (!query.exec()) {
            m_db.rollback();
            return false;
        }
    }
    
    return m_db.commit();
}

Database::ApiCredentials Database::getApiCredentials() const
{
    ApiCredentials credentials;
    QSqlQuery query(m_db);
    
    // Carica username QRZ
    query.prepare("SELECT value FROM settings WHERE key = 'qrz_username'");
    if (query.exec() && query.next()) {
        credentials.qrzUsername = query.value(0).toString();
    }
    
    // Carica password QRZ
    query.prepare("SELECT value FROM settings WHERE key = 'qrz_password'");
    if (query.exec() && query.next()) {
        credentials.qrzPassword = query.value(0).toString();
    }
    
    // Carica API key Clublog
    query.prepare("SELECT value FROM settings WHERE key = 'clublog_apikey'");
    if (query.exec() && query.next()) {
        credentials.clublogApiKey = query.value(0).toString();
    }
    
    // Carica flag abilitazione QRZ
    query.prepare("SELECT value FROM settings WHERE key = 'enable_qrz'");
    if (query.exec() && query.next()) {
        credentials.enableQrz = query.value(0).toString() == "1";
    }
    
    // Carica flag abilitazione Clublog
    query.prepare("SELECT value FROM settings WHERE key = 'enable_clublog'");
    if (query.exec() && query.next()) {
        credentials.enableClublog = query.value(0).toString() == "1";
    }
    
    return credentials;
}

int Database::getTotalContacts() const
{
    QSqlQuery query("SELECT COUNT(*) FROM contacts", m_db);
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }
    
    return 0;
}

QStringList Database::getUniqueBands() const
{
    QStringList bands;
    QSqlQuery query("SELECT DISTINCT band FROM contacts ORDER BY band", m_db);
    
    while (query.next()) {
        bands.append(query.value(0).toString());
    }
    
    return bands;
}

QStringList Database::getUniqueModes() const
{
    QStringList modes;
    QSqlQuery query("SELECT DISTINCT mode FROM contacts ORDER BY mode", m_db);
    
    while (query.next()) {
        modes.append(query.value(0).toString());
    }
    
    return modes;
}

QStringList Database::getUniqueDXCC() const
{
    QStringList dxcc;
    QSqlQuery query("SELECT DISTINCT dxcc FROM contacts WHERE dxcc IS NOT NULL AND dxcc != '' ORDER BY dxcc", m_db);
    
    while (query.next()) {
        dxcc.append(query.value(0).toString());
    }
    
    return dxcc;
}

bool Database::clearAllSettings()
{
    QSqlQuery query(m_db);
    
    if (!query.exec("DELETE FROM settings")) {
        m_lastError = "Errore durante il reset delle impostazioni: " + query.lastError().text();
        return false;
    }
    
    return true;
}

bool Database::clearAllContacts()
{
    QSqlQuery query(m_db);
    
    if (!query.exec("DELETE FROM contacts")) {
        m_lastError = "Errore durante la cancellazione dei contatti: " + query.lastError().text();
        return false;
    }
    
    return true;
}

QString Database::lastError() const
{
    return m_lastError;
}