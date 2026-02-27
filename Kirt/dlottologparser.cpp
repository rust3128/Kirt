#include "dlottologparser.h"
#include <QFile>
#include <QFileInfo>
#include <QStringDecoder>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

DLottoLogParser::DLottoLogParser()
{
    // Дата формату: 26.02.24 07:05:00.234
    m_entryStartRegex.setPattern("^(\\d{1,2}\\.\\d{2}\\.\\d{2}\\s\\d{2}:\\d{2}:\\d{2}\\.\\d{3})\\s+(.*)");
}

bool DLottoLogParser::canParse(const QString& filePath)
{
    // Шукаємо DLotto в імені файлу
    if (!QFileInfo(filePath).fileName().contains("DLotto", Qt::CaseInsensitive)) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    // Цей лог використовує UTF-8 для відповідей JSON
    QStringDecoder decoder("utf-8");
    for (int i = 0; i < 10 && !file.atEnd(); ++i) {
        QByteArray rawLine = file.readLine();
        QString line = decoder.isValid() ? decoder(rawLine) : QString::fromUtf8(rawLine);

        if (m_entryStartRegex.match(line).hasMatch()) {
            return true;
        }
    }
    return false;
}

QList<LogEntry> DLottoLogParser::parseFile(const QString& filePath, std::function<void(int)> progressCallback)
{
    QList<LogEntry> entries;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly)) return entries;

    qint64 totalSize = file.size();
    qint64 currentRead = 0;
    int lastPercent = -1;

    QString shortFileName = QFileInfo(filePath).fileName();
    QStringDecoder decoder("utf-8");

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

        QString line = decoder.isValid() ? decoder(rawLine) : QString::fromUtf8(rawLine);
        while (line.endsWith('\n') || line.endsWith('\r')) {
            line.chop(1);
        }

        QRegularExpressionMatch match = m_entryStartRegex.match(line);

        if (match.hasMatch()) {
            if (hasCurrentEntry) entries.append(currentEntry);

            currentEntry = LogEntry();
            currentEntry.sourceFile = filePath;
            currentEntry.sourceName = shortFileName;

            QString timeStr = match.captured(1);
            currentEntry.timestamp = QDateTime::fromString(timeStr, "dd.MM.yy HH:mm:ss.zzz");

            currentEntry.summary = match.captured(2);

            // ==========================================================
            // === ШУКАЄМО JSON (І об'єкти {}, і масиви [])           ===
            // ==========================================================
            QString detailedText = line;

            // Щоб не сплутати початок JSON-масиву '[' з тегом логу '[TDLotto...]',
            // знаходимо кінець тегу ']' і шукаємо тільки після нього!
            int tagEnd = detailedText.indexOf(']');
            int searchStart = (tagEnd != -1) ? tagEnd : 0;

            // Шукаємо першу { або [
            int jsonStart = detailedText.indexOf(QRegularExpression("[{\\[]"), searchStart);
            int jsonEnd = -1;

            if (jsonStart != -1) {
                QChar startChar = detailedText[jsonStart];
                QChar endChar = (startChar == '{') ? '}' : ']';
                jsonEnd = detailedText.lastIndexOf(endChar);
            }

            if (jsonStart != -1 && jsonEnd != -1 && jsonEnd > jsonStart) {
                QString jsonStr = detailedText.mid(jsonStart, jsonEnd - jsonStart + 1);

                QJsonParseError parseError;
                QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonStr.toUtf8(), &parseError);

                if (parseError.error == QJsonParseError::NoError) {
                    // Форматуємо. QJsonDocument автоматично розкодує \u041a у кирилицю!
                    QString prettyJson = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Indented));
                    detailedText.replace(jsonStart, jsonEnd - jsonStart + 1, "\n" + prettyJson);
                }
            }
            // ==========================================================

            currentEntry.fullText = detailedText;
            hasCurrentEntry = true;
        } else {
            if (hasCurrentEntry) currentEntry.fullText += "\n" + line;
        }
    }

    if (hasCurrentEntry) entries.append(currentEntry);

    return entries;
}
