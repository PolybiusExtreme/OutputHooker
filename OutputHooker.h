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
#include <QListWidgetItem>
#include <QSystemTrayIcon>
#include <QFileDialog>
#include <QCloseEvent>
#include <QLocalServer>
#include <QLocalSocket>
#include <QActionGroup>

#include "gui/DeviceWindow.h"
#include "gui/TestOutputWindow.h"
#include "gui/EditorWindow.h"
#include "gui/AboutWindow.h"

#include "core/OutputHookerCore.h"
#include "core/OutputHookerConfig.h"

#include <Windows.h>

class QMessageBox;

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

    // If OutputHooker should start in the system tray instead of showing the window
    bool getStartMinimized() const { return startMinimized; }

    // Show the main window and let the core know it is no longer minimized
    void showMainWindow();

public slots:
    // Display data in the listWidget when no game found
    void displayNoGame();

    // Display data in the listWidget when game is connected to the TCP Socket
    void displayGame(QString gName, QString iName, bool iniGame, bool noGameFound);

    // Display data in the listWidget when empty game is found
    void displayEmptyGame();

    // Add output signals to the display data in the listWidget
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
    void on_actionConnectedDevices_triggered();

    // On close of DeviceWindow
    void DeviceWindow_closed();

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

    // Set the Network output source as the one with priority
    void on_actionPriorityNetwork_triggered();

    // Set the Windows message system output source as the one with priority
    void on_actionPriorityWinMsg_triggered();

    // Set how the two output streams are handled
    void on_actionMethodPriority_triggered();
    void on_actionMethodExclusive_triggered();
    void on_actionMethodConcurrent_triggered();

    // Change the start in the system tray setting
    void on_actionStartMinimized_triggered();

    // Change the start with Windows setting
    void on_actionAutostart_triggered();

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

    // On close of AboutWindow
    void AboutWindow_closed();

protected:
    // Device connect or disconnect event
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

    // Hide from the Taskbar when minimized
    void changeEvent(QEvent *event) override;

private:
    // Main Window
    Ui::OutputHooker *ui;

    // Makes the default display data that is shown in the listWidget
    void makeTopDisplayText();

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
    bool startMinimized;

    // Output source that is preferred
    OutputSource outputSourcePriority;

    // How the two output streams are handled
    OutputProcessingMethod outputProcessingMethod;

    // Make each set of menu entries exclusive
    QActionGroup *outputPriorityGroup;
    QActionGroup *processingMethodGroup;

    // Store the preferred output source and restart the OutputHookerCore
    void applyOutputSourcePriority(OutputSource osPriority);

    // Store the output processing method and restart the OutputHookerCore
    void applyOutputProcessingMethod(OutputProcessingMethod opMethod);

    // The preferred output source has no effect when both streams are processed
    void updateOutputPriorityMenu();

    // If a child window is open, stop the core
    // If a child window is closed, restart the core
    bool coreRunning;

    // Maps where the Output data is in the display data
    QMap<QString, QListWidgetItem*> signalItemMap;

    // Error dialogs that are currently open, keyed by the error text. A repeat of the same
    // error updates its open dialog with a count instead of stacking a new dialog
    QMap<QString, QMessageBox*> openErrorBoxes;
    QMap<QString, int> errorBoxCounts;

    // Pointer for the static info lines
    QListWidgetItem* romItem = nullptr;
    QListWidgetItem* fileItem = nullptr;
    QListWidgetItem* orientationItem = nullptr;
    QListWidgetItem* pauseItem = nullptr;

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
