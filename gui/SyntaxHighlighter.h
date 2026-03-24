#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QList>

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit SyntaxHighlighter(QTextDocument *parent = nullptr) : QSyntaxHighlighter(parent) {
        HighlightingRule rule;

        sectionFormat.setForeground(Qt::darkGreen);
        sectionFormat.setFontWeight(QFont::Bold);
        rule.pattern = QRegularExpression("\\[.*?\\]");
        rule.format = sectionFormat;
        highlightingRules.append(rule);

        // keyFormat.setForeground(Qt::darkBlue);
        // rule.pattern = QRegularExpression("^[\\w\\.\\s]+(?=\\=)");
        // rule.format = keyFormat;
        // highlightingRules.append(rule);

        valueFormat.setForeground(Qt::darkYellow);
        rule.pattern = QRegularExpression("(?<=\\=).*");
        rule.format = valueFormat;
        highlightingRules.append(rule);

        commentFormat.setForeground(Qt::gray);
        commentFormat.setFontItalic(true);
        rule.pattern = QRegularExpression("(^|\\s)(;|#).*");
        rule.format = commentFormat;
        highlightingRules.append(rule);
    }

protected:
    void highlightBlock(const QString &text) override {
        for (const HighlightingRule &rule : std::as_const(highlightingRules)) {
            QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }
    }

private:
    struct HighlightingRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QList<HighlightingRule> highlightingRules;

    QTextCharFormat sectionFormat;
    QTextCharFormat keyFormat;
    QTextCharFormat valueFormat;
    QTextCharFormat commentFormat;
};

#endif // SYNTAXHIGHLIGHTER_H
