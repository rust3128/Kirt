#include "logsyntaxhighlighter.h"

LogSyntaxHighlighter::LogSyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // 1. XML Теги (наприклад: <Data>, </Type>)
    xmlTagFormat.setForeground(QColor("#000080")); // Navy (Темно-синій)
    xmlTagFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("<[^>]+>");
    rule.format = xmlTagFormat;
    highlightingRules.append(rule);

    // 2. Рядки в лапках (значення в JSON або атрибути в XML: "value")
    stringFormat.setForeground(QColor("#008000")); // Green (Зелений)
    rule.pattern = QRegularExpression("\".*?\"");
    rule.format = stringFormat;
    highlightingRules.append(rule);

    // 3. Ключі JSON (наприклад: "holdId":)
    // Це правило йде ПІСЛЯ рядків, щоб перетерти зелений колір на синій для ключів
    jsonKeyFormat.setForeground(QColor("#005CC5")); // Яскраво-синій
    jsonKeyFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("\"[^\"]*\"(?=\\s*:)");
    rule.format = jsonKeyFormat;
    highlightingRules.append(rule);

    // 4. Числа (наприклад: 123, 61.90)
    numberFormat.setForeground(QColor("#D73A49")); // Red (Червоний)
    rule.pattern = QRegularExpression("\\b\\d+(\\.\\d+)?\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    // 5. Ключові слова JSON (true, false, null)
    keywordFormat.setForeground(QColor("#D73A49")); // Червоний
    keywordFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("\\b(true|false|null)\\b");
    rule.format = keywordFormat;
    highlightingRules.append(rule);
}

void LogSyntaxHighlighter::highlightBlock(const QString &text)
{
    // Проходимось по всіх правилах і розфарбовуємо блок тексту
    for (const HighlightingRule &rule : std::as_const(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
