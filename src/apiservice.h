#ifndef APISERVICE_H
#define APISERVICE_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class ApiService : public QObject
{
    Q_OBJECT

public:
    explicit ApiService(QObject *parent = nullptr);
    
    // Configurazione API
    void setQrzCredentials(const QString &username, const QString &password);
    void setClublogApiKey(const QString &apiKey);
    
    // Lookup nominativo
    void lookupCallsign(const QString &callsign);
    
    // Validazione nominativo
    bool isValidCallsign(const QString &callsign) const;
    
signals:
    void callsignLookupFinished(const QString &callsign, const QString &dxcc, const QString &locator);
    void callsignLookupError(const QString &callsign, const QString &error);
    
private slots:
    void onQrzLoginFinished();
    void onQrzLookupFinished();
    void onClublogLookupFinished();
    void onNetworkError(QNetworkReply::NetworkError error);
    
private:
    enum ApiProvider {
        QRZ_COM,
        CLUBLOG_ORG
    };
    
    // Metodi per QRZ.com
    void loginToQrz();
    void lookupOnQrz(const QString &callsign);
    QString parseQrzResponse(const QString &xml);
    QString parseQrzDxcc(const QString &xml);
    QString parseQrzLocator(const QString &xml);
    
    // Metodi per Clublog.org
    void lookupOnClublog(const QString &callsign);
    QString parseClublogResponse(const QString &json);
    QString parseClublogDxcc(const QString &json);
    QString parseClublogLocator(const QString &json);
    
    // Fallback locale
    QString extractDXCCFromCallsign(const QString &callsign);
    QString getDefaultLocator(const QString &dxcc);
    
    QNetworkAccessManager *m_networkManager;
    
    // Credenziali QRZ.com
    QString m_qrzUsername;
    QString m_qrzPassword;
    QString m_qrzSessionKey;
    bool m_qrzLoggedIn;
    
    // API Key Clublog.org
    QString m_clublogApiKey;
    
    // Stato corrente
    QString m_currentCallsign;
    ApiProvider m_preferredProvider;
    bool m_fallbackToLocal;
    
    // Timer per timeout
    QTimer *m_timeoutTimer;
};

#endif // APISERVICE_H