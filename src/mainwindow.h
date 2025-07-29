#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QTimer>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QString>
#include <QEvent>
#include <QFocusEvent>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QStatusBar>
#include <QMenuBar>
#include <QAction>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
#include <QOverload>
#include <QTableView>

#include "database.h"
#include "apiservice.h"
#include "contact.h"
#include "logbookmodel.h"
#include "settingsdialog.h"
#include "adifhandler.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddContact();
    void onClearForm();
    void onCallsignTextEdited(const QString &text);
    void onCallsignChanged();
    void onModeChanged();
    void onCallsignLookupFinished(const QString &callsign, const QString &dxcc, const QString &locator);
    void onCallsignLookupError(const QString &callsign, const QString &error);
    void updateDateTime();
    void onAbout();
    void onSettings();
    void onImportADIF();
    void onExportADIF();

protected:
    void changeEvent(QEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void loadBandsAndModes();
    void loadRSTData();
    QJsonDocument loadJsonFile(const QString &filePath);
    void clearForm();
    bool validateForm();
    void showValidationError(const QString &message);
    void updateContactsTable();
    void configureApiService();
    void pauseTimerForAccessibility();
    void resumeTimerForAccessibility();
    
    // UI Components
    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;
    QGridLayout *m_formLayout;
    QHBoxLayout *m_buttonLayout;
    
    // Form fields
    QLabel *m_dateTimeLabel;
    QLineEdit *m_dateTimeEdit;
    QLabel *m_callsignLabel;
    QLineEdit *m_callsignEdit;
    QLabel *m_bandLabel;
    QComboBox *m_bandCombo;
    QLabel *m_modeLabel;
    QComboBox *m_modeCombo;
    QLabel *m_rstSentLabel;
    QComboBox *m_rstSentCombo;
    QLabel *m_rstReceivedLabel;
    QComboBox *m_rstReceivedCombo;
    QJsonDocument m_modesData;
    QJsonDocument m_rstData;
    QLabel *m_dxccLabel;
    QLineEdit *m_dxccEdit;
    QLabel *m_locatorLabel;
    QLineEdit *m_locatorEdit;
    QLabel *m_operatorLabel;
    QLineEdit *m_operatorEdit;
    
    // Buttons
    QPushButton *m_addButton;
    QPushButton *m_clearButton;
    
    // Table
    QTableView *m_contactsTable;
    LogbookModel *m_contactsModel;
    
    // Services
    Database *m_database;
    ApiService *m_apiService;
    
    // Timer for date/time updates
    QTimer *m_dateTimeTimer;
    
    // Menu actions
    QAction *m_exitAction;
    QAction *m_aboutAction;
    QAction *m_settingsAction;
    QAction *m_importADIFAction;
    QAction *m_exportADIFAction;
};

#endif // MAINWINDOW_H