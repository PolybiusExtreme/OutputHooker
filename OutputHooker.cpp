/*
 * Original Copyright (c) 2026 PolybiusExtreme
 * Portions Copyright (c) 2026 6Bolt
 *
 * Licensed under the GNU GPLv3.
 */

#include "OutputHooker.h"
#include "ui_OutputHooker.h"

#include "Global.h"
#include "gui/GuiUtilities.h"

#include <windows.h>
#include <dbt.h>

OutputHooker::OutputHooker(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::OutputHooker)
{
    ui->setupUi(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(ui->centralwidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(ui->listWidget);
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

    // HWND is passed from here to LedWizModule & WinMsgModule via the OutputHookerCore
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
        showMainWindow();
    });
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    connectedDevicesAction = new QAction("Connected Device(s)", this);
    connect(connectedDevicesAction, &QAction::triggered, this, &OutputHooker::on_actionConnectedDevices_triggered);
    trayIconMenu->addAction(connectedDevicesAction);
    testOutputAction = new QAction("Test Output(s)", this);
    connect(testOutputAction, &QAction::triggered, this, &OutputHooker::on_actionTestOutputs_triggered);
    trayIconMenu->addAction(testOutputAction);
    scriptEditorAction = new QAction("Script Editor", this);
    connect(scriptEditorAction, &QAction::triggered, this, &OutputHooker::on_actionEditSpecificGameINI_triggered);
    trayIconMenu->addAction(scriptEditorAction);
    trayIconMenu->addSeparator();
    quitAction = new QAction("Quit", this);
    connect(quitAction, &QAction::triggered, this, &QApplication::quit);
    trayIconMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayIconMenu);

    // When Tray Icon triggered or double clicked, bring up the program
    connect(trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason)
    {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick){
            showMainWindow();
        }
    });

    // Load settings
    // Get "Notification on new Output(s)" setting
    useNewOutputsNotification = p_config->getUseNewOutputsNotification();
    // Get "Add new Output(s) to default.ini" setting
    addNewOutputsToDefaultINI = p_config->getAddNewOutputsToDefaultINI();
    // If "Notification on new Output(s)" setting set, check the menu entry
    if (useNewOutputsNotification)
        ui->actionNotification->setChecked(true);
    else
        ui->actionNotification->setChecked(false);
    // If "Add new Output(s) to default.ini" setting set, check the menu entry
    if (addNewOutputsToDefaultINI)
        ui->actionDefaultINI->setChecked(true);
    else
        ui->actionDefaultINI->setChecked(false);

    // Get "Preferred Output Source" setting
    outputSourcePriority = p_config->getOutputSourcePriority();
    // Get "Output Processing Method" setting
    outputProcessingMethod = p_config->getOutputProcessingMethod();

    // Only one output source can be preferred, so the menu entries are made exclusive
    outputPriorityGroup = new QActionGroup(this);
    outputPriorityGroup->setExclusive(true);
    outputPriorityGroup->addAction(ui->actionPriorityNetwork);
    outputPriorityGroup->addAction(ui->actionPriorityWinMsg);

    // Only one processing method can be used at a time
    processingMethodGroup = new QActionGroup(this);
    processingMethodGroup->setExclusive(true);
    processingMethodGroup->addAction(ui->actionMethodPriority);
    processingMethodGroup->addAction(ui->actionMethodExclusive);
    processingMethodGroup->addAction(ui->actionMethodConcurrent);

    // Check the menu entry of the preferred output source
    if (outputSourcePriority == SourceWinMsg)
        ui->actionPriorityWinMsg->setChecked(true);
    else
        ui->actionPriorityNetwork->setChecked(true);

    // Check the menu entry of the processing method in use
    if (outputProcessingMethod == MethodExclusive)
        ui->actionMethodExclusive->setChecked(true);
    else if (outputProcessingMethod == MethodConcurrent)
        ui->actionMethodConcurrent->setChecked(true);
    else
        ui->actionMethodPriority->setChecked(true);

    // Get "Start OutputHooker Minimized" setting
    startMinimized = p_config->getStartMinimized();
    ui->actionStartMinimized->setChecked(startMinimized);

    // The autostart state is read back from the registry, so an entry removed outside of
    // OutputHooker (Task Manager, msconfig) is reflected here
    ui->actionAutostart->setChecked(p_config->getAutostartWithSystem());

    // Grey out the preferred output source, if both output streams are processed
    updateOutputPriorityMenu();

    // Disable menu entry "Edit ini file for current game"
    ui->actionEditCurrentGameINI->setEnabled(false);

    // Signals and slots (used to display info on MainWindow)
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

    // Initialize server for command line arguments
    initServer();
}

