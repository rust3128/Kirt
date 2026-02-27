#ifndef TALONSHTTPLOGPARSER_H
#define TALONSHTTPLOGPARSER_H

#pragma once

#include "ilogparser.h"
#include <QRegularExpression>

class TalonsHttpLogParser : public ILogParser
{
public:
    TalonsHttpLogParser();
    ~TalonsHttpLogParser() override = default;

    QList<LogEntry> parseFile(const QString& filePath, std::function<void(int)> progressCallback = nullptr) override;
    bool canParse(const QString& filePath) override;

private:
    QRegularExpression m_entryStartRegex;

    // Допоміжна функція для красивого форматування XML
    QString formatXml(const QString& xml) const;
};

#endif // TALONSHTTPLOGPARSER_H
