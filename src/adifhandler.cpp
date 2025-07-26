#include "adifhandler.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QRegularExpression>
#include <QDebug>
#include <QIODevice>
#include <QStringConverter>

ADIFHandler::ADIFHandler()
{
    initializeBandMappings();
    initializeModeMappings();
}

ADIFHandler::ImportResult ADIFHandler::importFromFile(const QString &filePath, const QList<Contact> &existingContacts)
{
    ImportResult result;
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.errorMessage = "Impossibile aprire il file: " + file.errorString();
        return result;
    }
    
    QString content = file.readAll();
    file.close();
    
    // Verifica che sia un file ADIF valido
    if (!content.contains("<EOH>") && !content.contains("<eoh>")) {
        result.errorMessage = "File ADIF non valido: manca il marcatore di fine header <EOH>";
        return result;
    }
    
    // Parsing dei record ADIF
    QList<QMap<QString, QString>> adifRecords = parseADIFRecords(content);
    result.totalRecords = adifRecords.size();
    
    for (const auto &record : adifRecords) {
        Contact contact = adifRecordToContact(record);
        
        if (contact.isValid()) {
            // Verifica duplicati
            if (isDuplicate(contact, existingContacts)) {
                result.duplicateContacts.append(contact);
                result.duplicatesFound++;
            } else {
                result.importedContacts.append(contact);
                result.successfulImports++;
            }
        }
    }
    
    result.success = true;
    return result;
}

ADIFHandler::ExportResult ADIFHandler::exportToFile(const QString &filePath, const QList<Contact> &contacts, const QString &operatorCall)
{
    ExportResult result;
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        result.errorMessage = "Impossibile creare il file: " + file.errorString();
        return result;
    }
    
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    
    // Scrivi header ADIF
    out << generateADIFHeader(operatorCall);
    
    // Scrivi i contatti
    result.totalRecords = contacts.size();
    for (const Contact &contact : contacts) {
        if (contact.isValid()) {
            out << contactToADIFRecord(contact);
            result.successfulExports++;
        }
    }
    
    file.close();
    result.success = true;
    return result;
}

bool ADIFHandler::generateDuplicateReport(const QString &filePath, const QList<Contact> &duplicates, const QString &operatorCall)
{
    if (duplicates.isEmpty()) {
        return true; // Nessun duplicato da segnalare
    }
    
    ExportResult result = exportToFile(filePath, duplicates, operatorCall);
    return result.success;
}

bool ADIFHandler::isValidADIFFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    
    QString content = file.readAll();
    file.close();
    
    // Verifica presenza di marcatori ADIF essenziali
    return content.contains(QRegularExpression("<EOH>", QRegularExpression::CaseInsensitiveOption)) ||
           content.contains(QRegularExpression("<eoh>", QRegularExpression::CaseInsensitiveOption));
}

QList<QMap<QString, QString>> ADIFHandler::parseADIFRecords(const QString &content)
{
    QList<QMap<QString, QString>> records;
    
    // Trova la fine dell'header
    int headerEnd = content.indexOf(QRegularExpression("<EOH>", QRegularExpression::CaseInsensitiveOption));
    if (headerEnd == -1) {
        headerEnd = content.indexOf(QRegularExpression("<eoh>", QRegularExpression::CaseInsensitiveOption));
    }
    
    if (headerEnd == -1) {
        return records; // Header non trovato
    }
    
    // Estrai la parte dei dati dopo l'header
    QString dataSection = content.mid(headerEnd + 5); // +5 per saltare <EOH>
    
    // Regex per trovare i campi ADIF: <FIELD:LENGTH>VALUE
    QRegularExpression fieldRegex("<([^:>]+):(\\d+)>([^<]*)", QRegularExpression::CaseInsensitiveOption);
    QRegularExpression eorRegex("<EOR>", QRegularExpression::CaseInsensitiveOption);
    
    QMap<QString, QString> currentRecord;
    QRegularExpressionMatchIterator iterator = fieldRegex.globalMatch(dataSection);
    
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString fieldName = match.captured(1).toUpper();
        int length = match.captured(2).toInt();
        QString value = match.captured(3).left(length);
        
        currentRecord[fieldName] = value;
        
        // Verifica se c'è un EOR dopo questo campo
        int nextPos = match.capturedEnd();
        if (nextPos < dataSection.length()) {
            QRegularExpressionMatch eorMatch = eorRegex.match(dataSection, nextPos);
            if (eorMatch.hasMatch() && eorMatch.capturedStart() == nextPos) {
                // Fine del record
                if (!currentRecord.isEmpty()) {
                    records.append(currentRecord);
                    currentRecord.clear();
                }
            }
        }
    }
    
    // Aggiungi l'ultimo record se non vuoto
    if (!currentRecord.isEmpty()) {
        records.append(currentRecord);
    }
    
    return records;
}

