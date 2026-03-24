#include "OutputHookerCore.h"

#include "../Global.h"

OutputHookerCore::OutputHookerCore(OutputHookerConfig *ohConfig, QObject *parent)
    : QObject{parent}
{
    // If core is started
    isCoreStarted = false;

    // If Windows message system or TCP Socket is connected
    isWinMsgConnected = false;
    isTCPSocketConnected = false;

    // If game is found
    isGameFound = false;
    // If a game has ran
    gameHasRun = false;
    // If no game is loaded
    isEmptyGame = false;
    // If game has stopped
    gameHasStopped = false;

    // OutputHooker start minimized in the Tray
    isOutputHookerMinimized = true;

    // INI game file loaded or failed loading
    iniFileLoaded = false;
    iniFileLoadFail = false;
    isGameINI = false;
    isDefaultINI = false;

    // Bool to check if mkdir
    canMKDIR = true;

    // Start of directory and path checking. If directory doesn't exists, it will be created
    currentPath = QApplication::applicationDirPath();

    // INI stuff
    iniPathDir.setPath(currentPath);

    iniDirExists = iniPathDir.exists(INIFILEDIR);

    if (!iniDirExists)
        canMKDIR = iniPathDir.mkdir(INIFILEDIR);
    else
        canMKDIR = true;

    if (!canMKDIR)
    {
        emit showErrorMessage("Error", "Could not create the ini directory!");
        return;
    }

    iniPathDir.setPath(currentPath + "/" + INIFILEDIR);
    iniPath = currentPath + "/" + INIFILEDIR;

    // Set up the config
    p_config = ohConfig;

    // Needed setting value
    useMultiThreading = p_config->getUseMultiThreading();

    // Set up Windows message system
    p_winMsg = new WinMsgModule();

    // Set up the TCP Socket
    p_tcpSocket = new TCPSocketModule();

    if (useMultiThreading)
    {
        // Move Windows message system to different thread
        p_winMsg->moveToThread(&threadForWinMsg);
        connect(&threadForWinMsg, &QThread::finished, p_winMsg, &QObject::deleteLater);

        // Move TCP Socket to different thread
        p_tcpSocket->moveToThread(&threadForTCPSocket);
        connect(&threadForTCPSocket, &QThread::finished, p_tcpSocket, &QObject::deleteLater);
    }

    // Windows message system connections

    // Connect the signals & slots for Windows message system
    connect(this, &OutputHookerCore::startWinMsg, p_winMsg, &WinMsgModule::connectWinMsg);
    connect(this, &OutputHookerCore::stopWinMsg, p_winMsg, &WinMsgModule::disconnectWinMsg);
    connect(p_winMsg,&WinMsgModule::dataRead, this, &OutputHookerCore::processData);
    connect(p_winMsg,&WinMsgModule::winMsgConnectedSignal, this, &OutputHookerCore::winMsgConnected);
    connect(p_winMsg,&WinMsgModule::winMsgDisconnectedSignal, this, &OutputHookerCore::winMsgDisconnected);

    // When game or an empty game has started
    connect(p_winMsg,&WinMsgModule::gameHasStarted, this, &OutputHookerCore::gameStart);
    connect(p_winMsg,&WinMsgModule::emptyGameHasStarted, this, &OutputHookerCore::emptyGameStart);

    // When game has stopped
    connect(p_winMsg,&WinMsgModule::gameHasStopped, this, &OutputHookerCore::gameStopped);

    // TCP Socket connections

    // Connect the signals & slots for TCP Socket
    connect(this, &OutputHookerCore::startTCPSocket, p_tcpSocket, &TCPSocketModule::connectTCP);
    connect(this, &OutputHookerCore::stopTCPSocket, p_tcpSocket, &TCPSocketModule::disconnectTCP);
    connect(p_tcpSocket,&TCPSocketModule::dataRead, this, &OutputHookerCore::processData);
    connect(p_tcpSocket,&TCPSocketModule::tcpConnectedSignal, this, &OutputHookerCore::tcpConnected);
    connect(p_tcpSocket,&TCPSocketModule::tcpDisconnectedSignal, this, &OutputHookerCore::tcpDisconnected);

    // When game or an empty game has started
    connect(p_tcpSocket,&TCPSocketModule::gameHasStarted, this, &OutputHookerCore::gameStart);
    connect(p_tcpSocket,&TCPSocketModule::emptyGameHasStarted, this, &OutputHookerCore::emptyGameStart);

    // When game has stopped
    connect(p_tcpSocket,&TCPSocketModule::gameHasStopped, this, &OutputHookerCore::gameStopped);

    if (useMultiThreading)
    {
        // Start TCP Socket thread
        threadForTCPSocket.start(QThread::HighPriority);

        // Start Windows message system thread
        threadForWinMsg.start(QThread::HighPriority);
    }

    // Set up the Serial COM Port(s)
    p_comPort = new COMPortModule();

    if (useMultiThreading)
    {
        // Move Serial COM Port(s) to different thread
        p_comPort->moveToThread(&threadForCOMPort);
        connect(&threadForCOMPort, &QThread::finished, p_comPort, &QObject::deleteLater);
    }

    // COM Port connections

    // Connect the signals & slots for Serial COM Port
    connect(this, &OutputHookerCore::startComPort, p_comPort, &COMPortModule::connectComPort);
    connect(this, &OutputHookerCore::stopComPort, p_comPort, &COMPortModule::disconnectComPort);
    connect(this, &OutputHookerCore::writeComPort, p_comPort, &COMPortModule::writeData);
    connect(this, &OutputHookerCore::setComPortBypassWriteChecks, p_comPort, &COMPortModule::setBypassSerialWriteChecks);
    connect(this, &OutputHookerCore::setBypassComPortConnectFailWarning, p_comPort, &COMPortModule::setBypassCOMPortConnectFailWarning);
    connect(this, &OutputHookerCore::stopAllConnections, p_comPort, &COMPortModule::disconnectAll);
    connect(p_comPort, &COMPortModule::showErrorMessage, this, &OutputHookerCore::errorMessage);

    if (useMultiThreading)
    {
        threadForCOMPort.start(QThread::HighPriority);
    }

    // Set up Ultimarc PacDrive
    p_pacDrive = new PacDriveModule();

    if (useMultiThreading)
    {
    // Move PacDriveModule to different thread
    p_pacDrive->moveToThread(&threadForLight);
    connect(&threadForLight, &QThread::finished, p_pacDrive, &QObject::deleteLater);
    }

    // PacDrive connections

    // Connect the signals & slots for PacDrive
    connect(this, &OutputHookerCore::setPinState, p_pacDrive, &PacDriveModule::setPinState);
    connect(this, &OutputHookerCore::setLightIntensity, p_pacDrive, &PacDriveModule::setLightIntensity);
    connect(this, &OutputHookerCore::turnAllLightsOff, p_pacDrive, &PacDriveModule::turnAllLightsOff);
    connect(p_pacDrive,&PacDriveModule::showErrorMessage, this, &OutputHookerCore::errorMessage);

    if (useMultiThreading)
    {
        // Start PacDrive thread
        threadForLight.start(QThread::HighPriority);
    }

    // Load settings
    loadSettingsFromList();
}

