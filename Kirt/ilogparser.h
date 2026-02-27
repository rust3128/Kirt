#pragma once

#include <QString>
#include <QDateTime>
#include <QList>
#include <functional>

// Базова "цеглинка" нашого логу
struct LogEntry {
    QDateTime timestamp; // Точний час події
    QString sourceFile;  // Повний шлях до файлу
    QString sourceName;  // Коротка назва джерела (напр., "Operator")
    QString summary;     // Перший рядок (для таблиці)
    QString fullText;    // Повний текст запису (для деталей)
};

// Інтерфейс для всіх майбутніх парсерів
class ILogParser
{
public:
    virtual ~ILogParser() = default;

    // Головний метод: прочитати файл і повернути список подій
    virtual QList<LogEntry> parseFile(const QString& filePath, std::function<void(int)> progressCallback = nullptr) = 0;

    // Метод для Фабрики: чи вміє цей парсер читати цей конкретний файл?
    virtual bool canParse(const QString& filePath) = 0;
};
