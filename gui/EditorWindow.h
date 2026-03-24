#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QMimeData>
#include <QClipboard>
#include <QCloseEvent>

namespace Ui {
class EditorWindow;
}

class EditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EditorWindow(QWidget *parent = nullptr);
    ~EditorWindow();

    // Load file from OutputHooker
    void fileLoad(QString fileName);

    // Open file
    void fileOpen();

protected:
    // Close event
    virtual void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    // Save file
    bool fileSave();

    // Clipboard data changed
    void clipboardDataChanged();

    // Menu entry - Add - Command
    void on_actionCommand_triggered();

    // Menu entry - Add - Command Break
    void on_actionCommandBreak_triggered();

    // Menu entry - Add - Command State
    void on_actionCommandState_triggered();

    // Menu entry - Add - State Break
    void on_actionStateBreak_triggered();

private:
    Ui::EditorWindow *ui;

    // Current game INI file or default.ini
    QString currentFile;

    // If text is changed and file or editor should be closed
    // then ask to save, discard or cancel
    bool maybeSave();
};

#endif // EDITORWINDOW_H