Contact ADIFHandler::adifRecordToContact(const QMap<QString, QString> &record)
{
    Contact contact;
    
    // Campi obbligatori
    if (record.contains("CALL")) {
        contact.setCallsign(record["CALL"]);
    }
    
    if (record.contains("BAND")) {
        contact.setBand(record["BAND"]);
    } else if (record.contains("FREQ")) {
        // Converti frequenza in banda
        QString band = frequencyToBand(record["FREQ"]);
        contact.setBand(band);
    }
    
    if (record.contains("MODE")) {
        QString mode = record["MODE"];
        // Mappa i modi ADIF ai modi interni se necessario
        if (m_modeMapping.contains(mode)) {
            mode = m_modeMapping[mode];
        }
        contact.setMode(mode);
    }
    
    // Data e ora
    QString date = record.value("QSO_DATE");
    QString time = record.value("TIME_ON");
    if (!date.isEmpty() && !time.isEmpty()) {
        QDateTime dateTime = parseADIFDateTime(date, time);
        contact.setDateTime(dateTime);
    }
    
    // RST
    if (record.contains("RST_SENT")) {
        contact.setRstSent(record["RST_SENT"]);
    }
    if (record.contains("RST_RCVD")) {
        contact.setRstReceived(record["RST_RCVD"]);
    }
    
    // DXCC e locatore
    if (record.contains("DXCC")) {
        contact.setDxcc(record["DXCC"]);
    } else if (record.contains("COUNTRY")) {
        contact.setDxcc(record["COUNTRY"]);
    }
    
    if (record.contains("GRIDSQUARE")) {
        contact.setLocator(record["GRIDSQUARE"]);
    }
    
    // Operatore
    if (record.contains("OPERATOR")) {
        contact.setOperatorCall(record["OPERATOR"]);
    } else if (record.contains("STATION_CALLSIGN")) {
        contact.setOperatorCall(record["STATION_CALLSIGN"]);
    }
    
    return contact;
}

QString ADIFHandler::contactToADIFRecord(const Contact &contact)
{
    QString record;
    
    // Campi obbligatori
    record += formatADIFField("CALL", contact.callsign());
    record += formatADIFField("BAND", contact.band());
    record += formatADIFField("MODE", contact.mode());
    
    // Data e ora
    QString dateTime = formatADIFDateTime(contact.dateTime());
    record += formatADIFField("QSO_DATE", dateTime.left(8)); // YYYYMMDD
    record += formatADIFField("TIME_ON", dateTime.mid(8, 6)); // HHMMSS
    
    // RST
    if (!contact.rstSent().isEmpty()) {
        record += formatADIFField("RST_SENT", contact.rstSent());
    }
    if (!contact.rstReceived().isEmpty()) {
        record += formatADIFField("RST_RCVD", contact.rstReceived());
    }
    
    // DXCC e locatore
    if (!contact.dxcc().isEmpty()) {
        record += formatADIFField("COUNTRY", contact.dxcc());
    }
    if (!contact.locator().isEmpty()) {
        record += formatADIFField("GRIDSQUARE", contact.locator());
    }
    
    // Operatore
    if (!contact.operatorCall().isEmpty()) {
        record += formatADIFField("OPERATOR", contact.operatorCall());
    }
    
    // Frequenza (opzionale)
    QString freq = bandToFrequency(contact.band());
    if (!freq.isEmpty()) {
        record += formatADIFField("FREQ", freq);
    }
    
    record += "<EOR>\n";
    return record;
}

QString ADIFHandler::generateADIFHeader(const QString &operatorCall)
{
    QString header;
    header += "ADIF Export from QT Logbook\n";
    header += "<ADIF_VER:5>3.1.4\n";
    header += "<PROGRAMID:10>QT Logbook\n";
    header += "<PROGRAMVERSION:5>1.0.0\n";
    
    QDateTime now = QDateTime::currentDateTimeUtc();
    QString timestamp = now.toString("yyyyMMdd HHmmss");
    header += QString("<CREATED_TIMESTAMP:%1>%2\n").arg(timestamp.length()).arg(timestamp);
    
    if (!operatorCall.isEmpty()) {
        header += formatADIFField("OPERATOR", operatorCall);
    }
    
    header += "<EOH>\n";
    return header;
}

QString ADIFHandler::formatADIFField(const QString &fieldName, const QString &value)
{
    if (value.isEmpty()) {
        return QString();
    }
    return QString("<%1:%2>%3").arg(fieldName).arg(value.length()).arg(value);
}

QString ADIFHandler::formatADIFDateTime(const QDateTime &dateTime)
{
    // Formato ADIF: YYYYMMDDHHMMSS
    return dateTime.toUTC().toString("yyyyMMddHHmmss");
}

QDateTime ADIFHandler::parseADIFDateTime(const QString &date, const QString &time)
{
    // Date format: YYYYMMDD
    // Time format: HHMMSS or HHMM
    QString dateTimeStr = date + time.leftJustified(6, '0'); // Assicura 6 cifre per il tempo
    return QDateTime::fromString(dateTimeStr, "yyyyMMddHHmmss").toUTC();
}

