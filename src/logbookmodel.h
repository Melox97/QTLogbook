#ifndef LOGBOOKMODEL_H
#define LOGBOOKMODEL_H

#include <QtCore/QAbstractTableModel>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include "contact.h"

class LogbookModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        ColumnDateTime = 0,
        ColumnCallsign,
        ColumnBand,
        ColumnMode,
        ColumnRSTSent,
        ColumnRSTReceived,
        ColumnDXCC,
        ColumnLocator,
        ColumnOperator,
        ColumnCount
    };

    explicit LogbookModel(QObject *parent = nullptr);

    // QAbstractTableModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Data management
    void setContacts(const QList<Contact> &contacts);
    void addContact(const Contact &contact);
    void updateContact(int row, const Contact &contact);
    void removeContact(int row);
    Contact getContact(int row) const;
    void clear();
    void refresh();

    // Sorting and filtering
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    void setFilter(const QString &filter);
    QString getFilter() const { return m_filter; }

private:
    void applyFilter();
    QString formatDateTime(const QDateTime &dateTime) const;
    
    QList<Contact> m_contacts;
    QList<Contact> m_filteredContacts;
    QString m_filter;
    int m_sortColumn;
    Qt::SortOrder m_sortOrder;
};

#endif // LOGBOOKMODEL_H