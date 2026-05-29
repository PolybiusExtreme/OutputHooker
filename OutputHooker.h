/*
 * Original Copyright (c) 2026 PolybiusExtreme
 * Portions Copyright (c) 2026 6Bolt
 *
 * Licensed under the GNU GPLv3.
 */

#ifndef OUTPUTHOOKER_H
#define OUTPUTHOOKER_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QPointer>
#include <QSystemTrayIcon>
#include <QFileDialog>
#include <QCloseEvent>
#include <QLocalServer>
#include <QLocalSocket>

#include "gui/DeviceWindow.h"
#include "gui/TestOutputWindow.h"
#include "gui/EditorWindow.h"
#include "gui/AboutWindow.h"

#include "core/OutputHookerCore.h"
#include "core/OutputHookerConfig.h"

#include <Windows.h>

namespace Ui {
class OutputHooker;
}

class OutputHooker : public QMainWindow
{
    Q_OBJECT

public:
    OutputHooker(QWidget *parent = nullptr);
    ~OutputHooker();

    // Initialize server for command line arguments
    void initServer();

    // Process command line arguments
    void processCommandLineArgs(const QStringList &args);

    // Disable the Task Icon Menu - Open OutputHooker
    void setVisible(bool visible) override;

    // getCore needed for DeviceWindow
    OutputHookerCore* getCore() const { return p_core; }

public slots:
    // Display data in the textBrowser when no game found
    void displayNoGame();

    // Display data in the textBrowser when game is connected to the TCP Socket
    void displayGame(QString gName, QString iName, bool iniGame, bool noGameFound);

    // Display data in the textBrowser when empty game is found
    void displayEmptyGame();

    // Add output signals to the display data in textBrowser
    void addSignalDisplay(const QString &sig, const QString &dat);

    // Update output signal values in display data
    void updateSignalDisplay(const QString &sig, const QString &dat);

    // Update the pause value in the display data
    void updatePauseDisplay(QString dat);

    // Update the orientation value in the display data
    void updateOrientationDisplay(QString sig, QString dat);

    // Update the connection status
    void updateConnectionStatus(OutputHookerCore::ConnectionType type, bool status);

    // Handle error message from a different thread
    void errorMessage(const QString title, const QString message);

private slots:
    // Process new connection (needed for command line arguments)
    void onNewConnection();

    // Read data (command line arguments) from socket
    void readSocket();

    // Open DeviceWindow
    void on_actionConnected_Devices_triggered();

    // Open TestOutputWindow
    void on_actionTestOutputs_triggered();

    // On close of TestOutputWindow
    void TestOutputWindow_closed();

    // Close OutputHooker
    void on_actionExit_triggered();

    // Change notification setting
    void on_actionNotification_triggered();

    // Change default.ini setting
    void on_actionDefaultINI_triggered();

    // Open the default.ini in EditorWindow
    void on_actionEditDefaultINI_triggered();

    // Open EditorWindow
    void on_actionEditSpecificGameINI_triggered();

    // Open the current gamename.ini in EditorWindow
    void on_actionEditCurrentGameINI_triggered();

    //  On close of EditorWindow
    void EditorWindow_closed();

    // Open AboutWindow
    void on_actionAboutWindow_triggered();

protected:
    // Device connect or disconnect event
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

    // Hide from the Taskbar when minimized
    void changeEvent(QEvent *event) override;

private:
    // Main Window
    Ui::OutputHooker *ui;

    // Makes the default display data that is shown in the textBrowser
    void makeTopDisplayText();

    // Display the display data in the textBrowser
    void displayText();

    // Local server for command line arguments
    QLocalServer *p_localServer;

    // QPointer - OutputHooker Core
    OutputHookerCore *p_core;

    // QPointer - OutputHooker Config
    OutputHookerConfig *p_config;

    // QPointer - Connected Device(s) Window
    QPointer<DeviceWindow> p_deviceWindow;

    // QPointer - Test Outputs Window
    QPointer<TestOutputWindow> p_testOutputWindow;

    // QPointer - Editor Window
    QPointer<EditorWindow> p_editorWindow;

    // QPointer - About Window
    QPointer<AboutWindow> p_aboutWindow;

    // Bool settings
    bool useNewOutputsNotification;
    bool addNewOutputsToDefaultINI;

    // If a child window is open, stop the core
    // If a child window is closed, restart the core
    bool coreRunning;

    // Used to display text on the textBrowser
    QStringList displayTextList;

    // Maps where the Output data is in the display data
    QMap<QString,quint8> signalNumberMap;

    // If the Windows Message system is connected or not
    bool isWinMsgConnected;

    // If the TCP Socket is connected or not
    bool isTCPConnected;

    // Name of the game or ROM
    QString gameName;

    // Name of the INI file
    QString iniName;

    // Is game INI
    bool isGameINI;

    // No game file found
    bool noGameFileFound;

    // Tray Icon
    QSystemTrayIcon *trayIcon;

    // Tray Icon Menu
    QMenu *trayIconMenu;

    // Tray Icon Menu Action - Open OutputHooker
    QAction *restoreAction;

    // Tray Icon Menu Action - Open Connected Device(s)
    QAction *connectedDevicesAction;

    // Tray Icon Menu Action - Open Test Output(s)
    QAction *testOutputAction;

    // Tray Icon Menu Action - Open Editor
    QAction *scriptEditorAction;

    // Tray Icon Menu Action - Quit
    QAction *quitAction;
};
#endif // OUTPUTHOOKER_H
