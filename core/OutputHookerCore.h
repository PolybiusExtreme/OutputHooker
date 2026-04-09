#ifndef OUTPUTHOOKERCORE_H
#define OUTPUTHOOKERCORE_H

#include <QObject>
#include <QThread>
#include <QTimer>
#include <QByteArray>
#include <QStringList>
#include <QTextStream>
#include <QMap>
#include <QFile>
#include <QDir>
#include <QScreen>
#include <QProcess>
#include <QSoundEffect>

#include "OutputHookerConfig.h"
#include "TCPSocketModule.h"
#include "WinMsgModule.h"
#include "COMPortModule.h"
#include "PacDriveModule.h"

#include <windows.h>
#include <shellapi.h>

class OutputHookerCore : public QObject
{
    Q_OBJECT

    // Threads for Multi-Threading
    QThread threadForWinMsg;
    QThread threadForTCPSocket;
    QThread threadForCOMPort;
    QThread threadForLight;

    // Timer for KeyStates
    QTimer *keyStateTimer;

public:
    explicit OutputHookerCore(OutputHookerConfig *ohConfig, QObject *parent = nullptr);
    ~OutputHookerCore();

    // Connection types
    enum ConnectionType { WinMsg, TCP };

    // Start & stop the OutputHookerCore
    void startCore();
    void stopCore();

    // Load settings
    void loadSettingsFromList();

    // OutputHooker window state
    void mainWindowState(bool isMin);

    // Pass the HWND through from OutputHooker to WinMsgModule
    void setWinID(HWND handle);

public slots:
    // Execute command from TestOutputWindow
    void executeCommand(const FunctionCommand &cmd);

    // Handle error message from a different thread
    void errorMessage(const QString &title, const QString &message);

signals:
    // Starts and stops the Windows message system
    void startWinMsg();
    void stopWinMsg();

    // Starts and stops the TCP Socket
    void startTCPSocket();
    void stopTCPSocket();

    // Connection status
    void connectionStatus(OutputHookerCore::ConnectionType type, bool status);

    // Connects & disconnects a certain Serial COM Port
    void startComPort(const quint8 &comPortNum, const QString &comPortName, const qint32 &comPortBaud, const quint8 &comPortData, const quint8 &comPortParity, const quint8 &comPortStop, const quint8 &comPortFlow, const QString &comPortPath, const bool &isWriteOnly);
    void stopComPort(const quint8 &comPortNum);

    // Write data on a certain Serial COM Port
    void writeComPort(const quint8 &comPortNum, const QByteArray &writeData);

    // Set the bypass of the Serial COM Port write checks
    void setComPortBypassWriteChecks(const bool &cpBWC);

    // Set the bypass of Serial COM port connection fail warning pop-up
    void setBypassComPortConnectFailWarning(const bool &cpBCPCFW);

    // Closes all open Serial COM ports
    void stopAllConnections();

    // Set PacDrive pin state
    void setPinState(const quint8 &pdID, const quint8 &pdPin, const bool &pdState);

    // Set PacDrive light intensity
    void setLightIntensity(const quint8 &pdID, const quint8 &pdPin, const quint8 &pdIntensity);

    // Turn all PacDrive lights off
    void turnAllLightsOff(const quint8 &pdID);

    // Update the display data
    void noConnectedGame();
    void connectedGame(QString gName, QString iName, bool iniGame, bool noGameFound);
    void connectedEmptyGame();
    void addSignalFromGame(const QString &sig, const QString &dat);
    void updateSignalFromGame(const QString &sig, const QString &dat);
    void updatePauseFromGame(QString dat);
    void updateOrientationFromGame(QString sig, QString dat);

    // Signal used to handle error message from a different thread
    void showErrorMessage(const QString &title, const QString &message);

private slots:
    // Windows message system connected
    void winMsgConnected();

    // Windows message system disconnected
    void winMsgDisconnected();

    // TCP Socket connected
    void tcpConnected();

    // TCP Socket disconnected
    void tcpDisconnected();

    // Check KeyStates
    void checkKeyStates();

