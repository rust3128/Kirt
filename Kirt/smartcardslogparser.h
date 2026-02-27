#ifndef SMARTCARDSLOGPARSER_H
#define SMARTCARDSLOGPARSER_H

#pragma once

#include "ilogparser.h"
#include <QRegularExpression>

class SmartCardsLogParser : public ILogParser
{
public:
    SmartCardsLogParser();
    ~SmartCardsLogParser() override = default;

    QList<LogEntry> parseFile(const QString& filePath, std::function<void(int)> progressCallback = nullptr) override;
    bool canParse(const QString& filePath) override;

private:
    QRegularExpression m_entryStartRegex;
};

#endif // SMARTCARDSLOGPARSER_H
