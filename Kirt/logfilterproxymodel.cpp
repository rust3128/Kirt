#include "logfilterproxymodel.h"
#include "logmodel.h"

LogFilterProxyModel::LogFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent),
    m_startTime(0, 0, 0),        // За замовчуванням від початку доби
    m_endTime(23, 59, 59, 999)   // До самої останньої мілісекунди
{
}

void LogFilterProxyModel::setEnabledFiles(const QSet<QString>& files)
{
    m_enabledFiles = files;
    invalidateFilter();
}

// === НОВИЙ МЕТОД ===
void LogFilterProxyModel::setSearchText(const QString& text)
{
    m_searchText = text;
    invalidateFilter(); // Кажемо таблиці перемалюватися
}

// === НОВИЙ МЕТОД ===
void LogFilterProxyModel::setTimeRange(const QTime& start, const QTime& end)
{
    m_startTime = start;
    m_endTime = end;
    invalidateFilter(); // Кажемо таблиці перемалюватися
}

bool LogFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    // Отримуємо індекси потрібних колонок
    QModelIndex timeIndex = sourceModel()->index(source_row, LogModel::Time, source_parent);
    QModelIndex sourceIndex = sourceModel()->index(source_row, LogModel::Source, source_parent);

    // 1. ПЕРЕВІРКА ФАЙЛУ
    QString filePath = sourceModel()->data(sourceIndex, Qt::UserRole).toString();
    if (!m_enabledFiles.contains(filePath)) {
        return false;
    }

    // 2. ПЕРЕВІРКА ПОШУКУ
    if (!m_searchText.isEmpty()) {
        QString fullText = sourceModel()->data(sourceIndex, Qt::UserRole + 1).toString();
        if (!fullText.contains(m_searchText, Qt::CaseInsensitive)) {
            return false;
        }
    }

    // 3. === ПЕРЕВІРКА ЧАСУ ===
    QDateTime timestamp = sourceModel()->data(timeIndex, Qt::UserRole).toDateTime();
    QTime rowTime = timestamp.time();

    if (rowTime < m_startTime || rowTime > m_endTime) {
        return false; // Час запису не потрапляє у вибраний діапазон
    }

    return true; // Усі перевірки пройдено
}