    // Process data
    void processData(const QString &signal, const QString &data);

    // Game start process
    void gameStart(const QString &data);

    // Empty game start process
    void emptyGameStart();

    // Game stopped process
    void gameStopped();

private:
    // Config settings
    OutputHookerConfig *p_config;

    // Windows message system module
    WinMsgModule *p_winMsg;

    // TCP Socket module
    TCPSocketModule *p_tcpSocket;

    // Serial COM Port module
    COMPortModule *p_comPort;

    // PacDrive module
    PacDriveModule *p_pacDrive;

    // Game found process
    void gameFound();

    // Check if an INI file exists for the game
    bool isINIFile();

    // Check if an default INI file exists
    bool isDefaultINIFile();

    // Load INI file for the game
    void loadINIFile();

    // Create a new INI File for the game
    void newINIFile();

    // Check the commands loaded from INI file
    bool checkINICommands(QStringList commandsNotChk, quint16 lineNumber, QString filePathName);

    // Check single command loaded from INI file
    bool checkINICommand(QString commandNotChk, quint16 lineNumber, QString filePathName);

    // Map key name to virtual key code
    int mapKeyNameToCode(const QString &name);

    // Process commands based on INI file
    void processINICommands(QString signalName, QString value, bool isState);

    // Execute commands based on INI file
    void executeINICommands(const QStringList &commands, const QString &value = "");

    // Open COM port
    void comPortOpen(quint8 cpNum);

    // Close COM port
    void comPortClose(quint8 cpNum);

    // Write to COM port
    void comPortWrite(quint8 cpNum, QString cpData);

    // Set PacDrive pin state
    void setPacDrivePinState(quint8 pacID, quint8 pacPin, bool pacState);

    // Set PacDrive light intensity
    void setPacDriveLightIntensity(quint8 pacID, quint8 pacPin, quint8 pacIntensity);

    // Turn all PacDrive lights off
    void turnAllPacDriveLightsOff(quint8 pacID);

    // Launch Application
    void launchApplication(QString executable, QString parameter, quint8 mode);

    // Close Application
    void closeApplication(QString executable);

    // Play WAV audio file
    void playWavAudioFile(QString file);

    // Clear things on a Window message system or TCP Socket disconnect
    void clearOnDisconnect();

    // Current path
    QString currentPath;

    // Create directory, if path not exist
    bool canMKDIR;

    QString gameName;
    bool isGameFound;
    bool gameHasRun;
    bool isEmptyGame;
    bool gameHasStopped;

    QString iniName;
    QString iniPath;
    QString gameINIFilePath;
    QDir iniPathDir;
    bool iniDirExists;
    bool iniFileLoaded;
    bool iniFileLoadFail;
    bool isGameINI;
    bool isDefaultINI;

    // Signals and commands
    QMap<QString,QStringList> signalsAndCommands;
    QStringList signalsNoCommands;

    // States and commands. States are Pause, Orientation and so on.
    QMap<QString,QStringList> stateAndCommands;
    QStringList statesNoCommands;

    // Signals & data and states & data collected from Windows message system & TCP Socket
    QMap<QString,QString> signalsAndData;
    QMap<QString,QString> statesAndData;

    // KeyStates and commands
    QMap<int, QStringList> keyStatesAndCommands;
    QMap<int, bool> lastKeyStates;
    int keyStatesRefreshTime;

    // Open COM port check
    QList<quint8> openComPortCheck;

    // Serial COM Port data map
    QMap<quint8,ComPortStruct> comPortMap;

    // If Windows message system is connected
    bool isWinMsgConnected;

    // If TCP Socket is connected
    bool isTCPSocketConnected;

    // If Core is running, to start/stop Windows message system & TCP Socket & timer
    bool isCoreStarted;

    // Settings used in OutputHookerCore
    bool useNewOutputsNotification;
    bool saveNewOutputsToDefaultINI;
    bool useMultiThreading;
    bool bypassSerialWriteChecks;

    // Is OutputHooker minimized in the tray
    bool isOutputHookerMinimized;
};

#endif // OUTPUTHOOKERCORE_H
