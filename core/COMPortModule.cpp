#include "COMPortModule.h"

COMPortModule::COMPortModule(QObject *parent)
    : QObject{parent}
{
    numPortOpen = 0;
    isPortOpen = false;
    bypassCOMPortConnectFailWarning = false;

    for (quint8 i = 0; i < MAXCOMPORTS; i++)
    {
        comPortOpen[i] = false;
        comPortArray[i] = nullptr;
        noLightGunWarning[i] = false;
    }

    comPortDCBList.resize(MAXCOMPORTS);
    comPortTOList.resize(MAXCOMPORTS);
    comPortLPCList.resize(MAXCOMPORTS);
}

COMPortModule::~COMPortModule()
{
    // Disconnect all interfaces
    disconnectAll();
}

bool COMPortModule::reconnectComPort(quint8 comPortNum)
{
    comPortArray[comPortNum] = CreateFile(comPortLPCList[comPortNum], GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (comPortArray[comPortNum] == INVALID_HANDLE_VALUE)
    {
        DWORD lastError = GetLastError();
        qDebug() << "Failed on COM Port Handle, error" << lastError;
        return false;
    }

    DCB dcbSerialParam = comPortDCBList[comPortNum];
    dcbSerialParam.DCBlength = sizeof(dcbSerialParam);

    // Set the params to the Serial COM Port and fail handling
    if (!SetCommState(comPortArray[comPortNum], &dcbSerialParam))
    {
        COMSTAT status;
        DWORD errors;
        DWORD lastError = GetLastError();

        ClearCommError(comPortArray[comPortNum], &errors, &status);

        QString errorMsg = "Could not set the CommState for the Serial COM Port: "+QString::number(comPortNum)+"!\nLast error: "+QString::number(lastError)+"\nErrors: "+QString::number(errors);
        emit showErrorMessage("Serial COM Port Error", errorMsg);
        return false;
    }

    // Set the Times Out for the Serial COM Port
    COMMTIMEOUTS timeout = comPortTOList[comPortNum];
    // Set the timeouts to the Serial COM Port and fail handling
    if (!SetCommTimeouts(comPortArray[comPortNum], &timeout))
    {
        COMSTAT status;
        DWORD errors;
        DWORD lastError = GetLastError();

        ClearCommError(comPortArray[comPortNum], &errors, &status);

        QString errorMsg = "Serial COM Port failed to set timeouts on COM Port: "+QString::number(comPortNum)+"!\nError: "+QString::number(errors)+"\nLast error: "+QString::number(lastError);
        emit showErrorMessage("Serial COM Port Error", errorMsg);
        return false;
    }

    comPortOpen[comPortNum] = true;
    numPortOpen++;
    isPortOpen = true;

    return true;
}

void COMPortModule::connectComPort(const quint8 &comPortNum, const QString &comPortName, const qint32 &comPortBaud, const quint8 &comPortData, const quint8 &comPortParity, const quint8 &comPortStop, const quint8 &comPortFlow, const QString &comPortPath, const bool &isWriteOnly)
{
    // Check if COM Port is open
    if (comPortOpen[comPortNum])
    {
        DCB dcb = {0};
        dcb.DCBlength = sizeof(DCB);

        if (!GetCommState(comPortArray[comPortNum], &dcb) || comPortArray[comPortNum] == INVALID_HANDLE_VALUE)
        {
            CloseHandle(comPortArray[comPortNum]);
            comPortOpen[comPortNum] = false;

            if (numPortOpen > 0)
                numPortOpen--;

            if (numPortOpen == 0)
                isPortOpen = false;

            reconnectComPort(comPortNum);
        }
    }
    // Check if it is already open, if so, do nothing
    else
    {
        // Create COM Port
        // Check if comPortNum matches the comPortName ending number
        if (!comPortName.endsWith (QString::number (comPortNum)))
        {
            QString errorMsg = "Serial COM Port name number: "+comPortName+" doesn't match COM Port number: "+QString::number(comPortNum);
            emit showErrorMessage("Serial COM Port Error", errorMsg);
            return;
        }

        std::wstring portName = comPortName.toStdWString ();
        LPCWSTR portNameLPC = portName.c_str ();

        if (isWriteOnly)
            comPortArray[comPortNum] = CreateFile(portNameLPC, GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        else
            comPortArray[comPortNum] = CreateFile(portNameLPC, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

        // Use old COM Port naming, for people who could not update their drivers
        if (comPortArray[comPortNum] == INVALID_HANDLE_VALUE)
        {
            portName = comPortPath.toStdWString ();
            portNameLPC = portName.c_str ();

            if (isWriteOnly)
                comPortArray[comPortNum] = CreateFile(portNameLPC, GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
            else
                comPortArray[comPortNum] = CreateFile(portNameLPC, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        }

        // If Serial COM Port connection failed
        if (comPortArray[comPortNum] == INVALID_HANDLE_VALUE)
        {
            quint16 lastError = GetLastError();

            if (!bypassCOMPortConnectFailWarning)
            {
                if ((lastError == ERROR_FILE_NOT_FOUND || lastError == ERROR_PATH_NOT_FOUND) && !noLightGunWarning[comPortNum])
                {
                    // Serial COM Port not found. Handle error here.
                    QString errorMsg = "Could not open the Serial COM Port: "+comPortName+" on Port: "+QString::number(comPortNum)+"!\nCOM port not found!\nError Number: "+QString::number(lastError);
                    emit showErrorMessage("Serial COM Port Error", errorMsg);
                    noLightGunWarning[comPortNum] = true;
                    return;
                }
                else if ((lastError == ERROR_FILE_NOT_FOUND || lastError == ERROR_PATH_NOT_FOUND) && noLightGunWarning[comPortNum])
                {
                    // Do nothing, as it already gave a warning. Lightgun may not be plugged in.
                    return;
                }
                else
                {
                    QString errorMsg = "Could not open the Serial COM Port: "+comPortName+" on Port: "+QString::number(comPortNum)+"!\nUnknown error!\nError Number: "+QString::number(lastError);
                    emit showErrorMessage("Serial COM Port Error", errorMsg);
                    return;
                }
            }
        }
        else
        {
            // Set COM Port params, if connection did not fail

            DCB dcbSerialParam = {0};
            dcbSerialParam.DCBlength = sizeof(dcbSerialParam);

            // Get Serial Port params and fail handling
            if (!GetCommState(comPortArray[comPortNum], &dcbSerialParam))
            {
                COMSTAT status;
                DWORD errors;

                ClearCommError(comPortArray[comPortNum], &errors, &status);

                QString errorMsg = "Could not get the CommState for the Serial COM Port: "+comPortName+" on Port: "+QString::number(comPortNum)+"!\nThis is the default setting for the Serial COM Port.";
                emit showErrorMessage("Serial COM Port Error", errorMsg);
                return;
            }

            // Setting params based on old Qt values

            // Baud rates
            // 115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200

            if (comPortBaud == 115200)
                dcbSerialParam.BaudRate = CBR_115200;
            else if (comPortBaud == 9600)
                dcbSerialParam.BaudRate = CBR_9600;
            else if (comPortBaud == 57600)
                dcbSerialParam.BaudRate = CBR_57600;
            else if (comPortBaud == 38400)
                dcbSerialParam.BaudRate = CBR_38400;
            else if (comPortBaud == 19200)
                dcbSerialParam.BaudRate = CBR_19200;
            else if (comPortBaud == 4800)
                dcbSerialParam.BaudRate = CBR_4800;
            else if (comPortBaud == 2400)
                dcbSerialParam.BaudRate = CBR_2400;
            else if (comPortBaud == 1200)
                dcbSerialParam.BaudRate = CBR_1200;
            else
                dcbSerialParam.BaudRate = CBR_115200;  // Default to 115200 as it is most common

            if (comPortData >= DATABITS_MIN && comPortData <= DATABITS_MAX)
                dcbSerialParam.ByteSize = comPortData;
            else
                dcbSerialParam.ByteSize = DATABITS_MAX; // Default to 8, as it is most common

            if (comPortStop == 1)
                dcbSerialParam.StopBits = ONESTOPBIT;
            else if (comPortStop == 2)
                dcbSerialParam.StopBits = TWOSTOPBITS;
            else if (comPortStop == 3)
                dcbSerialParam.StopBits = ONE5STOPBITS;
            else
                dcbSerialParam.StopBits = ONESTOPBIT;  // Default to 1, as it is most common

            if (comPortParity == 0)
                dcbSerialParam.Parity = NOPARITY;
            else if (comPortParity == 2)
                dcbSerialParam.Parity = EVENPARITY;
            else if (comPortParity == 3)
                dcbSerialParam.Parity = ODDPARITY;
            else if (comPortParity == 4)
                dcbSerialParam.Parity = SPACEPARITY;
            else if (comPortParity == 5)
                dcbSerialParam.Parity = MARKPARITY;
            else
                dcbSerialParam.Parity = NOPARITY;  // Default to no parity, as it is most common

            // Setup flowcontrol
            if (comPortFlow == 0)  // None - based on Qt numbering
            {
                dcbSerialParam.fOutxCtsFlow = false;
                dcbSerialParam.fRtsControl = RTS_CONTROL_DISABLE;
                dcbSerialParam.fOutX = false;
                dcbSerialParam.fInX = false;
            }
            else if (comPortFlow == 2)  // Software - based on Qt numbering
            {
                dcbSerialParam.fOutxCtsFlow = false;
                dcbSerialParam.fRtsControl = RTS_CONTROL_DISABLE;
                dcbSerialParam.fOutX = true;
                dcbSerialParam.fInX = true;
            }
            else if (comPortFlow == 1)  // Hardware - based on Qt numbering
            {
                dcbSerialParam.fOutxCtsFlow = true;
                dcbSerialParam.fRtsControl = RTS_CONTROL_HANDSHAKE;
                dcbSerialParam.fOutX = false;
                dcbSerialParam.fInX = false;
            }

            // Set the params to the Serial COM Port and fail handling
            if (!SetCommState(comPortArray[comPortNum], &dcbSerialParam))
            {
                COMSTAT status;
                DWORD errors;

                ClearCommError(comPortArray[comPortNum], &errors, &status);

                QString errorMsg = "Could not set the CommState for the Serial COM Port: "+comPortName+" on Port: "+QString::number(comPortNum)+"!\nThis is the default setting for the Serial COM Port.";
                emit showErrorMessage("Serial COM Port Error", errorMsg);
                return;
            }

            // Set the timeouts for the Serial COM Port
            COMMTIMEOUTS timeout = {0};
            // Safe
            // timeout.ReadIntervalTimeout = 60;
            // timeout.ReadTotalTimeoutConstant = 60;
            // timeout.ReadTotalTimeoutMultiplier = 15;
            // timeout.WriteTotalTimeoutConstant = 60;
            // timeout.WriteTotalTimeoutMultiplier = 8;

            // Works Good So Far
            // timeout.ReadIntervalTimeout = 15;
            // timeout.ReadTotalTimeoutConstant = 15;
            // timeout.ReadTotalTimeoutMultiplier = 5;
            // timeout.WriteTotalTimeoutConstant = 15;
            // timeout.WriteTotalTimeoutMultiplier = 2;

            // Max Out
            timeout.ReadIntervalTimeout = MAXDWORD;
            timeout.ReadTotalTimeoutConstant = 0;
            timeout.ReadTotalTimeoutMultiplier = 0;
            timeout.WriteTotalTimeoutConstant = 0;
            timeout.WriteTotalTimeoutMultiplier = 0;

            // Set the timeouts to the Serial COM Port and fail handling
            if (!SetCommTimeouts(comPortArray[comPortNum], &timeout))
            {
                COMSTAT status;
                DWORD errors;

                ClearCommError(comPortArray[comPortNum], &errors, &status);

                QString errorMsg = "Serial COM Port failed to set timeouts on COM Port: "+QString::number(comPortNum)+"!\nPlease check your Serial COM Port connections.\nError: "+QString::number(errors);
                emit showErrorMessage("Serial COM Port Error", errorMsg);
                return;
            }

            comPortDCBList[comPortNum] = dcbSerialParam;
            comPortTOList[comPortNum] = timeout;
            comPortLPCList[comPortNum] = portNameLPC;

            // COM Port is set, set connection bools

            comPortOpen[comPortNum] = true;
            numPortOpen++;
            isPortOpen = true;
        }
    }
}

void COMPortModule::disconnectComPort(const quint8 &comPortNum)
{
    // Check if COM Port is open
    if (comPortOpen[comPortNum] == true)
    {
        CloseHandle(comPortArray[comPortNum]);
        comPortOpen[comPortNum] = false;

        if (numPortOpen > 0)
            numPortOpen--;

        if (numPortOpen == 0)
            isPortOpen = false;
    }
}

void COMPortModule::writeData(const quint8 &comPortNum, const QByteArray &writeData)
{
    // Check if COM Port is open
    if (comPortOpen[comPortNum] == true)
    {
        quint32 size = writeData.size ();
        char* charArray = new char[size + 1];
        std::memcpy(charArray, writeData.constData (), size);
        charArray[size] = '\0';
        quint8 retry = 0;
        DWORD dwWrite = 0;
        bool writeOutput;
        COMSTAT status;
        DWORD errors;
        bool didReconnect = true;

        writeOutput = WriteFile(comPortArray[comPortNum], charArray, size, &dwWrite, NULL);

        // Check if Serial COM Port is still connected
        if (!writeOutput)
        {
            DWORD lastError = GetLastError();

            if (lastError == ERROR_GEN_FAILURE || lastError == ERROR_OPERATION_ABORTED || lastError == ERROR_NO_SUCH_DEVICE)
            {
                DCB dcb = {0};
                dcb.DCBlength = sizeof(DCB);

                if (!GetCommState(comPortArray[comPortNum], &dcb))
                {
                    CloseHandle(comPortArray[comPortNum]);
                    comPortOpen[comPortNum] = false;

                    if (numPortOpen > 0)
                        numPortOpen--;

                    if (numPortOpen == 0)
                        isPortOpen = false;

                    didReconnect = reconnectComPort(comPortNum);
                }
            }
        }

        // If write failed, then retry
        while (!writeOutput && retry != WRITERETRYATTEMPTS && didReconnect)
        {
            // Clean out error & set dwWrite back to 0
            ClearCommError(comPortArray[comPortNum], &errors, &status);
            dwWrite = 0;

            // Retry write again
            writeOutput = WriteFile(comPortArray[comPortNum], charArray, size, &dwWrite, NULL);

            retry++;
        }

        // Bypass errors if bypassSerialWriteChecks is true
        if (!bypassSerialWriteChecks)
        {
            // If write failed
            if (writeOutput == 0)
            {
                ClearCommError(comPortArray[comPortNum], &errors, &status);

                QString errorMsg = "Serial COM Port write failed on Port: "+QString::number(comPortNum)+"!\nPlease check your Serial COM Port connections.\nError: "+QString::number(errors);
                emit showErrorMessage("Serial COM Port Error", errorMsg);
                delete [] charArray;
                return;
            }

            // If size doesn't match Byte written
            if (size != dwWrite)
            {
                ClearCommError(comPortArray[comPortNum], &errors, &status);

                QString errorMsg = "Serial COM Port write failed on number of bytes written on Port: "+QString::number(comPortNum)+"!\nPlease check your Serial COM Port connections.\nError: "+QString::number(errors);
                emit showErrorMessage("Serial COM Port Error", errorMsg);
            }
        }
        delete [] charArray;
    }
}

void COMPortModule::disconnectAll()
{
    // Close Serial COM Port connections
    if (isPortOpen)
    {
        // Close all connections
        for (quint8 i = 0; i < MAXCOMPORTS; i++)
        {
            if (comPortOpen[i])
            {
                CloseHandle(comPortArray[i]);
                comPortOpen[i] = false;
            }
        }
        isPortOpen = false;
        numPortOpen = 0;
    }
}

void COMPortModule::setBypassSerialWriteChecks(const bool &bypassSPWC)
{
    bypassSerialWriteChecks = bypassSPWC;
}

void COMPortModule::setBypassCOMPortConnectFailWarning(const bool &bypassCPCFW)
{
    bypassCOMPortConnectFailWarning = bypassCPCFW;
}
