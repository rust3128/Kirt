#include "mposloyaltylogparser.h"
#include <QFile>
#include <QFileInfo>
#include <QStringDecoder>

MposLoyaltyLogParser::MposLoyaltyLogParser()
{
    // Дата формату: 26.02.24 06:21:29.337
    m_entryStartRegex.setPattern("^(\\d{1,2}\\.\\d{2}\\.\\d{2}\\s\\d{2}:\\d{2}:\\d{2}\\.\\d{3})\\s+(.*)");
}

bool MposLoyaltyLogParser::canParse(const QString& filePath)
{
    // Шукаємо слово "loyalty"
    if (!QFileInfo(filePath).fileName().contains("loyalty", Qt::CaseInsensitive)) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QStringDecoder decoder("windows-1251");
    for (int i = 0; i < 10 && !file.atEnd(); ++i) {
        QByteArray rawLine = file.readLine();
        QString line = decoder.isValid() ? decoder(rawLine) : QString::fromLocal8Bit(rawLine);

        if (m_entryStartRegex.match(line).hasMatch()) {
            return true;
        }
    }
    return false;
}

// Форматування XML
QString MposLoyaltyLogParser::formatXml(const QString& xml) const
{
    QString formatted;
    int indentLevel = 0;
    bool inTag = false;
    bool inClosingTag = false;

    for (int i = 0; i < xml.length(); ++i) {
        QChar c = xml[i];

        if (c == '<') {
            inTag = true;
            if (i + 1 < xml.length() && xml[i + 1] == '/') {
                inClosingTag = true;
                indentLevel--;
            } else if (i + 1 < xml.length() && xml[i + 1] == '?') {
                // <?xml ...>
            } else {
                if (!formatted.isEmpty() && formatted.endsWith('>')) {
                    formatted += '\n' + QString(indentLevel * 4, ' ');
                }
            }
            formatted += c;
        } else if (c == '>') {
            inTag = false;
            formatted += c;

            if (!inClosingTag && !(i > 0 && xml[i - 1] == '/') && !(i > 1 && xml[i - 1] == '?')) {
                indentLevel++;
            }
            inClosingTag = false;
        } else {
            formatted += c;
        }
    }
    return formatted;
}

QList<LogEntry> MposLoyaltyLogParser::parseFile(const QString& filePath, std::function<void(int)> progressCallback)
{
    QList<LogEntry> entries;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) return entries;

    qint64 totalSize = file.size();
    qint64 currentRead = 0;
    int lastPercent = -1;

    QString shortFileName = QFileInfo(filePath).fileName();
    QStringDecoder decoder("windows-1251");

    LogEntry currentEntry;
    bool hasCurrentEntry = false;

    while (!file.atEnd()) {
        QByteArray rawLine = file.readLine();
        currentRead += rawLine.size();

        if (progressCallback && totalSize > 0) {
            int percent = static_cast<int>((currentRead * 100) / totalSize);
            if (percent != lastPercent) {
                progressCallback(percent);
                lastPercent = percent;
            }
        }

        QString line = decoder.isValid() ? decoder(rawLine) : QString::fromLocal8Bit(rawLine);
        while (line.endsWith('\n') || line.endsWith('\r')) {
            line.chop(1);
        }

        QRegularExpressionMatch match = m_entryStartRegex.match(line);

        if (match.hasMatch()) {
            if (hasCurrentEntry) {
                int xmlStart = currentEntry.fullText.indexOf("<?xml");
                if (xmlStart != -1) {
                    QString beforeXml = currentEntry.fullText.left(xmlStart);
                    QString xmlPart = currentEntry.fullText.mid(xmlStart);
                    currentEntry.fullText = beforeXml + formatXml(xmlPart);
                }
                entries.append(currentEntry);
            }

            currentEntry = LogEntry();
            currentEntry.sourceFile = filePath;
            currentEntry.sourceName = shortFileName;

            QString timeStr = match.captured(1);
            currentEntry.timestamp = QDateTime::fromString(timeStr, "dd.MM.yy HH:mm:ss.zzz");

            currentEntry.summary = match.captured(2);
            currentEntry.fullText = line;

            hasCurrentEntry = true;
        } else {
            if (hasCurrentEntry) currentEntry.fullText += line;
        }
    }

    if (hasCurrentEntry) {
        int xmlStart = currentEntry.fullText.indexOf("<?xml");
        if (xmlStart != -1) {
            QString beforeXml = currentEntry.fullText.left(xmlStart);
            QString xmlPart = currentEntry.fullText.mid(xmlStart);
            currentEntry.fullText = beforeXml + formatXml(xmlPart);
        }
        entries.append(currentEntry);
    }

    return entries;
}
