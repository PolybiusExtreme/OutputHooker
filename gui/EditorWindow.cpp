#include "EditorWindow.h"
#include "ui_EditorWindow.h"

#include "GuiUtilities.h"
#include "SyntaxHighlighter.h"
#include "EditorAddCmdWindow.h"
#include "EditorAddKeyStateWindow.h"

EditorWindow::EditorWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::EditorWindow)
{
    ui->setupUi(this);

    QVBoxLayout *editorLayout = new QVBoxLayout(ui->centralwidget);
    editorLayout->setContentsMargins(5, 5, 5, 5);
    editorLayout->setSpacing(0);
    editorLayout->addWidget(ui->textEdit);
    setCentralWidget(ui->centralwidget);

    move(pos()+(QGuiApplication::primaryScreen()->geometry().center()-geometry().center()));
    setWindowTitle(QCoreApplication::applicationName()+"-Editor");
    setWindowModality(Qt::ApplicationModal);

    new SyntaxHighlighter(ui->textEdit->document());

    connect(ui->actionOpen, &QAction::triggered, this, &EditorWindow::fileOpen);
    connect(ui->textEdit->document(), &QTextDocument::modificationChanged, ui->actionSave, &QAction::setEnabled);
    connect(ui->actionSave, &QAction::triggered, this, &EditorWindow::fileSave);
    connect(ui->textEdit, &QTextEdit::undoAvailable, ui->actionUndo, &QAction::setEnabled);
    //connect(ui->textEdit->document(), &QTextDocument::undoAvailable, ui->actionUndo, &QAction::setEnabled);
    connect(ui->actionUndo, &QAction::triggered, ui->textEdit, &QTextEdit::undo);
    connect(ui->textEdit, &QTextEdit::redoAvailable, ui->actionRedo, &QAction::setEnabled);
    //connect(ui->textEdit->document(), &QTextDocument::redoAvailable, ui->actionRedo, &QAction::setEnabled);
    connect(ui->actionRedo, &QAction::triggered, ui->textEdit, &QTextEdit::redo);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);

    ui->actionSave->setEnabled(ui->textEdit->document()->isModified());
    ui->actionUndo->setEnabled(ui->textEdit->document()->isUndoAvailable());
    ui->actionRedo->setEnabled(ui->textEdit->document()->isRedoAvailable());

#if QT_CONFIG(clipboard)
    connect(ui->textEdit, &QTextEdit::copyAvailable, ui->actionCopy, &QAction::setEnabled);
    connect(ui->actionCopy, &QAction::triggered, ui->textEdit, &QTextEdit::copy);
    connect(ui->textEdit, &QTextEdit::copyAvailable, ui->actionCut, &QAction::setEnabled);
    connect(ui->actionCut, &QAction::triggered, ui->textEdit, &QTextEdit::cut);
    connect(ui->actionPaste, &QAction::triggered, ui->textEdit, &QTextEdit::paste);
#endif

#ifndef QT_NO_CLIPBOARD
    ui->actionCut->setEnabled(false);
    ui->actionCopy->setEnabled(false);
    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &EditorWindow::clipboardDataChanged);
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        ui->actionPaste->setEnabled(md->hasText());
#endif
}

EditorWindow::~EditorWindow()
{
    delete ui;
}

// Load file from OutputHooker
void EditorWindow::fileLoad(QString fileName)
{
    QFile file(fileName);
    currentFile = fileName;
    QFileInfo fi(fileName);
    QString fileNameInfo = fi.fileName();
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        GuiUtilities::showMessageBoxCentered(this, QCoreApplication::applicationName()+"-Editor", "Could not open game ini file: " + fileNameInfo, QMessageBox::Warning);
        return;
    }
    QTextStream in(&file);
    QString text = in.readAll();
    ui->textEdit->setText(text);
    setWindowTitle(QCoreApplication::applicationName()+"-Editor - "+fileNameInfo);
    statusBar()->showMessage(fileNameInfo+" opened!", 2000);
    file.close();
}