OutputHookerCore::~OutputHookerCore()
{
    // Close all Serial COM Port connections
    emit stopAllConnections();

    // Clean up threads
    if (useMultiThreading)
    {
        threadForTCPSocket.quit();
        threadForWinMsg.quit();
        threadForCOMPort.quit();
        threadForLight.quit();
        threadForTCPSocket.wait();
        threadForWinMsg.wait();
        threadForCOMPort.wait();
        threadForLight.wait();
    }
    else
    {
        delete p_tcpSocket;
        delete p_winMsg;
        delete p_comPort;
        delete p_pacDrive;
    }
}

// Start the OutputHookerCore
void OutputHookerCore::startCore()
{
    // Windows message system connection status
    emit connectionStatus(OutputHookerCore::WinMsg, isWinMsgConnected);

    // TCP connection status
    emit connectionStatus(OutputHookerCore::TCP, isTCPSocketConnected);

    // Load Settings
    loadSettingsFromList();

    isCoreStarted = true;

    // Start the Windows message system connection
    emit startWinMsg();

    // Start the TCP connection
    emit startTCPSocket();
}

// Stop the OutputHookerCore
void OutputHookerCore::stopCore()
{
    isCoreStarted = false;

    // Stop the Windows message system connection
    emit stopWinMsg();

    // Stop the TCP connection
    emit stopTCPSocket();
}

// Load settings
void OutputHookerCore::loadSettingsFromList()
{
    // Settings values
    useNewOutputsNotification = p_config->getUseNewOutputsNotification();
    saveNewOutputsToDefaultINI = p_config->getSaveNewOutputsToDefaultINI();
    bypassSerialWriteChecks = p_config->getSerialPortWriteCheckBypass();
    // Don't get Multi-Threading, as it needs a application reset
    // useMultiThreading = p_config->getUseMultiThreading();

    emit setComPortBypassWriteChecks(bypassSerialWriteChecks);
}

// OutputHooker window state
void OutputHookerCore::mainWindowState(bool isMin)
{
    // OutputHooker minimized status
    isOutputHookerMinimized = isMin;
}

// Pass the HWND through from OutputHooker to WinMsgModule
void OutputHookerCore::setWinID(HWND handle)
{
    p_winMsg->setWinID(handle);
}

// Execute command from TestOutputWindow
void OutputHookerCore::executeCommand(const FunctionCommand &cmd)
{
    // COM port open command
    if (cmd.commandCode == COMPORTOPEN || cmd.commandCode == COMPORTSETTINGS)
    {
        quint8 comPortNumber = cmd.param1.toUInt();
        ComPortStruct portTemp;

        // Split the Settings into 4 strings = 1: Baud  2: Parity  3: Data  4: Stop
        QStringList temp = cmd.param2.split('_', Qt::SkipEmptyParts);

        if (temp.size() >= 4)
        {
            temp[0].remove(BAUDREMOVE);
            temp[1].remove(PARITYREMOVE);
            temp[2].remove(DATAREMOVE);
            temp[3].remove(STOPREMOVE);

            portTemp.baud = temp[0].toUInt();

            if (temp[1] == "N")
                portTemp.parity = 0;
            else if (temp[1] == "E")
                portTemp.parity = 2;
            else if (temp[1] == "O")
                portTemp.parity = 3;
            else if (temp[1] == "S")
                portTemp.parity = 4;
            else if (temp[1] == "M")
                portTemp.parity = 5;
            else
                portTemp.parity = 1;

            portTemp.data = temp[2].toUInt();

            if (temp[3] == "1.5")
                temp[3] = "3";

            portTemp.stop = temp[3].toUInt();

            comPortMap.insert(comPortNumber,portTemp);
            comPortOpen(comPortNumber);
        }
    }
    // COM port close command
    else if (cmd.commandCode == COMPORTCLOSE)
    {
        quint8 comPortNumber = cmd.param1.toUInt();
        comPortClose(comPortNumber);
    }
    // COM port write command
    else if (cmd.commandCode == COMPORTWRITE)
    {
        quint8 comPortNumber = cmd.param1.toUInt();
        comPortWrite(comPortNumber, cmd.param2);
    }
    // PacDrive set pin state command
    else if (cmd.commandCode == PACSETSTATE)
    {
        quint8 pacID = cmd.param1.toUInt() - 1;
        quint8 pacPin = cmd.param2.toUInt() - 1;
        bool pacState = (cmd.param3.toUInt() > 0);
        setPacDrivePinState(pacID, pacPin, pacState);
    }
    // PacDrive set light intensity command
    else if (cmd.commandCode == PACSETINTENSITY)
    {
        quint8 pacID = cmd.param1.toUInt() - 1;
        quint8 pacPin = cmd.param2.toUInt() - 1;
        quint8 pacIntensity = cmd.param3.toUInt();
        setPacDriveLightIntensity(pacID, pacPin, pacIntensity);
    }
    // PacDrive kill all LEDs command
    else if (cmd.commandCode == PACKILLALLLEDS)
    {
        quint8 pacID = cmd.param1.toUInt() - 1;
        turnAllPacDriveLightsOff(pacID);
    }
    // Play WAV audio file command
    else if (cmd.commandCode == PLAYWAVAUDIO)
    {
        QString file = cmd.param1;
        playWavAudioFile(file);
    }
}

