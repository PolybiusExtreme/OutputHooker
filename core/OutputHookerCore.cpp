/*
 * Original Copyright (c) 2026 PolybiusExtreme
 * Portions Copyright (c) 2026 6Bolt
 *
 * Licensed under the GNU GPLv3.
 */

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

    //If USB HID is initialized
    isUSBHIDInit = false;

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
    connect(p_winMsg, &WinMsgModule::dataRead, this, &OutputHookerCore::processData);
    connect(p_winMsg, &WinMsgModule::winMsgConnectedSignal, this, &OutputHookerCore::winMsgConnected);
    connect(p_winMsg, &WinMsgModule::winMsgDisconnectedSignal, this, &OutputHookerCore::winMsgDisconnected);

    // When game or an empty game has started
    connect(p_winMsg, &WinMsgModule::gameHasStarted, this, &OutputHookerCore::gameStart);
    connect(p_winMsg, &WinMsgModule::emptyGameHasStarted, this, &OutputHookerCore::emptyGameStart);

    // When game has stopped
    connect(p_winMsg, &WinMsgModule::gameHasStopped, this, &OutputHookerCore::gameStopped);

    // TCP Socket connections

    // Connect the signals & slots for TCP Socket
    connect(this, &OutputHookerCore::startTCPSocket, p_tcpSocket, &TCPSocketModule::tcpConnect);
    connect(this, &OutputHookerCore::stopTCPSocket, p_tcpSocket, &TCPSocketModule::tcpDisconnect);
    connect(p_tcpSocket, &TCPSocketModule::dataRead, this, &OutputHookerCore::processData);
    connect(p_tcpSocket, &TCPSocketModule::tcpConnectedSignal, this, &OutputHookerCore::tcpConnected);
    connect(p_tcpSocket, &TCPSocketModule::tcpDisconnectedSignal, this, &OutputHookerCore::tcpDisconnected);

    // When game or an empty game has started
    connect(p_tcpSocket, &TCPSocketModule::gameHasStarted, this, &OutputHookerCore::gameStart);
    connect(p_tcpSocket, &TCPSocketModule::emptyGameHasStarted, this, &OutputHookerCore::emptyGameStart);

    // When game has stopped
    connect(p_tcpSocket, &TCPSocketModule::gameHasStopped, this, &OutputHookerCore::gameStopped);

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

    // Set up USB HID
    p_usbHID = new HidapiModule();

    if (useMultiThreading)
    {
        // Move USB HID to different thread
        p_usbHID->moveToThread(&threadForUSBHID);
        connect(&threadForUSBHID, &QThread::finished, p_usbHID, &QObject::deleteLater);
    }

    // USB HID connections

    //Connect the signals & slots for USB HID
    connect(this, &OutputHookerCore::startUSBHID, p_usbHID, &HidapiModule::connectHID);
    connect(this, &OutputHookerCore::stopUSBHID, p_usbHID, &HidapiModule::disconnectHID);
    connect(this, &OutputHookerCore::writeUSBHID, p_usbHID, &HidapiModule::writeDataHID);
    connect(this, &OutputHookerCore::stopAllConnections, p_usbHID, &HidapiModule::disconnectAll);
    connect(p_usbHID, &HidapiModule::showErrorMessage, this, &OutputHookerCore::errorMessage);

    if (useMultiThreading)
    {
        threadForUSBHID.start(QThread::HighPriority);
    }

    // Set up LedWiz
    p_ledWiz = new LedWizModule();

    if (useMultiThreading)
    {
        // Move LedWizModule to different thread
        p_ledWiz->moveToThread(&threadForLedWiz);
        connect(&threadForLedWiz, &QThread::finished, p_ledWiz, &QObject::deleteLater);
    }

    // LedWiz connections
    connect(this, &OutputHookerCore::setLwPinState, p_ledWiz, &LedWizModule::setPinState);
    connect(this, &OutputHookerCore::setLwPowerLevel, p_ledWiz, &LedWizModule::setPowerLevel);
    connect(this, &OutputHookerCore::setLwRGBColor, p_ledWiz, &LedWizModule::setRGBColor);
    connect(this, &OutputHookerCore::setLwPulseRate, p_ledWiz, &LedWizModule::setPulseRate);
    connect(this, &OutputHookerCore::turnAllLwLightsOff, p_ledWiz, &LedWizModule::turnAllLightsOff);
    connect(this, &OutputHookerCore::refreshLwDevices, p_ledWiz, &LedWizModule::refreshDevices);
    connect(p_ledWiz, &LedWizModule::ledWizDeviceList, this, &OutputHookerCore::ledWizDeviceList);
    connect(p_ledWiz, &LedWizModule::showErrorMessage, this, &OutputHookerCore::errorMessage);

    if (useMultiThreading)
    {
        // Start LedWiz thread
        threadForLedWiz.start(QThread::HighPriority);
    }

    // Set up Ultimarc PacDrive
    p_pacDrive = new PacDriveModule();

    if (useMultiThreading)
    {
        // Move PacDriveModule to different thread
        p_pacDrive->moveToThread(&threadForUltimarc);
        connect(&threadForUltimarc, &QThread::finished, p_pacDrive, &QObject::deleteLater);
    }

    // PacDrive connections

    // Connect the signals & slots for PacDrive
    connect(this, &OutputHookerCore::setPdPinState, p_pacDrive, &PacDriveModule::setPinState);
    connect(this, &OutputHookerCore::setPdLightIntensity, p_pacDrive, &PacDriveModule::setLightIntensity);
    connect(this, &OutputHookerCore::setPdLightFadeTime, p_pacDrive, &PacDriveModule::setLightFadeTime);
    connect(this, &OutputHookerCore::setPdRGBColor, p_pacDrive, &PacDriveModule::setRGBColor);
    connect(this, &OutputHookerCore::turnAllPdLightsOff, p_pacDrive, &PacDriveModule::turnAllLightsOff);
    connect(p_pacDrive, &PacDriveModule::ultimarcDeviceList, this, &OutputHookerCore::ultimarcDeviceList);
    connect(p_pacDrive, &PacDriveModule::showErrorMessage, this, &OutputHookerCore::errorMessage);

    if (useMultiThreading)
    {
        // Start PacDrive thread
        threadForUltimarc.start(QThread::HighPriority);
    }

    // Set up SDL3 controller commands
    p_sdlCtrl = new SdlCtrlModule();

    if (useMultiThreading)
    {
        // Move SdlCtrlModule to different thread
        p_sdlCtrl->moveToThread(&threadForSdlCtrl);
        connect(&threadForSdlCtrl, &QThread::finished, p_sdlCtrl, &QObject::deleteLater);
    }

    // SDL3 controller command connections

    // Connect the signals & slots for SDL3 controller commands
    connect(this, &OutputHookerCore::setRumble, p_sdlCtrl, &SdlCtrlModule::setRumble);
    connect(p_sdlCtrl, &SdlCtrlModule::sdlDeviceList, this, &OutputHookerCore::sdlDeviceList);
    connect(p_sdlCtrl, &SdlCtrlModule::showErrorMessage, this, &OutputHookerCore::errorMessage);

    if (useMultiThreading)
    {
        // Start SDL3 controller command thread
        threadForSdlCtrl.start(QThread::HighPriority);
    }

    // Set up network commands
    p_netCmd = new NetCmdModule();

    if (useMultiThreading)
    {
        // Move NetCmdModule to different thread
        p_netCmd->moveToThread(&threadForNetCmd);
        connect(&threadForNetCmd, &QThread::finished, p_netCmd, &QObject::deleteLater);
    }

    // Network command connections

    // Connect the signals & slots for network commands
    connect(this, &OutputHookerCore::connectTcpHost, p_netCmd, &NetCmdModule::connectTcpHost);
    connect(this, &OutputHookerCore::disconnectTcpHost, p_netCmd, &NetCmdModule::disconnectTcpHost);
    connect(this, &OutputHookerCore::sendTcpCommand, p_netCmd, &NetCmdModule::sendTcpCommand);
    connect(this, &OutputHookerCore::sendUdpCommand, p_netCmd, &NetCmdModule::sendUdpCommand);
    connect(this, &OutputHookerCore::sendHttpPostRequest, p_netCmd, &NetCmdModule::sendHttpPostRequest);
    connect(p_netCmd, &NetCmdModule::showErrorMessage, this, &OutputHookerCore::errorMessage);

    if (useMultiThreading)
    {
        // Start network command thread
        threadForNetCmd.start(QThread::HighPriority);
    }

    // KeyStates timer
    keyStatesRefreshTime = 33;
    keyStateTimer = new QTimer(this);
    connect(keyStateTimer, &QTimer::timeout, this, &OutputHookerCore::checkKeyStates);

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
        threadForUSBHID.quit();
        threadForLedWiz.quit();
        threadForUltimarc.quit();
        threadForSdlCtrl.quit();
        threadForNetCmd.quit();
        threadForTCPSocket.wait();
        threadForWinMsg.wait();
        threadForCOMPort.wait();
        threadForUSBHID.wait();
        threadForLedWiz.wait();
        threadForUltimarc.wait();
        threadForSdlCtrl.wait();
        threadForNetCmd.wait();
    }
    else
    {
        delete p_tcpSocket;
        delete p_winMsg;
        delete p_comPort;
        delete p_usbHID;
        delete p_ledWiz;
        delete p_pacDrive;
        delete p_sdlCtrl;
        delete p_netCmd;
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
    addNewOutputsToDefaultINI = p_config->getAddNewOutputsToDefaultINI();
    bypassSerialWriteChecks = p_config->getSerialPortWriteCheckBypass();
    // Don't get Multi-Threading, as it needs a application reset
    // useMultiThreading = p_config->getUseMultiThreading();

    comPortPlaceholders = p_config->getComPortPlaceholders();

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
    p_ledWiz->setWinID(handle);
}

// Execute command line commands
void OutputHookerCore::executeCommandLineCommands(const QStringList &commands, const QString &value)
{
    executeINICommands(commands, value);
}

