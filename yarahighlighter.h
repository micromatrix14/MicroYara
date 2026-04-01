#ifndef YARAHIGHLIGHTER_H
#define YARAHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QFont>

class YaraHighlighter : public QSyntaxHighlighter
{
public:
    explicit YaraHighlighter(QObject *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightingRule> highlightingRules;

    QTextCharFormat keywords1Format;  // rule, meta, strings, condition
    QTextCharFormat keywords2Format;  // int8, int16, int32, etc.
    QTextCharFormat keywords3Format;  // global, private, ascii, etc.
    QTextCharFormat keywords4Format;  // all, any, at, contains, etc.
    QTextCharFormat stringFormat;
    QTextCharFormat regexFormat;
    QTextCharFormat commentFormat;
    QTextCharFormat multiLineCommentFormat;

    QRegularExpression commentStartExpression;
    QRegularExpression commentEndExpression;
};

#endif // YARAHIGHLIGHTER_H
