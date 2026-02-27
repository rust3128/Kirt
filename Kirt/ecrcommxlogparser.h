#pragma once

#include "ilogparser.h"
#include <QRegularExpression>

class EcrCommXLogParser : public ILogParser
{
public:
    EcrCommXLogParser();
    ~EcrCommXLogParser() override = default;

    QList<LogEntry> parseFile(const QString& filePath, std::function<void(int)> progressCallback = nullptr) override;
    bool canParse(const QString& filePath) override;

private:
    QRegularExpression m_entryStartRegex;
};