OutputHooker::~OutputHooker()
{
    delete ui;
    delete p_config;
    delete p_core;
}

// Initialize server for command line arguments
void OutputHooker::initServer()
{
    p_localServer = new QLocalServer(this);
    QString serverName = "OutputHooker_CommandLine_Server";

    QLocalServer::removeServer(serverName);

    if (p_localServer->listen(serverName))
    {
        connect(p_localServer, &QLocalServer::newConnection, this, &OutputHooker::onNewConnection);
    }
}

// Process command line arguments
void OutputHooker::processCommandLineArgs(const QStringList &args)
{
    p_core->executeCommandLineCommands(args);
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
    signalItemMap.clear();

    // listWidget needs to be cleared
    ui->listWidget->clear();

    // Reset pointers
    romItem = nullptr;
    fileItem = nullptr;
    orientationItem = nullptr;
    pauseItem = nullptr;

    // Disable menu entry "Edit ini file for current game"
    ui->actionEditCurrentGameINI->setEnabled(false);
}

// Displays top of game data on MainWindow
void OutputHooker::displayGame(QString gName, QString iName, bool iniGame, bool noGameFound)
{
    // Display text QMap and listWidget needs to be cleared
    signalItemMap.clear();
    ui->listWidget->clear();

    isGameINI = iniGame;
    noGameFileFound = noGameFound;
    gameName = gName;
    iniName = iName;

    makeTopDisplayText();

    ui->listWidget->addItem(" ");
    QListWidgetItem *signalsItem = new QListWidgetItem(OUTPUTSIGNALS, ui->listWidget);
    QFont font = signalsItem->font();
    font.setPointSize(11);
    font.setUnderline(true);
    signalsItem->setFont(font);

    // Enable menu entry "Edit ini file for current game"
    ui->actionEditCurrentGameINI->setEnabled(true);
}

// Displays empty game data on MainWindow
void OutputHooker::displayEmptyGame()
{
    // Display text QMap and listWidget needs to be cleared
    signalItemMap.clear();
    ui->listWidget->clear();

    gameName = MAMENOGAMEEMPTY;

    makeTopDisplayText();

    // Disable menu entry "Edit ini file for current game"
    ui->actionEditCurrentGameINI->setEnabled(false);
}

// Add Output signals to the MainWindow
void OutputHooker::addSignalDisplay(const QString &sig, const QString &dat)
{
    QString temp = sig + " = " + dat;
    QListWidgetItem *item = new QListWidgetItem(temp, ui->listWidget);
    signalItemMap.insert(sig, item);
}

// Update output signals data on the MainWindow
void OutputHooker::updateSignalDisplay(const QString &sig, const QString &dat)
{
    if (signalItemMap.contains(sig))
    {
        QString temp = sig + " = " + dat;
        signalItemMap[sig]->setText(temp);
    }
    else
    {
        addSignalDisplay(sig, dat);
    }
}

// Update the pause data on the MainWindow
void OutputHooker::updatePauseDisplay(QString dat)
{
    if (pauseItem)
    {
        pauseItem->setText(PAUSEEQUALS + dat);
    }
}

// Update the orientation data on the MainWindow
void OutputHooker::updateOrientationDisplay(QString sig, QString dat)
{
    if (orientationItem)
    {
        orientationItem->setText(sig + "=" + dat);
    }
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

    if (!isWinMsgConnected && !isTCPConnected)
    {
        ui->actionEditCurrentGameINI->setEnabled(false);
    }
}

// Handle Error MessageBox from a different thread
void OutputHooker::errorMessage(const QString title, const QString message)
{
    GuiUtilities::showMessageBoxCentered(this, title, message, QMessageBox::Critical);
}

// Process new connection (needed for command line arguments)
void OutputHooker::onNewConnection()
{
    QLocalSocket *clientSocket = p_localServer->nextPendingConnection();
    connect(clientSocket, &QLocalSocket::readyRead, this, &OutputHooker::readSocket);
    connect(clientSocket, &QLocalSocket::disconnected, clientSocket, &QLocalSocket::deleteLater);
}

// Read data (command line arguments) from socket
void OutputHooker::readSocket()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket*>(sender());

    if (!socket)
        return;

    QDataStream in(socket);
    QStringList args;
    in >> args;

    if (!args.isEmpty())
    {
        this->processCommandLineArgs(args);
    }
}

