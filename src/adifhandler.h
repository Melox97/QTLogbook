#ifndef ADIFHANDLER_H
#define ADIFHANDLER_H

#include <QString>
#include <QList>
#include <QDateTime>
#include <QTextStream>
#include <QFile>
#include <QMap>
#include "contact.h"

class ADIFHandler
{
public:
    struct ImportResult {
        QList<Contact> importedContacts;
        QList<Contact> duplicateContacts;
        int totalRecords = 0;
        int successfulImports = 0;
        int duplicatesFound = 0;
        QString errorMessage;
        bool success = false;
    };
    
    struct ExportResult {
        int totalRecords = 0;
        int successfulExports = 0;
        QString errorMessage;
        bool success = false;
    };
    
    ADIFHandler();
    
    // Importazione ADIF
    ImportResult importFromFile(const QString &filePath, const QList<Contact> &existingContacts);
    
    // Esportazione ADIF
    ExportResult exportToFile(const QString &filePath, const QList<Contact> &contacts, const QString &operatorCall = QString());
    
    // Generazione report duplicati
    bool generateDuplicateReport(const QString &filePath, const QList<Contact> &duplicates, const QString &operatorCall = QString());
    
    // Validazione formato ADIF
    bool isValidADIFFile(const QString &filePath);
    
    // Conversione banda da frequenza a nome
    QString frequencyToBand(const QString &frequency);
    
    // Conversione banda da nome a frequenza
    QString bandToFrequency(const QString &band);
    
private:
    // Parsing ADIF
    QList<QMap<QString, QString>> parseADIFRecords(const QString &content);
    Contact adifRecordToContact(const QMap<QString, QString> &record);
    
    // Generazione ADIF
    QString contactToADIFRecord(const Contact &contact);
    QString generateADIFHeader(const QString &operatorCall = QString());
    
    // Utilità
    QString formatADIFField(const QString &fieldName, const QString &value);
    QString formatADIFDateTime(const QDateTime &dateTime);
    QDateTime parseADIFDateTime(const QString &date, const QString &time);
    bool isDuplicate(const Contact &contact, const QList<Contact> &existingContacts);
    
    // Mappature bande/frequenze
    void initializeBandMappings();
    QMap<QString, QString> m_bandToFreq;
    QMap<QString, QString> m_freqToBand;
    
    // Mappature modi ADIF
    void initializeModeMappings();
    QMap<QString, QString> m_modeMapping;
};

#endif // ADIFHANDLER_H