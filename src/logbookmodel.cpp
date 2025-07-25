#include "logbookmodel.h"
#include <QtCore/QDateTime>
#include <QtCore/QDebug>

LogbookModel::LogbookModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_sortColumn(-1)
    , m_sortOrder(Qt::AscendingOrder)
{
}

int LogbookModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_filteredContacts.size();
}

int LogbookModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return ColumnCount;
}

QVariant LogbookModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_filteredContacts.size()) {
        return QVariant();
    }
    
    const Contact &contact = m_filteredContacts.at(index.row());
    
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case ColumnDateTime:
            return formatDateTime(contact.dateTime());
        case ColumnCallsign:
            return contact.callsign();
        case ColumnBand:
            return contact.band();
        case ColumnMode:
            return contact.mode();
        case ColumnRSTSent:
            return contact.rstSent();
        case ColumnRSTReceived:
            return contact.rstReceived();
        case ColumnDXCC:
            return contact.dxcc();
        case ColumnLocator:
            return contact.locator();
        case ColumnOperator:
            return contact.operatorCall();
        }
    } else if (role == Qt::TextAlignmentRole) {
        switch (index.column()) {
        case ColumnDateTime:
        case ColumnRSTSent:
        case ColumnRSTReceived:
            return Qt::AlignCenter;
        default:
            return Qt::AlignLeft;
        }
    }
    
    return QVariant();
}

QVariant LogbookModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case ColumnDateTime:
            return "Data/Ora";
        case ColumnCallsign:
            return "Nominativo";
        case ColumnBand:
            return "Banda";
        case ColumnMode:
            return "Modo";
        case ColumnRSTSent:
            return "RST TX";
        case ColumnRSTReceived:
            return "RST RX";
        case ColumnDXCC:
            return "DXCC";
        case ColumnLocator:
            return "Locatore";
        case ColumnOperator:
            return "Operatore";
        }
    }
    
    return QVariant();
}

Qt::ItemFlags LogbookModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void LogbookModel::setContacts(const QList<Contact> &contacts)
{
    beginResetModel();
    m_contacts = contacts;
    applyFilter();
    endResetModel();
}

void LogbookModel::addContact(const Contact &contact)
{
    beginInsertRows(QModelIndex(), 0, 0);
    m_contacts.prepend(contact);
    applyFilter();
    endInsertRows();
}

void LogbookModel::updateContact(int row, const Contact &contact)
{
    if (row >= 0 && row < m_filteredContacts.size()) {
        // Trova l'indice nel vettore originale
        int originalIndex = -1;
        for (int i = 0; i < m_contacts.size(); ++i) {
            if (m_contacts[i].id() == m_filteredContacts[row].id()) {
                originalIndex = i;
                break;
            }
        }
        
        if (originalIndex >= 0) {
            m_contacts[originalIndex] = contact;
            applyFilter();
            emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
        }
    }
}

void LogbookModel::removeContact(int row)
{
    if (row >= 0 && row < m_filteredContacts.size()) {
        beginRemoveRows(QModelIndex(), row, row);
        
        // Trova e rimuovi dal vettore originale
        int contactId = m_filteredContacts[row].id();
        for (int i = 0; i < m_contacts.size(); ++i) {
            if (m_contacts[i].id() == contactId) {
                m_contacts.removeAt(i);
                break;
            }
        }
        
        applyFilter();
        endRemoveRows();
    }
}

Contact LogbookModel::getContact(int row) const
{
    if (row >= 0 && row < m_filteredContacts.size()) {
        return m_filteredContacts[row];
    }
    return Contact();
}

void LogbookModel::clear()
{
    beginResetModel();
    m_contacts.clear();
    m_filteredContacts.clear();
    endResetModel();
}

void LogbookModel::refresh()
{
    beginResetModel();
    applyFilter();
    endResetModel();
}

void LogbookModel::sort(int column, Qt::SortOrder order)
{
    if (column < 0 || column >= ColumnCount) {
        return;
    }
    
    m_sortColumn = column;
    m_sortOrder = order;
    
    beginResetModel();
    
    std::sort(m_filteredContacts.begin(), m_filteredContacts.end(), 
              [this](const Contact &a, const Contact &b) -> bool {
        
        switch (m_sortColumn) {
        case ColumnDateTime:
            if (m_sortOrder == Qt::AscendingOrder) {
                return a.dateTime() < b.dateTime();
            } else {
                return a.dateTime() > b.dateTime();
            }
        case ColumnCallsign:
            if (m_sortOrder == Qt::AscendingOrder) {
                return a.callsign() < b.callsign();
            } else {
                return a.callsign() > b.callsign();
            }
        case ColumnBand:
            if (m_sortOrder == Qt::AscendingOrder) {
                return a.band() < b.band();
            } else {
                return a.band() > b.band();
            }
        case ColumnMode:
            if (m_sortOrder == Qt::AscendingOrder) {
                return a.mode() < b.mode();
            } else {
                return a.mode() > b.mode();
            }
        case ColumnRSTSent:
            if (m_sortOrder == Qt::AscendingOrder) {
                return a.rstSent() < b.rstSent();
            } else {
                return a.rstSent() > b.rstSent();
            }
        case ColumnRSTReceived:
            if (m_sortOrder == Qt::AscendingOrder) {
                return a.rstReceived() < b.rstReceived();
            } else {
                return a.rstReceived() > b.rstReceived();
            }
        case ColumnDXCC:
            if (m_sortOrder == Qt::AscendingOrder) {
                return a.dxcc() < b.dxcc();
            } else {
                return a.dxcc() > b.dxcc();
            }
        case ColumnLocator:
            if (m_sortOrder == Qt::AscendingOrder) {
                return a.locator() < b.locator();
            } else {
                return a.locator() > b.locator();
            }
        case ColumnOperator:
            if (m_sortOrder == Qt::AscendingOrder) {
                return a.operatorCall() < b.operatorCall();
            } else {
                return a.operatorCall() > b.operatorCall();
            }
        }
        return false;
    });
    
    endResetModel();
}

void LogbookModel::setFilter(const QString &filter)
{
    m_filter = filter;
    beginResetModel();
    applyFilter();
    endResetModel();
}

void LogbookModel::applyFilter()
{
    m_filteredContacts.clear();
    
    if (m_filter.isEmpty()) {
        m_filteredContacts = m_contacts;
    } else {
        QString filterLower = m_filter.toLower();
        
        for (const Contact &contact : m_contacts) {
            if (contact.callsign().toLower().contains(filterLower) ||
                contact.band().toLower().contains(filterLower) ||
                contact.mode().toLower().contains(filterLower) ||
                contact.dxcc().toLower().contains(filterLower) ||
                contact.locator().toLower().contains(filterLower)) {
                m_filteredContacts.append(contact);
            }
        }
    }
}

QString LogbookModel::formatDateTime(const QDateTime &dateTime) const
{
    return dateTime.toString("yyyy-MM-dd hh:mm");
}