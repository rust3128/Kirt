#ifndef LOGMODEL_H
#define LOGMODEL_H

#pragma once

#include <QAbstractTableModel>
#include <QList>
#include <QMap>
#include <QColor>
#include "ilogparser.h" // Тут лежить наша структура LogEntry

class LogModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    // Назви колонок для зручності
    enum Columns { Time = 0, Source, Message, ColumnCount };

    explicit LogModel(QObject *parent = nullptr);
    ~LogModel() override = default;

    // Обов'язкові методи для таблиці
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Наші власні методи для керування даними
    void addEntries(const QList<LogEntry>& entries, const QColor& fileColor);
    void clear();
    LogEntry getEntry(int row) const;

private:
    QList<LogEntry> m_entries;             // Всі записи (з усіх файлів)
    QMap<QString, QColor> m_fileColors;    // Словник кольорів: Шлях_до_файлу -> Колір
};
#endif // LOGMODEL_H
