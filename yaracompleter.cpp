#include "yaracompleter.h"

YaraCompleter::YaraCompleter(QPlainTextEdit *targetEdit, QWidget* parent): QWidget(parent), targetEdit(targetEdit)
{

    // Setup autocomplete with all YARA keywords
    QStringList keywords = {
        // Keywords1
        "rule", "meta", "strings", "condition",
        // Keywords2
        "int8", "int16", "int32",
        "int8be", "int16be", "int32be",
        "uint16", "uint32",
        "uint8be", "uint16be", "uint32be",
        // Keywords3
        "global", "private", "ascii", "base64", "base64wide",
        "nocase", "wide", "fullword",
        // Keywords4
        "all", "any", "at", "contains", "entrypoint",
        "false", "filesize", "for", "in", "matches",
        "of", "them", "true", "and", "or", "not", "xor"
    };
    keywords.sort(Qt::CaseInsensitive);

    completer = new QCompleter(keywords, this);
    completer->popup()->installEventFilter(this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    completer->setWidget(targetEdit);

    connect(completer, QOverload<const QString &>::of(&QCompleter::activated), this, &YaraCompleter::insertCompletion);
    connect(targetEdit, &QPlainTextEdit::textChanged, this, &YaraCompleter::updateCompleter);

}

bool YaraCompleter::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == completer->popup()) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            switch (keyEvent->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Tab:
            case Qt::Key_Backtab: {
                if(completer->completionCount() == 1) {
                    // If there's only one completion, insert it directly
                    QString completion = completer->currentCompletion();
                    insertCompletion(completion);
                }
                else if (QModelIndex index = completer->popup()->currentIndex(); index.isValid()) {
                    QString completion = index.data(Qt::DisplayRole).toString();
                    insertCompletion(completion);
                }
                return true;
            }
            case Qt::Key_Escape:
                completer->popup()->hide();
                return true;
            default:
                break;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void YaraCompleter::insertCompletion(const QString &completion)
{
    QTextCursor tc = targetEdit->textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    tc.insertText(completion);
    targetEdit->setTextCursor(tc);
}

void YaraCompleter::updateCompleter()
{
    QString prefix = wordUnderCursor();

    if (prefix.length() < 2) {
        completer->popup()->hide();
        return;
    }

    completer->setCompletionPrefix(prefix);

    if (completer->completionCount() == 0) {
        completer->popup()->hide();
        return;
    }

    // Don't show popup if the only match equals what's already typed
    if (completer->completionCount() == 1 &&
        completer->currentCompletion() == prefix) {
        completer->popup()->hide();
        return;
    }

    QRect cr = targetEdit->cursorRect();
    cr.setWidth(completer->popup()->sizeHintForColumn(0)
                + completer->popup()->verticalScrollBar()->sizeHint().width());
    completer->complete(cr);
}

QString YaraCompleter::wordUnderCursor() const
{
    QTextCursor tc = targetEdit->textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}
