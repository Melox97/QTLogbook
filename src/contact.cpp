#include "contact.h"
#include <QRegularExpression>
#include <QJsonObject>

Contact::Contact()
    : m_id(-1)
    , m_dateTime(QDateTime::currentDateTimeUtc())
{
}

Contact::Contact(const QString &callsign, const QString &band, const QString &mode,
                 const QString &rstSent, const QString &rstReceived, const QString &operatorCall)
    : m_id(-1)
    , m_dateTime(QDateTime::currentDateTimeUtc())
    , m_callsign(callsign.toUpper())
    , m_band(band)
    , m_mode(mode)
    , m_rstSent(rstSent)
    , m_rstReceived(rstReceived)
    , m_operatorCall(operatorCall.toUpper())
{
}

bool Contact::isValid() const
{
    // Validazione nominativo (8-10 caratteri alfanumerici)
    QRegularExpression callsignRegex("^[A-Z0-9]{1,3}[0-9][A-Z0-9]{0,3}[A-Z]$");
    if (!callsignRegex.match(m_callsign).hasMatch() || m_callsign.length() < 3 || m_callsign.length() > 10) {
        return false;
    }

    // Validazione RST in base al modo di modulazione
    QRegularExpression rstRegex2Digits("^[1-5][1-9]$");     // 2 cifre per modi vocali
    QRegularExpression rstRegex3Digits("^[1-5][1-9][1-9]$"); // 3 cifre per modi digitali/CW tradizionali
    QRegularExpression rstRegexFT("^[+-]?[0-9]{1,2}$");      // Per FT8/FT4: valori da -30 a +30
    
    bool isVoiceMode = (m_mode == "SSB" || m_mode == "AM" || m_mode == "FM" || 
                       m_mode == "USB" || m_mode == "LSB");
    bool isFTMode = (m_mode == "FT8" || m_mode == "FT4" || m_mode == "JT65" || m_mode == "JT9" || m_mode == "MSK144");
    
    if (isVoiceMode) {
        // Modi vocali: accetta 2 cifre (es. 59)
        if (!rstRegex2Digits.match(m_rstSent).hasMatch() || !rstRegex2Digits.match(m_rstReceived).hasMatch()) {
            return false;
        }
    } else if (isFTMode) {
        // Modi FT/JT: accetta valori da -30 a +30 (es. -10, +15)
        if (!rstRegexFT.match(m_rstSent).hasMatch() || !rstRegexFT.match(m_rstReceived).hasMatch()) {
            return false;
        }
        // Verifica range valido per FT modes
        bool sentOk, rcvdOk;
        int sentValue = m_rstSent.toInt(&sentOk);
        int rcvdValue = m_rstReceived.toInt(&rcvdOk);
        if (!sentOk || !rcvdOk || sentValue < -30 || sentValue > 30 || rcvdValue < -30 || rcvdValue > 30) {
            return false;
        }
    } else {
        // Modi digitali/CW tradizionali: richiede 3 cifre (es. 599)
        if (!rstRegex3Digits.match(m_rstSent).hasMatch() || !rstRegex3Digits.match(m_rstReceived).hasMatch()) {
            return false;
        }
    }

    // Validazione campi obbligatori
    if (m_band.isEmpty() || m_mode.isEmpty() || m_operatorCall.isEmpty()) {
        return false;
    }

    return true;
}

