#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QDir>
#include <QProcess>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include "yarahighlighter.h"
#include "yaracompleter.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    enum class LoadedState_enum {
        None,
        File,
        Folder
    };

    LoadedState_enum getLoadedState() const;
    void scanWithYara();

    Ui::Widget *ui;
    YaraHighlighter m_highlighter;
    YaraCompleter *m_completer;
    int m_compilationTimeout = 5;   //Default compilation timeout in seconds
    QTimer m_compilationTimer;

    QDir m_currentDir;

    QString m_sourceFilePath;
    QString m_compiledFilePath;
    bool m_compiledFileExist = false;
    QString m_compilerFilePath;
    QString m_yaraEngineFilePath;

    QProcess m_compilerProcess;
    QProcess m_scannerProcess;

    LoadedState_enum m_loadedState = LoadedState_enum::None;
    QString m_loadedFileFolderPath;

};
#endif // WIDGET_H