// Handle error message from a different thread
void OutputHookerCore::errorMessage(const QString &title, const QString &message)
{
    emit showErrorMessage(title, message);
}

// Windows message system connected
void OutputHookerCore::winMsgConnected()
{
    isWinMsgConnected = true;
    emit connectionStatus(OutputHookerCore::WinMsg, isWinMsgConnected);
}

// Windows message system disconnected
void OutputHookerCore::winMsgDisconnected()
{
    isWinMsgConnected = false;
    emit connectionStatus(OutputHookerCore::WinMsg, isWinMsgConnected);

    if (!gameHasStopped)
        clearOnDisconnect();

    if (!p_winMsg->isConnected && !p_winMsg->isConnecting)
        emit startWinMsg();
}

// TCP Socket connected
void OutputHookerCore::tcpConnected()
{
    isTCPSocketConnected = true;
    emit connectionStatus(OutputHookerCore::TCP, isTCPSocketConnected);
}

// TCP Socket disconnected
void OutputHookerCore::tcpDisconnected()
{
    isTCPSocketConnected = false;
    emit connectionStatus(OutputHookerCore::TCP, isTCPSocketConnected);

    if (!gameHasStopped)
        clearOnDisconnect();

    if (!p_tcpSocket->isConnected && !p_tcpSocket->isConnecting)
        emit startTCPSocket();
}

// Process data
void OutputHookerCore::processData(const QString &signal, const QString &data)
{
    QString tempSignal = signal;

    if (signal == ORIENTATION)
    {
        tempSignal = ROTATE;
    }

    // Check if no game is found
    if (!isGameFound)
    {
        if (isEmptyGame)
        {
            if (signal == PAUSE)
                emit updatePauseFromGame(data);
            else if (signal.startsWith(ORIENTATION))
                emit updateOrientationFromGame(signal,data);
        }
    }
    else
    {
        static const QStringList generalKey = {MAMESTART, MAMESTOP, STATECHANGE, ROTATE, PAUSE};

        // Check if it is a state, else it is a signal
        if (generalKey.contains(tempSignal) || tempSignal.startsWith(ORIENTATION))
        {
            // Process the command(s) attached to the state
            if (isGameINI || isDefaultINI)
            {
                if (stateAndCommands.contains(tempSignal))
                    processINICommands(tempSignal, data, true);
            }
            // Update Pause and Orientation to the display
            if (signal == PAUSE)
                emit updatePauseFromGame(data);
            else
                emit updateOrientationFromGame(signal,data);
        }
        else
        {
            // If not in the QMap, add it
            if (!isOutputHookerMinimized)
            {
                if (signalsAndData.contains(signal) == false)
                {
                    signalsAndData.insert(signal,data);
                    emit addSignalFromGame(signal, data);
                }
                else
                {
                    signalsAndData[signal] = data;
                    emit updateSignalFromGame(signal, data);
                }
            }
            if (signalsAndCommands.contains(signal))
            {
                if (iniFileLoaded)
                    processINICommands(signal, data, false);
            }
        }
    }
}

// Game start process
void OutputHookerCore::gameStart(const QString &data)
{
    QObject* dynamicSender = sender();

    if (dynamicSender == p_winMsg) {
        emit stopTCPSocket();
    }
    else if (dynamicSender == p_tcpSocket) {
        emit stopWinMsg();
    }

    gameName = data;

    if (saveNewOutputsToDefaultINI)
        iniName = DEFAULTFILE;
    else
        iniName = gameName;

    isGameFound = true;
    gameHasRun = true;
    isEmptyGame = false;
    gameHasStopped = false;

    // Start looking for game files to load
    gameFound();
}

// Empty game start process
void OutputHookerCore::emptyGameStart()
{
    QObject* dynamicSender = sender();

    if (dynamicSender == p_winMsg) {
        emit stopTCPSocket();
    }
    else if (dynamicSender == p_tcpSocket) {
        emit stopWinMsg();
    }

    emit connectedEmptyGame();
    isEmptyGame = true;
    gameHasStopped = false;
}

// Game stopped process
void OutputHookerCore::gameStopped()
{
    quint8 j;

    if (iniFileLoaded)
    {
        // Run the commands attached to 'mame_stop'
        processINICommands(MAMESTOP, "", true);

        // If INI file loaded, search for any new signal(s) not in the file
        // These signals are appended to the INI game file and closed
        quint8 foundCount = 0;
        bool foundNewSignal = false;
        QStringList nemSignalList;

        // Searching the 2 QMaps for any new signal(s)
        QMapIterator<QString, QString> x(signalsAndData);
        while (x.hasNext())
        {
            x.next();
            if (!signalsAndCommands.contains(x.key()) && !signalsNoCommands.contains(x.key()))
            {
                nemSignalList << x.key();
                foundCount++;
                foundNewSignal = true;
            }
        }

        // If any new signal(s) found, then append to the INI game file
        if (foundNewSignal)
        {
            // Play notification sound if setting is set
            if(useNewOutputsNotification)
            {
                playWavAudioFile("notification.wav");
            }

            // Open INI file to append
            QFile iniFileTemp(gameINIFilePath);
            if (!iniFileTemp.open(QIODevice::Append | QIODevice::Text))
            {
                emit showErrorMessage("Write Error", "Could not write to " + gameName + ENDOFINIFILE + "!");
                return;
            }
            QTextStream out(&iniFileTemp);

            for (j = 0; j < foundCount; j++){
                // If output signal is mame_stop or game_stop, then skip the entry
                if (nemSignalList[j] == MAMESTOP || nemSignalList[j] == GAMESTOP) {
                    continue;
                }
                out << nemSignalList[j] << "=\n";
            }
            iniFileTemp.close();
        }
        iniFileLoaded = false;
    }

    isGameFound = false;
    gameHasRun = false;
    isEmptyGame = false;
    signalsAndCommands.clear();
    stateAndCommands.clear();
    signalsNoCommands.clear();
    statesNoCommands.clear();
    signalsAndData.clear();
    statesAndData.clear();
    iniFileLoaded = false;
    gameHasStopped = true;

    if (isCoreStarted) {
        emit startWinMsg();
        emit startTCPSocket();

        emit connectionStatus(OutputHookerCore::WinMsg, isWinMsgConnected);
        emit connectionStatus(OutputHookerCore::TCP, isTCPSocketConnected);
    }

    emit noConnectedGame();
}

