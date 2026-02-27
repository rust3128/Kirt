#include "talonshttplogparser.h"
#include <QFile>
#include <QFileInfo>
#include <QStringDecoder>

TalonsHttpLogParser::TalonsHttpLogParser()
{
    // Стандартна дата "26.02.24 06:57:04.948"
    m_entryStartRegex.setPattern("^(\\d{1,2}\\.\\d{2}\\.\\d{2}\\s\\d{2}:\\d{2}:\\d{2}\\.\\d{3})\\s+(.*)");
}

bool TalonsHttpLogParser::canParse(const QString& filePath)
{
    // Шукаємо слово "talonshttp"
    if (!QFileInfo(filePath).fileName().contains("talonshttp", Qt::CaseInsensitive)) {
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

// Допоміжна функція: робить красивий XML з відступами
QString TalonsHttpLogParser::formatXml(const QString& xml) const
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
                indentLevel--; // Зменшуємо відступ для закриваючого тегу
            } else if (i + 1 < xml.length() && xml[i + 1] == '?') {
                // Це <?xml...>, не змінюємо відступ
            } else {
                // Відкриваючий тег - робимо перенос рядка і відступ (якщо це не перший тег)
                if (!formatted.isEmpty() && formatted.endsWith('>')) {
                    formatted += '\n' + QString(indentLevel * 4, ' ');
                }
            }
            formatted += c;
        } else if (c == '>') {
            inTag = false;
            formatted += c;

            // Якщо це був відкриваючий тег (і не самозакриваючийся на кшталт <tag/>)
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

QList<LogEntry> TalonsHttpLogParser::parseFile(const QString& filePath, std::function<void(int)> progressCallback)
{
    QList<LogEntry> entries;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) return entries;

    qint64 totalSize = file.size();
    qint64 currentRead = 0;
    int lastPercent = -1;

    QString shortFileName = QFileInfo(filePath).fileName();

    // Тут точно Windows-1251, про це каже сам XML!
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
                // Перед збереженням форматуємо XML, якщо він є
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

            // Для короткої таблиці
            currentEntry.summary = match.captured(2);
            currentEntry.fullText = line;

            hasCurrentEntry = true;
        } else {
            if (hasCurrentEntry) currentEntry.fullText += line; // Склеюємо XML в один рядок перед форматуванням
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
