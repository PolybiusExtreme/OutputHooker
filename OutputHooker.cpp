#include "OutputHooker.h"
#include "ui_OutputHooker.h"

#include "Global.h"
#include "gui/GuiUtilities.h"

OutputHooker::OutputHooker(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::OutputHooker)
{
    ui->setupUi(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(ui->centralwidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(ui->textBrowser);
    setCentralWidget(ui->centralwidget);

    QRect screenrect = QApplication::primaryScreen()->geometry();
    move(screenrect.left(), screenrect.top());

    // Windows message system is not connected yet
    isWinMsgConnected = false;

    // TCP Socket is not connected yet
    isTCPConnected = false;

    // Is game INI
    isGameINI = false;

    // Load config
    p_config = new OutputHookerConfig();

    // Creates the OutputHookerCore and gives pointer address of the OutputHookerConfig
    p_core = new OutputHookerCore(p_config, this);

    // HWND is passed from here to WindowsMessage via the OutputHookerCore
    p_core->setWinID((HWND)this->winId());

    // Set Tray Icon and show it
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(":/icon/OutputHooker.ico"));
    trayIcon->setToolTip("OutputHooker");
    trayIcon->show();

    // Set Tray Icon menu
    trayIconMenu = new QMenu(this);
    restoreAction = new QAction("OutputHooker", this);
    // restoreAction->setIcon(QIcon(":/icon/OutputHooker.ico"));
    connect(restoreAction, &QAction::triggered, this, [this](){
        this->show();
        this->setWindowState(Qt::WindowActive);
        p_core->mainWindowState(false);
    });
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    testOutputAction = new QAction("Test Outputs", this);
    // testOutputAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart));
    connect(testOutputAction, &QAction::triggered, this, &OutputHooker::on_actionTestOutputs_triggered);
    trayIconMenu->addAction(testOutputAction);
    scriptEditorAction = new QAction("Script Editor", this);
    // scriptEditorAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MailMessageNew));
    connect(scriptEditorAction, &QAction::triggered, this, &OutputHooker::on_actionEditSpecificGameINI_triggered);
    trayIconMenu->addAction(scriptEditorAction);
    trayIconMenu->addSeparator();
    quitAction = new QAction("Quit", this);
    // quitAction->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::EditClear));
    connect(quitAction, &QAction::triggered, this, &QApplication::quit);
    trayIconMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayIconMenu);

    // When Tray Icon triggered or double clicked, bring up the program
    connect(trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason)
    {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick){
            this->show();
            this->setWindowState(Qt::WindowActive);
            p_core->mainWindowState(false);
        }
    });

    // Load settings
    // Get "Notification on new Output(s)" setting
    useNewOutputsNotification = p_config->getUseNewOutputsNotification();
    // Get "Add new Output(s) to default.ini" setting
    saveNewOutputsToDefaultINI = p_config->getSaveNewOutputsToDefaultINI();
    // If "Notification on new Output(s)" setting set, check the menu entry
    if (useNewOutputsNotification)
        ui->actionNotification->setChecked(true);
    else
        ui->actionNotification->setChecked(false);
    // If "Add new Output(s) to default.ini" setting set, check the menu entry
    if (saveNewOutputsToDefaultINI)
        ui->actionDefaultINI->setChecked(true);
    else
        ui->actionDefaultINI->setChecked(false);

    // Disable menu entry "Edit ini file for current game"
    ui->actionEditCurrentGameINI->setEnabled(false);

    // Signals and slots (used to display info on MainWindows)
    // OutputHooker Core updates
    connect(p_core, &OutputHookerCore::noConnectedGame, this, &OutputHooker::displayNoGame);
    connect(p_core, &OutputHookerCore::connectedGame, this, &OutputHooker::displayGame);
    connect(p_core, &OutputHookerCore::connectedEmptyGame, this, &OutputHooker::displayEmptyGame);
    connect(p_core, &OutputHookerCore::addSignalFromGame, this, &OutputHooker::addSignalDisplay);
    connect(p_core, &OutputHookerCore::updateSignalFromGame, this, &OutputHooker::updateSignalDisplay);
    connect(p_core, &OutputHookerCore::updatePauseFromGame, this, &OutputHooker::updatePauseDisplay);
    connect(p_core, &OutputHookerCore::updateOrientationFromGame, this, &OutputHooker::updateOrientationDisplay);
    connect(p_core, &OutputHookerCore::connectionStatus, this, &OutputHooker::updateConnectionStatus);
    connect(p_core, &OutputHookerCore::showErrorMessage, this, &OutputHooker::errorMessage);

    // Start OutputHookerCore
    p_core->startCore();
    coreRunning = true;
}

OutputHooker::~OutputHooker()
{
    delete ui;
    delete p_config;
    delete p_core;
}

// Disable the Task Icon Menu - Open OutputHooker
void OutputHooker::setVisible(bool visible)
{
    restoreAction->setEnabled(isMaximized() || !visible);
    QMainWindow::setVisible(visible);
}