// Game found process
void OutputHookerCore::gameFound()
{
    // If setting is set, save new outputs to default.ini
    if (saveNewOutputsToDefaultINI)
    {
        // Checks if a default INI file exists
        isDefaultINI = isDefaultINIFile();

        // If there is a default INI file, then load it
        if (isDefaultINI)
        {
            emit connectedGame(gameName, iniName, true, false);
            loadINIFile();
        }
        else
        {
            // Play notification sound if setting is set
            if (useNewOutputsNotification)
            {
                playWavAudioFile("notification.wav");
            }

            emit connectedGame(gameName, iniName, true, false);
            newINIFile();
        }
    }
    else
    {
        // Check if a game INI file exists
        isGameINI = isINIFile();

        // If there is a game INI file, then load it
        if (isGameINI)
        {
            emit connectedGame(gameName, iniName, true, false);
            loadINIFile();
        }
        else
        {
            // Play notification sound if setting is set
            if (useNewOutputsNotification)
            {
                playWavAudioFile("notification.wav");
            }

            emit connectedGame(gameName, iniName, true, false);
            newINIFile();
        }
    }
}

// Check if an INI file exists for the game
bool OutputHookerCore::isINIFile()
{
    gameINIFilePath = iniPath + "/" + gameName + ENDOFINIFILE;
    return QFile::exists(gameINIFilePath);
}

// Check if an default INI file exists
bool OutputHookerCore::isDefaultINIFile()
{
    gameINIFilePath = iniPath + "/" + DEFAULTFILE + ENDOFINIFILE;
    return QFile::exists(gameINIFilePath);
}

// Load INI file for the game
void OutputHookerCore::loadINIFile()
{
    QString line;
    QString signal;
    QString commands;
    QStringList tempSplit;
    quint16 indexEqual;
    bool isOutput = false;
    bool goodCommand;
    quint16 lineNumber = 0;

    iniFileLoadFail = false;
    openComPortCheck.clear();

    // Open file. If failed to open, show Critical Message Box
    QFile iniFile(gameINIFilePath);
    // Set write permission to iniFile
    iniFile.setPermissions(iniFile.permissions() | QFile::WriteOwner);

    bool openFile = iniFile.open(QIODeviceBase::ReadOnly | QIODevice::Text);
    if (!openFile)
    {
        QString errorMsg = "Could not open " + gameName + ENDOFINIFILE + "!";
        emit showErrorMessage("Error", errorMsg);
        return;
    }

    // Create a TextStream, to stream easier in the data
    QTextStream in(&iniFile);

    while (!in.atEnd())
    {
        // Get a line of data from file
        line = in.readLine();
        lineNumber++;

        // All lines have an '=', except for the one with '[' and ']'
        if (!line.startsWith("["))
        {
            // Get the index of the equal
            indexEqual = line.indexOf('=',0);

            // If there is nothing after '=', put in no commands list
            // If something is after '=' then split it up into signals and command(s)
            if (line.length() != indexEqual+1)
            {
                if (line[indexEqual-1] == ' ')
                    signal = line.first(indexEqual-1);
                else
                    signal = line.first(indexEqual);

                if (line[indexEqual+1] == ' ')
                    commands = line.sliced(indexEqual+2);
                else
                    commands = line.sliced(indexEqual+1);

                commands.replace(", ", ",");
                commands.replace(" ,", ",");

                tempSplit = commands.split(',', Qt::SkipEmptyParts);

                if (!isOutput)
                {
                    if (signal.startsWith("On"))
                        signal.remove(0,2);

                    if (signal.startsWith(JUSTMAME, Qt::CaseInsensitive))
                        signal.insert(4,'_');

                    signal = signal.toLower();
                }

                goodCommand = checkINICommands(tempSplit, lineNumber, gameINIFilePath);

                // If bad command file, then fail load
                if (!goodCommand)
                {
                    iniFileLoadFail = true;
                    iniFile.close();
                    return;
                }

                if (isOutput)
                {
                    signalsAndCommands.insert(signal, tempSplit);
                }
                else
                {
                    static const QStringList generalKey = {MAMESTART, MAMESTOP, STATECHANGE, ROTATE, PAUSE};

                    if (generalKey.contains(signal))
                    {
                        stateAndCommands.insert(signal, tempSplit);
                    }
                }
            }
            else
            {
                line.chop(1);
                if (isOutput)
                    signalsNoCommands << line;
                else
                    statesNoCommands << line;
            }
        }
        else
        {
            // This is the lines with the '[' and ']'
            if (line.contains(SIGNALSTATE, Qt::CaseInsensitive))
                isOutput = true;
            else
                isOutput = false;
        }
    }

    // Close file
    iniFile.close();
    iniFileLoaded = true;

    // Bypass the COM port connection fail warning pop-up, as MAMEHooker does
    emit setBypassComPortConnectFailWarning(true);

    // Process the "mame_start" signal
    processINICommands(MAMESTART, "", true);
}

