#ifndef CARDPAYLOGPARSER_H
#define CARDPAYLOGPARSER_H

#pragma once

#include "ilogparser.h"
#include <QRegularExpression>

class CardPayLogParser : public ILogParser
{
public:
    CardPayLogParser();
    ~CardPayLogParser() override = default;

    QList<LogEntry> parseFile(const QString& filePath, std::function<void(int)> progressCallback = nullptr) override;
    bool canParse(const QString& filePath) override;

private:
    QRegularExpression m_entryStartRegex;
};

#endif // CARDPAYLOGPARSER_H
