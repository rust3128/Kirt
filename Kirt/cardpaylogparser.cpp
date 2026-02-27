#include "cardpaylogparser.h"
#include <QFile>
#include <QFileInfo>
#include <QStringDecoder>

CardPayLogParser::CardPayLogParser()
{
    // Новий Regex: шукає "24.02.2026 06:26:21"
    m_entryStartRegex.setPattern("^(\\d{2}\\.\\d{2}\\.\\d{4}\\s\\d{2}:\\d{2}:\\d{2})\\s+(.*)");
}

bool CardPayLogParser::canParse(const QString& filePath)
{
    // Шукаємо слово "cardpay" у назві файлу
    if (!QFileInfo(filePath).fileName().contains("cardpay", Qt::CaseInsensitive)) {
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

QList<LogEntry> CardPayLogParser::parseFile(const QString& filePath, std::function<void(int)> progressCallback)
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
            if (hasCurrentEntry) entries.append(currentEntry);

            currentEntry = LogEntry();
            currentEntry.sourceFile = filePath;
            currentEntry.sourceName = shortFileName;

            // === ЗМІНЕНО: Новий шаблон часу ===
            QString timeStr = match.captured(1);
            currentEntry.timestamp = QDateTime::fromString(timeStr, "dd.MM.yyyy HH:mm:ss");
            // ===================================

            currentEntry.summary = match.captured(2);
            currentEntry.fullText = line;

            hasCurrentEntry = true;
        } else {
            // Багаторядковий чек (наприклад, між print і GET_NEXT)
            if (hasCurrentEntry) currentEntry.fullText += "\n" + line;
        }
    }

    if (hasCurrentEntry) entries.append(currentEntry);

    // =========================================================
    // === ІНТЕРПОЛЯЦІЯ МІЛІСЕКУНД (така сама як в EcrCommX) ===
    // =========================================================
    int i = 0;
    while (i < entries.count()) {
        int j = i + 1;

        while (j < entries.count() && entries[i].timestamp == entries[j].timestamp) {
            j++;
        }

        int count = j - i;

        if (count > 1) {
            int step = 1000 / count;
            for (int k = 0; k < count; ++k) {
                entries[i + k].timestamp = entries[i + k].timestamp.addMSecs(k * step);
            }
        }
        i = j;
    }

    return entries;
}