// Execute command from TestOutputWindow
void OutputHookerCore::executeTestCommand(const FunctionCommand &cmd)
{
    // COM port commands, starts with "cm" or "cs"
    if (cmd.commandCode.startsWith(PORTCMDSTART1, Qt::CaseInsensitive) == true || cmd.commandCode.startsWith(PORTCMDSTART2, Qt::CaseInsensitive) == true)
    {
        // COM port open command
        if (cmd.commandCode.startsWith(COMPORTOPEN, Qt::CaseInsensitive) || cmd.commandCode.startsWith(COMPORTSETTINGS, Qt::CaseInsensitive))
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
        else if (cmd.commandCode.startsWith(COMPORTCLOSE, Qt::CaseInsensitive))
        {
            quint8 comPortNumber = cmd.param1.toUInt();
            comPortClose(comPortNumber);
        }
        // COM port write command
        else if (cmd.commandCode.startsWith(COMPORTWRITE, Qt::CaseInsensitive))
        {
            quint8 comPortNumber = cmd.param1.toUInt();
            comPortWrite(comPortNumber, cmd.param2);
        }
    }
    // USB HID write command, starts with "ghd"
    else if (cmd.commandCode.startsWith(USBHIDCMD, Qt::CaseInsensitive))
    {
        bool isNumber;
        quint8 deviceNumber = cmd.param1.toUInt(&isNumber);

        // Vendor ID handling
        QString vIDStr = cmd.param2;
        vIDStr.remove(0, 2); // Remove &H or &h
        quint16 vendorID = vIDStr.toUShort(&isNumber, 16);

        // Product ID handling
        QString pIDStr = cmd.param3;
        pIDStr.remove(0, 2); // Remove &H or &h
        quint16 productID = pIDStr.toUShort(&isNumber, 16);

        // Process bytes
        QStringList settings = cmd.param5.split(':', Qt::SkipEmptyParts);
        QString dataBytes;

        for (int j = 0; j < settings.count(); j++)
        {
            QString byteStr = settings[j];
            byteStr.remove(0, 2); // Remove &h

            if (byteStr.length() == 1)
                byteStr.prepend('0');

            dataBytes.append(byteStr);
        }

        // Generate HID key for QMap search
        QString hidKey = QString::number(vendorID, 16);
        hidKey.append(QString::number(productID, 16));
        hidKey.append(QString::number(deviceNumber, 10));

        if (hidPlayerMap.contains(hidKey))
        {
            quint8 player = hidPlayerMap[hidKey];
            QByteArray cpBA = QByteArray::fromHex(dataBytes.toUtf8());
            emit writeUSBHID(player, cpBA);
        }
        else
        {
            // If the device is not yet initialized, try to find it
            if (FindUSBHIDDevice(vendorID, productID, deviceNumber))
            {
                quint8 player = hidPlayerMap[hidKey];
                QByteArray cpBA = QByteArray::fromHex(dataBytes.toUtf8());
                emit writeUSBHID(player, cpBA);
            }
        }
    }
    // LedWiz commands, starts with "lw"
    else if (cmd.commandCode.startsWith(LWCMDSTART, Qt::CaseInsensitive) == true)
    {
        // LedWiz set pin state command
        if (cmd.commandCode.startsWith(LWSETSTATE, Qt::CaseInsensitive))
        {
            quint8 lwID = cmd.param1.toUInt() - 1;
            quint8 lwPin = cmd.param2.toUInt();
            bool lwState = (cmd.param3.toUInt() > 0);
            setLedWizPinState(lwID, lwPin, lwState);
        }
        // LedWiz set power level command
        else if (cmd.commandCode.startsWith(LWSETPOWER, Qt::CaseInsensitive))
        {
            quint8 lwID = cmd.param1.toUInt() - 1;
            quint8 lwPin = cmd.param2.toUInt();
            quint8 lwPower = cmd.param3.toUInt();
            setLedWizPowerLevel(lwID, lwPin, lwPower);
        }
        // LedWiz set RGB LED color command
        else if (cmd.commandCode.startsWith(LWSETCOLOR, Qt::CaseInsensitive))
        {
            quint8 lwID = cmd.param1.toUInt() - 1;
            quint8 lwPin = cmd.param2.toUInt();
            quint8 lwValueR = cmd.param3.toUInt();
            quint8 lwValueG = cmd.param4.toUInt();
            quint8 lwValueB = cmd.param5.toUInt();
            setLedWizRGBColor(lwID, lwPin, lwValueR, lwValueG, lwValueB);
        }
        // LedWiz set pulse rate command
        else if (cmd.commandCode.startsWith(LWSETPULSE, Qt::CaseInsensitive))
        {
            quint8 lwID = cmd.param1.toUInt() - 1;
            quint8 lwPulse = cmd.param2.toUInt();
            setLedWizPulseRate(lwID, lwPulse);
        }
        // LedWiz kill all LEDs command
        else if (cmd.commandCode.startsWith(LWKILLALLLEDS, Qt::CaseInsensitive))
        {
            quint8 lwID = cmd.param1.toUInt() - 1;
            turnAllLedWizLightsOff(lwID);
        }
    }
    // PacDrive commands, starts with "ul"
    else if (cmd.commandCode.startsWith(PACCMDSTART, Qt::CaseInsensitive) == true)
    {
        // PacDrive set pin state command
        if (cmd.commandCode.startsWith(PACSETSTATE, Qt::CaseInsensitive))
        {
            quint8 pacID = cmd.param1.toUInt() - 1;
            quint8 pacPin = cmd.param2.toUInt() - 1;
            bool pacState = (cmd.param3.toUInt() > 0);
            setPacDrivePinState(pacID, pacPin, pacState);
        }
        // PacDrive set light intensity command
        else if (cmd.commandCode.startsWith(PACSETINTENSITY, Qt::CaseInsensitive))
        {
            quint8 pacID = cmd.param1.toUInt() - 1;
            quint8 pacPin = cmd.param2.toUInt() - 1;
            quint8 pacIntensity = cmd.param3.toUInt();
            setPacDriveLightIntensity(pacID, pacPin, pacIntensity);
        }
        // PacDrive set light fade time command
        else if (cmd.commandCode.startsWith(PACSETFADETIME, Qt::CaseInsensitive))
        {
            quint8 pacID = cmd.param1.toUInt() - 1;
            quint8 pacFadetime = cmd.param2.toUInt();
            setPacDriveLightFadeTime(pacID, pacFadetime);
        }
        // PacDrive set RGB LED color command
        else if (cmd.commandCode.startsWith(PACSETCOLOR, Qt::CaseInsensitive))
        {
            quint8 pacID = cmd.param1.toUInt() - 1;
            quint8 pacPin = cmd.param2.toUInt() - 1;
            quint8 pacValueR = cmd.param3.toUInt();
            quint8 pacValueG = cmd.param4.toUInt();
            quint8 pacValueB = cmd.param5.toUInt();
            setPacDriveRGBColor(pacID, pacPin, pacValueR, pacValueG, pacValueB);
        }
        // PacDrive kill all LEDs command
        else if (cmd.commandCode.startsWith(PACKILLALLLEDS, Qt::CaseInsensitive))
        {
            quint8 pacID = cmd.param1.toUInt() - 1;
            turnAllPacDriveLightsOff(pacID);
        }
    }
    // SDL3 controller rumble commands, starts with "ff"
    else if (cmd.commandCode.startsWith(FFBCMDSTART, Qt::CaseInsensitive) == true)
    {
        // Force Feedback command
        if (cmd.commandCode.startsWith(SDL3FFB, Qt::CaseInsensitive))
        {
            quint8 sdlID = cmd.param1.toUInt();
            bool sdlState = (cmd.param2.toUInt() > 0);
            uint16_t sdlLeftStrenth = 65535;
            uint16_t sdlRightStrenth = 65535;
            uint32_t sdlDuration = 30000;
            setCtrlRumble(sdlID, sdlState, sdlLeftStrenth, sdlRightStrenth, sdlDuration);
        }
        // Force Feedback Advanced command
        else if (cmd.commandCode.startsWith(SDL3FFA, Qt::CaseInsensitive))
        {
            quint8 sdlID = cmd.param1.toUInt();
            bool sdlState = (cmd.param2.toUInt() > 0);
            uint16_t sdlLeftStrenth = cmd.param3.toUInt();
            uint16_t sdlRightStrenth = cmd.param4.toUInt();
            uint32_t sdlDuration = cmd.param5.toUInt();
            setCtrlRumble(sdlID, sdlState, sdlLeftStrenth, sdlRightStrenth, sdlDuration);
        }
    }
    // TCP commands, starts with "ts"
    else if (cmd.commandCode.startsWith(TCPCMDSTART, Qt::CaseInsensitive) == true)
    {
        // TCP connect command
        if (cmd.commandCode.startsWith(TCPSOCKETCONNECT, Qt::CaseInsensitive))
        {
            quint8 socket = cmd.param1.toUInt();
            QString host = cmd.param2;
            quint16 port = cmd.param3.toUInt();
            tcpConnect(socket, host, port);
        }
        // TCP disconnect command
        else if (cmd.commandCode.startsWith(TCPSOCKETDISCONNECT, Qt::CaseInsensitive))
        {
            quint8 socket = cmd.param1.toUInt();
            tcpDisconnect(socket);
        }
        // TCP send command
        else if (cmd.commandCode.startsWith(TCPSOCKETSEND, Qt::CaseInsensitive))
        {
            quint8 socket = cmd.param1.toUInt();
            QString tempCommand = cmd.param2 + "\n";
            QByteArray command = tempCommand.toUtf8();
            tcpSendCommand(socket, command);
        }
    }
    // UDP send command
    else if (cmd.commandCode.startsWith(UDPSOCKETSEND, Qt::CaseInsensitive))
    {
        quint8 type = cmd.param1.toUInt();
        QString address = cmd.param2;
        quint16 port = cmd.param3.toUInt();
        QString command = cmd.param4;
        udpSendCommand(type, address, port, command);
    }
    // HTTP POST request
    else if (cmd.commandCode.startsWith(HTTPPOSTREQUEST, Qt::CaseInsensitive))
    {
        QString url = cmd.param1;
        QString contentType = cmd.param2;
        QByteArray request = cmd.param3.toUtf8();
        httpPostRequest(url, contentType, request);
    }
    // Launch and Close Application commands, starts with "ap"
    else if (cmd.commandCode.startsWith(APPCMDSTART, Qt::CaseInsensitive) == true)
    {
        // Launch Application command
        if (cmd.commandCode.startsWith(APPLAUNCH, Qt::CaseInsensitive))
        {
            QString executable = cmd.param1;
            QString parameter = cmd.param2;
            quint8 mode = cmd.param3.toUInt();
            launchApplication(executable, parameter, mode);
        }
        // Close Application command
        else if (cmd.commandCode.startsWith(APPCLOSE, Qt::CaseInsensitive))
        {
            QString executable = cmd.param1;
            closeApplication(executable);
        }
    }
    // Play WAV audio file command
    else if (cmd.commandCode.startsWith(PLAYWAVAUDIO, Qt::CaseInsensitive))
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
        gameStopped();

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
        gameStopped();

    if (!p_tcpSocket->isConnected && !p_tcpSocket->isConnecting)
        emit startTCPSocket();
}

