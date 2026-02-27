#include "operatorlogparser.h"
#include <QFile>
#include <QFileInfo>
#include <QStringDecoder>

OperatorLogParser::OperatorLogParser()
{
    // Наш регулярний вираз для дати
    m_entryStartRegex.setPattern("^(\\d{1,2}\\.\\d{2}\\.\\d{2}\\s\\d{2}:\\d{2}:\\d{2}\\.\\d{3})\\s+(.*)");
}

bool OperatorLogParser::canParse(const QString& filePath)
{
    QString fileName = QFileInfo(filePath).fileName();
    if (!fileName.contains("Operator", Qt::CaseInsensitive)) {
        return false;
    }



    QFile file(filePath);


    // УВАГА: Прибрали QIODevice::Text, тепер читаємо "сирі" байти
    if (!file.open(QIODevice::ReadOnly))
        return false;

    // Створюємо декодер
    QStringDecoder decoder("windows-1251");

    for (int i = 0; i < 10 && !file.atEnd(); ++i) {
        QByteArray rawLine = file.readLine();

        // Магія Qt 6: якщо є словник 1251 - беремо його. Якщо немає - просимо Windows перекласти байти
        QString line = decoder.isValid() ? decoder(rawLine) : QString::fromLocal8Bit(rawLine);

        if (m_entryStartRegex.match(line).hasMatch()) {
            return true;
        }
    }
    return false;
}

QList<LogEntry> OperatorLogParser::parseFile(const QString& filePath, std::function<void(int)> progressCallback)
{
    QList<LogEntry> entries;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly))
        return entries;

    // === ДОДАНО ДЛЯ ПРОГРЕСУ ===
    qint64 totalSize = file.size();
    qint64 currentRead = 0;
    int lastPercent = -1;
    // ===========================

    QString shortFileName = QFileInfo(filePath).fileName();
    QStringDecoder decoder("windows-1251");

    LogEntry currentEntry;
    bool hasCurrentEntry = false;

    while (!file.atEnd()) {
        // Читаємо рядок побайтово
        QByteArray rawLine = file.readLine();

        // === ДОДАНО ДЛЯ ПРОГРЕСУ ===
        currentRead += rawLine.size();
        if (progressCallback && totalSize > 0) {
            int percent = static_cast<int>((currentRead * 100) / totalSize);
            if (percent != lastPercent) {
                progressCallback(percent);
                lastPercent = percent;
            }
        }
        // ===========================

        // Декодуємо в нормальний текст
        QString line = decoder.isValid() ? decoder(rawLine) : QString::fromLocal8Bit(rawLine);

        // Оскільки ми читаємо байти напряму, треба прибрати невидимі символи переносу рядка (\r, \n)
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

    // Зберігаємо останній запис
    if (hasCurrentEntry) {
        entries.append(currentEntry);
    }

    return entries;
}
