#include "logmodel.h"

LogModel::LogModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

// Скільки у нас рядків?
int LogModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_entries.count();
}

// Скільки у нас колонок?
int LogModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return ColumnCount;
}

// Найважливіший метод: що малювати в конкретній клітинці?
QVariant LogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_entries.count())
        return QVariant();

    const LogEntry &entry = m_entries.at(index.row());

    // 1. Для відображення тексту в таблиці
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case Time:    return entry.timestamp.toString("dd.MM.yy HH:mm:ss.zzz");
        case Source: {
            // Обрізаємо назву до знаку підкреслення "_"
            int underscorePos = entry.sourceName.indexOf('_');
            if (underscorePos > 0) {
                return entry.sourceName.left(underscorePos); // Повертаємо ліву частину
            }
            return entry.sourceName; // Якщо "_" немає, повертаємо як є
        }
        case Message: return entry.summary;
        }
    }
    // 2. Для кольору фону
    else if (role == Qt::BackgroundRole) {
        return m_fileColors.value(entry.sourceFile, QColor(Qt::white));
    }
    // 3. === НОВЕ: Для правильного математичного сортування ===
    else if (role == Qt::UserRole) {
        if (index.column() == Time) {
            return entry.timestamp; // Віддаємо сам об'єкт часу, а не текст!
        }
        // Для інших колонок просто віддаємо текст, щоб вони теж могли сортуватися за алфавітом
        if (index.column() == Source) return entry.sourceFile;
        if (index.column() == Message) return entry.summary;
    }
    else if (role == Qt::UserRole + 1) {
        return entry.fullText;
    }
    return QVariant();
}

// Заголовки колонок
QVariant LogModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case Time:    return "Час";
        case Source:  return "Джерело";
        case Message: return "Повідомлення";
        }
    }
    return QVariant();
}

// Додавання нових розпарсених записів у модель
void LogModel::addEntries(const QList<LogEntry>& entries, const QColor& fileColor)
{
    if (entries.isEmpty()) return;

    // Обов'язково попереджаємо таблицю, що ми додаємо нові рядки
    // Це потрібно, щоб таблиця плавно оновилася і не "вилетіла"
    beginInsertRows(QModelIndex(), m_entries.count(), m_entries.count() + entries.count() - 1);

    m_entries.append(entries);

    // Зберігаємо колір для цього файлу
    if (!entries.first().sourceFile.isEmpty()) {
        m_fileColors.insert(entries.first().sourceFile, fileColor);
    }

    endInsertRows(); // Кажемо таблиці, що закінчили додавати
}

// Очищення моделі
void LogModel::clear()
{
    beginResetModel();
    m_entries.clear();
    m_fileColors.clear();
    endResetModel();
}

// Отримати повний об'єкт LogEntry (знадобиться нам, коли будемо клікати на рядок)
LogEntry LogModel::getEntry(int row) const
{
    if (row >= 0 && row < m_entries.count())
        return m_entries.at(row);
    return LogEntry();
}