// Displays no game data on MainWindow
void OutputHooker::displayNoGame()
{
    // Display text QMap needs to be cleared
    signalNumberMap.clear();

    // textBrowser needs to be cleared
    ui->textBrowser->clear();

    // Disable menu entry "Edit ini file for current game"
    ui->actionEditCurrentGameINI->setEnabled(false);
}

// Displays top of game data on MainWindow
void OutputHooker::displayGame(QString gName, QString iName, bool iniGame, bool noGameFound)
{
    // Display text QMap needs to be cleared
    signalNumberMap.clear();
    isGameINI = iniGame;
    noGameFileFound = noGameFound;
    gameName = gName;
    iniName = iName;
    makeTopDisplayText();
    displayTextList << " " << OUTPUTSIGNALS << OUTPUTSIGNALSDASHES;
    displayText();

    // Enable menu entry "Edit ini file for current game",
    // if "Add new output(s) to default.ini" is not set
    if (!saveNewOutputsToDefaultINI)
        ui->actionEditCurrentGameINI->setEnabled(true);
}

// Displays empty game data on MainWindow
void OutputHooker::displayEmptyGame()
{
    // Display text QMap needs to be cleared
    signalNumberMap.clear();
    gameName = MAMENOGAMEEMPTY;
    makeTopDisplayText();
    displayText();

    // Disable menu entry "Edit ini file for current game"
    ui->actionEditCurrentGameINI->setEnabled(false);
}

// Add Output signals to the MainWindow
void OutputHooker::addSignalDisplay(const QString &sig, const QString &dat)
{
    QString temp = sig + " = " + dat;
    quint16 signalCount = displayTextList.count();
    displayTextList << temp;
    signalNumberMap.insert(sig, signalCount);
    displayText();
}

// Update output signals data on the MainWindow
void OutputHooker::updateSignalDisplay(const QString &sig, const QString &dat)
{
    QString temp = sig + " = " + dat;
    quint16 displayIndex = signalNumberMap[sig];
    displayTextList[displayIndex] = temp;
    displayText();
}

// Update the pause data on the MainWindow
void OutputHooker::updatePauseDisplay(QString dat)
{
    QString pTemp = PAUSEEQUALS + dat;
    displayTextList[PAUSEINDEX] = pTemp;
    displayText();
}

// Update the orientation data on the MainWindow
void OutputHooker::updateOrientationDisplay(QString sig, QString dat)
{
    QString oTemp = sig + "=" + dat;
    displayTextList[ORIENTATIONINDEX] = oTemp;
    displayText();
}

// Update the connection status
void OutputHooker::updateConnectionStatus(OutputHookerCore::ConnectionType type, bool status)
{
    if (type == OutputHookerCore::WinMsg)
    {
        isWinMsgConnected = status;
    }
    else if (type == OutputHookerCore::TCP)
    {
        isTCPConnected = status;
    }
}

// Handle Error MessageBox from a different thread
void OutputHooker::errorMessage(const QString title, const QString message)
{
    GuiUtilities::showMessageBoxCentered(this, title, message, QMessageBox::Critical);
}

// Open TestOutputWindow
void OutputHooker::on_actionTestOutputs_triggered()
{
    if (isTCPConnected || isWinMsgConnected)
    {
        GuiUtilities::showMessageBoxCentered(this, "Error", "Close the game or emulator, to test the output(s)!", QMessageBox::Critical);
        return;
    }
    else if (!p_editorWindow)
    {
        testOutputAction->setEnabled(false);
        p_testOutputWindow = new TestOutputWindow(this);
        connect(p_testOutputWindow, &TestOutputWindow::sendTestCommand, p_core, &OutputHookerCore::executeCommand);
        connect(p_testOutputWindow, &TestOutputWindow::rejected, this, &OutputHooker::TestOutputWindow_closed);
    }
    p_testOutputWindow->exec();
}

// On close of TestOutputWindow
void OutputHooker::TestOutputWindow_closed()
{
    testOutputAction->setEnabled(true);
}

// Close OutputHooker
void OutputHooker::on_actionExit_triggered()
{
    close();
}

// Change notification setting
void OutputHooker::on_actionNotification_triggered()
{
    if (coreRunning)
    {
        // Stop OutputHookerCore
        p_core->stopCore();
        coreRunning = false;
    }

    useNewOutputsNotification = ui->actionNotification->isChecked();
    p_config->setUseNewOutputsNotification(useNewOutputsNotification);
    p_config->saveSettings();

    // Start OutputHookerCore
    p_core->startCore();
    coreRunning = true;
}

// Change default.ini setting
void OutputHooker::on_actionDefaultINI_triggered()
{
    if (coreRunning)
    {
        // Stop OutputHookerCore
        p_core->stopCore();
        coreRunning = false;
    }

    saveNewOutputsToDefaultINI = ui->actionDefaultINI->isChecked();
    p_config->setSaveNewOutputsToDefaultINI(saveNewOutputsToDefaultINI);
    p_config->saveSettings();

    // Start OutputHookerCore
    p_core->startCore();
    coreRunning = true;
}

