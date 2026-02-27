#ifndef LOGSYNTAXHIGHLIGHTER_H
#define LOGSYNTAXHIGHLIGHTER_H

#pragma once

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>

class LogSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit LogSyntaxHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QList<HighlightingRule> highlightingRules;

    QTextCharFormat xmlTagFormat;
    QTextCharFormat stringFormat;
    QTextCharFormat jsonKeyFormat;
    QTextCharFormat numberFormat;
    QTextCharFormat keywordFormat;
};
#endif // LOGSYNTAXHIGHLIGHTER_H
