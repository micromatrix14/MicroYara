#ifndef YARACOMPLETER_H
#define YARACOMPLETER_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QCompleter>
#include <QAbstractItemView>
#include <QScrollBar>

class YaraCompleter : public QWidget
{
    Q_OBJECT
public:
    explicit YaraCompleter(QPlainTextEdit *targetEdit, QWidget* parent);
    virtual ~YaraCompleter() = default;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void insertCompletion(const QString &completion);
    void updateCompleter();

private:
    QString wordUnderCursor() const;
    QCompleter *completer;
    QPlainTextEdit *targetEdit;
};

#endif // YARACOMPLETER_H
