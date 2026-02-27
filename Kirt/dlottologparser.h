#ifndef DLOTTOLOGPARSER_H
#define DLOTTOLOGPARSER_H

#pragma once

#include "ilogparser.h"
#include <QRegularExpression>

class DLottoLogParser : public ILogParser
{
public:
    DLottoLogParser();
    ~DLottoLogParser() override = default;

    QList<LogEntry> parseFile(const QString& filePath, std::function<void(int)> progressCallback = nullptr) override;
    bool canParse(const QString& filePath) override;

private:
    QRegularExpression m_entryStartRegex;
};

#endif // DLOTTOLOGPARSER_H
