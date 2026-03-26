#include "PacDriveModule.h"

PacDriveModule::PacDriveModule(QObject *parent)
    : QObject{parent}
{
    // Initialize Ultimarc controllers
    numberUltimarcDevices = PacInitialize();

    // Collect Ultimarc data
    collectUltimarcData();

    // Turn the lights on all boards off
    turnLightsOnAllBoardsOff();
}

PacDriveModule::~PacDriveModule()
{
    // Turn the lights on all boards off
    turnLightsOnAllBoardsOff();

    // Shut down Ultimarc controllers
    PacShutdown();
}

// Collect Ultimarc data
void PacDriveModule::collectUltimarcData()
{
    if (numberUltimarcDevices > 0)
    {
        quint8 i, j;
        PWCHAR buffer = new wchar_t[256];
        numberUltimarcDevicesValid = 0;

        for (i = 0; i < numberUltimarcDevices; i++)
        {
            bool invalid = false;
            quint8 type = PacGetDeviceType(i);

            if (type >= PACDRIVE && type <= IPACULTIMATEIO)
            {
                dataUltimarc[i].id = i;

                dataUltimarc[i].type = type;

                dataUltimarc[i].typeName = ULTIMARCTYPENAME[dataUltimarc[i].type];

                dataUltimarc[i].vendorID = PacGetVendorId(i);
                dataUltimarc[i].productID = PacGetProductId(i);

                dataUltimarc[i].vendorIDS = QString("0x%1").arg( dataUltimarc[i].vendorID, 4, 16, QChar('0')).toUpper();
                dataUltimarc[i].productIDS = QString("0x%1").arg(dataUltimarc[i].productID, 4, 16, QChar('0')).toUpper();

                dataUltimarc[i].version = PacGetVersionNumber(i);
                dataUltimarc[i].versionS = QString::number(dataUltimarc[i].version);

                PacGetVendorName(i, buffer);
                dataUltimarc[i].vendorName = QString::fromWCharArray(buffer);

                if (dataUltimarc[i].vendorName.size() > 255)
                    invalid = true;

                PacGetProductName(i, buffer);
                dataUltimarc[i].productName = QString::fromWCharArray(buffer);

                if (dataUltimarc[i].productName.size() > 255)
                    invalid = true;

                PacGetSerialNumber(i, buffer);
                dataUltimarc[i].serialNumber = QString::fromWCharArray(buffer);

                if (dataUltimarc[i].serialNumber.size() > 255)
                    invalid = true;

                PacGetDevicePath(i, buffer);
                dataUltimarc[i].devicePath = QString::fromWCharArray(buffer);

                if (dataUltimarc[i].devicePath.size() > 255)
                    invalid = true;

                // Get Ultimarc controller ID
                if (dataUltimarc[i].type == PACDRIVE)
                    dataUltimarc[i].deviceID = dataUltimarc[i].version;
                else if (dataUltimarc[i].type == UHID)
                    dataUltimarc[i].deviceID = dataUltimarc[i].productID - UHID_LOW;
                else if (dataUltimarc[i].type == NANOLED)
                    dataUltimarc[i].deviceID = dataUltimarc[i].productID - NANOLED_LOW;
                else if (dataUltimarc[i].type == PACLED64)
                    dataUltimarc[i].deviceID = dataUltimarc[i].productID - PACLED64_LOW;
                else if (dataUltimarc[i].type == IPACULTIMATEIO)
                    dataUltimarc[i].deviceID = dataUltimarc[i].productID - IPACULTIMATEIO_LOW;
                else if (dataUltimarc[i].type == BLUEHID)
                    dataUltimarc[i].deviceID = 69;
                else
                    dataUltimarc[i].deviceID = 69;

                if (dataUltimarc[i].deviceID > 7)
                    invalid = true;

                numberPins.insert(i, ULTIMARCTYPELEDCOUNT[dataUltimarc[i].type]);
                int pins = numberPins[i];

                QList<bool> states;
                QList<quint8> intensity;

                for (j = 0; j < pins; j++)
                {
                    states << false;
                    intensity << 0;
                }

                lightStateMap.insert(i, states);
                lightIntensityMap.insert(i, intensity);

                if (type == IPACULTIMATEIO)
                {
                    numberGroups.insert(i, ULTIMATEGRPS);
                }
                else if (type == NANOLED || type == PACLED64)
                {
                    numberGroups.insert(i, OTHERGRPS);
                }
                else
                {
                    numberGroups.insert(i, SMALLGROUPS);
                }

                if (type == IPACULTIMATEIO || type == PACLED64 || type == NANOLED)
                {
                    Pac64SetLEDFadeTime(i, 0);

                    int numGroupsToInit = numberGroups[i];
                    for (int g = 1; g <= numGroupsToInit; g++)
                    {
                        Pac64SetLEDStates(i, g, 0xFF);
                    }

                    BYTE initialIntensities[96] = {0};
                    Pac64SetLEDIntensities(i, initialIntensities);
                }

                QList<quint8> stateData;

                for (j = 0; j < numberGroups[i]; j++)
                {
                    if (j == 7 && type == NANOLED)
                        stateData.insert(j, 0x0F);
                    else
                        stateData.insert(j, 0xFF);
                }

                groupStateData.insert(i, stateData);

                if (!invalid)
                {
                    dataUltimarc[i].valid = true;
                    numberUltimarcDevicesValid++;
                }
            }
        }
        delete[] buffer;
    }
}

