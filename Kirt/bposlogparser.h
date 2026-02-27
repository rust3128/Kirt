#ifndef BPOSLOGPARSER_H
#define BPOSLOGPARSER_H

#pragma once

#include "ilogparser.h"
#include <QRegularExpression>

class BPosLogParser : public ILogParser
{
public:
    BPosLogParser();
    ~BPosLogParser() override = default;

    QList<LogEntry> parseFile(const QString& filePath, std::function<void(int)> progressCallback = nullptr) override;
    bool canParse(const QString& filePath) override;

private:
    QRegularExpression m_entryStartRegex;
    QString formatXml(const QString& xml) const;
};
#endif // BPOSLOGPARSER_H