// Open file
void EditorWindow::fileOpen()
{
    if (maybeSave())
    {
        QString iniPath = QApplication::applicationDirPath() + "/ini/";
        QFileDialog dialog(this, "Open game ini file", iniPath, "INI File (*.ini)");
        dialog.setFileMode(QFileDialog::ExistingFile);
        dialog.setOption(QFileDialog::DontUseNativeDialog);
        dialog.setDirectory(iniPath);

        QWidget *sidebar = dialog.findChild<QWidget*>("sidebar");
        if (sidebar)
        {
            sidebar->hide();
        }

        GuiUtilities::centerWidgetOnScreen(&dialog);

        if (dialog.exec())
        {
            const QStringList files = dialog.selectedFiles();
            if (files.isEmpty())
            {
                return;
            }

            QString fileName = files.first();
            QFile file(fileName);
            currentFile = fileName;
            QFileInfo fi(fileName);
            QString fileNameInfo = fi.fileName();
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                GuiUtilities::showMessageBoxCentered(this, QCoreApplication::applicationName()+"-Editor", "Could not open game ini file: " + fileNameInfo, QMessageBox::Warning);
                return;
            }

            QTextStream in(&file);
            QString text = in.readAll();
            ui->textEdit->setText(text);
            setWindowTitle(QCoreApplication::applicationName()+"-Editor - "+fileNameInfo);
            statusBar()->showMessage(fileNameInfo+" opened!", 2000);
            file.close();
        }
    }
}

// Close event
void EditorWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

// Save file
bool EditorWindow::fileSave()
{
    QString fileName;
    fileName = currentFile;
    QFileInfo fi(fileName);
    QString fileNameInfo = fi.fileName();
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        GuiUtilities::showMessageBoxCentered(this, QCoreApplication::applicationName()+"-Editor", fileNameInfo+" not saved! Write Error!", QMessageBox::Warning);
        return false;
    }

    QTextStream out(&file);
    QString text = ui->textEdit->toPlainText();
    out << text;
    file.flush();
    file.close();
    ui->textEdit->document()->setModified(false);
    statusBar()->showMessage(fileNameInfo+" saved!", 2000);
    return true;
}

// Clipboard data changed
void EditorWindow::clipboardDataChanged()
{
#ifndef QT_NO_CLIPBOARD
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        ui->actionPaste->setEnabled(md->hasText());
#endif
}

// Menu entry - Add - Command
void EditorWindow::on_actionCommand_triggered()
{
    QTextCursor cursor = ui->textEdit->textCursor();
    int pos = cursor.position();
    QTextDocument *doc = ui->textEdit->document();

    bool isValid = false;

    for (int i = pos - 1; i >= 0; --i)
    {
        QChar c = doc->characterAt(i);

        if (c.isSpace())
        {
            continue;
        }

        if (c == "=" || c == "," || c == "|")
        {
            isValid = true;
        }

        break;
    }

    if (!isValid)
    {
        GuiUtilities::showMessageBoxCentered(this, QCoreApplication::applicationName()+"-Editor", "Commands are only allowed after the \"=\" sign,\nthe command break \",\" or the state break \"|\" !", QMessageBox::Warning);
        return;
    }

    EditorAddCmdWindow diag(this);
    if (diag.exec() == QDialog::Accepted)
    {
        FunctionCommand res = diag.getCommand();
        QString finalString;
        QStringList parts;

        switch(res.type)
        {
        case CmdAppLaunch:
            if (!res.commandCode.isEmpty()) parts << res.commandCode;
            if (!res.param1.isEmpty())      parts << res.param1;
            if (!res.param2.isEmpty())      parts << res.param2;
            if (!res.param3.isEmpty())      parts << res.param3;
            finalString = parts.join(" ");
            break;

        case CmdUltimarcIntensity:
        case CmdUltimarcState:
            // [Command] [Parameter1] [Parameter2] [Parameter3]
            finalString = QString("%1 %2 %3 %4").arg(res.commandCode, res.param1, res.param2, res.param3);
            break;

        case CmdComWrite:
        case CmdComOpen:
        case CmdComSet:
        case CmdGun4irSSM:
        case CmdGun4irESM:
        case CmdGun4irIM:
        case CmdGun4irOR:
        case CmdGun4irPM:
        case CmdGun4irAR:
        case CmdGun4irTS:
        case CmdGun4irRL:
        case CmdGun4irRO:
        case CmdGun4irFA:
            // [Command] [Parameter1] [Parameter2]
            finalString = QString("%1 %2 %3").arg(res.commandCode, res.param1, res.param2);
            break;

        case CmdGun4irIGF:
            // [Parameter1] [Parameter2] [Parameter3]
            finalString = QString("%1x%2x%3").arg(res.param1, res.param2, res.param3);
            break;

        case CmdComClose:
        case CmdUltimarcKill:
        case CmdAppClose:
        case CmdPlayWav:
            // [Command] [Parameter1]
            finalString = QString("%1 %2").arg(res.commandCode, res.param1);
            break;

        case CmdNull:
            // [Command]
            finalString = res.commandCode;
            break;

        default:
            return;
        }

        QTextCursor cursor = ui->textEdit->textCursor();
        cursor.insertText(finalString);
        ui->textEdit->setFocus();
    }
}

