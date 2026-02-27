#ifndef OPTIMUMLOGPARSER_H
#define OPTIMUMLOGPARSER_H

#pragma once

#include "ilogparser.h"
#include <QRegularExpression>

class OptimumLogParser : public ILogParser
{
public:
    OptimumLogParser();
    ~OptimumLogParser() override = default;

    QList<LogEntry> parseFile(const QString& filePath, std::function<void(int)> progressCallback = nullptr) override;
    bool canParse(const QString& filePath) override;

private:
    QRegularExpression m_entryStartRegex;
};

#endif // OPTIMUMLOGPARSER_H
