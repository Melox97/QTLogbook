#ifndef CONTACT_H
#define CONTACT_H

#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QJsonObject>

class Contact
{
public:
    Contact();
    Contact(const QString &callsign, const QString &band, const QString &mode,
            const QString &rstSent, const QString &rstReceived, const QString &operatorCall);

    // Getters
    int id() const { return m_id; }
    QDateTime dateTime() const { return m_dateTime; }
    QString callsign() const { return m_callsign; }
    QString band() const { return m_band; }
    QString mode() const { return m_mode; }
    QString rstSent() const { return m_rstSent; }
    QString rstReceived() const { return m_rstReceived; }
    QString dxcc() const { return m_dxcc; }
    QString locator() const { return m_locator; }
    QString operatorCall() const { return m_operatorCall; }

    // Setters
    void setId(int id) { m_id = id; }
    void setDateTime(const QDateTime &dateTime) { m_dateTime = dateTime; }
    void setCallsign(const QString &callsign) { m_callsign = callsign; }
    void setBand(const QString &band) { m_band = band; }
    void setMode(const QString &mode) { m_mode = mode; }
    void setRstSent(const QString &rst) { m_rstSent = rst; }
    void setRstReceived(const QString &rst) { m_rstReceived = rst; }
    void setDxcc(const QString &dxcc) { m_dxcc = dxcc; }
    void setLocator(const QString &locator) { m_locator = locator; }
    void setOperatorCall(const QString &operatorCall) { m_operatorCall = operatorCall; }

    // Validation
    bool isValid() const;
    QString validationError() const;

    // JSON serialization
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

private:
    int m_id;
    QDateTime m_dateTime;
    QString m_callsign;
    QString m_band;
    QString m_mode;
    QString m_rstSent;
    QString m_rstReceived;
    QString m_dxcc;
    QString m_locator;
    QString m_operatorCall;
};

#endif // CONTACT_H