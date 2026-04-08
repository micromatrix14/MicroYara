#include "yarahighlighter.h"

YaraHighlighter::YaraHighlighter(QObject *parent)
    : QSyntaxHighlighter{parent}
{
    HighlightingRule rule;

    // Keywords1: rule, meta, strings, condition — (#FF8080)
    keywords1Format.setForeground(QColor(255, 128, 128));
    for (const char* pattern : {"\\brule\\b", "\\bmeta\\b", "\\bstrings\\b", "\\bcondition\\b"}) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywords1Format;
        highlightingRules.append(rule);
    }

    // Keywords2: integer functions — (#FF8080)
    keywords2Format.setForeground(QColor(255, 128, 128));
    for (const char* pattern : {"\\bint8\\b", "\\bint16\\b", "\\bint32\\b",
             "\\bint8be\\b", "\\bint16be\\b", "\\bint32be\\b",
             "\\buint16\\b", "\\buint32\\b",
             "\\buint8be\\b", "\\buint16be\\b", "\\buint32be\\b"}) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywords2Format;
        highlightingRules.append(rule);
    }

    // Keywords3: modifiers — (#66A334)
    keywords3Format.setForeground(QColor(102, 163, 52));
    for (const char *pattern : {"\\bglobal\\b", "\\bprivate\\b", "\\bascii\\b",
             "\\bbase64\\b", "\\bbase64wide\\b", "\\bnocase\\b",
             "\\bwide\\b", "\\bfullword\\b"}) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywords3Format;
        highlightingRules.append(rule);
    }

    // Keywords4: operators/boolean/built-ins — (#45C6D6)
    keywords4Format.setForeground(QColor(69, 198, 214));
    for (const char* pattern : {"\\ball\\b", "\\bany\\b", "\\bat\\b",
             "\\bcontains\\b", "\\bentrypoint\\b", "\\bfalse\\b",
             "\\bfilesize\\b", "\\bfor\\b", "\\bin\\b",
             "\\bmatches\\b", "\\bof\\b", "\\bthem\\b",
             "\\btrue\\b", "\\band\\b", "\\bor\\b",
             "\\bnot\\b", "\\bxor\\b"}) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywords4Format;
        highlightingRules.append(rule);
    }

    // Strings "..." with backslash escapes — (#D69545)
    stringFormat.setForeground(QColor(214, 149, 69));
    rule.pattern = QRegularExpression("\"(?:[^\\\"\\\\]|\\\\.)*\"");
    rule.format = stringFormat;
    highlightingRules.append(rule);

    // Regex /.../  with backslash escapes — (#D69545) bold
    regexFormat.setForeground(QColor(214, 149, 69));
    regexFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("/(?:[^\\\\/]|\\\\.)+/");
    rule.format = regexFormat;
    highlightingRules.append(rule);

    // Single-line comments // — (#A8ABB0)
    commentFormat.setForeground(QColor(168, 171, 176));
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = commentFormat;
    highlightingRules.append(rule);

    // Multi-line comments /* ... */
    multiLineCommentFormat.setForeground(QColor(168, 171, 176));
    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");
}

void YaraHighlighter::highlightBlock(const QString &text)
{
    // Apply single-line rules
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Handle multi-line comments /* ... */
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}
