#ifndef LOYALTYLOGPARSER_H
#define LOYALTYLOGPARSER_H

#pragma once

#include "ilogparser.h"
#include <QRegularExpression>

class LoyaltyLogParser : public ILogParser
{
public:
    LoyaltyLogParser();
    ~LoyaltyLogParser() override = default;

    QList<LogEntry> parseFile(const QString& filePath, std::function<void(int)> progressCallback = nullptr) override;
    bool canParse(const QString& filePath) override;

private:
    QRegularExpression m_entryStartRegex;
};
#endif // LOYALTYLOGPARSER_H