// Create a new INI File for the game
void OutputHookerCore::newINIFile()
{
    // Copy template INI file over with game name as the file name
    bool copyFile = QFile::copy(":/ini/template.ini", gameINIFilePath);

    if (!copyFile)
    {
        emit showErrorMessage("Error", "Could not copy template INI file to ini path!");
        return;
    }

    loadINIFile();
}

// Check the commands loaded from INI file
bool OutputHookerCore::checkINICommands(QStringList commandsNotChk, quint16 lineNumber, QString filePathName)
{
    QString command;
    QStringList subCommands;
    quint8 subCmdCnt;
    QString subCmd;
    quint8 commandCount = commandsNotChk.length();
    bool isCommandsGood = true;
    quint8 i, j;

    for (i = 0; i < commandCount; i++)
    {
        command = commandsNotChk[i];

        if (command.contains('|'))
        {
            commandsNotChk[i].replace(" |","|");
            commandsNotChk[i].replace("| ","|");

            subCommands = commandsNotChk[i].split('|');
            subCmdCnt = subCommands.length();

            for (j = 0; j < subCmdCnt; j++)
            {
                subCmd = subCommands[j];

                // Check for %s%, if so replace with 0
                if (subCmd.contains(SIGNALDATAVARIABLE))
                    subCmd.replace(SIGNALDATAVARIABLE, "0");

                isCommandsGood = checkINICommand(subCmd, lineNumber, filePathName);

                if (!isCommandsGood)
                {
                    QString errorMsg = "Loaded INI file contains a faulty command!\nLine Number: " + QString::number(lineNumber) + "\nCommand: " + subCommands[j] + "\nFile: " + gameName + ENDOFINIFILE;
                    emit showErrorMessage("Error", errorMsg);
                    return false;
                }
            }
        }
        else
        {
            // Check for %s%, if so replace with 0
            if (command.contains(SIGNALDATAVARIABLE))
                command.replace(SIGNALDATAVARIABLE, "0");

            isCommandsGood = checkINICommand(command, lineNumber, filePathName);

            if (!isCommandsGood)
            {
                QString errorMsg = "Loaded INI file contains a faulty command!\nLine Number: " + QString::number(lineNumber) + "\nCommand: " + command + "\nFile: "+ gameName + ENDOFINIFILE;
                emit showErrorMessage("Error", errorMsg);
                return false;
            }
        }
    }
    return isCommandsGood;
}