// Set pin state
void PacDriveModule::setPinState(quint8 id, quint8 pin, bool state)
{
    if (id < numberUltimarcDevices && dataUltimarc[id].valid)
    {
        if (pin >= numberPins[id])
            return;

        if (dataUltimarc[id].pins[pin] != state)
        {
            bool writePass = false;

            if (dataUltimarc[id].type >= NANOLED && dataUltimarc[id].type <= IPACULTIMATEIO)
            {
                BYTE intensity = state ? 255 : 0;
                quint8 writeCount = 0;

                do
                {
                    writePass = Pac64SetLEDIntensity(id, pin, intensity);
                    writeCount++;
                }
                while (!writePass && writeCount < WRITERETRYATTEMPTS + 1);
            }
            else if (dataUltimarc[id].type >= PACDRIVE && dataUltimarc[id].type <= BLUEHID)
            {
                writePass = PacSetLEDState(id, pin, state);
            }

            if (writePass)
            {
                dataUltimarc[id].pins[pin] = state;
                lightIntensityMap[id][pin] = state ? 255 : 0;
            }
            else
            {
                QString errorMsg = "Write to Ultimarc Controller with ID " + QString::number(id + 1) + " failed after 3 write attempts!";
                emit showErrorMessage("Write to Ultimarc Controller failed!", errorMsg);
            }
        }
    }
    else
    {
        QString errorMsg = "ID " + QString::number(id + 1) + " is not in the valid Ultimarc controller list!";
        emit showErrorMessage("Invalid Ultimarc Controller ID!", errorMsg);
    }
}

// Set light intensity
void PacDriveModule::setLightIntensity(quint8 id, quint8 pin, quint8 intensity)
{
    if (id < numberUltimarcDevices && dataUltimarc[id].valid)
    {
        if (pin >= numberPins[id])
            return;

        if (lightIntensityMap[id][pin] != intensity)
        {
            bool writePass = false;
            quint8 writeCount = 0;

            do
            {
                writePass = Pac64SetLEDIntensity(id, pin, intensity);
                writeCount++;
            }
            while (!writePass && writeCount < WRITERETRYATTEMPTS + 1);

            if (writePass)
            {
                lightIntensityMap[id][pin] = intensity;
                dataUltimarc[id].pins[pin] = (intensity > 0);
            }
            else
            {
                QString errorMsg = "Write to Ultimarc Controller with ID " + QString::number(id + 1) + " failed after 3 write attempts!";
                emit showErrorMessage("Write to Ultimarc Controller failed!", errorMsg);
            }
        }
    }
    else
    {
        QString errorMsg = "ID " + QString::number(id + 1) + " is not in the valid Ultimarc controller list!";
        emit showErrorMessage("Invalid Ultimarc Controller ID!", errorMsg);
    }
}

// Turn all lights on one board off
void PacDriveModule::turnAllLightsOff(quint8 id)
{
    if (id < numberUltimarcDevices && dataUltimarc[id].valid)
    {
        int pins = numberPins[id];

        if (dataUltimarc[id].type >= NANOLED && dataUltimarc[id].type <= IPACULTIMATEIO)
        {
            BYTE intensityData[96] = {0};

            if (Pac64SetLEDIntensities(id, intensityData))
            {
                for (int i = 0; i < pins; i++)
                {
                    lightIntensityMap[id][i] = 0;
                    dataUltimarc[id].pins[i] = false;
                }
            }
        }
        else if (dataUltimarc[id].type >= PACDRIVE && dataUltimarc[id].type <= BLUEHID)
        {
            PacSetLEDStates(id, 0);

            for (int i = 0; i < pins; i++) {
                dataUltimarc[id].pins[i] = false;
            }
        }
    }
    else
    {
        QString errorMsg = "ID " + QString::number(id + 1) + " is not in the valid Ultimarc controller list!";
        emit showErrorMessage("Invalid Ultimarc Controller ID!", errorMsg);
    }
}

// Turn the lights on all boards off
void PacDriveModule::turnLightsOnAllBoardsOff()
{
    for (quint8 i = 0; i < numberUltimarcDevices; i++)
    {
        if (dataUltimarc[i].valid)
        {
            turnAllLightsOff(i);
        }
    }
}