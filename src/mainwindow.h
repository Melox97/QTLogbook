#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTableView>
#include <QStatusBar>
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QTimer>
#include "contact.h"
#include "database.h"
#include "apiservice.h"
#include "logbookmodel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddContact();
    void onClearForm();
    void onCallsignTextEdited(const QString &text);
    void onCallsignChanged();
    void onCallsignLookupFinished(const QString &callsign, const QString &dxcc, const QString &locator);
    void onCallsignLookupError(const QString &callsign, const QString &error);
    void updateDateTime();
    void onAbout();
    void onSettings();

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void loadBandsAndModes();
    void clearForm();
    bool validateForm();
    void showValidationError(const QString &message);
    void updateContactsTable();
    void configureApiService();
    
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
    QLineEdit *m_rstSentEdit;
    QLabel *m_rstReceivedLabel;
    QLineEdit *m_rstReceivedEdit;
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
};

#endif // MAINWINDOW_H