// Check single command loaded from INI file
bool OutputHookerCore::checkINICommand(QString commandNotChk, quint16 lineNumber, QString filePathName)
{
    QStringList cmd, settings;
    ComPortStruct portTemp;
    quint8 comPortNumber;
    bool isCommandsGood = true;
    bool isNumber;
    quint8 i;

    // COM port commands, starts with "cm" or "cs"
    if (commandNotChk.startsWith(PORTCMDSTART1, Qt::CaseInsensitive) == true || commandNotChk.startsWith(PORTCMDSTART2, Qt::CaseInsensitive) == true)
    {
        // COM port open command
        if (commandNotChk.startsWith(COMPORTOPEN, Qt::CaseInsensitive) || commandNotChk.startsWith(COMPORTSETTINGS, Qt::CaseInsensitive))
        {
            // This will give 3 strings = 1: cmo/css  2: Com Port #  3: Settings
            cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() < 3)
            {
                QString errorMsg = "Command requires 2 parameters (Port, Settings)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
                emit showErrorMessage("COM Port Open - Error", errorMsg);
                return false;
            }

            comPortNumber = cmd[1].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Port number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nPort: "+cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("COM Port Open - Error", errorMsg);
                return false;
            }

            // Add in COM port number for checks
            openComPortCheck << comPortNumber;

            if (cmd[2].isEmpty())
            {
                QString errorMsg = "COM Port settings are not set!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("COM Port Open - Error", errorMsg);
                return false;
            }

            // Split the settings into 4 strings = 1: Baud  2: Parity  3: Data  4: Stop
            settings = cmd[2].split('_', Qt::SkipEmptyParts);
            settings[0].remove(BAUDREMOVE);
            settings[1].remove(PARITYREMOVE);
            settings[2].remove(DATAREMOVE);
            settings[3].remove(STOPREMOVE);

            if (settings[0].isEmpty())
            {
                QString errorMsg = "Baud rate value is empty!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;
                showErrorMessage("COM Port Open - Error", errorMsg);
                return false;
            }

            portTemp.baud = settings[0].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Baud rate is not a number!\nLine Number: " + QString::number(lineNumber) + "\nBaud rate: " + settings[0] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("COM Port Open - Error", errorMsg);
                return false;
            }

            bool chkSetting = false;

            for (i = 0; i < BAUD_NUMBER; i++)
            {
                if (portTemp.baud == BAUDDATA_ARRAY[i])
                    chkSetting = true;
            }

            if (!chkSetting)
            {
                QString errorMsg = "Baud rate is not a correct rate!\nLine Number: " + QString::number(lineNumber) + "\nBaud rate: " + settings[0] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("COM Port Open - Error", errorMsg);
                return false;
            }

            if (settings[1].isEmpty())
            {
                QString errorMsg = "Parity value is empty!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("COM Port Open - Error", errorMsg);
                return false;
            }

            if (settings[1] == "N")
                portTemp.parity = 0;
            else if (settings[1] == "E")
                portTemp.parity = 2;
            else if (settings[1] == "O")
                portTemp.parity = 3;
            else if (settings[1] == "S")
                portTemp.parity = 4;
            else if (settings[1] == "M")
                portTemp.parity = 5;
            else
            {
                QString errorMsg = "Parity is not a correct char (N,E,O,S,M)!\nLine Number: " + QString::number(lineNumber) + "\nParity char: " + settings[1] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("COM Port Open - Error", errorMsg);
                return false;
            }

            if (settings[2].isEmpty())
            {
                QString errorMsg = "Data bits value is empty!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("COM Port Open - Error", errorMsg);
                return false;
            }

            portTemp.data = settings[2].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Data bits is not a number!\nLine Number: " + QString::number(lineNumber) + "\nData bits: " + settings[2] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("COM Port Open - Error", errorMsg);
                return false;
            }

            // Can be 5-8
            if (portTemp.data < 5 || portTemp.data > 8)
            {
                QString errorMsg = "Data bits is not in range (5-8)!\nLine Number: " + QString::number(lineNumber) + "\nData bits: " + settings[2] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("COM Port Open - Error", errorMsg);
                return false;
            }

            if (settings[3].isEmpty())
            {
                QString errorMsg = "Stop bits value is empty!";
                emit showErrorMessage("COM Port Open - Error", errorMsg);
                return false;
            }

            if (settings[3] == "1.5")
                settings[3] = "3";

            portTemp.stop = settings[3].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Stop bits is not a number!\nLine Number: " + QString::number(lineNumber) + "\nStop bits: " + settings[3] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("COM Port Open - Error", errorMsg);
                return false;
            }

            // Can be 1-3
            if (portTemp.stop == 0 || portTemp.stop > 3)
            {
                QString errorMsg = "Stop bits is not in range (1,1.5,2)!\nLine Number: " + QString::number(lineNumber) + "\nData bits: " + settings[3] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("COM Port Open - Error", errorMsg);
                return false;
            }

            // Good open or settings command
            return true;

        }
        // COM port close command
        else if (commandNotChk.startsWith(COMPORTCLOSE, Qt::CaseInsensitive))
        {
            // This will give 2 Strings = 1: cmc  2: Com Port #
            cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() < 2)
            {
                QString errorMsg = "Command requires 1 parameter (Port)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
                emit showErrorMessage("COM Port Close - Error", errorMsg);
                return false;
            }

            comPortNumber = cmd[1].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Port number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nPort: " + cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("COM Port Close - Error", errorMsg);
                return false;
            }

            if (!openComPortCheck.isEmpty())
            {
                if (!openComPortCheck.contains(comPortNumber))
                {
                    QString errorMsg = "Port number doesn't match any open port number(s)!\nLine Number: " + QString::number(lineNumber) + "\nPort: " + cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
                    emit showErrorMessage("COM Port Close - Error", errorMsg);
                    return false;
                }
            }

            // Good close command
            return true;
        }
        // COM port read command
        else if (commandNotChk.startsWith(COMPORTREAD, Qt::CaseInsensitive))
        {
            // This will give 4 strings = 1: cmr  2: Com Port #  3: Buffer # 4: length
            // cmd = commands[i].split(' ', Qt::SkipEmptyParts);

            QString errorMsg = "Reads are not implemented!\nPlease remove the read command!\nLine Number: " + QString::number(lineNumber) + "\nRead command: " + commandNotChk + "\nFile: " + gameName + ENDOFINIFILE;
            emit showErrorMessage("COM Port Read - Error", errorMsg);
        }
        // COM port write command
        else if (commandNotChk.startsWith(COMPORTWRITE, Qt::CaseInsensitive))
        {
            // This will give 3 Strings = 1: cmw  2: Com Port #  3: Data
            cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() < 3)
            {
                QString errorMsg = "Command requires 2 parameters (Port, Data)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
                emit showErrorMessage("COM Port Write - Error", errorMsg);
                return false;
            }

            comPortNumber = cmd[1].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Port number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nPort: " + cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("COM Port Write - Error", errorMsg);
                return false;
            }

            if (!openComPortCheck.isEmpty())
            {
                if (!openComPortCheck.contains(comPortNumber))
                {
                    QString errorMsg = "Port number doesn't match any open port number(s)!\nLine Number: " + QString::number(lineNumber) + "\nPort: " + cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
                    emit showErrorMessage("COM Port Write - Error", errorMsg);
                    return false;
                }
            }

            // Good write command;
            return true;
        }
    }
    // PacDrive commands, starts with "ul"
    else if (commandNotChk.startsWith(PACCMDSTART, Qt::CaseInsensitive) == true)
    {
        // PacDrive set pin state command
        if (commandNotChk.startsWith(PACSETSTATE, Qt::CaseInsensitive))
        {
            // This will give 4 strings = 1: uls 2: ID  3: Pin  4: State
            cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() < 4)
            {
                QString errorMsg = "Command requires 3 parameters (ID, Pin, State)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
                emit showErrorMessage("Ultimarc - Set LED State - Error", errorMsg);
                return false;
            }

            cmd[1].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Device number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nDevice: " + cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("Ultimarc - Set LED State - Error", errorMsg);
                return false;
            }

            cmd[2].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Pin number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nPin: " + cmd[2] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("Ultimarc - Set LED State - Error", errorMsg);
                return false;
            }

            // Good command
            return true;
        }
        // PacDrive set light intensity command
        else if (commandNotChk.startsWith(PACSETINTENSITY, Qt::CaseInsensitive))
        {
            // This will give 4 strings = 1: uli  2: ID  3: Pin  4: Intensity
            cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() < 4)
            {
                QString errorMsg = "Command requires 3 parameters (ID, Pin, Intensity)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
                emit showErrorMessage("Ultimarc - Set LED Intensity - Error", errorMsg);
                return false;
            }

            cmd[1].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Device number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nDevice: " + cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("Ultimarc - Set LED Intensity - Error", errorMsg);
                return false;
            }

            cmd[2].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Pin number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nPin: " + cmd[2] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("Ultimarc - Set LED Intensity - Error", errorMsg);
                return false;
            }

            cmd[3].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Intensity value is not a number!\nLine Number: " + QString::number(lineNumber) + "\nValue: " + cmd[3] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("Ultimarc - Set LED Intensity - Error", errorMsg);
                return false;
            }

            // Good command
            return true;
        }
        // PacDrive kill all LEDs command
        else if (commandNotChk.startsWith(PACKILLALLLEDS, Qt::CaseInsensitive))
        {
            // This will give 2 strings = 1: ulk  2: ID
            cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() < 2)
            {
                QString errorMsg = "Command requires 1 parameter (ID)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
                emit showErrorMessage("Ultimarc - Kill All LEDs - Error", errorMsg);
                return false;
            }

            cmd[1].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Device number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nDevice Number: " + cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("Ultimarc - Kill All LEDs - Error", errorMsg);
                return false;
            }

            // Good command
            return true;
        }
    }
    // Play WAV audio file command
    else if (commandNotChk.startsWith(PLAYWAVAUDIO, Qt::CaseInsensitive))
    {
        // This will give 2 strings = 1: ply  2: File
        cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

        if (cmd.size() < 2)
        {
            QString errorMsg = "Command requires 1 parameter (File)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
            emit showErrorMessage("Play WAV Audio File - Error", errorMsg);
            return false;
        }

        QString wavFile = cmd[1];
        QString wavFilePath = QApplication::applicationDirPath() + "/sounds/" + wavFile;

        if (!QFile::exists(wavFilePath))
        {
            QString errorMsg = wavFile + " not found!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;
            emit showErrorMessage("Play WAV Audio File - Error", errorMsg);
            return false;
        }

        // Good command
        return true;
    }
    // Null command
    else if (commandNotChk.startsWith(NULLCMD, Qt::CaseInsensitive))
    {
        // Do nothing, but return true
        return true;
    }
    else
    {
        return false;
    }
    // Something bad happened to get here
    return false;
}