QString Contact::validationError() const
{
    QRegularExpression callsignRegex("^[A-Z0-9]{1,3}[0-9][A-Z0-9]{0,3}[A-Z]$");
    if (!callsignRegex.match(m_callsign).hasMatch() || m_callsign.length() < 3 || m_callsign.length() > 10) {
        return "Nominativo non valido. Deve essere di 3-10 caratteri alfanumerici.";
    }

    // Validazione RST in base al modo di modulazione
    QRegularExpression rstRegex2Digits("^[1-5][1-9]$");     // 2 cifre per modi vocali
    QRegularExpression rstRegex3Digits("^[1-5][1-9][1-9]$"); // 3 cifre per modi digitali/CW tradizionali
    QRegularExpression rstRegexFT("^[+-]?[0-9]{1,2}$");      // Per FT8/FT4: valori da -30 a +30
    
    bool isVoiceMode = (m_mode == "SSB" || m_mode == "AM" || m_mode == "FM" || 
                       m_mode == "USB" || m_mode == "LSB");
    bool isFTMode = (m_mode == "FT8" || m_mode == "FT4" || m_mode == "JT65" || m_mode == "JT9" || m_mode == "MSK144");
    
    if (isVoiceMode) {
        // Modi vocali: 2 cifre
        if (!rstRegex2Digits.match(m_rstSent).hasMatch()) {
            return "RST inviato non valido per modo vocale. Deve essere di 2 cifre (es. 59).";
        }
        if (!rstRegex2Digits.match(m_rstReceived).hasMatch()) {
            return "RST ricevuto non valido per modo vocale. Deve essere di 2 cifre (es. 59).";
        }
    } else if (isFTMode) {
        // Modi FT/JT: valori da -30 a +30
        if (!rstRegexFT.match(m_rstSent).hasMatch()) {
            return "RST inviato non valido per modo FT/JT. Deve essere un valore da -30 a +30 (es. -10).";
        }
        if (!rstRegexFT.match(m_rstReceived).hasMatch()) {
            return "RST ricevuto non valido per modo FT/JT. Deve essere un valore da -30 a +30 (es. -15).";
        }
        // Verifica range
        bool sentOk, rcvdOk;
        int sentValue = m_rstSent.toInt(&sentOk);
        int rcvdValue = m_rstReceived.toInt(&rcvdOk);
        if (!sentOk || sentValue < -30 || sentValue > 30) {
            return "RST inviato fuori range. Deve essere tra -30 e +30 per modi FT/JT.";
        }
        if (!rcvdOk || rcvdValue < -30 || rcvdValue > 30) {
            return "RST ricevuto fuori range. Deve essere tra -30 e +30 per modi FT/JT.";
        }
    } else {
        // Modi digitali/CW tradizionali: 3 cifre
        if (!rstRegex3Digits.match(m_rstSent).hasMatch()) {
            return "RST inviato non valido per modo digitale/CW. Deve essere di 3 cifre (es. 599).";
        }
        if (!rstRegex3Digits.match(m_rstReceived).hasMatch()) {
            return "RST ricevuto non valido per modo digitale/CW. Deve essere di 3 cifre (es. 599).";
        }
    }

    if (m_band.isEmpty()) {
        return "Banda obbligatoria.";
    }
    
    if (m_mode.isEmpty()) {
        return "Modo obbligatorio.";
    }
    
    if (m_operatorCall.isEmpty()) {
        return "Nominativo operatore obbligatorio.";
    }

    return QString();
}

QJsonObject Contact::toJson() const
{
    QJsonObject obj;
    obj["id"] = m_id;
    obj["dateTime"] = m_dateTime.toString(Qt::ISODate);
    obj["callsign"] = m_callsign;
    obj["band"] = m_band;
    obj["mode"] = m_mode;
    obj["rstSent"] = m_rstSent;
    obj["rstReceived"] = m_rstReceived;
    obj["dxcc"] = m_dxcc;
    obj["locator"] = m_locator;
    obj["operatorCall"] = m_operatorCall;
    return obj;
}

void Contact::fromJson(const QJsonObject &json)
{
    m_id = json["id"].toInt(-1);
    m_dateTime = QDateTime::fromString(json["dateTime"].toString(), Qt::ISODate);
    m_callsign = json["callsign"].toString();
    m_band = json["band"].toString();
    m_mode = json["mode"].toString();
    m_rstSent = json["rstSent"].toString();
    m_rstReceived = json["rstReceived"].toString();
    m_dxcc = json["dxcc"].toString();
    m_locator = json["locator"].toString();
    m_operatorCall = json["operatorCall"].toString();
}