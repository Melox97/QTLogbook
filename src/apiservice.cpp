#include "apiservice.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QXmlStreamReader>
#include <QRegularExpression>
#include <QDebug>
#include <QTimer>
#include <QMap>

ApiService::ApiService(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_qrzLoggedIn(false)
    , m_preferredProvider(QRZ_COM)
    , m_fallbackToLocal(true)
    , m_timeoutTimer(new QTimer(this))
{
    // Configura il timeout per le richieste di rete
    m_timeoutTimer->setSingleShot(true);
    m_timeoutTimer->setInterval(10000); // 10 secondi
    
    connect(m_timeoutTimer, &QTimer::timeout, [this]() {
        emit callsignLookupError(m_currentCallsign, "Timeout della richiesta di rete");
    });
}

void ApiService::setQrzCredentials(const QString &username, const QString &password)
{
    m_qrzUsername = username;
    m_qrzPassword = password;
    m_qrzLoggedIn = false;
    m_qrzSessionKey.clear();
}

void ApiService::setClublogApiKey(const QString &apiKey)
{
    m_clublogApiKey = apiKey;
}

bool ApiService::isValidCallsign(const QString &callsign) const
{
    // Validazione regex per nominativi radioamatoriali
    QRegularExpression regex("^[A-Z0-9]{1,3}[0-9][A-Z0-9]{0,3}[A-Z]$");
    return regex.match(callsign.toUpper()).hasMatch() && 
           callsign.length() >= 3 && callsign.length() <= 10;
}

void ApiService::lookupCallsign(const QString &callsign)
{
    if (!isValidCallsign(callsign)) {
        emit callsignLookupError(callsign, "Nominativo non valido");
        return;
    }
    
    m_currentCallsign = callsign.toUpper();
    m_timeoutTimer->start();
    
    // Check if any API is enabled and configured
    bool qrzEnabled = !m_qrzUsername.isEmpty() && !m_qrzPassword.isEmpty();
    bool clublogEnabled = !m_clublogApiKey.isEmpty();
    
    // Prova prima con QRZ.com se configurato
    if (qrzEnabled && m_preferredProvider == QRZ_COM) {
        if (!m_qrzLoggedIn) {
            loginToQrz();
        } else {
            lookupOnQrz(m_currentCallsign);
        }
    }
    // Altrimenti prova con Clublog.org se configurato
    else if (clublogEnabled && (m_preferredProvider == CLUBLOG_ORG || !qrzEnabled)) {
        lookupOnClublog(m_currentCallsign);
    }
    // Fallback locale se abilitato o nessuna API configurata
    else if (m_fallbackToLocal || (!qrzEnabled && !clublogEnabled)) {
        m_timeoutTimer->stop();
        QString dxcc = extractDXCCFromCallsign(m_currentCallsign);
        QString locator = getDefaultLocator(dxcc);
        emit callsignLookupFinished(m_currentCallsign, dxcc, locator);
    }
    else {
        m_timeoutTimer->stop();
        emit callsignLookupError(m_currentCallsign, "Nessun servizio di lookup disponibile o configurato");
    }
}

void ApiService::onQrzLoginFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    if (reply->error() == QNetworkReply::NoError) {
        QString response = reply->readAll();
        m_qrzSessionKey = parseQrzResponse(response);
        if (!m_qrzSessionKey.isEmpty()) {
            m_qrzLoggedIn = true;
            lookupOnQrz(m_currentCallsign);
        } else {
            emit callsignLookupError(m_currentCallsign, "Login QRZ.com fallito");
        }
    } else {
        emit callsignLookupError(m_currentCallsign, "Errore di rete QRZ.com: " + reply->errorString());
    }
    
    reply->deleteLater();
}

void ApiService::onQrzLookupFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    m_timeoutTimer->stop();
    
    if (reply->error() == QNetworkReply::NoError) {
        QString response = reply->readAll();
        QString dxcc = parseQrzDxcc(response);
        QString locator = parseQrzLocator(response);
        
        if (!dxcc.isEmpty()) {
            emit callsignLookupFinished(m_currentCallsign, dxcc, locator);
        } else {
            // Fallback a Clublog o locale
            if (!m_clublogApiKey.isEmpty()) {
                lookupOnClublog(m_currentCallsign);
            } else {
                QString fallbackDxcc = extractDXCCFromCallsign(m_currentCallsign);
                QString fallbackLocator = getDefaultLocator(fallbackDxcc);
                emit callsignLookupFinished(m_currentCallsign, fallbackDxcc, fallbackLocator);
            }
        }
    } else {
        emit callsignLookupError(m_currentCallsign, "Errore QRZ.com: " + reply->errorString());
    }
    
    reply->deleteLater();
}

void ApiService::onClublogLookupFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    m_timeoutTimer->stop();
    
    if (reply->error() == QNetworkReply::NoError) {
        QString response = reply->readAll();
        QString dxcc = parseClublogDxcc(response);
        QString locator = parseClublogLocator(response);
        
        if (!dxcc.isEmpty()) {
            emit callsignLookupFinished(m_currentCallsign, dxcc, locator);
        } else {
            // Fallback locale
            QString fallbackDxcc = extractDXCCFromCallsign(m_currentCallsign);
            QString fallbackLocator = getDefaultLocator(fallbackDxcc);
            emit callsignLookupFinished(m_currentCallsign, fallbackDxcc, fallbackLocator);
        }
    } else {
        emit callsignLookupError(m_currentCallsign, "Errore Clublog.org: " + reply->errorString());
    }
    
    reply->deleteLater();
}