// Open the default.ini in EditorWindow
void OutputHooker::on_actionEditDefaultINI_triggered()
{
    if (!p_editorWindow)
    {
        if (coreRunning)
        {
            // Stop OutputHookerCore
            p_core->stopCore();
            coreRunning = false;
        }

        scriptEditorAction->setEnabled(false);
        QString fileName = QApplication::applicationDirPath() + "/" + INIFILEDIR + "/" + DEFAULTFILE + ENDOFINIFILE;
        p_editorWindow = new EditorWindow(this);
        p_editorWindow->setAttribute(Qt::WA_DeleteOnClose);
        p_editorWindow->fileLoad(fileName);
        connect(p_editorWindow, &EditorWindow::destroyed, this, &OutputHooker::EditorWindow_closed);
    }
    p_editorWindow->show();
}

// Open EditorWindow
void OutputHooker::on_actionEditSpecificGameINI_triggered()
{
    if (!p_editorWindow)
    {
        if (coreRunning)
        {
            // Stop OutputHookerCore
            p_core->stopCore();
            coreRunning = false;
        }

        scriptEditorAction->setEnabled(false);

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
                scriptEditorAction->setEnabled(true);
                p_core->startCore();
                coreRunning = true;
                return;
            }

            QString fileName = files.first();
            QFile file(fileName);
            QFileInfo fi(fileName);
            QString fileNameInfo = fi.fileName();

            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                GuiUtilities::showMessageBoxCentered(this, QCoreApplication::applicationName()+"-Editor", "Could not open game ini file: " + fileNameInfo, QMessageBox::Warning);
                p_core->startCore();
                coreRunning = true;
                return;
            }

            p_editorWindow = new EditorWindow(this);
            p_editorWindow->setAttribute(Qt::WA_DeleteOnClose);
            p_editorWindow->fileLoad(fileName);
            connect(p_editorWindow, &EditorWindow::destroyed, this, &OutputHooker::EditorWindow_closed);
            p_editorWindow->show();
        }
        else
        {
            scriptEditorAction->setEnabled(true);
            p_core->startCore();
            coreRunning = true;
        }
    }
}

// Open the current gamename.ini in EditorWindow
void OutputHooker::on_actionEditCurrentGameINI_triggered()
{
    if (!p_editorWindow)
    {
        if(coreRunning)
        {
            // Stop OutputHookerCore
            p_core->stopCore();
            coreRunning = false;
        }

        scriptEditorAction->setEnabled(false);
        QString fileName = QApplication::applicationDirPath() + "/" + INIFILEDIR + "/" + gameName + ENDOFINIFILE;
        p_editorWindow = new EditorWindow(this);
        p_editorWindow->setAttribute(Qt::WA_DeleteOnClose);
        p_editorWindow->fileLoad(fileName);
        connect(p_editorWindow, &EditorWindow::destroyed, this, &OutputHooker::EditorWindow_closed);
    }
    p_editorWindow->show();
}

// On close of EditorWindow
void OutputHooker::EditorWindow_closed()
{
    if (isTCPConnected || isWinMsgConnected)
    {
        // Stop OutputHookerCore
        p_core->stopCore();
        coreRunning = false;
    }

    // Start OutputHookerCore
    p_core->startCore();
    coreRunning = true;
    scriptEditorAction->setEnabled(true);
}

// Open AboutWindow
void OutputHooker::on_actionAboutWindow_triggered()
{
    p_aboutWindow = new AboutWindow(this);
    p_aboutWindow->exec();
}

// Hide from the Taskbar when minimized
void OutputHooker::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        if(windowState() & Qt::WindowMinimized) {
            hide();
            trayIcon->show();
            p_core->mainWindowState(true);
        }
    }
    QMainWindow::changeEvent(event);
}

// Makes the default display data that is shown in the textBrowser
void OutputHooker::makeTopDisplayText()
{
    displayTextList.clear();

    displayTextList << GAMEINFO << GAMEINFODASHES;

    QString temp = ROMEQUALS;

    temp.append(gameName);

    displayTextList << temp;

    if (gameName != MAMENOGAMEEMPTY && !gameName.isEmpty())
    {
        temp = GAMEFILE;

        temp.append(iniName);

        if (isGameINI)
            temp.append(ENDOFINIFILE);
    }
    else
    {
        temp = GAMEFILE;
    }

    displayTextList << temp << ORIENTATIONEQUAL0 << PAUSEEQUALS0;
}

// Display the display data in the textBrowser
void OutputHooker::displayText()
{
    quint16 lineCount = displayTextList.count();
    ui->textBrowser->clear();
    ui->textBrowser->setPlainText(displayTextList[0]);
    for (quint8 i = 1; i < lineCount; i++)
        ui->textBrowser->append(displayTextList[i]);
}
