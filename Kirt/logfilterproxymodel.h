#ifndef LOGFILTERPROXYMODEL_H
#define LOGFILTERPROXYMODEL_H

#pragma once

#include <QSortFilterProxyModel>
#include <QSet>
#include <QString>
#include <QTime>

class LogFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit LogFilterProxyModel(QObject *parent = nullptr);

    // Метод, яким ми будемо передавати список увімкнених файлів
    void setEnabledFiles(const QSet<QString>& files);

    void setSearchText(const QString& text);

    void setTimeRange(const QTime& start, const QTime& end);

protected:
    // Головний метод Qt для фільтрації: повертає true, якщо рядок треба показати
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    QSet<QString> m_enabledFiles;
    QString m_searchText;

    QTime m_startTime;
    QTime m_endTime;
};
#endif // LOGFILTERPROXYMODEL_H
