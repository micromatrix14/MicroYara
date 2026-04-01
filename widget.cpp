#include "widget.h"
#include "ui_widget.h"
#include <QPalette>
#include <QStringListModel>
#include <QAbstractItemView>
#include <QTextCursor>
#include <QScrollBar>
#include <QLineEdit>
#include <QAbstractTextDocumentLayout>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    //Set the current directory to the application director
    m_currentDir = QApplication::applicationDirPath();

    //Set the source and compilation file paths
    m_sourceFilePath = QDir(m_currentDir.filePath("files")).absoluteFilePath("yara.yara");
    m_compiledFilePath = QDir(m_currentDir.filePath("files")).absoluteFilePath("yara.bin");
    m_compilerFilePath = m_currentDir.absoluteFilePath("yarac64.exe");
    m_yaraEngineFilePath = m_currentDir.absoluteFilePath("yara64.exe");
    QFile::remove(m_compiledFilePath); //Remove the compiled file if it exists to avoid confusion

    //reading the existing yara file if exists and set its content to the code editor
    QFile existingFile(m_sourceFilePath);
    if(existingFile.exists() && existingFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&existingFile);
        ui->code_editor->setPlainText(in.readAll());
        existingFile.close();
    }

    //Adjust colors for the code editor
    QPalette p = ui->code_editor->palette();
    p.setColor(QPalette::Text, QColor(214, 187, 154));
    p.setColor(QPalette::PlaceholderText, QColor(130, 130, 130));
    ui->code_editor->setPalette(p);

    //Adjust compilation result text
    QPalette cp = ui->output_txt->palette();
    cp.setColor(QPalette::Text, Qt::white);
    cp.setColor(QPalette::PlaceholderText, QColor(130, 130, 130));
    ui->output_txt->setPalette(cp);

    //Adjust loaded file/folder path text
    QPalette fpp = ui->output_txt->palette();
    fpp.setColor(QPalette::Text, Qt::white);
    fpp.setColor(QPalette::PlaceholderText, QColor(130, 130, 130));
    ui->loaded_file_folder_path_txt->setPalette(fpp);

    //Adjust the checkbox colors
    QPalette cbp = ui->scan_auto_checkbox->palette();
    cbp.setColor(QPalette::WindowText, Qt::white);
    cbp.setColor(QPalette::Disabled, QPalette::WindowText, QColor(130, 130, 130));
    ui->scan_auto_checkbox->setPalette(cbp);

    //Set a monospaced font for the code editor, compilation result, compilation timeout, namespace, compilation args and scanning args text edits
    QFont font("Consolas", 11);
    font.setStyleHint(QFont::Monospace);
    ui->code_editor->setFont(font);
    ui->output_txt->setFont(font);
    ui->compilation_timeout_txt->setFont(font);
    ui->namespace_txt->setFont(font);
    ui->compilation_args_txt->setFont(font);
    ui->scanning_args_txt->setFont(font);

    //Set tab width to 4 spaces
    ui->code_editor->setTabStopDistance(
        QFontMetricsF(font).horizontalAdvance(' ') * 4);

    //Initialize the syntax highlighter and code completer
    m_completer = new YaraCompleter(ui->code_editor, this);
    m_highlighter.setDocument(ui->code_editor->document());

    // Set initial splitter sizes: 80% for the code_editor and 20% for the compilation result, and the same for development and scanning result
    QList<int> sizes;
    sizes << 800 << 200; // These values are relative; QSplitter normalizes them.
    ui->code_compilationresult_splitter->setSizes(sizes);
    ui->developing_scanning_splitter->setSizes(sizes);

    //Use regular expression for the compilation timeout to be between (1-10) seconds
    ui->compilation_timeout_txt->setValidator(new QRegularExpressionValidator(QRegularExpression("^[1-9]$|^10$")));
    connect(ui->compilation_timeout_txt, &QLineEdit::textChanged, this, [this](const QString &text) {
        m_compilationTimeout = text.toInt();
        if(m_compilationTimeout < 1) {
            m_compilationTimeout = 1;
            ui->compilation_timeout_txt->setText("1");
        } else if(m_compilationTimeout > 10) {
            m_compilationTimeout = 10;
            ui->compilation_timeout_txt->setText("10");
        }
    });

    //Adjust colors for the compilation timeout, namespace, compilation arguments and scanning arguments text edit
    QPalette ctp = ui->compilation_timeout_txt->palette();
    ctp.setColor(QPalette::Base, QColor(46, 47, 48));
    ctp.setColor(QPalette::Text, Qt::white);
    ctp.setColor(QPalette::PlaceholderText, QColor(130, 130, 130));
    ui->compilation_timeout_txt->setPalette(ctp);
    ui->namespace_txt->setPalette(ctp);
    ui->compilation_args_txt->setPalette(ctp);
    ui->scanning_args_txt->setPalette(ctp);

    //Set the timer for the compilation timeout to start the compilation process after the user stops typing for the specified timeout duration
    m_compilationTimer.setSingleShot(true);
    connect(&m_compilationTimer, &QTimer::timeout, this, [this]() {
        //create the source and compilation directory if it doesn't exist
        if(!QDir().mkpath(m_currentDir.absoluteFilePath("files"))) {
            ui->output_txt->setPlainText("Failed to create temporary directory for compilation.");
            return;
        }

        QString code = ui->code_editor->toPlainText();
        QString compilation_args = ui->compilation_args_txt->text().trimmed();
        QString namespace_txt = ui->namespace_txt->text().trimmed();

        //Save the source code to the source file
        QFile sourceFile(m_sourceFilePath);
        if(!sourceFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            ui->output_txt->setPlainText("Failed to save source code to temporary file.");
            return;
        }
        QTextStream out(&sourceFile);
        out << code;
        sourceFile.close();

        //Compile the source file using the compiler process
        QStringList arguments;
        if(!compilation_args.isEmpty()) {
            arguments << compilation_args.split(' ', Qt::SkipEmptyParts);
        }

        arguments
            << (namespace_txt.isEmpty()?m_sourceFilePath:namespace_txt + ":" + m_sourceFilePath)
            << m_compiledFilePath;

        //Start the process and handle errors
        m_compilerProcess.start(m_compilerFilePath, arguments);

    });
    connect(&m_compilerProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        ui->output_sign_lbl->setPixmap(QPixmap("://failed.png").scaled(ui->output_sign_lbl->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        QString errorMessage;
        switch (error) {
            case QProcess::FailedToStart:
                errorMessage = "Failed to start the compiler process. Please check the compiler path.";
                break;
            case QProcess::Crashed:
                errorMessage = "The compiler process crashed during execution.";
                break;
            case QProcess::Timedout:
                errorMessage = "The compiler process timed out. Please check your compilation timeout setting.";
                break;
            case QProcess::WriteError:
                errorMessage = "An error occurred while writing to the compiler process.";
                break;
            case QProcess::ReadError:
                errorMessage = "An error occurred while reading from the compiler process.";
                break;
            case QProcess::UnknownError:
            default:
                errorMessage = "An unknown error occurred with the compiler process.";
                break;
        }
        ui->output_txt->setPlainText(errorMessage);
    });
    connect(&m_compilerProcess, &QProcess::finished, this, [this]() {
        if(m_compilerProcess.exitStatus() != QProcess::NormalExit || m_compilerProcess.exitCode() != 0) {
            ui->output_sign_lbl->setPixmap(QPixmap("://failed.png").scaled(ui->output_sign_lbl->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->output_txt->setPlainText("Compilation failed:\n" + m_compilerProcess.readAllStandardError());
        }
        else{
            m_compiledFileExist = true;
            ui->output_sign_lbl->setPixmap(QPixmap("://success.png").scaled(ui->output_sign_lbl->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            ui->output_txt->setPlainText("Compilation succeeded:\n" + m_compilerProcess.readAllStandardError());

            //If the auto scan checkbox is checked, start scanning with Yara after successful compilation
            if(ui->scan_auto_checkbox->isEnabled() && ui->scan_auto_checkbox->isChecked()) {
                scanWithYara();
            }
        }
    });
    connect(&m_scannerProcess, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        QString errorMessage;
        switch (error) {
            case QProcess::FailedToStart:
                errorMessage = "Failed to start the scanner process. Please check the Yara engine path.";
                break;
            case QProcess::Crashed:
                errorMessage = "The scanner process crashed during execution.";
                break;
            case QProcess::Timedout:
                errorMessage = "The scanner process timed out. Please check your scanning arguments.";
                break;
            case QProcess::WriteError:
                errorMessage = "An error occurred while writing to the scanner process.";
                break;
            case QProcess::ReadError:
                errorMessage = "An error occurred while reading from the scanner process.";
                break;
            case QProcess::UnknownError:
            default:
                errorMessage = "An unknown error occurred with the scanner process.";
                break;
        }
        ui->output_txt->appendPlainText(errorMessage);
    });
    connect(&m_scannerProcess, &QProcess::finished, this, [this]() {
        if(m_scannerProcess.exitStatus() != QProcess::NormalExit || m_scannerProcess.exitCode() != 0) {
            ui->output_txt->appendPlainText("Scanning failed:\n" + m_scannerProcess.readAllStandardError());
        }
        else{
            ui->output_txt->appendPlainText("Scanning succeeded:\n" + m_scannerProcess.readAllStandardOutput());
        }
    });

    //reset the timer, clear the output and show the loading sign when the user types in the code editor, namespace or compilation arguments
    connect(ui->code_editor, &QPlainTextEdit::textChanged, this, [this]() {
        if(m_compilationTimer.isActive()) {
            m_compilationTimer.start(m_compilationTimeout * 1000);
            return;
        }
        m_compiledFileExist = false;
        ui->output_sign_lbl->setPixmap(QPixmap("://loading.png").scaled(ui->output_sign_lbl->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->output_txt->clear();
        m_compilerProcess.close();
        m_compilationTimer.start(m_compilationTimeout * 1000);
    });
    connect(ui->namespace_txt, &QLineEdit::textChanged, this, [this]() {
        //Trigger the text changed signal of the code editor to start the compilation process
        emit ui->code_editor->textChanged();
    });
    connect(ui->compilation_args_txt, &QLineEdit::textChanged, this, [this]() {
        //Trigger the text changed signal of the code editor to start the compilation process
        emit ui->code_editor->textChanged();
    });

    //Load and unloading file/folder
    connect(ui->load_file_folder_btn, &QPushButton::clicked, this, [&](){
        m_loadedState = LoadedState_enum::None;
        m_loadedFileFolderPath.clear();
        ui->loaded_file_folder_path_txt->clear();
        ui->scan_auto_checkbox->setChecked(false);
        ui->scan_auto_checkbox->setEnabled(false);

        QMessageBox msgBox;
        msgBox.setWindowTitle("Select an option");
        msgBox.setText("Which option do you want to load?");
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setStandardButtons(QMessageBox::Cancel);
        QPushButton *fileButton = msgBox.addButton(tr("File"), QMessageBox::ActionRole);
        QPushButton *folderButton = msgBox.addButton(tr("Folder"), QMessageBox::ActionRole);
        msgBox.exec();
        if (msgBox.clickedButton() == fileButton) {
            m_loadedFileFolderPath = QFileDialog::getOpenFileName(nullptr, "Select a file to load");
            if (!m_loadedFileFolderPath.isEmpty()) {
                // Handle file loading
                m_loadedState = LoadedState_enum::File;
                ui->loaded_file_folder_path_txt->setPlainText(m_loadedFileFolderPath);
                ui->scan_auto_checkbox->setEnabled(true);
                scanWithYara();
            }
        } else if (msgBox.clickedButton() == folderButton) {
            m_loadedFileFolderPath = QFileDialog::getExistingDirectory(nullptr, "Select a folder to load");
            if (!m_loadedFileFolderPath.isEmpty()) {
                // Handle folder loading
                m_loadedState = LoadedState_enum::Folder;
                ui->loaded_file_folder_path_txt->setPlainText(m_loadedFileFolderPath);
                ui->scan_auto_checkbox->setEnabled(true);
                scanWithYara();
            }
        }
    });
    connect(ui->unload_loaded_file_folder_btn, &QPushButton::clicked, this, [&](){
        m_loadedState = LoadedState_enum::None;
        m_loadedFileFolderPath.clear();
        ui->loaded_file_folder_path_txt->clear();
        ui->scan_auto_checkbox->setChecked(false);
        ui->scan_auto_checkbox->setEnabled(false);
    });

    //clear the output when the user clicks the clear output button
    connect(ui->clear_output_btn, &QPushButton::clicked, this, [&](){
        ui->output_txt->clear();
    });

    //build the YARA code
    connect(ui->build_btn, &QPushButton::clicked, this, [&](){
        //Trigger the text changed signal of the code editor to start the compilation process
        emit ui->code_editor->textChanged();
    });

    //Trigger recompilation and scanning when the scanning arguments are changed
    connect(ui->scanning_args_txt, &QLineEdit::textChanged, this, [this]() {
        if(ui->scan_auto_checkbox->isEnabled() && ui->scan_auto_checkbox->isChecked()) {
            //Trigger the text changed signal of the code editor to start the compilation process and scanning after successful compilation
            emit ui->code_editor->textChanged();
        }
    });

}

Widget::~Widget()
{
    m_highlighter.setDocument(nullptr);
    delete ui;
}

void Widget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() == 1 && event->mimeData()->urls().first().isLocalFile()) {
        QPoint pos = ui->load_file_folder_btn->mapFrom(this, event->position().toPoint());
        if (ui->load_file_folder_btn->rect().contains(pos)) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    }
    else{
        event->ignore();
    }
}

void Widget::dropEvent(QDropEvent *event)
{
    m_loadedState = LoadedState_enum::None;
    m_loadedFileFolderPath.clear();
    ui->loaded_file_folder_path_txt->clear();
    ui->scan_auto_checkbox->setChecked(false);
    ui->scan_auto_checkbox->setEnabled(false);

    if (!event->mimeData()->hasUrls() || event->mimeData()->urls().size() != 1)
        return;

    QString path = event->mimeData()->urls().first().toLocalFile();
    QFileInfo info(path);

    if (info.isFile()) {
        m_loadedState = LoadedState_enum::File;
        m_loadedFileFolderPath = path;
        ui->loaded_file_folder_path_txt->setPlainText(m_loadedFileFolderPath);
        ui->scan_auto_checkbox->setEnabled(true);
        scanWithYara();
    } else if(info.isDir()) {
        m_loadedState = LoadedState_enum::Folder;
        m_loadedFileFolderPath = path;
        ui->loaded_file_folder_path_txt->setPlainText(m_loadedFileFolderPath);
        ui->scan_auto_checkbox->setEnabled(true);
        scanWithYara();
    }
}

void Widget::scanWithYara()
{
    if(!m_compiledFileExist)
        return

    m_scannerProcess.close();

    if(getLoadedState() == LoadedState_enum::None) {
        ui->output_txt->appendPlainText("No file or folder loaded for scanning.");
        return;
    }

    // Implement the scanning logic here, using m_loadedFileFolderPath as the target for scanning with Yara.
    // You can use m_scannerProcess to run the Yara scanning process and handle its output similarly to how the compilation process is handled.
    QString scanning_args = ui->scanning_args_txt->text().trimmed();
    QStringList arguments;
    if(!scanning_args.isEmpty()) {
        arguments << scanning_args.split(' ', Qt::SkipEmptyParts);
    }

    arguments
        << "-C" << m_compiledFilePath
        << m_loadedFileFolderPath;

    //Start the process and handle errors
    m_scannerProcess.start(m_yaraEngineFilePath, arguments);

}

Widget::LoadedState_enum Widget::getLoadedState() const
{
    return m_loadedState;
}