void ApiService::onNetworkError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)
    m_timeoutTimer->stop();
    
    // Fallback locale in caso di errore di rete
    QString dxcc = extractDXCCFromCallsign(m_currentCallsign);
    QString locator = getDefaultLocator(dxcc);
    emit callsignLookupFinished(m_currentCallsign, dxcc, locator);
}

void ApiService::loginToQrz()
{
    QUrl url("https://xmldata.qrz.com/xml/current/");
    QUrlQuery query;
    query.addQueryItem("username", m_qrzUsername);
    query.addQueryItem("password", m_qrzPassword);
    query.addQueryItem("agent", "QTLogbook-1.0");
    url.setQuery(query);
    
    QNetworkRequest request(url);
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &ApiService::onQrzLoginFinished);
}

void ApiService::lookupOnQrz(const QString &callsign)
{
    QUrl url("https://xmldata.qrz.com/xml/current/");
    QUrlQuery query;
    query.addQueryItem("s", m_qrzSessionKey);
    query.addQueryItem("callsign", callsign);
    url.setQuery(query);
    
    QNetworkRequest request(url);
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &ApiService::onQrzLookupFinished);
}

void ApiService::lookupOnClublog(const QString &callsign)
{
    QUrl url(QString("https://clublog.org/dxcc?call=%1&api=%2&full=1").arg(callsign, m_clublogApiKey));
    
    QNetworkRequest request(url);
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &ApiService::onClublogLookupFinished);
}

QString ApiService::parseQrzResponse(const QString &xml)
{
    QXmlStreamReader reader(xml);
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement() && reader.name() == "Key") {
            return reader.readElementText();
        }
    }
    return QString();
}

QString ApiService::parseQrzDxcc(const QString &xml)
{
    QXmlStreamReader reader(xml);
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement() && reader.name() == "dxcc") {
            return reader.readElementText();
        }
    }
    return QString();
}

QString ApiService::parseQrzLocator(const QString &xml)
{
    QXmlStreamReader reader(xml);
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement() && reader.name() == "grid") {
            return reader.readElementText();
        }
    }
    return QString();
}

QString ApiService::parseClublogResponse(const QString &json)
{
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    return doc.object().value("DXCC").toString();
}

QString ApiService::parseClublogDxcc(const QString &json)
{
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    QJsonObject obj = doc.object();
    return obj.value("Name").toString();
}

QString ApiService::parseClublogLocator(const QString &json)
{
    // Clublog non fornisce sempre il locatore, ritorna vuoto
    Q_UNUSED(json)
    return QString();
}

QString ApiService::extractDXCCFromCallsign(const QString &callsign)
{
    // Mappa semplificata dei prefissi DXCC più comuni
    static QMap<QString, QString> dxccMap = {
        {"I", "Italy"}, {"IZ", "Italy"}, {"IU", "Italy"}, {"IW", "Italy"},
        {"IK", "Italy"}, {"IN", "Italy"}, {"IO", "Italy"}, {"IP", "Italy"},
        {"W", "United States"}, {"K", "United States"}, {"N", "United States"},
        {"G", "England"}, {"M", "England"}, {"2E", "England"},
        {"F", "France"}, {"TM", "France"}, {"TN", "France"},
        {"DL", "Germany"}, {"DA", "Germany"}, {"DB", "Germany"},
        {"JA", "Japan"}, {"JE", "Japan"}, {"JF", "Japan"},
        {"VK", "Australia"}, {"VH", "Australia"}, {"VI", "Australia"},
        {"VE", "Canada"}, {"VA", "Canada"}, {"VY", "Canada"},
        {"EA", "Spain"}, {"EB", "Spain"}, {"EC", "Spain"},
        {"LU", "Argentina"}, {"AY", "Argentina"}, {"AZ", "Argentina"},
        {"PY", "Brazil"}, {"PP", "Brazil"}, {"PQ", "Brazil"}
    };
    
    QRegularExpression regex("^([A-Z0-9]{1,3})[0-9]");
    QRegularExpressionMatch match = regex.match(callsign);
    
    if (match.hasMatch()) {
        QString prefix = match.captured(1);
        
        // Cerca il prefisso nella mappa
        for (auto it = dxccMap.begin(); it != dxccMap.end(); ++it) {
            if (prefix.startsWith(it.key())) {
                return it.value();
            }
        }
        
        // Se non trovato, cerca per prefisso più corto
        QString shortPrefix = prefix.left(1);
        if (dxccMap.contains(shortPrefix)) {
            return dxccMap[shortPrefix];
        }
    }
    
    return "Unknown";
}

QString ApiService::getDefaultLocator(const QString &dxcc)
{
    // Locatori di default per i paesi più comuni
    static QMap<QString, QString> locatorMap = {
        {"Italy", "JN45"},
        {"United States", "FN20"},
        {"England", "IO91"},
        {"France", "JN18"},
        {"Germany", "JO62"},
        {"Japan", "PM95"},
        {"Australia", "QF22"},
        {"Canada", "FN25"},
        {"Spain", "IN80"},
        {"Argentina", "GF05"},
        {"Brazil", "GG66"}
    };
    
    return locatorMap.value(dxcc, "JN00");
}