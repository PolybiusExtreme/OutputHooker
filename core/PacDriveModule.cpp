#include "PacDriveModule.h"

PacDriveModule::PacDriveModule(QObject *parent)
    : QObject{parent}
{
    // Initialize Ultimarc controllers
    numberUltimarcDevices = PacInitialize();

    collectUltimarcData();
}

PacDriveModule::~PacDriveModule()
{
    // Shut down Ultimarc controller
    PacShutdown();
}

// Collect Ultimarc data
void PacDriveModule::collectUltimarcData()
{
    if (numberUltimarcDevices > 0)
    {
        // Variables
        quint8 i, j;
        quint8 index = 0;
        PWCHAR buffer = new wchar_t[256];
        numberUltimarcDevicesValid = 0;

        for (i = 0; i < numberUltimarcDevices; i++)
        {
            bool invalid = false;

            quint8 type = PacGetDeviceType(i);

            if (type >= PACDRIVE && type <= IPACULTIMATEIO)
            {
                // 2 IDs used, first is postition in list, second is what is programmed to
                // The position in the list ID is the important one
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

                // Get number of pins
                numberPins.insert (i,ULTIMARCTYPELEDCOUNT[dataUltimarc[i].type]);

                // Set pins state and intensity to 0, for a known state and intensity
                QList<bool> states;
                QList<quint8> intensity;

                for (j = 0; j < numberPins[i]; j++)
                {
                    states << true;
                    intensity << 0;
                }

                lightStateMap.insert(i,states);
                lightIntensityMap.insert(i,intensity);

                if (type == IPACULTIMATEIO)
                    numberGroups.insert(i, ULTIMATEGRPS);
                else if (type == NANOLED || type == PACLED64)
                    numberGroups.insert(i, OTHERGRPS);
                else
                    numberGroups.insert(i, SMALLGROUPS);

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

                index++;
            }
            else
                dataUltimarc[i].valid = false;
        }
        delete[] buffer;
    }
}

// Set pin state
void PacDriveModule::setPinState(quint8 id, quint8 pin, bool state)
{
    if (id < numberUltimarcDevices && dataUltimarc[id].valid)
    {
        if (dataUltimarc[id].pins[pin] != state)
        {
            dataUltimarc[id].pins[pin] = state;

            if (dataUltimarc[id].type >= NANOLED && dataUltimarc[id].type <= IPACULTIMATEIO)
            {
                BYTE intensity = state ? 255 : 0;

                Pac64SetLEDIntensity(id, pin, intensity);
            }
            else if (dataUltimarc[id].type >= PACDRIVE && dataUltimarc[id].type <= BLUEHID)
            {
                PacSetLEDState(id, pin, state);
            }
        }
    }
    else
    {
        QString errorMsg = "ID " + QString::number(id) + " is not in the valid Ultimarc controller list!";
        emit showErrorMessage("Invalid Ultimarc Controller ID!", errorMsg);
    }
}

// Set light intensity
void PacDriveModule::setLightIntensity(quint8 id, quint8 pin, quint8 intensity)
{
    if (id < numberUltimarcDevices && dataUltimarc[id].valid)
    {
        if (dataUltimarc[id].intensities[pin] != intensity)
        {
            dataUltimarc[id].intensities[pin] = intensity;

            if (dataUltimarc[id].type >= NANOLED && dataUltimarc[id].type <= IPACULTIMATEIO)
            {
                Pac64SetLEDIntensity(id, pin, (BYTE)intensity);

                dataUltimarc[id].pins[pin] = (intensity > 0);
            }
        }
    }
    else
    {
        QString errorMsg = "ID " + QString::number(id) + " is not in the valid Ultimarc controller list!";
        emit showErrorMessage("Invalid Ultimarc Controller ID!", errorMsg);
    }
}

// Turn all lights off
void PacDriveModule::turnAllLightsOff(quint8 id)
{
    if (dataUltimarc[id].valid)
    {
        quint8 i, pins;

        pins = numberPins[id];

        if (dataUltimarc[id].type >= NANOLED && dataUltimarc[id].type <= IPACULTIMATEIO)
        {
            PBYTE intensityData = new BYTE[pins];

            for (i = 0; i < pins; i++)
                intensityData[i] = 0;

            // Turn states on
            for (i = 0; i < numberGroups[id]; i++)
            {
                if (i == 7 && dataUltimarc[id].type == NANOLED)
                    Pac64SetLEDStates(id, i+1, 0x0F);
                else
                    Pac64SetLEDStates(id, i+1, 0xFF);
            }

            // Turn intensity for all pins off
            Pac64SetLEDIntensities(id,intensityData);

            delete[] intensityData;
        }
        else if (dataUltimarc[id].type >= PACDRIVE && dataUltimarc[id].type <= BLUEHID)
        {
            // Turn off the 16 states
            PacSetLEDStates(id, 0);

            groupStateData[id][0] = 0;
            groupStateData[id][1] = 0;
        }
    }
    else
    {
        QString errorMsg = "ID " + QString::number(id + 1) + " was not found in the valid Ultimarc controller list!";
        emit showErrorMessage("Invalid Ultimarc Controller ID!", errorMsg);
    }
}