// Check KeyStates
void OutputHookerCore::checkKeyStates()
{
    QMapIterator<int, QStringList> i(keyStatesAndCommands);
    while (i.hasNext())
    {
        i.next();

        if (GetAsyncKeyState(i.key()) & 0x8000) {
            if (!lastKeyStates[i.key()])
            {
                // Execute commands. Pass "1" as the value for %s%
                executeINICommands(i.value(), "1");
                lastKeyStates[i.key()] = true;
            }
        }
        else
        {
            lastKeyStates[i.key()] = false;
        }
    }
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
            if (signalsAndData.contains(signal) == false)
            {
                signalsAndData.insert(signal, data);
                if (!isOutputHookerMinimized)
                {
                    emit addSignalFromGame(signal, data);
                }
            }
            else
            {
                signalsAndData[signal] = data;
                if (!isOutputHookerMinimized)
                {
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

    if (dynamicSender == p_winMsg)
    {
        emit stopTCPSocket();
    }
    else if (dynamicSender == p_tcpSocket)
    {
        emit stopWinMsg();
    }

    gameName = data;
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

    if (dynamicSender == p_winMsg)
    {
        emit stopTCPSocket();
    }
    else if (dynamicSender == p_tcpSocket)
    {
        emit stopWinMsg();
    }

    emit connectedEmptyGame();
    isEmptyGame = true;
    gameHasStopped = false;
}

// Game stopped process
void OutputHookerCore::gameStopped()
{
    if (iniFileLoaded)
    {
        // Run the commands attached to 'mame_stop'
        processINICommands(MAMESTOP, "", true);

        // If INI file loaded, search for any new signal(s) not in the file
        // These signals are appended to the INI file and closed
        QStringList gameNewSignals;
        QStringList defaultNewSignals;
        bool foundNewSignal = false;

        QString defaultINIFilePath = iniPath + "/" + DEFAULTFILE + ENDOFINIFILE;

        QString defaultINIFileContent;
        QFile defaultINIFile(defaultINIFilePath);

        if (defaultINIFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&defaultINIFile);
            defaultINIFileContent = in.readAll();
            defaultINIFile.close();
        }

        // Searching the QMap for any new signal(s)
        QMapIterator<QString, QString> x(signalsAndData);

        while (x.hasNext())
        {
            x.next();
            QString currentSignal = x.key();

            if (currentSignal == MAMESTOP || currentSignal == GAMESTOP)
            {
                continue;
            }

            // Check if signal is in game-specific INI file
            if (!signalsAndCommands.contains(currentSignal) && !signalsNoCommands.contains(currentSignal))
            {
                gameNewSignals << currentSignal;
                foundNewSignal = true;
            }

            if (addNewOutputsToDefaultINI)
            {
                // Check if signal is in default.ini
                if (!defaultINIFileContent.contains(currentSignal))
                {
                    defaultNewSignals << currentSignal;
                    foundNewSignal = true;
                }
            }
        }

        // If any new signal(s) found, then append to the INI file
        if (foundNewSignal)
        {
            // Play notification sound if setting is set
            if(useNewOutputsNotification)
            {
                playWavAudioFile("notification.wav");
            }

            // Write to the game-specific INI file (if new signals are available)
            if (!gameNewSignals.isEmpty())
            {
                QFile iniFileTemp(gameINIFilePath);

                if (iniFileTemp.open(QIODevice::Append | QIODevice::Text))
                {
                    QTextStream out(&iniFileTemp);

                    for (const QString &sig : std::as_const(gameNewSignals))
                    {
                        out << sig << "=\n";
                    }
                    iniFileTemp.close();
                }
                else
                {
                    emit showErrorMessage("Write Error", "Could not write to " + gameName + ENDOFINIFILE + "!");
                }
            }

            // Write to default.ini (if addNewOutputsToDefaultINI is active & signals are missing)
            if (addNewOutputsToDefaultINI && !defaultNewSignals.isEmpty())
            {
                QFile defaultIniFileTemp(defaultINIFilePath);

                if (defaultIniFileTemp.open(QIODevice::Append | QIODevice::Text))
                {
                    QTextStream out(&defaultIniFileTemp);

                    for (const QString &sig : std::as_const(defaultNewSignals))
                    {
                        out << sig << "=\n";
                    }
                    defaultIniFileTemp.close();
                }
                else
                {
                    emit showErrorMessage("Write Error", "Could not write to " + QString(DEFAULTFILE) + ENDOFINIFILE + "!");
                }
            }
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
    keyStatesAndCommands.clear();
    lastKeyStates.clear();
    iniFileLoaded = false;
    gameHasStopped = true;

    if (isCoreStarted)
    {
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
    // The game INI file path is used later, when new outputs are appended after the game
    // stops. It is set here explicitly so it no longer depends on the order the INI file
    // checks below happen to run in
    gameINIFilePath = iniPath + "/" + gameName + ENDOFINIFILE;

    // Checks if a default INI file exists
    isDefaultINI = isDefaultINIFile();

    // Check if a game INI file exists
    isGameINI = isINIFile();

    // If there is a default INI file and a game ini, then load it
    if (isDefaultINI || isGameINI)
    {
        // Load default.ini
        if (isDefaultINI)
        {
            QString defaultPath = iniPath + "/" + DEFAULTFILE + ENDOFINIFILE;
            loadINIFile(defaultPath);
        }

        // Load game ini
        if (isGameINI)
        {
            QString gamePath = iniPath + "/" + gameName + ENDOFINIFILE;
            loadINIFile(gamePath);
        }
        else
        {
            // Play notification sound if setting is set
            if (useNewOutputsNotification)
            {
                playWavAudioFile("notification.wav");
            }

            newINIFile();
        }

        emit connectedGame(gameName, iniName, true, false);
    }

}

// Check if an INI file exists for the game
bool OutputHookerCore::isINIFile()
{
    return QFile::exists(iniPath + "/" + gameName + ENDOFINIFILE);
}

// Check if an default INI file exists
bool OutputHookerCore::isDefaultINIFile()
{
    return QFile::exists(iniPath + "/" + DEFAULTFILE + ENDOFINIFILE);
}

QStringList splitCommands(const QString &commands)
{
    QStringList result;
    QString currentToken;

    int braceCount = 0;
    int bracketCount = 0;
    bool inQuotes = false;

    for (int i = 0; i < commands.length(); ++i)
    {
        QChar ch = commands.at(i);

        // Check for quotation marks (and ignore escaped quotation marks "\")
        if (ch == '"' && (i == 0 || commands.at(i - 1) != '\\'))
        {
            inQuotes = !inQuotes;
        }

        // Count parentheses when we are not in quotation marks
        if (!inQuotes)
        {
            if (ch == '{') braceCount++;
            else if (ch == '}') braceCount--;
            else if (ch == '[') bracketCount++;
            else if (ch == ']') bracketCount--;
        }

        // If a comma is found AND we are outside of any parentheses/quotes
        if (ch == ',' && braceCount == 0 && bracketCount == 0 && !inQuotes)
        {
            QString trimmed = currentToken.trimmed();
            if (!trimmed.isEmpty())
            {
                result.append(trimmed);
            }
            currentToken.clear();
        }
        else
        {
            currentToken.append(ch);
        }
    }

    // Add the last command after the last comma
    QString trimmed = currentToken.trimmed();
    if (!trimmed.isEmpty())
    {
        result.append(trimmed);
    }

    return result;
}

// Load INI file for the game
void OutputHookerCore::loadINIFile(const QString &filePath)
{
    QString targetFilePath = filePath.isEmpty() ? gameINIFilePath : filePath;

    QString line;
    QString key;
    QString commands;
    QStringList tempSplit;
    quint16 lineNumber = 0;
    int indexEqual;
    bool isOutput = false;
    bool isKeyStates = false;
    iniFileLoadFail = false;

    if (targetFilePath.contains(DEFAULTFILE, Qt::CaseInsensitive) || !iniFileLoaded)
    {
        openComPortCheck.clear();
        closeUSBHID();
        hidPlayerMap.clear();
    }

    // Open file. If failed to open, show Critical Message Box
    QFile iniFile(targetFilePath);
    // Set write permission to iniFile
    iniFile.setPermissions(iniFile.permissions() | QFile::WriteOwner);

    if (!iniFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit showErrorMessage("Error", "Could not open " + targetFilePath + "!");
        return;
    }

    // Create a TextStream, to stream easier in the data
    QTextStream in(&iniFile);

    while (!in.atEnd())
    {
        // Get a line of data from file
        line = in.readLine().trimmed();
        lineNumber++;

        if (line.isEmpty() || line.startsWith(";")) continue;

        // All lines have an '=', except for the one with '[' and ']'
        if (line.startsWith("["))
        {
            if (line.contains(SIGNALSTATE, Qt::CaseInsensitive))
            {
                isOutput = true;
                isKeyStates = false;
            }
            else if (line.contains(KEYSTATE, Qt::CaseInsensitive))
            {
                isKeyStates = true;
                isOutput = false;
            }
            else
            {
                isOutput = false;
                isKeyStates = false;
            }
            continue;
        }

        // Get the index of the equal
        indexEqual = line.indexOf('=');

        if (indexEqual != -1)
        {
            key = line.left(indexEqual).trimmed();
            commands = line.mid(indexEqual + 1).trimmed();

            if (!commands.isEmpty())
            {
                tempSplit = splitCommands(commands);

                // Process RefreshTime value immediately and do not check it in checkINICommands
                if (isKeyStates && key.compare("RefreshTime", Qt::CaseInsensitive) == 0)
                {
                    keyStatesRefreshTime = commands.toInt();
                    continue;
                }

                // Syntax check of the commands
                if (!checkINICommands(tempSplit, lineNumber, targetFilePath))
                {
                    iniFileLoadFail = true;
                    iniFile.close();
                    return;
                }

                if (isKeyStates)
                {
                    // Convert the key name to virtual key code
                    int vkCode = mapKeyNameToCode(key);
                    if (vkCode > 0)
                    {
                        keyStatesAndCommands.insert(vkCode, tempSplit);
                        lastKeyStates.insert(vkCode, false);
                    }
                }
                else if (isOutput)
                {
                    // Normal game outputs
                    signalsAndCommands.insert(key, tempSplit);
                    signalsNoCommands.removeOne(key);
                }
                else
                {
                    // General/States (mame_start, pause, etc.)
                    QString signal = key.toLower();

                    // Correction logic for general keys
                    if (signal.startsWith("on"))
                        signal.remove(0, 2);
                    if (signal.startsWith("mame") && !signal.contains("_"))
                        signal.insert(4, '_');

                    stateAndCommands.insert(signal, tempSplit);
                    statesNoCommands.removeOne(signal);
                }
            }
            else
            {
                // Mark entries without commands for automatic completion
                if (isOutput)
                {
                    if (!signalsAndCommands.contains(key) && !signalsNoCommands.contains(key))
                    {
                        signalsNoCommands << key;
                    }
                }
                else if (!isKeyStates)
                {
                    QString signal = key.toLower();

                    if (signal.startsWith("on"))
                        signal.remove(0, 2);
                    if (signal.startsWith("mame") && !signal.contains("_"))
                        signal.insert(4, '_');

                    if (!stateAndCommands.contains(signal) && !statesNoCommands.contains(signal))
                    {
                        statesNoCommands << signal;
                    }
                }
            }
        }
    }

    // Close file
    iniFile.close();
    iniFileLoaded = true;

    if (targetFilePath.contains(gameName + ENDOFINIFILE, Qt::CaseInsensitive) || !isGameINI)
    {
        // Timer logic for KeyStates
        if (!keyStatesAndCommands.isEmpty())
        {
            int interval = (keyStatesRefreshTime < 10) ? 100 : keyStatesRefreshTime;
            keyStateTimer->start(interval);
        }

        // Bypass the COM port connection fail warning pop-up, as MAMEHooker does
        emit setBypassComPortConnectFailWarning(true);

        // Process the "mame_start" signal
        processINICommands(MAMESTART, "", true);
    }
}

// Create a new INI File for the game
void OutputHookerCore::newINIFile()
{
    gameINIFilePath = iniPath + "/" + gameName + ENDOFINIFILE;

    // Copy template INI file over with game name as the file name
    bool copyFile = QFile::copy(":/ini/template.ini", gameINIFilePath);

    if (!copyFile)
    {
        emit showErrorMessage("Error", "Could not copy template INI file to ini path!");
        return;
    }

    loadINIFile(gameINIFilePath);
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

                // Replace COM Port placeholder with COM Port number
                QMapIterator<QString, QString> it(comPortPlaceholders);
                while (it.hasNext())
                {
                    it.next();
                    if (subCmd.contains(it.key()))
                    {
                        subCmd.replace(it.key(), it.value());
                    }
                }

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
            // Replace COM Port placeholder with COM Port number
            QMapIterator<QString, QString> it(comPortPlaceholders);
            while (it.hasNext())
            {
                it.next();
                if (command.contains(it.key()))
                {
                    command.replace(it.key(), it.value());
                }
            }

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
    // USB HID write command, starts with "ghd"
    else if (commandNotChk.startsWith(USBHIDCMD, Qt::CaseInsensitive))
    {
        // This will give 6 Strings = 1: ghd  2: Device#  3: Vendor ID  4: Product ID  5: Number of Bytes  6: Bytes
        cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

        if (cmd.count() != 6)
        {
            QString errorMsg = "Command requires 5 parameters (Device, Vendor ID, Product ID, Number of Bytes, Bytes)!\nLine Number: " + QString::number(lineNumber)+"\nCommand: " + commandNotChk + "\nFile: " + filePathName;
            emit showErrorMessage("USB HID Write - Error", errorMsg);
            return false;
        }

        bool isNumber;
        quint8 deviceNumber = cmd[1].toUInt(&isNumber);
        quint16 vendorID;
        quint16 productID;

        if (!isNumber)
        {
            QString errorMsg = "Device number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nDevice Number: " + cmd[1] + "\nFile: " + filePathName;
            emit showErrorMessage("USB HID Write - Error", errorMsg);
            return false;
        }

        if (cmd[2][0] == '&' && (cmd[2][1] == 'H' || cmd[2][1] == 'h'))
        {
            // Remove the '&H' or '&h' so that only the hex number is left
            cmd[2].remove(0,2);
            vendorID = cmd[2].toUShort(&isNumber, 16);

            if (!isNumber)
            {
                QString errorMsg = "Vendor ID number is not a hex number!\nLine Number: " + QString::number(lineNumber) + "\nVendor ID Number: " + cmd[2] + "\nFile: " + filePathName;
                emit showErrorMessage("USB HID Write - Error", errorMsg);
                return false;
            }
        }
        else
        {
            QString errorMsg = "Vendor ID doesn't have the needed &H in front of the number!\nLine Number: " + QString::number(lineNumber) + "\nVendor ID: " + cmd[2] + "\nFile: " + filePathName;
            emit showErrorMessage("USB HID Write - Error", errorMsg);
            return false;
        }

        if (cmd[3][0] == '&' && (cmd[3][1] == 'H' || cmd[3][1] == 'h'))
        {
            // Remove the '&H' or '&h' so that only the hex number is left
            cmd[3].remove(0,2);
            productID = cmd[3].toUShort(&isNumber, 16);

            if (!isNumber)
            {
                QString errorMsg = "Product ID number is not a hex number!\nLine Number: " + QString::number(lineNumber) + "\nProduct ID Number: " + cmd[3] + "\nFile: " + filePathName;
                emit showErrorMessage("USB HID Write - Error", errorMsg);
                return false;
            }
        }
        else
        {
            QString errorMsg = "Product ID doesn't have the needed &H in front of the number!\nLine Number: " + QString::number(lineNumber) + "\nProduct ID: " + cmd[3]  +"\nFile: " + filePathName;
            emit showErrorMessage("USB HID Write - Error", errorMsg);
            return false;
        }

        bool foundHID = FindUSBHIDDevice(vendorID, productID, deviceNumber);

        if (!foundHID)
        {
            QString errorMsg = "Could not find the HID with the Vendor ID, Product ID and device number!\nLine Number: " + QString::number(lineNumber) + "\nProductID: " + cmd[3] + "\nFile: " + filePathName;
            emit showErrorMessage("USB HID Write - Error", errorMsg);
            return false;
        }

        quint8 byteNumber = cmd[4].toUInt(&isNumber);

        if (!isNumber)
        {
            QString errorMsg = "Number of Bytes is not a number.\nLine Number: " + QString::number(lineNumber) + "\nNumber of Bytes: " + cmd[4] + "\nFile: " + filePathName;
            emit showErrorMessage("USB HID Write - Error", errorMsg);
            return false;
        }

        settings = cmd[5].split(':', Qt::SkipEmptyParts);

        if (settings.count() != byteNumber)
        {
            QString errorMsg = "Number of Bytes doesn't equal number of data bytes!\nLine Number: " + QString::number(lineNumber) + "\nNumber of Bytes: " + cmd[4] + "\nData Bytes: " + cmd[5] + "\nFile: " + filePathName;
            emit showErrorMessage("USB HID Write - Error", errorMsg);
            return false;
        }

        for (quint8 i = 0; i < settings.count(); i++)
        {
            if (settings[i][0] == '&' && settings[i][1] == 'h')
            {
                // Remove the '&h' so that only the hex number is left
                settings[i].remove(0,2);
                quint16 byteData = settings[i].toUShort(&isNumber, 16);

                if (!isNumber)
                {
                    QString errorMsg = "Data Byte is not a hex number!\nLine Number: " + QString::number(lineNumber) + "\nData Byte: " + settings[i] + "\nFile: " + filePathName;
                    emit showErrorMessage("USB HID Write - Error", errorMsg);
                    return false;
                }

                if (byteData > 255)
                {
                    QString errorMsg = "Data byte is out of range (00 - FF)!\nLine Number: " + QString::number(lineNumber) + "\nData Byte: " + settings[i] + "\nFile: " + filePathName;
                    emit showErrorMessage("USB HID Write - Error", errorMsg);
                    return false;
                }
            }
            else
            {
                QString errorMsg = "Data Byte doesn't have the needed &h in front of the number!\nLine Number: " + QString::number(lineNumber) + "\nData Byte: " + settings[i] + "\nFile: " + filePathName;
                emit showErrorMessage("USB HID Write - Error", errorMsg);
                return false;
            }
        }

        // Good command
        return true;
    }
    // LedWiz commands, starts with "lw"
    else if (commandNotChk.startsWith(LWCMDSTART, Qt::CaseInsensitive) == true)
    {
        // LedWiz set pin state command
        if (commandNotChk.startsWith(LWSETSTATE, Qt::CaseInsensitive))
        {
            // This will give 4 strings = 1: lws  2: ID  3: Pin  4: State
            cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() < 4)
            {
                QString errorMsg = "Command requires 3 parameters (ID, Pin, State)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
                emit showErrorMessage("LedWiz - Set Pin State - Error", errorMsg);
                return false;
            }

            cmd[1].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Device number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nDevice: " + cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("LedWiz - Set Pin State - Error", errorMsg);
                return false;
            }

            cmd[2].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Pin number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nPin: " + cmd[2] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("LedWiz - Set Pin State - Error", errorMsg);
                return false;
            }

            // Good command
            return true;
        }
        // LedWiz set power level command
        else if (commandNotChk.startsWith(LWSETPOWER, Qt::CaseInsensitive))
        {
            // This will give 4 strings = 1: lwp  2: ID  3: Pin  4: Power Level
            cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() < 4)
            {
                QString errorMsg = "Command requires 3 parameters (ID, Pin, Power Level)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
                emit showErrorMessage("LedWiz - Set Power Level - Error", errorMsg);
                return false;
            }

            cmd[1].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Device number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nDevice: " + cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("LedWiz - Set Power Level - Error", errorMsg);
                return false;
            }

            cmd[2].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Pin number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nPin: " + cmd[2] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("LedWiz - Set Power Level - Error", errorMsg);
                return false;
            }

            cmd[3].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Power level is not a number!\nLine Number: " + QString::number(lineNumber) + "\nPower Level: " + cmd[2] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("LedWiz - Set Power Level - Error", errorMsg);
                return false;
            }

            // Good command
            return true;
        }
        // LedWiz set RGB LED color command
        else if (commandNotChk.startsWith(LWSETCOLOR, Qt::CaseInsensitive))
        {
            // This will give 6 strings = 1: lwc  2: ID  3: Pin  4: Red Value  5: Green Value  6: Blue Value
            cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() < 6)
            {
                QString errorMsg = "Command requires 5 parameters (ID, Pin, Red Value, Green Value, Blue Value)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
                emit showErrorMessage("LedWiz - Set RGB LED Color - Error", errorMsg);
                return false;
            }

            cmd[1].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Device number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nDevice: " + cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("LedWiz - Set RGB LED Color - Error", errorMsg);
                return false;
            }

            cmd[2].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Pin number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nPin: " + cmd[2] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("LedWiz - Set RGB LED Color - Error", errorMsg);
                return false;
            }

            cmd[3].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Red value is not a number!\nLine Number: " + QString::number(lineNumber) + "\nRed Value: " + cmd[2] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("LedWiz - Set RGB LED Color - Error", errorMsg);
                return false;
            }

            cmd[4].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Green value is not a number!\nLine Number: " + QString::number(lineNumber) + "\nGreen Value: " + cmd[2] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("LedWiz - Set RGB LED Color - Error", errorMsg);
                return false;
            }

            cmd[5].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Blue value is not a number!\nLine Number: " + QString::number(lineNumber) + "\nBlue Value: " + cmd[2] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("LedWiz - Set RGB LED Color - Error", errorMsg);
                return false;
            }

            // Good command
            return true;
        }
        // LedWiz set pulse rate
        else if (commandNotChk.startsWith(LWSETPULSE, Qt::CaseInsensitive))
        {
            // This will give 3 strings = 1: lwr  2: ID  3: Pulse Rate
            cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() < 3)
            {
                QString errorMsg = "Command requires 2 parameters (ID, Pulse Rate)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
                emit showErrorMessage("LedWiz - Set Pulse Rate - Error", errorMsg);
                return false;
            }

            cmd[1].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Device number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nDevice: " + cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("LedWiz - Set Pulse Rate - Error", errorMsg);
                return false;
            }

            cmd[2].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Pulse rate is not a number!\nLine Number: " + QString::number(lineNumber) + "\nPulse Rate: " + cmd[2] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("LedWiz - Set Pulse Rate - Error", errorMsg);
                return false;
            }

            // Good command
            return true;
        }
        // LedWiz kill all LEDs command
        else if (commandNotChk.startsWith(LWKILLALLLEDS, Qt::CaseInsensitive))
        {
            // This will give 2 strings = 1: lwk  2: ID
            cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() < 2)
            {
                QString errorMsg = "Command requires 1 parameter (ID)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
                emit showErrorMessage("LedWiz - Kill All LEDs - Error", errorMsg);
                return false;
            }

            cmd[1].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Device number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nDevice Number: " + cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("LedWiz - Kill All LEDs - Error", errorMsg);
                return false;
            }

            // Good command
            return true;
        }
    }
    // PacDrive commands, starts with "ul"
    else if (commandNotChk.startsWith(PACCMDSTART, Qt::CaseInsensitive) == true)
    {
        // PacDrive set pin state command
        if (commandNotChk.startsWith(PACSETSTATE, Qt::CaseInsensitive))
        {
            // This will give 4 strings = 1: uls  2: ID  3: Pin  4: State
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
        // PacDrive set light fade time command
        else if (commandNotChk.startsWith(PACSETFADETIME, Qt::CaseInsensitive))
        {
            // This will give 3 strings = 1: ulf  2: ID  3: Fade Time
            cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() < 3)
            {
                QString errorMsg = "Command requires 2 parameters (ID, Fade Time)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
                emit showErrorMessage("Ultimarc - Set LED Fade Time - Error", errorMsg);
                return false;
            }

            cmd[1].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Device number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nDevice: " + cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("Ultimarc - Set LED Fade Time - Error", errorMsg);
                return false;
            }

            cmd[2].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Fade time value is not a number!\nLine Number: " + QString::number(lineNumber) + "\nValue: " + cmd[2] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("Ultimarc - Set LED Fade Time - Error", errorMsg);
                return false;
            }

            // Good command
            return true;
        }
        // PacDrive set RGB LED color command
        else if (commandNotChk.startsWith(PACSETCOLOR, Qt::CaseInsensitive))
        {
            // This will give 6 strings = 1: ulc  2: ID  3: Pin  4: Red Value  5: Green Value  6: Blue Value
            cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() < 6)
            {
                QString errorMsg = "Command requires 5 parameters (ID, Pin, Red Value, Green Value, Blue Value)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
                emit showErrorMessage("Ultimarc - Set RGB LED Color - Error", errorMsg);
                return false;
            }

            cmd[1].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Device number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nDevice: " + cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("Ultimarc - Set RGB LED Color - Error", errorMsg);
                return false;
            }

            cmd[2].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Pin number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nPin: " + cmd[2] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("Ultimarc - Set RGB LED Color - Error", errorMsg);
                return false;
            }

            cmd[3].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Red value is not a number!\nLine Number: " + QString::number(lineNumber) + "\nRed Value: " + cmd[2] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("Ultimarc - Set RGB LED Color - Error", errorMsg);
                return false;
            }

            cmd[4].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Green value is not a number!\nLine Number: " + QString::number(lineNumber) + "\nGreen Value: " + cmd[2] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("Ultimarc - Set RGB LED Color - Error", errorMsg);
                return false;
            }

            cmd[5].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Blue value is not a number!\nLine Number: " + QString::number(lineNumber) + "\nBlue Value: " + cmd[2] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("Ultimarc - Set RGB LED Color - Error", errorMsg);
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
    // SDL3 controller rumble commands, starts with "ff"
    else if (commandNotChk.startsWith(FFBCMDSTART, Qt::CaseInsensitive) == true)
    {
        // Force Feedback command
        if (commandNotChk.startsWith(SDL3FFB, Qt::CaseInsensitive))
        {
            // This will give 3 strings = 1: ffb  2: Device  3: State
            cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() < 3)
            {
                QString errorMsg = "Command requires 2 parameters (Device, State)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
                emit showErrorMessage("Force Feedback - Error", errorMsg);
                return false;
            }

            cmd[1].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Device number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nDevice: " + cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("Force Feedback - Error", errorMsg);
                return false;
            }

            // Good command
            return true;
        }
        // Force Feedback Advanced command
        else if (commandNotChk.startsWith(SDL3FFA, Qt::CaseInsensitive))
        {
            // This will give 6 strings = 1: ffa  2: Device  3: State  4: Left Strength Value  5: Right Strength Value  6: Duration Value
            cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() < 6)
            {
                QString errorMsg = "Command requires 5 parameters (Device, State, Left Strength Value, Right Strength Value, Duration Value)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
                emit showErrorMessage("Force Feedback Advanced - Error", errorMsg);
                return false;
            }

            cmd[1].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Device number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nDevice: " + cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("Force Feedback Advanced - Error", errorMsg);
                return false;
            }

            cmd[3].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Left strength value is not a number!\nLine Number: " + QString::number(lineNumber) + "\nLeft Strength Value: " + cmd[2] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("Force Feedback Advanced - Error", errorMsg);
                return false;
            }

            cmd[4].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Right strength value is not a number!\nLine Number: " + QString::number(lineNumber) + "\nRight Strength Value: " + cmd[2] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("Force Feedback Advanced - Error", errorMsg);
                return false;
            }

            cmd[5].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Duration value is not a number!\nLine Number: " + QString::number(lineNumber) + "\nDuration Value: " + cmd[2] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("Force Feedback Advanced - Error", errorMsg);
                return false;
            }

            // Good command
            return true;
        }
    }
    // TCP commands, starts with "ts"
    else if (commandNotChk.startsWith(TCPCMDSTART, Qt::CaseInsensitive) == true)
    {
        // TCP connect command
        if (commandNotChk.startsWith(TCPSOCKETCONNECT, Qt::CaseInsensitive))
        {
            // This will give 4 strings = 1: tsc  2: Socket  3: Address  4: Port
            cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() < 4)
            {
                QString errorMsg = "Command requires 3 parameters (Socket, Address, Port)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
                emit showErrorMessage("TCP - Connect - Error", errorMsg);
                return false;
            }

            cmd[1].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Socket number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nSocket: " + cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("TCP - Connect - Error", errorMsg);
                return false;
            }

            cmd[3].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Port number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nPort: " + cmd[3] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("TCP - Connect - Error", errorMsg);
                return false;
            }

            // Good command
            return true;
        }
        // TCP disconnect command
        else if (commandNotChk.startsWith(TCPSOCKETDISCONNECT, Qt::CaseInsensitive))
        {
            // This will give 2 strings = 1: tsd  2: Socket
            cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() < 2)
            {
                QString errorMsg = "Command requires 1 parameter (Socket)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
                emit showErrorMessage("TCP - Disconnect - Error", errorMsg);
                return false;
            }

            cmd[1].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Socket number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nSocket: " + cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("TCP - Disconnect - Error", errorMsg);
                return false;
            }

            // Good command
            return true;
        }
        // TCP send command
        else if (commandNotChk.startsWith(TCPSOCKETSEND, Qt::CaseInsensitive))
        {
            // This will give 3 strings = 1: tss  2: Socket  3: Command
            cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() < 3)
            {
                QString errorMsg = "Command requires 2 parameters (Socket, Command)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
                emit showErrorMessage("TCP - Send Command - Error", errorMsg);
                return false;
            }

            cmd[1].toUInt(&isNumber);

            if (!isNumber)
            {
                QString errorMsg = "Socket number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nSocket: " + cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("TCP - Send Command - Error", errorMsg);
                return false;
            }

            // Good command
            return true;
        }
    }
    // UDP send command
    else if (commandNotChk.startsWith(UDPSOCKETSEND, Qt::CaseInsensitive))
    {
        // This will give 5 strings = 1: udp  2: Type  3: Address  4: Port  5: Command
        cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

        if (cmd.size() < 5)
        {
            QString errorMsg = "Command requires 4 parameters (Type, Address, Port, Command)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
            emit showErrorMessage("UDP - Send Command - Error", errorMsg);
            return false;
        }

        cmd[1].toUInt(&isNumber);

        if (!isNumber)
        {
            QString errorMsg = "Type number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nType: " + cmd[1] + "\nFile: " + gameName + ENDOFINIFILE;
            emit showErrorMessage("UDP - Send Command - Error", errorMsg);
            return false;
        }

        cmd[3].toUInt(&isNumber);

        if (!isNumber)
        {
            QString errorMsg = "Port number is not a number!\nLine Number: " + QString::number(lineNumber) + "\nPort: " + cmd[3] + "\nFile: " + gameName + ENDOFINIFILE;
            emit showErrorMessage("UDP - Send Command - Error", errorMsg);
            return false;
        }

        // Good command
        return true;
    }
    // HTTP POST request
    else if (commandNotChk.startsWith(HTTPPOSTREQUEST, Qt::CaseInsensitive))
    {
        // This will give 4 strings = 1: hpr  2: URL  3: Content-Type  4: Request
        cmd = commandNotChk.split(' ', Qt::SkipEmptyParts);

        if (cmd.size() < 4)
        {
            QString errorMsg = "Command requires 3 parameters (URL, Content-Type, Request)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;;
            emit showErrorMessage("HTTP POST - Send Request - Error", errorMsg);
            return false;
        }

        // Good command
        return true;
    }
    // Launch and Close Application commands, starts with "ap"
    else if (commandNotChk.startsWith(APPCMDSTART, Qt::CaseInsensitive) == true)
    {
        QStringList args = QProcess::splitCommand(commandNotChk);

        // Launch Application command
        if (commandNotChk.startsWith(APPLAUNCH, Qt::CaseInsensitive))
        {
            if (args.size() < 2)
            {
                QString errorMsg = "Command requires at least 1 parameter (Path & Executable)!\nLine Number: " + QString::number(lineNumber) + "\nFile: " + gameName + ENDOFINIFILE;
                emit showErrorMessage("Launch Application - Error", errorMsg);
                return false;
            }

            QString executableFilePath = args[1];
            executableFilePath.remove('"');

            if (!QFile::exists(executableFilePath))
            {
                QString errorMsg = executableFilePath + " not found!\nLine Number: " + QString::number(lineNumber);
                emit showErrorMessage("Launch Application - Error", errorMsg);
                return false;
            }

            // Good command
            return true;
        }
        // Close Application command
        else if (commandNotChk.startsWith(APPCLOSE, Qt::CaseInsensitive))
        {
            if (args.size() < 2)
            {
                QString errorMsg = "Command requires 1 parameter (Executable)!\nLine Number: " + QString::number(lineNumber);
                emit showErrorMessage("Close Application - Error", errorMsg);
                return false;
            }

            //Good command
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

// Find USB HID device
bool OutputHookerCore::FindUSBHIDDevice(quint16 vendorID, quint16 productID, quint8 deviceNumber)
{
    HIDInfo foundHIDInfo;
    QString path;
    QStringList pathList;
    quint8 numberOfDevices = 0;
    QString hidKey;

    //USB HID initialize
    if (!isUSBHIDInit)
    {
        //Initialize the USB HID
        if (hid_init())
        {
            QString errorMsg = "Failed to initialize USB HID!";
            emit showErrorMessage("USB HID Initialisation - Error!", errorMsg);
            return false;
        }
        else
            isUSBHIDInit = true;
    }

    //Key used for the QMap
    hidKey = QString::number(vendorID, 16);
    hidKey.append (QString::number(productID, 16));
    hidKey.append (QString::number(deviceNumber, 10));

    //Check if it is in QMap already
    if (hidPlayerMap.contains(hidKey))
        return true;

    hid_device_info *devs;

    //Find USB HID devices with the same vendorID & productID
    devs = hid_enumerate(vendorID, productID);

    //Go through the USB HID devices
    //Multiple devices can have the same vendorID, productID and serial number
    for (; devs; devs = devs->next)
    {
        path = QString::fromLatin1(devs->path);

        if (numberOfDevices == 0)
        {
            pathList << path;
            numberOfDevices++;
        }
        else
        {
            if (!pathList.contains(path))
            {
                pathList << path;
                numberOfDevices++;
            }
        }

        if (numberOfDevices == deviceNumber)
        {

            foundHIDInfo.vendorID = devs->vendor_id;
            QString tempVID = QString::number(devs->vendor_id, 16).rightJustified(4, '0');
            tempVID = tempVID.toUpper();
            tempVID.prepend("0x");
            foundHIDInfo.vendorIDString = tempVID;

            foundHIDInfo.productID = devs->product_id;
            QString tempPID = QString::number(devs->product_id, 16).rightJustified(4, '0');
            tempPID = tempPID.toUpper();
            tempPID.prepend("0x");
            foundHIDInfo.productIDString = tempPID;

            foundHIDInfo.path = QString::fromLatin1(devs->path);
            QString tempDP = foundHIDInfo.path;
            tempDP.remove(0, FRONTPATHREM);
            foundHIDInfo.displayPath = tempDP;

            foundHIDInfo.serialNumber = QString::fromWCharArray(devs->serial_number);
            foundHIDInfo.releaseNumber = devs->release_number;
            QString tempR = QString::number(devs->release_number, 16).rightJustified(4, '0');
            tempR = tempR.toUpper();
            tempR.prepend("0x");
            foundHIDInfo.releaseString = tempR;

            foundHIDInfo.manufacturer = QString::fromWCharArray(devs->manufacturer_string);
            foundHIDInfo.productDiscription = QString::fromWCharArray(devs->product_string);
            foundHIDInfo.interfaceNumber = devs->interface_number;
            foundHIDInfo.usagePage = devs->usage_page;
            foundHIDInfo.usage = devs->usage;
            foundHIDInfo.usageString = processHIDUsage(foundHIDInfo.usagePage, foundHIDInfo.usage);

            quint8 playerNum = hidPlayerMap.count();

            if(playerNum >= 4)
            {
                QString errorMsg = "Only 4 HIDs open at the same time are supported (Player 1-4)!\nVendorID: " + QString::number(vendorID, 16) + "\nProductID: " + QString::number(productID, 16) + "\nDevice Number: " + QString::number(deviceNumber);
                emit showErrorMessage("USB HID - Error!", errorMsg);
                return false;
            }

            hidPlayerMap.insert(hidKey, playerNum);

            //Connect the USB HID
            emit startUSBHID(playerNum, foundHIDInfo);

            //Release HID enumeration
            hid_free_enumeration(devs);

            return true;
        }
    }

    //Release HID enumeration
    hid_free_enumeration(devs);

    QString errorMsg = "Could not find the USB HID device!\nVendorID: " + QString::number(vendorID, 16) + "\nProductID: " + QString::number(productID, 16) + "\nDevice Number: " + QString::number(deviceNumber);
    emit showErrorMessage("USB HID - Error!", errorMsg);
    return false;
}

// Map key name to virtual key code
int OutputHookerCore::mapKeyNameToCode(const QString &name)
{
    QString n = name.toUpper().trimmed();

    // Mouse & System
    if (n == "LBUTTON" || n == "Left mouse button") return 0x01;
    if (n == "RBUTTON" || n == "Right mouse button") return 0x02;
    if (n == "CANCEL") return 0x03;
    if (n == "MBUTTON" || n == "Middle mouse button") return 0x04;

    // Edit keys
    if (n == "BACK") return 0x08;
    if (n == "TAB") return 0x09;
    if (n == "CLEAR") return 0x0C;
    if (n == "RETURN") return 0x0D;
    if (n == "PAUSE") return 0x13;
    if (n == "CAPSLOCK") return 0x14;
    if (n == "ESCAPE") return 0x1B;
    if (n == "SPACE") return 0x20;

    // Navigation
    if (n == "PAGE UP") return 0x21;
    if (n == "PAGE DOWN") return 0x22;
    if (n == "END") return 0x23;
    if (n == "HOME") return 0x24;
    if (n == "LEFT") return 0x25;
    if (n == "UP") return 0x26;
    if (n == "RIGHT") return 0x27;
    if (n == "DOWN") return 0x28;
    if (n == "SELECT") return 0x29;
    if (n == "PRINT SCREEN") return 0x2C;
    if (n == "EXECUTE") return 0x2D;
    if (n == "SNAPSHOT") return 0x2C;
    if (n == "INSERT") return 0x2D;
    if (n == "DELETE") return 0x2E;
    if (n == "HELP") return 0x2F;

    // Modifiers
    if (n == "LSHIFT") return 0xA0;
    if (n == "RSHIFT") return 0xA1;
    if (n == "LCONTROL") return 0xA2;
    if (n == "RCONTROL") return 0xA3;
    if (n == "LALT") return 0xA4;
    if (n == "RALT") return 0xA5;

    // Letters A-Z (ASCII values ​​correspond to VK codes)
    if (n.length() == 1 && n[0] >= 'A' && n[0] <= 'Z') {
        return (int)n[0].toLatin1();
    }

    // Numbers 0-9
    if (n.length() == 1 && n[0] >= '0' && n[0] <= '9') {
        return (int)n[0].toLatin1();
    }

    // Numpad
    if (n == "NUMPAD0") return 0x60;
    if (n == "NUMPAD1") return 0x61;
    if (n == "NUMPAD2") return 0x62;
    if (n == "NUMPAD3") return 0x63;
    if (n == "NUMPAD4") return 0x64;
    if (n == "NUMPAD5") return 0x65;
    if (n == "NUMPAD6") return 0x66;
    if (n == "NUMPAD7") return 0x67;
    if (n == "NUMPAD8") return 0x68;
    if (n == "NUMPAD9") return 0x69;
    if (n == "MULTIPLY") return 0x6A;
    if (n == "ADD") return 0x6B;
    if (n == "NUMPADENTER") return 0x0D;
    if (n == "SUBTRACT") return 0x6D;
    if (n == "DECIMAL") return 0x6E;
    if (n == "DIVIDE") return 0x6F;

    // Function keys
    if (n == "F1") return 0x70;
    if (n == "F2") return 0x71;
    if (n == "F3") return 0x72;
    if (n == "F4") return 0x73;
    if (n == "F5") return 0x74;
    if (n == "F6") return 0x75;
    if (n == "F7") return 0x76;
    if (n == "F8") return 0x77;
    if (n == "F9") return 0x78;
    if (n == "F10") return 0x79;
    if (n == "F11") return 0x7A;
    if (n == "F12") return 0x7B;

    // Special characters (OEM codes)
    if (n == "NUMLOCK") return 0x90;
    if (n == "SCROLLLOCK") return 0x91;
    if (n == "`") return 0xC0;
    if (n == "MINUS") return 0xBD;
    if (n == "EQUALS") return 0xBB;
    if (n == "LBRACKET") return 0xDB;
    if (n == "RBRACKET") return 0xDD;
    if (n == "BACKSLASH") return 0xDC;
    if (n == "SEMICOLON") return 0xBA;
    if (n == "APOSTROPHE") return 0xDE;
    if (n == ",") return 0xBC;
    if (n == "PERIOD" || n == ".") return 0xBE;
    if (n == "SLASH" || n == "/") return 0xBF;

    return 0; // Not found
}

// Process commands based on INI file
void OutputHookerCore::processINICommands(QString signalName, QString value, bool isState)
{
    QStringList commands;
    QString searchName = signalName.trimmed();

    if (isState) {
        if (stateAndCommands.contains(searchName))
            commands = stateAndCommands[searchName];
    } else {
        if (signalsAndCommands.contains(searchName))
            commands = signalsAndCommands[searchName];
    }

    if (!commands.isEmpty()) {
        executeINICommands(commands, value);
    }
}

// Execute commands based on INI file
void OutputHookerCore::executeINICommands(const QStringList &commands, const QString &value)
{
    QStringList cmd, settings;
    quint8 comPortNumber;
    ComPortStruct portTemp;

    for (int i = 0; i < commands.size(); i++)
    {
        QString currentCommand = commands[i];

        // Replace COM Port placeholder with COM Port number
        QMapIterator<QString, QString> it(comPortPlaceholders);
        while (it.hasNext())
        {
            it.next();
            if (currentCommand.contains(it.key()))
            {
                currentCommand.replace(it.key(), it.value());
            }
        }

        // Branching logic with "|"
        if (currentCommand.contains('|'))
        {
            currentCommand.replace(" |", "|");
            currentCommand.replace("| ", "|");
            cmd = currentCommand.split('|', Qt::SkipEmptyParts);

            if (!value.isEmpty())
            {
                quint16 dataNum = value.toUInt();
                qint16 mvSize = cmd.size() - 1;
                if (dataNum > mvSize) dataNum = mvSize;
                currentCommand = cmd[dataNum];
            }
            else
            {
                currentCommand = cmd[0];
            }
        }

        // Replace placeholder %s%
        if (currentCommand.contains(SIGNALDATAVARIABLE) && !currentCommand.startsWith(USBHIDCMD))
        {
            currentCommand.replace(SIGNALDATAVARIABLE, value);
        }

        // COM port commands, starts with "cm" or "cs"
        if (currentCommand.startsWith(PORTCMDSTART1, Qt::CaseInsensitive) || currentCommand.startsWith(PORTCMDSTART2, Qt::CaseInsensitive))
        {
            // COM port open command
            if (currentCommand.startsWith(COMPORTOPEN, Qt::CaseInsensitive) || currentCommand.startsWith(COMPORTSETTINGS, Qt::CaseInsensitive))
            {
                // This will give 3 strings = 1: cmo/css  2: Com Port #  3: Settings
                cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

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
            else if (currentCommand.startsWith(COMPORTCLOSE, Qt::CaseInsensitive))
            {
                // This will give 2 strings = 1: cmc  2: Com Port #
                cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

                quint8 comPortNumber = cmd[1].toUInt();

                comPortClose(comPortNumber);
            }
            // COM port write command
            else if (currentCommand.startsWith(COMPORTWRITE, Qt::CaseInsensitive))
            {
                // This will give 3 strings = 1: cmw  2: Com Port #  3: Data
                cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

                quint8 comPortNumber = cmd[1].toUInt();

                comPortWrite(comPortNumber, cmd[2]);
            }
        }
        // USB HID write command, starts with "ghd"
        else if(currentCommand.startsWith(USBHIDCMD, Qt::CaseInsensitive))
        {
            // This will give 6 Strings = 1: ghd  2: Device#  3: Vendor ID  4: Product ID  5: Number of Bytes  6: Bytes
            cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

            QString hidKey;
            bool isNumber;
            quint8 deviceNumber = cmd[1].toUInt(&isNumber);

            // Remove the '&H' or '&h' so that only the hex number is left
            cmd[2].remove(0,2);
            quint16 vendorID = cmd[2].toUShort(&isNumber, 16);

            // Remove the '&H' or '&h' so that only the hex number is left
            cmd[3].remove(0,2);
            quint16 productID = cmd[3].toUShort(&isNumber, 16);

            quint8 valueMarkers = cmd[5].count(SIGNALDATAVARIABLE);
            quint16 valueNum = value.toUShort();
            QString upperDigit, lowerDigit;

            if (valueMarkers > 1)
            {
                if (valueNum > 9)
                {
                    upperDigit = value[0];
                    lowerDigit = value[1];
                }
                else
                {
                    upperDigit= '0';
                    lowerDigit = value[0];
                }
            }
            else if (valueMarkers == 1)
                lowerDigit = value[0];

            // Split up the Data Bytes
            settings = cmd[5].split(':', Qt::SkipEmptyParts);

            QString dataBytes;

            for (quint8 j = 0; j < settings.count(); j++)
            {
                // Remove the '&h' so that only the hex number is left
                settings[j].remove(0,2);

                if (settings[j].contains(SIGNALDATAVARIABLE))
                {
                    if (valueMarkers > 1)
                    {
                        settings[j].replace(SIGNALDATAVARIABLE, upperDigit);
                        valueMarkers--;
                    }
                    else if (valueMarkers == 1)
                        settings[j].replace(SIGNALDATAVARIABLE, lowerDigit);
                }

                if (settings[j].length() == 1)
                    settings[j].prepend('0');

                dataBytes.append(settings[j]);
            }

            // Find the USB HID device
            // Key used for the QMap
            hidKey = QString::number(vendorID, 16);
            hidKey.append (QString::number(productID, 16));
            hidKey.append (QString::number(deviceNumber, 10));

            quint8 player = hidPlayerMap[hidKey];

            // Convert string to hex QByteArray
            QByteArray cpBA = QByteArray::fromHex(dataBytes.toUtf8());

            // Send data to USB HID
            emit writeUSBHID(player, cpBA);
        }
        // LedWiz commands, starts with "lw"
        else if (currentCommand.startsWith(LWCMDSTART, Qt::CaseInsensitive) == true)
        {
            // LedWiz set pin state command
            if (currentCommand.startsWith(LWSETSTATE, Qt::CaseInsensitive))
            {
                // This will give 4 strings = 1: lws  2: ID  3: Pin  4: State
                cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

                if (cmd.size() >= 4)
                {
                    quint8 lwID = cmd[1].toUInt() - 1;
                    quint8 lwPin = cmd[2].toUInt();
                    bool lwState = (cmd[3].toUInt() > 0);
                    setLedWizPinState(lwID, lwPin, lwState);
                }
            }
            // LedWiz set power level command
            else if (currentCommand.startsWith(LWSETPOWER, Qt::CaseInsensitive))
            {
                // This will give 4 strings = 1: lwp  2: ID  3: Pin  4: Power Level
                cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

                if (cmd.size() >= 4)
                {
                    quint8 lwID = cmd[1].toUInt() - 1;
                    quint8 lwPin = cmd[2].toUInt();
                    quint8 lwPower = cmd[3].toUInt();
                    setLedWizPowerLevel(lwID, lwPin, lwPower);
                }
            }
            // LedWiz set RGB LED color command
            else if (currentCommand.startsWith(LWSETCOLOR, Qt::CaseInsensitive))
            {
                // This will give 6 strings = 1: lwc  2: ID  3: Pin  4: Red Value  5: Green Value  6: Blue Value
                cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

                if (cmd.size() >= 6)
                {
                    quint8 lwID = cmd[1].toUInt() - 1;
                    quint8 lwPin = cmd[2].toUInt();
                    quint8 lwValueR = cmd[3].toUInt();
                    quint8 lwValueG = cmd[4].toUInt();
                    quint8 lwValueB = cmd[5].toUInt();
                    setLedWizRGBColor(lwID, lwPin, lwValueR, lwValueG, lwValueB);
                }
            }
            // LedWiz set pulse rate command
            else if (currentCommand.startsWith(LWSETPULSE, Qt::CaseInsensitive))
            {
                // This will give 4 strings = 1: lwr  2: ID  3: Pin  4: Pulse Rate
                cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

                if (cmd.size() >= 4)
                {
                    quint8 lwID = cmd[1].toUInt() - 1;
                    quint8 lwPulse = cmd[2].toUInt();
                    setLedWizPulseRate(lwID, lwPulse);
                }
            }
            // LedWiz kill all LEDs command
            else if (currentCommand.startsWith(LWKILLALLLEDS, Qt::CaseInsensitive))
            {
                // This will give 2 strings = 1: lwk  2: ID
                cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

                if (cmd.size() >= 2)
                {
                    quint8 lwID = cmd[1].toUInt() - 1;
                    turnAllLedWizLightsOff(lwID);
                }
            }
        }
        // PacDrive commands, starts with "ul"
        else if (currentCommand.startsWith(PACCMDSTART, Qt::CaseInsensitive) == true)
        {
            // PacDrive set pin state command
            if (currentCommand.startsWith(PACSETSTATE, Qt::CaseInsensitive))
            {
                // This will give 4 strings = 1: uls  2: ID  3: Pin  4: State
                cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

                if (cmd.size() >= 4)
                {
                    quint8 pacID = cmd[1].toUInt() - 1;
                    quint8 pacPin = cmd[2].toUInt() - 1;
                    bool pacState = (cmd[3].toUInt() > 0);
                    setPacDrivePinState(pacID, pacPin, pacState);
                }
            }
            // PacDrive set light intensity command
            else if (currentCommand.startsWith(PACSETINTENSITY, Qt::CaseInsensitive))
            {
                // This will give 4 strings = 1: uli  2: ID  3: Pin  4: Intensity
                cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

                if (cmd.size() >= 4)
                {
                    quint8 pacID = cmd[1].toUInt() - 1;
                    quint8 pacPin = cmd[2].toUInt() - 1;
                    quint8 pacIntensity = cmd[3].toUInt();
                    setPacDriveLightIntensity(pacID, pacPin, pacIntensity);
                }
            }
            // PacDrive set light fade time command
            else if (currentCommand.startsWith(PACSETFADETIME, Qt::CaseInsensitive))
            {
                // This will give 3 strings = 1: ulf  2: ID  3: Fade Time
                cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

                if (cmd.size() >= 3)
                {
                    quint8 pacID = cmd[1].toUInt() - 1;
                    quint8 pacFadetime = cmd[2].toUInt();
                    setPacDriveLightFadeTime(pacID, pacFadetime);
                }
            }
            // PacDrive set RGB LED color command
            else if (currentCommand.startsWith(LWSETCOLOR, Qt::CaseInsensitive))
            {
                // This will give 6 strings = 1: ulc  2: ID  3: Pin  4: Red Value  5: Green Value  6: Blue Value
                cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

                if (cmd.size() >= 6)
                {
                    quint8 pacID = cmd[1].toUInt() - 1;
                    quint8 pacPin = cmd[2].toUInt() - 1;
                    quint8 pacValueR = cmd[3].toUInt();
                    quint8 pacValueG = cmd[4].toUInt();
                    quint8 pacValueB = cmd[5].toUInt();
                    setPacDriveRGBColor(pacID, pacPin, pacValueR, pacValueG, pacValueB);
                }
            }
            // PacDrive kill all LEDs command
            else if (currentCommand.startsWith(PACKILLALLLEDS, Qt::CaseInsensitive))
            {
                // This will give 2 strings = 1: ulk  2: ID
                cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

                if (cmd.size() >= 2)
                {
                    quint8 pacID = cmd[1].toUInt() - 1;
                    turnAllPacDriveLightsOff(pacID);
                }
            }
        }
        // SDL3 controller rumble commands, starts with "ff"
        else if (currentCommand.startsWith(FFBCMDSTART, Qt::CaseInsensitive) == true)
        {
            // Force Feedback command
            if (currentCommand.startsWith(SDL3FFB, Qt::CaseInsensitive))
            {
                // This will give 3 strings = 1: ffb  2: Device  3: State
                cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

                if (cmd.size() >= 3)
                {
                    quint8 sdlID = cmd[1].toUInt();
                    bool sdlState = (cmd[2].toUInt() > 0);
                    uint16_t sdlLeftStrenth = 65535;
                    uint16_t sdlRightStrenth = 65535;
                    uint32_t sdlDuration = 30000;
                    setCtrlRumble(sdlID, sdlState, sdlLeftStrenth, sdlRightStrenth, sdlDuration);
                }
            }
            // Force Feedback Advanced command
            else if (currentCommand.startsWith(SDL3FFA, Qt::CaseInsensitive))
            {
                // This will give 6 strings = 1: ffa  2: Device  3: State  4: Left Strength  5: Right Strength  6: Duration
                cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

                if (cmd.size() >= 6)
                {
                    quint8 sdlID = cmd[1].toUInt();
                    bool sdlState = (cmd[2].toUInt() > 0);
                    uint16_t sdlLeftStrenth = cmd[3].toUInt();
                    uint16_t sdlRightStrenth = cmd[4].toUInt();
                    uint32_t sdlDuration = cmd[5].toUInt();
                    setCtrlRumble(sdlID, sdlState, sdlLeftStrenth, sdlRightStrenth, sdlDuration);
                }
            }
        }
        // TCP commands, starts with "ts"
        else if (currentCommand.startsWith(TCPCMDSTART, Qt::CaseInsensitive) == true)
        {
            // TCP connect command
            if (currentCommand.startsWith(TCPSOCKETCONNECT, Qt::CaseInsensitive))
            {
                // This will give 4 strings = 1: tsc  2: Socket  3: Adress  4: Port
                cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

                if (cmd.size() >= 4)
                {
                    quint8 socket = cmd[1].toUInt();
                    QString host = cmd[2];
                    quint16 port = cmd[3].toUInt();
                    tcpConnect(socket, host, port);
                }
            }
            // TCP disconnect command
            else if (currentCommand.startsWith(TCPSOCKETDISCONNECT, Qt::CaseInsensitive))
            {
                // This will give 2 strings = 1: tsd  2: Socket
                cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

                if (cmd.size() >= 2)
                {
                    quint8 socket = cmd[1].toUInt();
                    tcpDisconnect(socket);
                }
            }
            // TCP send command
            else if (currentCommand.startsWith(TCPSOCKETSEND, Qt::CaseInsensitive))
            {
                // This will give 3 strings = 1: tss  2: Socket  3: Command
                cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

                if (cmd.size() >= 3)
                {
                    quint8 socket = cmd[1].toUInt();
                    QString tempCommand = currentCommand.section(' ', 2, -1) + "\n";
                    QByteArray command = tempCommand.toUtf8();
                    tcpSendCommand(socket, command);
                }
            }
        }
        // UDP send command
        else if (currentCommand.startsWith(UDPSOCKETSEND, Qt::CaseInsensitive))
        {
            // This will give 5 strings = 1: udp  2: Type  3: Address  4: Port  5: Command
            cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() >= 5)
            {
                quint8 type = cmd[1].toUInt();
                QString address = cmd[2];
                quint16 port = cmd[3].toUInt();
                QString command = currentCommand.section(' ', 4, -1);
                udpSendCommand(type, address, port, command);
            }
        }
        // HTTP POST request
        else if (currentCommand.startsWith(HTTPPOSTREQUEST, Qt::CaseInsensitive))
        {
            // This will give 4 strings = 1: hpr  2: URL  3: Content-Type  4: Request
            cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() >= 4)
            {
                QString url = cmd[1];
                QString currentType = cmd[2];
                QByteArray request = cmd[3].toUtf8();
                httpPostRequest(url, currentType, request);
            }
        }
        // Launch and Close Application commands, starts with "ap"
        else if (currentCommand.startsWith(APPCMDSTART, Qt::CaseInsensitive) == true)
        {
            // Launch Application command
            if (currentCommand.startsWith(APPLAUNCH, Qt::CaseInsensitive))
            {
                QStringList args = QProcess::splitCommand(currentCommand);

                if (args.size() >= 2)
                {
                    QString executable = args[1];
                    QString parameter = (args.size() >= 3) ? args[2] : "";
                    quint8 mode = (args.size() >= 4) ? args[3].toUInt() : 0;
                    launchApplication(executable, parameter, mode);
                }
            }
            // Close Application command
            else if (currentCommand.startsWith(APPCLOSE, Qt::CaseInsensitive))
            {
                // Find the position of the first space after "apc"
                int firstSpace = currentCommand.indexOf(' ');

                if (firstSpace != -1)
                {
                    // Extracts everything from the first space to the end of the line
                    QString executable = currentCommand.sliced(firstSpace).trimmed();
                    executable.remove('"');

                    if (!executable.isEmpty())
                    {
                        closeApplication(executable);
                    }
                }
            }
        }
        // Play WAV audio file command
        else if (currentCommand.startsWith(PLAYWAVAUDIO, Qt::CaseInsensitive))
        {
            // This will give 2 strings = 1: ply  2: File
            cmd = currentCommand.split(' ', Qt::SkipEmptyParts);

            if (cmd.size() >= 2)
            {
                QString file = cmd[1];
                playWavAudioFile(file);
            }
        }
        // Null command
        else if (currentCommand.startsWith(NULLCMD, Qt::CaseInsensitive))
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

// Close all USB HID connections
void OutputHookerCore::closeUSBHID()
{
    quint8 numberPlayers = hidPlayerMap.count();
    quint8 i;

    if (numberPlayers > 0)
    {
        for (i = 0; i < numberPlayers; i++)
            emit stopUSBHID(i);
    }
}

// Set LedWiz pin state
void OutputHookerCore::setLedWizPinState(quint8 lwID, quint8 lwPin, bool lwState)
{
    emit setLwPinState(lwID, lwPin, lwState);
}

// Set LedWiz power level
void OutputHookerCore::setLedWizPowerLevel(quint8 lwID, quint8 lwPin, quint8 lwPower)
{
    emit setLwPowerLevel(lwID, lwPin, lwPower);
}

// Set LedWiz RGB LED color
void OutputHookerCore::setLedWizRGBColor(quint8 lwID, quint8 lwPin, quint8 lwValueR, quint8 lwValueG, quint8 lwValueB)
{
    emit setLwRGBColor(lwID, lwPin, lwValueR, lwValueG, lwValueB);
}

// Set LedWiz pulse rate
void OutputHookerCore::setLedWizPulseRate(quint8 lwID, quint8 lwPulse)
{
    emit setLwPulseRate(lwID, lwPulse);
}

// Turn all LedWiz lights off
void OutputHookerCore::turnAllLedWizLightsOff(quint8 lwID)
{
    emit turnAllLwLightsOff(lwID);
}

// Set PacDrive pin state
void OutputHookerCore::setPacDrivePinState(quint8 pacID, quint8 pacPin, bool pacState)
{
    emit setPdPinState(pacID, pacPin, pacState);
}

// Set PacDrive light intensity
void OutputHookerCore::setPacDriveLightIntensity(quint8 pacID, quint8 pacPin, quint8 pacIntensity)
{
    emit setPdLightIntensity(pacID, pacPin, pacIntensity);
}

// Set PacDrive light fade time
void OutputHookerCore::setPacDriveLightFadeTime(quint8 pacID, quint8 pacFadetime)
{
    emit setPdLightFadeTime(pacID, pacFadetime);
}

// Set PacDrive RGB LED color
void OutputHookerCore::setPacDriveRGBColor(quint8 pacID, quint8 pacPin, quint8 pacValueR, quint8 pacValueG, quint8 pacValueB)
{
    emit setPdRGBColor(pacID, pacPin, pacValueR, pacValueG, pacValueB);
}

// Turn all PacDrive lights off
void OutputHookerCore::turnAllPacDriveLightsOff(quint8 pacID)
{
    emit turnAllPdLightsOff(pacID);
}

// Set SDL3 gamecontroller rumble
void OutputHookerCore::setCtrlRumble(quint8 id, bool state, uint16_t leftStrength, uint16_t rightStrength, uint32_t duration)
{
    emit setRumble(id, state, leftStrength, rightStrength, duration);
}

// TCP connect
void OutputHookerCore::tcpConnect(quint8 socket, QString host, quint16 port)
{
    emit connectTcpHost(socket, host, port);
}

// TCP disconnect
void OutputHookerCore::tcpDisconnect(quint8 socket)
{
    emit disconnectTcpHost(socket);
}

// Send TCP command
void OutputHookerCore::tcpSendCommand(quint8 socket, QByteArray command)
{
    emit sendTcpCommand(socket, command);
}

// Send UDP command
void OutputHookerCore::udpSendCommand(quint8 type, QString address, quint16 port, QString command)
{
    emit sendUdpCommand(type, address, port, command);
}

// Send HTTP POST request
void OutputHookerCore::httpPostRequest(QString url, QString contentType, QByteArray request)
{
    emit sendHttpPostRequest(url, contentType, request);
}

// Launch Application
void OutputHookerCore::launchApplication(QString executable, QString parameter, quint8 mode)
{
    QString cleanExecutable = executable.trimmed();
    if (cleanExecutable.startsWith('"') && cleanExecutable.endsWith('"'))
    {
        cleanExecutable = cleanExecutable.mid(1, cleanExecutable.length() - 2);
    }

    std::wstring executableStd = cleanExecutable.toStdWString();
    std::wstring parameterStd = parameter.toStdWString();

    int showCmd = SW_SHOWNORMAL;
    switch (mode)
    {
    case 1: showCmd = SW_HIDE;
        break;
    case 2: showCmd = SW_SHOWMINIMIZED;
        break;
    case 3: showCmd = SW_SHOWMAXIMIZED;
        break;
    default: showCmd = SW_SHOWNORMAL;
    }

    ShellExecuteW(NULL, L"open", executableStd.c_str(), parameterStd.c_str(), NULL, showCmd);
}

// Close Application
void OutputHookerCore::closeApplication(QString executable)
{
    QProcess closeApp;
    executable.remove('"');
    closeApp.start("taskkill", QStringList() << "/IM" << executable << "/F");
    closeApp.waitForFinished();
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

// Process usage & usagePage from USB HID data
QString OutputHookerCore::processHIDUsage(quint16 usagePage, quint16 usage)
{
    if (usagePage == 1)
    {
        if (usage == 0)
            return "Undefined";
        else if (usage == 1)
            return "Pointer";
        else if (usage == 2)
            return "Mouse";
        else if (usage == 3)
            return "Reserved";
        else if (usage == 4)
            return "Joystick";
        else if (usage == 5)
            return "GamePad";
        else if (usage == 6)
            return "Keyboard";
        else if (usage == 7)
            return "Keypad";
        else if (usage == 8)
            return "Multi-Axis Controller";
        else if (usage == 9)
            return "Tablet PC System Controls";
        else if (usage == 0x0A)
            return "Water Cooling Device";
        else if (usage == 0x0B)
            return "Computer Chassis Device";
        else if (usage == 0x0C)
            return "Wireless Radio Controls";
        else if (usage == 0x0D)
            return "Portable Device Control";
        else if (usage == 0x0E)
            return "System Multi-Axis Controller";
        else if (usage == 0x0F)
            return "Spatial Controller";
        else if (usage == 0x10)
            return "Assistive Control";
        else if (usage == 0x11)
            return "Device Dock";
        else if (usage == 0x12)
            return "Dockable Device";
        else if (usage == 0x13)
            return "Call State Management Control";
        else if (usage == 0x3A)
            return "Counted Buffer";
        else if (usage == 0x80)
            return "System Control";
        else if (usage == 0x96)
            return "Thumbstick";
        else if (usage == 0xC5)
            return "Chassis Enclosure";
        else
            return "";
    }
    else if (usagePage == 0x05)
    {
        if (usage == 0)
            return "Undefined";
        else if (usage == 1)
            return "3D Game Controller";
        else if (usage == 2)
            return "Pinball Device";
        else if (usage == 3)
            return "Gun Device";
        else if (usage == 20)
            return "Point of View";
        else if (usage == 32)
            return "Gun Selector";
        else
            return "";
    }
    else if (usagePage == 0x0C)
    {
        if (usage == 1)
            return "Consumer Control";
        else if (usage == 2)
            return "Numeric Key Pad";
        else if (usage == 3)
            return "Programmable Buttons";
        else if (usage == 4)
            return "Microphone";
        else if (usage == 5)
            return "Headphone";
        else if (usage == 6)
            return "Graphic Equalizer";
        else if (usage == 0x36)
            return "Function Buttons";
        else if (usage == 0x80)
            return "Selection";
        else
            return "";
    }
    else if (usagePage >= 0xFF00)
        return "Vendor Defined";

    return "";
}
