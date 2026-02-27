#include "optimumlogparser.h"
#include <QFile>
#include <QFileInfo>
#include <QStringDecoder>

OptimumLogParser::OptimumLogParser()
{
    // Шукаємо дату, а потім витягуємо все інше.
    // У майбутньому цей Regex можна буде ускладнити спеціально для Optimum
    m_entryStartRegex.setPattern("^(\\d{1,2}\\.\\d{2}\\.\\d{2}\\s\\d{2}:\\d{2}:\\d{2}\\.\\d{3})\\s+(.*)");
}

bool OptimumLogParser::canParse(const QString& filePath)
{
    // 1. Спочатку перевіряємо, чи це взагалі файл Optimum
    QString fileName = QFileInfo(filePath).fileName();
    if (!fileName.contains("Optimum_", Qt::CaseInsensitive)) {
        return false;
    }

    // 2. Якщо це Optimum, перевіряємо, чи можемо ми прочитати його рядки
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

QList<LogEntry> OptimumLogParser::parseFile(const QString& filePath, std::function<void(int)> progressCallback)
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
            if (hasCurrentEntry) {
                currentEntry.fullText += "\n" + line;
            }
        }
    }

    if (hasCurrentEntry) {
        entries.append(currentEntry);
    }

    return entries;
}