// Menu entry - Add - Command Break
void EditorWindow::on_actionCommandBreak_triggered()
{
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.insertText(",");

    ui->textEdit->setFocus();
}

// Menu entry - Add - Command State
void EditorWindow::on_actionCommandState_triggered()
{
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.insertText("%s%");

    ui->textEdit->setFocus();
}

// Menu entry - Add - State Break
void EditorWindow::on_actionStateBreak_triggered()
{
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.insertText("|");

    ui->textEdit->setFocus();
}

// Menu entry - Add - Key State
void EditorWindow::on_actionKeyState_triggered()
{
    EditorAddKeyStateWindow diag(this);

    if (diag.exec() == QDialog::Accepted)
    {
        QString key = diag.getSelectedKey();

        if (key.isEmpty())
            return;

        QString text = ui->textEdit->toPlainText();
        QString searchString = "[KeyStates]";

        // Find the position after the header
        int headerPos = text.indexOf(searchString);
        int searchStart = headerPos + searchString.length();

        // Search for the next section starting from the header
        int nextSection = text.indexOf("[", searchStart);

        QTextCursor cursor = ui->textEdit->textCursor();

        if (nextSection != -1)
        {
            // If another section follows, insert before it
            cursor.setPosition(nextSection);
            // Insert the key and then add a line break
            cursor.insertText(key + "=\n");
            // Move the cursor one line up to the end of the key for direct input
            cursor.movePosition(QTextCursor::Up);
            cursor.movePosition(QTextCursor::EndOfLine);
        }
        else
        {
            // If [KeyStates] is the last section, insert at the end of the file
            cursor.movePosition(QTextCursor::End);

            // If the file does not end with a line break, add one
            if (!text.endsWith("\n") && !text.isEmpty())
            {
                cursor.insertText("\n");
            }

            cursor.insertText(key + "=");
        }

        // Make changes visible in the UI and set focus
        ui->textEdit->setTextCursor(cursor);
        ui->textEdit->setFocus();
    }
}

// If text is changed and file or editor should be closed
// then ask to save, discard or cancel
bool EditorWindow::maybeSave()
{
    if (!ui->textEdit->document()->isModified())
        return true;

    QMessageBox msgBox(QMessageBox::Warning, QCoreApplication::applicationName() + "-Editor", "Do you want to save your changes?",
                       QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, this);

    GuiUtilities::centerWidgetOnScreen(&msgBox);

    const int ret = msgBox.exec();

    if (ret == QMessageBox::Save)
        return fileSave();
    else if (ret == QMessageBox::Cancel)
        return false;

    return true;
}