// Open DeviceWindow
void OutputHooker::on_actionConnectedDevices_triggered()
{
    if (isTCPConnected || isWinMsgConnected)
    {
        GuiUtilities::showMessageBoxCentered(this, "Error!", "Close the game or emulator, to show the connected device(s)!", QMessageBox::Critical);
        return;
    }
    else if (!p_deviceWindow)
    {
        connectedDevicesAction->setEnabled(false);
        ui->actionConnectedDevices->setEnabled(false);
        p_deviceWindow = new DeviceWindow(this);
        connect(p_deviceWindow, &DeviceWindow::rejected, this, &OutputHooker::DeviceWindow_closed);
        connect(p_core, &OutputHookerCore::ledWizDeviceList, p_deviceWindow, &DeviceWindow::updateLedWizList);
        connect(p_core, &OutputHookerCore::ultimarcDeviceList, p_deviceWindow, &DeviceWindow::updateUltimarcList);
        connect(p_core, &OutputHookerCore::sdlDeviceList, p_deviceWindow, &DeviceWindow::updateSdlList);
        QMetaObject::invokeMethod(p_core->getLedWizModule(), "collectLedWizData", Qt::QueuedConnection);
        QMetaObject::invokeMethod(p_core->getPacDriveModule(), "collectUltimarcData", Qt::QueuedConnection);
        QMetaObject::invokeMethod(p_core->getSdlCtrlModule(), "collectSdlData", Qt::QueuedConnection);
    }
    p_deviceWindow->show();
}

// On close of DeviceWindow
void OutputHooker::DeviceWindow_closed()
{
    connectedDevicesAction->setEnabled(true);
    ui->actionConnectedDevices->setEnabled(true);
}

// Open TestOutputWindow
void OutputHooker::on_actionTestOutputs_triggered()
{
    if (isTCPConnected || isWinMsgConnected)
    {
        GuiUtilities::showMessageBoxCentered(this, "Error!", "Close the game or emulator, to test the output(s)!", QMessageBox::Critical);
        return;
    }
    else if (!p_testOutputWindow)
    {
        testOutputAction->setEnabled(false);
        ui->actionTestOutputs->setEnabled(false);
        p_testOutputWindow = new TestOutputWindow(this);
        connect(p_testOutputWindow, &TestOutputWindow::sendTestCommand, p_core, &OutputHookerCore::executeTestCommand);
        connect(p_testOutputWindow, &TestOutputWindow::rejected, this, &OutputHooker::TestOutputWindow_closed);
    }
    p_testOutputWindow->show();
}

// On close of TestOutputWindow
void OutputHooker::TestOutputWindow_closed()
{
    testOutputAction->setEnabled(true);
    ui->actionTestOutputs->setEnabled(true);
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

    addNewOutputsToDefaultINI = ui->actionDefaultINI->isChecked();
    p_config->setAddNewOutputsToDefaultINI(addNewOutputsToDefaultINI);
    p_config->saveSettings();

    // Start OutputHookerCore
    p_core->startCore();
    coreRunning = true;
}

// Set the Network output source as the one with priority
void OutputHooker::on_actionPriorityNetwork_triggered()
{
    applyOutputSourcePriority(SourceNetwork);
}

// Set the Windows message system output source as the one with priority
void OutputHooker::on_actionPriorityWinMsg_triggered()
{
    applyOutputSourcePriority(SourceWinMsg);
}

// Store the output source priority setting and restart the OutputHookerCore
void OutputHooker::applyOutputSourcePriority(OutputSource osPriority)
{
    // Selecting the output source that already has priority would restart the
    // OutputHookerCore for nothing and stop a game that is running
    if (osPriority == outputSourcePriority)
        return;

    if (coreRunning)
    {
        // Stop OutputHookerCore
        p_core->stopCore();
        coreRunning = false;
    }

    outputSourcePriority = osPriority;
    p_config->setOutputSourcePriority(outputSourcePriority);
    p_config->saveSettings();

    // Start OutputHookerCore
    p_core->startCore();
    coreRunning = true;
}

// Set how the two output streams are handled
void OutputHooker::on_actionMethodPriority_triggered()
{
    applyOutputProcessingMethod(MethodPriority);
}

void OutputHooker::on_actionMethodExclusive_triggered()
{
    applyOutputProcessingMethod(MethodExclusive);
}

void OutputHooker::on_actionMethodConcurrent_triggered()
{
    applyOutputProcessingMethod(MethodConcurrent);
}

// Store the output processing method and restart the OutputHookerCore
void OutputHooker::applyOutputProcessingMethod(OutputProcessingMethod opMethod)
{
    // Selecting the method that is already in use would restart the OutputHookerCore for
    // nothing and stop a game that is running
    if (opMethod == outputProcessingMethod)
        return;

    if (coreRunning)
    {
        // Stop OutputHookerCore
        p_core->stopCore();
        coreRunning = false;
    }

    outputProcessingMethod = opMethod;
    p_config->setOutputProcessingMethod(outputProcessingMethod);
    p_config->saveSettings();

    updateOutputPriorityMenu();

    // Start OutputHookerCore
    p_core->startCore();
    coreRunning = true;
}

