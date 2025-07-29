#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QtWidgets/QDialog>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#include <QtCore/QString>
#include "database.h"

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    
    struct OperatorData {
        QString callsign;
        QString firstName;
        QString lastName;
        QString locator;
    };
    
    struct ApiCredentials {
        bool enableQrz = false;
        bool enableClublog = false;
        QString qrzUsername;
        QString qrzPassword;
        QString clublogApiKey;
    };
    
    void setOperatorData(const OperatorData &data);
    void setApiCredentials(const ApiCredentials &credentials);
    
    OperatorData getOperatorData() const;
    ApiCredentials getApiCredentials() const;
    Database::ThemeMode getThemeSettings() const;

private slots:
    void onCallsignTextEdited(const QString &text);
    void onQrzCredentialsChanged();
    void onClublogCredentialsChanged();
    void onTestQrzConnection();
    void onTestClublogConnection();
    void onHelpQrz();
    void onHelpClublog();
    void onResetToDefaults();
    void onApplyTheme();
    void validateAndAccept();

private:
    void setupUI();
    void setupOperatorTab();
    void setupApiTab();
    void setupAdvancedTab();
    void updateApiStatus();
    bool validateOperatorData() const;
    bool validateApiCredentials() const;
    bool confirmReset();
    void resetAllSettings();
    bool createBackupBeforeReset();
    
    // UI Components
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
    
    // QRZ.com
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
    
    // Clublog.org
    QGroupBox *m_clublogGroup;
    QGridLayout *m_clublogLayout;
    QCheckBox *m_enableClublogCheck;
    QLabel *m_clublogApiKeyLabel;
    QLineEdit *m_clublogApiKeyEdit;
    QPushButton *m_testClublogButton;
    QPushButton *m_helpClublogButton;
    QLabel *m_clublogStatusLabel;
    
    // Tab Avanzate
    QWidget *m_advancedTab;
    QVBoxLayout *m_advancedLayout;
    
    // Gruppo Tema
    QGroupBox *m_themeGroup;
    QVBoxLayout *m_themeLayout;
    QLabel *m_themeLabel;
    QRadioButton *m_systemThemeRadio;
    QRadioButton *m_lightThemeRadio;
    QRadioButton *m_darkThemeRadio;
    QRadioButton *m_highContrastThemeRadio;
    QPushButton *m_applyThemeButton;
    
    // Gruppo Reset
    QGroupBox *m_resetGroup;
    QVBoxLayout *m_resetLayout;
    QPushButton *m_resetButton;
    QLabel *m_resetWarningLabel;
    
    // Pulsanti dialog
    QDialogButtonBox *m_buttonBox;
    
    // Flag per reset database
    bool m_resetDatabase = false;
};

#endif // SETTINGSDIALOG_H