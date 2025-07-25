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

    // Validazione RST (3 cifre numeriche)
    QRegularExpression rstRegex("^[1-5][1-9][1-9]$");
    if (!rstRegex.match(m_rstSent).hasMatch() || !rstRegex.match(m_rstReceived).hasMatch()) {
        return false;
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

    QRegularExpression rstRegex("^[1-5][1-9][1-9]$");
    if (!rstRegex.match(m_rstSent).hasMatch()) {
        return "RST inviato non valido. Deve essere di 3 cifre (es. 599).";
    }
    
    if (!rstRegex.match(m_rstReceived).hasMatch()) {
        return "RST ricevuto non valido. Deve essere di 3 cifre (es. 599).";
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