// Process commands based on INI file
void OutputHookerCore::processINICommands(QString signalName, QString value, bool isState)
{
    quint8 i;
    quint8 comPortNumber;
    ComPortStruct portTemp;
    QStringList cmd, settings;
    QStringList commands;

    if (isState)
        commands = stateAndCommands[signalName];
    else
        commands = signalsAndCommands[signalName];

    for (i = 0; i < commands.size(); i++)
    {
        // First, check if there are more values with '|', if so, split
        if(commands[i].contains('|'))
        {
            commands[i].replace(" |","|");
            commands[i].replace("| ","|");

            cmd = commands[i].split('|', Qt::SkipEmptyParts);

            // Pick the command to use, based on the value, as 0 starts on the right and increases to the left. If value is blank, then use 0
            if (value != "")
            {
                quint16 dataNum = value.toUInt();
                // It is -1 since it starts at 0
                qint16 mvSize = cmd.size() - 1;

                // If value is larger than the number of commands, then pick the highest command
                if(dataNum > mvSize)
                    dataNum = mvSize;

                commands[i] = cmd[dataNum];
            }
            else
            {
                commands[i] = cmd[0];
            }
        }

        // Second, check if there is the %s% variable. If so, replace with value
        if (commands[i].contains(SIGNALDATAVARIABLE))
        {
            commands[i].replace(SIGNALDATAVARIABLE, value);
        }

        // COM port commands, starts with "cm" or "cs"
        if (commands[i].startsWith(PORTCMDSTART1, Qt::CaseInsensitive) == true || commands[i].startsWith(PORTCMDSTART2, Qt::CaseInsensitive) == true)
        {
            // COM port open command
            if (commands[i].startsWith(COMPORTOPEN, Qt::CaseInsensitive) || commands[i].startsWith(COMPORTSETTINGS, Qt::CaseInsensitive))
            {
                // This will give 3 strings = 1: cmo/css  2: Com Port #  3: Settings
                cmd = commands[i].split(' ', Qt::SkipEmptyParts);

                comPortNumber = cmd[1].toUInt();

                // Split the Settings into 4 strings = 1: Baud  2: Parity  3: Data  4: Stop
                settings = cmd[2].split('_', Qt::SkipEmptyParts);
                settings[0].remove(BAUDREMOVE);
                settings[1].remove(PARITYREMOVE);
                settings[2].remove(DATAREMOVE);
                settings[3].remove(STOPREMOVE);

                portTemp.baud = settings[0].toUInt();

                if (settings[1] == "N")
                    portTemp.parity = 0;
                else if (settings[1] == "E")
                    portTemp.parity = 2;
                else if (settings[1] == "O")
                    portTemp.parity = 3;
                else if (settings[1] == "S")
                    portTemp.parity = 4;
                else if (settings[1] == "M")
                    portTemp.parity = 5;
                else
                    portTemp.parity = 1;

                portTemp.data = settings[2].toUInt();

                if (settings[3] == "1.5")
                    settings[3] = "3";

                portTemp.stop = settings[3].toUInt();

                comPortMap.insert(comPortNumber,portTemp);
                comPortOpen(comPortNumber);
            }
            // COM port close command
            else if (commands[i].startsWith(COMPORTCLOSE, Qt::CaseInsensitive))
            {
                // This will give 2 strings = 1: cmc  2: Com Port #
                cmd = commands[i].split(' ', Qt::SkipEmptyParts);

                quint8 comPortNumber = cmd[1].toUInt();

                comPortClose(comPortNumber);
            }
            // COM port write command
            else if (commands[i].startsWith(COMPORTWRITE, Qt::CaseInsensitive))
            {
                // This will give 3 strings = 1: cmw  2: Com Port #  3: Data
                cmd = commands[i].split(' ', Qt::SkipEmptyParts);

                quint8 comPortNumber = cmd[1].toUInt();

                comPortWrite(comPortNumber, cmd[2]);
            }
        }
        // PacDrive commands, starts with "ul"
        else if (commands[i].startsWith(PACCMDSTART, Qt::CaseInsensitive) == true)
        {
            // PacDrive set pin state command
            if (commands[i].startsWith(PACSETSTATE, Qt::CaseInsensitive))
            {
                // This will give 4 strings = 1: uls 2: ID  3: Pin  4: State
                cmd = commands[i].split(' ', Qt::SkipEmptyParts);

                if (cmd.size() >= 4)
                {
                    quint8 pacID = cmd[1].toUInt() - 1;
                    quint8 pacPin = cmd[2].toUInt() - 1;
                    bool pacState = (cmd[3].toUInt() > 0);
                    setPacDrivePinState(pacID, pacPin, pacState);
                }
            }
            // PacDrive set light intensity command
            else if (commands[i].startsWith(PACSETINTENSITY, Qt::CaseInsensitive))
            {
                // This will give 4 strings = 1: uli  2: ID  3: Pin  4: Intensity
                cmd = commands[i].split(' ', Qt::SkipEmptyParts);

                if (cmd.size() >= 4)
                {
                    quint8 pacID = cmd[1].toUInt() - 1;
                    quint8 pacPin = cmd[2].toUInt() - 1;
                    quint8 pacIntensity = cmd[3].toUInt();
                    setPacDriveLightIntensity(pacID, pacPin, pacIntensity);
                }
            }
            // PacDrive kill all LEDs command
            else if (commands[i].startsWith(PACKILLALLLEDS, Qt::CaseInsensitive))
            {
                // This will give 2 strings = 1: ulk  2: ID
                cmd = commands[i].split(' ', Qt::SkipEmptyParts);

                if (cmd.size() >= 2)
                {
                    quint8 pacID = cmd[1].toUInt() - 1;
                    turnAllPacDriveLightsOff(pacID);
                }
            }
        }
        // Play WAV audio file command
        else if (commands[i].startsWith(PLAYWAVAUDIO, Qt::CaseInsensitive))
        {
            // This will give 2 strings = 1: ply  2: File
            cmd = commands[i].split(' ', Qt::SkipEmptyParts);

            if (cmd.size() >= 2)
            {
                QString file = cmd[1];
                playWavAudioFile(file);
            }
        }
        // Null command
        else if (commands[i].startsWith(NULLCMD, Qt::CaseInsensitive))
        {
            // Do nothing...
        }
    }
}

