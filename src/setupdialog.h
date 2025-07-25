#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>
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
#include <QString>
#include <QWidget>

class SetupDialog : public QDialog
{
    Q_OBJECT

public:
    struct OperatorData {
        QString callsign;
        QString firstName;
        QString lastName;
        QString locator;
    };
    
    struct ApiCredentials {
        QString qrzUsername;
        QString qrzPassword;
        QString clublogApiKey;
        bool enableQrz;
        bool enableClublog;
    };

    explicit SetupDialog(QWidget *parent = nullptr);
    
    // Getters per i dati configurati
    OperatorData getOperatorData() const;
    ApiCredentials getApiCredentials() const;
    
    // Setters per pre-popolare i campi (per modifica configurazione)
    void setOperatorData(const OperatorData &data);
    void setApiCredentials(const ApiCredentials &credentials);
    
public slots:
    void onCallsignTextEdited(const QString &text);
    void onQrzCredentialsChanged();
    void onClublogCredentialsChanged();
    void onTestQrzConnection();
    void onTestClublogConnection();
    void validateAndAccept();
    void onHelpQrz();
    void onHelpClublog();
    
private:
    void setupUI();
    void setupOperatorTab();
    void setupApiTab();
    void updateApiStatus();
    bool validateOperatorData() const;
    bool validateApiCredentials() const;
    
    // Tab widget principale
    QTabWidget *m_tabWidget;
    
    // Tab Operatore
    QWidget *m_operatorTab;
    QGridLayout *m_operatorLayout;
    QLabel *m_callsignLabel;
    QLineEdit *m_callsignEdit;
    QLabel *m_firstNameLabel;
    QLineEdit *m_firstNameEdit;
    QLabel *m_lastNameLabel;
    QLineEdit *m_lastNameEdit;
    QLabel *m_locatorLabel;
    QLineEdit *m_locatorEdit;
    
    // Tab API
    QWidget *m_apiTab;
    QVBoxLayout *m_apiLayout;
    
    // Gruppo QRZ.com
    QGroupBox *m_qrzGroup;
    QGridLayout *m_qrzLayout;
    QCheckBox *m_enableQrzCheck;
    QLabel *m_qrzUsernameLabel;
    QLineEdit *m_qrzUsernameEdit;
    QLabel *m_qrzPasswordLabel;
    QLineEdit *m_qrzPasswordEdit;
    QPushButton *m_testQrzButton;
    QPushButton *m_helpQrzButton;
    QLabel *m_qrzStatusLabel;
    
    // Gruppo Clublog.org
    QGroupBox *m_clublogGroup;
    QGridLayout *m_clublogLayout;
    QCheckBox *m_enableClublogCheck;
    QLabel *m_clublogApiKeyLabel;
    QLineEdit *m_clublogApiKeyEdit;
    QPushButton *m_testClublogButton;
    QPushButton *m_helpClublogButton;
    QLabel *m_clublogStatusLabel;
    
    // Pulsanti dialog
    QDialogButtonBox *m_buttonBox;
    
    // Note informative
    QLabel *m_infoLabel;
};

#endif // SETUPDIALOG_H