// The preferred output source has no effect when both streams are processed
void OutputHooker::updateOutputPriorityMenu()
{
    ui->menuOutputPriority->setEnabled(outputProcessingMethod != MethodConcurrent);
}

// Change the start in the system tray setting
void OutputHooker::on_actionStartMinimized_triggered()
{
    // Only changes how OutputHooker starts next time, so the core keeps running
    startMinimized = ui->actionStartMinimized->isChecked();
    p_config->setStartMinimized(startMinimized);
    p_config->saveSettings();
}

// Change the start with Windows setting
void OutputHooker::on_actionAutostart_triggered()
{
    // Written straight to the registry Run key, nothing is kept in the INI file
    p_config->setAutostartWithSystem(ui->actionAutostart->isChecked());

    // Show what the registry actually ended up with, in case the write was refused
    ui->actionAutostart->setChecked(p_config->getAutostartWithSystem());
}

// Show the main window and let the core know it is no longer minimized
void OutputHooker::showMainWindow()
{
    show();
    setWindowState(Qt::WindowActive);
    p_core->mainWindowState(false);
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
        ui->actionEditDefaultINI->setEnabled(false);
        ui->actionEditSpecificGameINI->setEnabled(false);
        ui->actionEditCurrentGameINI->setEnabled(false);
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
        ui->actionEditDefaultINI->setEnabled(false);
        ui->actionEditSpecificGameINI->setEnabled(false);
        ui->actionEditCurrentGameINI->setEnabled(false);

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
            ui->actionEditDefaultINI->setEnabled(true);
            ui->actionEditSpecificGameINI->setEnabled(true);
            ui->actionEditCurrentGameINI->setEnabled(isTCPConnected || isWinMsgConnected);
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
        ui->actionEditDefaultINI->setEnabled(false);
        ui->actionEditSpecificGameINI->setEnabled(false);
        ui->actionEditCurrentGameINI->setEnabled(false);
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
    ui->actionEditDefaultINI->setEnabled(true);
    ui->actionEditSpecificGameINI->setEnabled(true);
    ui->actionEditCurrentGameINI->setEnabled(isTCPConnected || isWinMsgConnected);
}

// Open AboutWindow
void OutputHooker::on_actionAboutWindow_triggered()
{
    if (!p_aboutWindow)
    {
        ui->actionAboutWindow->setEnabled(false);
        p_aboutWindow = new AboutWindow(this);
        connect(p_aboutWindow, &DeviceWindow::rejected, this, &OutputHooker::AboutWindow_closed);
    }
    p_aboutWindow->show();
}

// On close of AboutWindow
void OutputHooker::AboutWindow_closed()
{
    ui->actionAboutWindow->setEnabled(true);
}

// Device connect or disconnect event
bool OutputHooker::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    MSG *msg = static_cast<MSG *>(message);

    if (msg->message == WM_DEVICECHANGE)
    {
        // DBT_DEVICEARRIVAL: Device connected
        // DBT_DEVICEREMOVECOMPLETE: Device disconnected
        if (msg->wParam == DBT_DEVICEARRIVAL || msg->wParam == DBT_DEVICEREMOVECOMPLETE)
        {
            if (p_deviceWindow)
            {
                QTimer::singleShot(100, p_deviceWindow, &DeviceWindow::checkComPorts);
            }

            if (p_core)
            {
                p_core->refreshLwDevices();
            }
        }
    }

    return QMainWindow::nativeEvent(eventType, message, result);
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

// Makes the default display data that is shown in the listWidget
void OutputHooker::makeTopDisplayText()
{
    QListWidgetItem *gameInfoItem = new QListWidgetItem(GAMEINFO, ui->listWidget);
    QFont font = gameInfoItem->font();
    font.setPointSize(11);
    font.setUnderline(true);
    gameInfoItem->setFont(font);

    // ROM name
    QString tempRom = ROMEQUALS + gameName;
    romItem = new QListWidgetItem(tempRom, ui->listWidget);

    // Game file
    QString tempFile = GAMEFILE;
    if (gameName != MAMENOGAMEEMPTY && !gameName.isEmpty())
    {
        tempFile.append(iniName);
        if (isGameINI)
            tempFile.append(ENDOFINIFILE);
    }
    fileItem = new QListWidgetItem(tempFile, ui->listWidget);

    // Orientation & Pause
    orientationItem = new QListWidgetItem(ORIENTATIONEQUAL0, ui->listWidget);
    pauseItem = new QListWidgetItem(PAUSEEQUALS0, ui->listWidget);
}