bool ADIFHandler::isDuplicate(const Contact &contact, const QList<Contact> &existingContacts)
{
    // Un contatto è considerato duplicato se ha stesso nominativo, banda, modo e data/ora entro 5 minuti
    for (const Contact &existing : existingContacts) {
        if (existing.callsign() == contact.callsign() &&
            existing.band() == contact.band() &&
            existing.mode() == contact.mode()) {
            
            // Verifica differenza temporale (entro 5 minuti)
            qint64 timeDiff = qAbs(existing.dateTime().secsTo(contact.dateTime()));
            if (timeDiff <= 300) { // 5 minuti = 300 secondi
                return true;
            }
        }
    }
    return false;
}

void ADIFHandler::initializeBandMappings()
{
    // Mappature banda -> frequenza centrale (MHz)
    m_bandToFreq["160m"] = "1.8";
    m_bandToFreq["80m"] = "3.5";
    m_bandToFreq["60m"] = "5.0";
    m_bandToFreq["40m"] = "7.0";
    m_bandToFreq["30m"] = "10.1";
    m_bandToFreq["20m"] = "14.0";
    m_bandToFreq["17m"] = "18.0";
    m_bandToFreq["15m"] = "21.0";
    m_bandToFreq["12m"] = "24.0";
    m_bandToFreq["10m"] = "28.0";
    m_bandToFreq["6m"] = "50.0";
    m_bandToFreq["4m"] = "70.0";
    m_bandToFreq["2m"] = "144.0";
    m_bandToFreq["1.25m"] = "222.0";
    m_bandToFreq["70cm"] = "432.0";
    m_bandToFreq["33cm"] = "902.0";
    m_bandToFreq["23cm"] = "1240.0";
    
    // Crea la mappa inversa
    for (auto it = m_bandToFreq.begin(); it != m_bandToFreq.end(); ++it) {
        m_freqToBand[it.value()] = it.key();
    }
}

void ADIFHandler::initializeModeMappings()
{
    // Mappature modi ADIF standard ai modi interni
    m_modeMapping["CW"] = "CW";
    m_modeMapping["SSB"] = "SSB";
    m_modeMapping["USB"] = "USB";
    m_modeMapping["LSB"] = "LSB";
    m_modeMapping["AM"] = "AM";
    m_modeMapping["FM"] = "FM";
    m_modeMapping["RTTY"] = "RTTY";
    m_modeMapping["PSK31"] = "PSK31";
    m_modeMapping["PSK63"] = "PSK63";
    m_modeMapping["FT8"] = "FT8";
    m_modeMapping["FT4"] = "FT4";
    m_modeMapping["JT65"] = "JT65";
    m_modeMapping["JT9"] = "JT9";
    m_modeMapping["MSK144"] = "MSK144";
    m_modeMapping["MFSK"] = "MFSK";
    m_modeMapping["OLIVIA"] = "OLIVIA";
    m_modeMapping["CONTESTIA"] = "CONTESTIA";
    m_modeMapping["HELL"] = "HELL";
    m_modeMapping["SSTV"] = "SSTV";
    m_modeMapping["FAX"] = "FAX";
    m_modeMapping["PACKET"] = "PACKET";
}

QString ADIFHandler::frequencyToBand(const QString &frequency)
{
    double freq = frequency.toDouble();
    
    // Converti frequenza in banda basandoti sui range standard
    if (freq >= 1.8 && freq <= 2.0) return "160m";
    if (freq >= 3.5 && freq <= 4.0) return "80m";
    if (freq >= 5.0 && freq <= 5.5) return "60m";
    if (freq >= 7.0 && freq <= 7.3) return "40m";
    if (freq >= 10.1 && freq <= 10.15) return "30m";
    if (freq >= 14.0 && freq <= 14.35) return "20m";
    if (freq >= 18.0 && freq <= 18.2) return "17m";
    if (freq >= 21.0 && freq <= 21.45) return "15m";
    if (freq >= 24.0 && freq <= 24.99) return "12m";
    if (freq >= 28.0 && freq <= 29.7) return "10m";
    if (freq >= 50.0 && freq <= 54.0) return "6m";
    if (freq >= 70.0 && freq <= 71.0) return "4m";
    if (freq >= 144.0 && freq <= 148.0) return "2m";
    if (freq >= 222.0 && freq <= 225.0) return "1.25m";
    if (freq >= 420.0 && freq <= 450.0) return "70cm";
    if (freq >= 902.0 && freq <= 928.0) return "33cm";
    if (freq >= 1240.0 && freq <= 1300.0) return "23cm";
    
    return "Unknown";
}

QString ADIFHandler::bandToFrequency(const QString &band)
{
    return m_bandToFreq.value(band, QString());
}