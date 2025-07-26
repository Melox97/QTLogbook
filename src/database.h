#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtCore/QString>
#include <QtCore/QList>
#include "contact.h"

class Database
{
public:
    static Database* instance();
    
    bool initialize(const QString &dbPath = QString());
    bool isOpen() const;
    void close();
    
    // Contact operations
    bool addContact(Contact &contact);
    bool updateContact(const Contact &contact);
    bool deleteContact(int contactId);
    Contact getContact(int contactId) const;
    QList<Contact> getAllContacts() const;
    QList<Contact> searchContacts(const QString &searchTerm) const;
    
    // Operator management
    bool setOperatorCall(const QString &operatorCall);
    QString getOperatorCall() const;
    
    // Extended operator data
    bool setOperatorData(const QString &callsign, const QString &firstName, 
                        const QString &lastName, const QString &locator);
    struct OperatorData {
        QString callsign;
        QString firstName;
        QString lastName;
        QString locator;
    };
    OperatorData getOperatorData() const;
    
    // API credentials management
    bool setApiCredentials(const QString &qrzUsername, const QString &qrzPassword,
                          const QString &clublogApiKey, bool enableQrz, bool enableClublog);
    struct ApiCredentials {
        QString qrzUsername;
        QString qrzPassword;
        QString clublogApiKey;
        bool enableQrz;
        bool enableClublog;
    };
    ApiCredentials getApiCredentials() const;
    
    // Statistics
    int getTotalContacts() const;
    QStringList getUniqueBands() const;
    QStringList getUniqueModes() const;
    QStringList getUniqueDXCC() const;
    
    // Settings management
    bool clearAllSettings();
    bool clearAllContacts();
    
    QString lastError() const;
    
private:
    Database();
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    bool createTables();
    bool createContactsTable();
    bool createSettingsTable();
    
    static Database* m_instance;
    QSqlDatabase m_db;
    QString m_lastError;
};

#endif // DATABASE_H