// Open COM port
void OutputHookerCore::comPortOpen(quint8 cpNum)
{
    QString cpName = BEGINCOMPORTNAME + QString::number(cpNum);
    QString cpPath = cpName;
    cpPath.prepend(COMPORTPATHFRONT);

    emit startComPort(cpNum, cpName, comPortMap[cpNum].baud, comPortMap[cpNum].data, comPortMap[cpNum].parity, comPortMap[cpNum].stop, 0, cpPath, true);
}

// Close COM port
void OutputHookerCore::comPortClose(quint8 cpNum)
{
    emit stopComPort(cpNum);
}

// Write to COM port
void OutputHookerCore::comPortWrite(quint8 cpNum, QString cpData)
{
    QByteArray cpBA = cpData.toUtf8();
    emit writeComPort(cpNum, cpBA);
}

// Set PacDrive pin state
void OutputHookerCore::setPacDrivePinState(quint8 pacID, quint8 pacPin, bool pacState)
{
    emit setPinState(pacID, pacPin, pacState);
}

// Set PacDrive light intensity
void OutputHookerCore::setPacDriveLightIntensity(quint8 pacID, quint8 pacPin, quint8 pacIntensity)
{
    emit setLightIntensity(pacID, pacPin, pacIntensity);
}

// Turn all PacDrive lights off based on INI file
void OutputHookerCore::turnAllPacDriveLightsOff(quint8 pacID)
{
    emit turnAllLightsOff(pacID);
}

// Play WAV audio file
void OutputHookerCore::playWavAudioFile(QString file)
{
    QSoundEffect *effect = new QSoundEffect(this);
    effect->setSource(QUrl::fromLocalFile(QCoreApplication::applicationDirPath() + "/sounds/" + file));
    effect->setVolume(1.0);

    connect(effect, &QSoundEffect::playingChanged, [effect]()
            {
                if (!effect->isPlaying())
                {
                    effect->deleteLater();
                }
            });

    effect->play();
}

// Clear things on a Window message system or TCP Socket disconnect
void OutputHookerCore::clearOnDisconnect()
{
    quint8 j;

    // If game files are still open, close them
    if (iniFileLoaded)
    {
        // If INI file loaded, must search for any new signal(s) not in the file
        // These signals are appended to the INI game file and closed
        quint8 foundCount = 0;
        bool foundNewSignal = false;
        QStringList nemSignalList;

        // Searching the 2 QMaps for any new signal(s)
        QMapIterator<QString, QString> x(signalsAndData);
        while (x.hasNext())
        {
            x.next();
            if (!signalsAndCommands.contains(x.key()) && !signalsNoCommands.contains(x.key()))
            {
                nemSignalList << x.key();
                foundCount++;
                foundNewSignal = true;
            }
        }

        // If any new signal(s) found, then append to the INI game file
        if (foundNewSignal)
        {
            // Play notification sound if setting is set
            if (useNewOutputsNotification)
            {
                playWavAudioFile("notification.wav");
            }

            // Open INI file to append
            QFile iniFileTemp(gameINIFilePath);
            if (!iniFileTemp.open(QIODevice::Append | QIODevice::Text))
            {
                emit showErrorMessage("Write Error", "Could not write to " + gameName + ENDOFINIFILE + "!");
                return;
            }
            QTextStream out(&iniFileTemp);

            for (j = 0; j < foundCount; j++){
                // If output signal is mame_stop or game_stop, then skip the entry
                if (nemSignalList[j] == MAMESTOP || nemSignalList[j] == GAMESTOP) {
                    continue;
                }
                out << nemSignalList[j] << "=\n";
            }
            iniFileTemp.close();
        }
    }

    isGameFound = false;
    gameHasRun = false;
    isEmptyGame = false;
    signalsAndCommands.clear();
    stateAndCommands.clear();
    signalsNoCommands.clear();
    statesNoCommands.clear();
    signalsAndData.clear();
    statesAndData.clear();
    iniFileLoaded = false;

    if (isCoreStarted) {
        emit startWinMsg();
        emit startTCPSocket();

        emit connectionStatus(OutputHookerCore::WinMsg, isWinMsgConnected);
        emit connectionStatus(OutputHookerCore::TCP, isTCPSocketConnected);
    }

    emit noConnectedGame();
}
