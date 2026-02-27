#include "ecrcommxlogparser.h"
#include <QFile>
#include <QFileInfo>
#include <QStringDecoder>

EcrCommXLogParser::EcrCommXLogParser()
{
    // Шукаємо дату у форматі 24/02/2026 03:36:48
    m_entryStartRegex.setPattern("^(\\d{2}/\\d{2}/\\d{4}\\s\\d{2}:\\d{2}:\\d{2})\\s+(.*)");
}

bool EcrCommXLogParser::canParse(const QString& filePath)
{
    if (!QFileInfo(filePath).fileName().contains("ECRCommX", Qt::CaseInsensitive)) {
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

QList<LogEntry> EcrCommXLogParser::parseFile(const QString& filePath, std::function<void(int)> progressCallback)
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

            // Читаємо час (мілісекунди автоматично стають .000)
            QString timeStr = match.captured(1);
            currentEntry.timestamp = QDateTime::fromString(timeStr, "dd/MM/yyyy HH:mm:ss");

            currentEntry.summary = match.captured(2);
            currentEntry.fullText = line;

            hasCurrentEntry = true;
        } else {
            if (hasCurrentEntry) currentEntry.fullText += "\n" + line;
        }
    }

    if (hasCurrentEntry) entries.append(currentEntry);

    // =========================================================
    // === МАГІЯ: РІВНОМІРНИЙ РОЗПОДІЛ МІЛІСЕКУНД ПО СЕКУНДІ ===
    // =========================================================
    int i = 0;
    while (i < entries.count()) {
        int j = i + 1;

        // Шукаємо, скільки записів підряд мають ту саму секунду
        while (j < entries.count() && entries[i].timestamp == entries[j].timestamp) {
            j++;
        }

        int count = j - i; // Кількість записів у цій секунді

        if (count > 1) {
            // Рахуємо крок. Наприклад, якщо 4 записи, крок = 1000 / 4 = 250 мс
            int step = 1000 / count;

            // Додаємо кожному запису його частку мілісекунд
            for (int k = 0; k < count; ++k) {
                entries[i + k].timestamp = entries[i + k].timestamp.addMSecs(k * step);
            }
        }

        i = j; // Перестрибуємо до наступної унікальної секунди
    }
    // =========================================================

    return entries;
}
