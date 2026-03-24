#ifndef GLOBAL_H
#define GLOBAL_H

#include <QObject>

// Version info
#define VERSION                 "1.0.0"
#define VERSIONMAIN             1
#define VERSIONMID              0
#define VERSIONLAST             0

// Settings
#define SETTINGSFILE            "OutputHooker.ini"

// INI game & default files
#define INIFILEDIR              "ini"
#define ENDOFINIFILE            ".ini"
#define DEFAULTFILE             "default"
#define JUSTMAME                "Mame"
#define SIGNALSTATE             "Output"

// Display
#define GAMEINFO                "Game Info:"
#define GAMEINFODASHES          "------------"
#define ROMEQUALS               "ROM="
#define GAMEFILE                "File="
#define MAMENOGAMEEMPTY         "___empty"
#define ORIENTATIONEQUAL0       "Orientation=0"
#define ORIENTATIONINDEX        4
#define PAUSEEQUALS0            "Pause=0"
#define PAUSEEQUALS             "Pause="
#define PAUSEINDEX              5
#define OUTPUTSIGNALS           "Outputs:"
#define OUTPUTSIGNALSDASHES     "----------"

// Process data
#define MAMESTART               "mame_start"
#define MAMESTOP                "mame_stop"
#define MAMEEMPTY               "___empty"
#define STATECHANGE             "statechange"
#define PAUSE                   "pause"
#define MAMEPAUSE               "MamePause"
#define ROTATE                  "rotate"
#define REFRESHTIME             "refreshtime"
#define ORIENTATION             "Orientation"
#define MAMEORIENTATION         "MameOrientation"
#define GAMESTART               "game"
#define GAMESTOP                "game_stop"

// Process commands
#define PORTCMDSTART1           "cm"
#define PORTCMDSTART2           "cs"
#define COMPORTOPEN             "cmo"
#define COMPORTCLOSE            "cmc"
#define COMPORTSETTINGS         "css"
#define COMPORTREAD             "cmr"
#define COMPORTWRITE            "cmw"
#define BAUDREMOVE               0,5
#define PARITYREMOVE             0,7
#define DATAREMOVE               0,5
#define STOPREMOVE               0,5
#define PACCMDSTART             "ul"
#define PACSETSTATE             "uls"
#define PACSETINTENSITY         "uli"
#define PACKILLALLLEDS          "ulk"
#define PLAYWAVAUDIO            "ply"
#define NULLCMD                 "nll"
#define SIGNALDATAVARIABLE      "%s%"

enum CommandType
{
    CmdNone = 0,
    CmdComOpen,
    CmdComClose,
    CmdComWrite,
    CmdComSet,
    CmdGun4irIGF,
    CmdGun4irSSM,
    CmdGun4irESM,
    CmdGun4irIM,
    CmdGun4irOR,
    CmdGun4irPM,
    CmdGun4irAR,
    CmdGun4irTS,
    CmdGun4irRL,
    CmdGun4irRO,
    CmdGun4irFA,
    CmdUltimarcState,
    CmdUltimarcIntensity,
    CmdUltimarcKill,
    CmdPlayWav,
    CmdNull
};

struct FunctionCommand
{
    CommandType type;
    QString commandCode;
    QString param1;
    QString param2;
    QString param3;
};

// Windows message system
#define WINMSGTIMERTIME         1000

// TCP Socket
#define TCPHOSTPORT             8000
#define TIMETOWAIT              3000
#define TCPTIMERTIME            3050

// Serial COM Port
#define MAXCOMPORTS             50
#define BEGINCOMPORTNAME        "COM"
#define COMPORTPATHFRONT        "\\\\.\\"
#define BAUD_NUMBER             8
#define WRITERETRYATTEMPTS      2
#define DATABITS_MAX            8
#define DATABITS_MIN            5

struct ComPortStruct
{
    qint32 baud;
    quint8 data;
    quint8 parity;
    quint8 stop;
};

inline qint32 BAUDDATA_ARRAY[BAUD_NUMBER] = {115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200};

// Ultimarc
// PacDrive, U-HID, and Blue-HID: 16 LED channels with No brightness (0 - Off and 1 - On)
// Nano-LED: 60 LED channels with 256 brightness levels (20 RGB LEDs)
// PacLED64: 64 LED channels with 256 Brightness levels (21 RGB LEDs)
// I-Pac Ultimate I/O: 96 LED channels with 256 brightness levels (32 RGB LEDs)

#define PACDRIVE                1
#define UHID                    2
#define BLUEHID                 3
#define NANOLED                 4
#define PACLED64                5
#define IPACULTIMATEIO          6

#define ULTIMARCTYPES           7
#define ULTIMARCMAXDEVICES      24

#define UHID_LOW                5377
#define NANOLED_LOW             5249
#define PACLED64_LOW            5121
#define IPACULTIMATEIO_LOW      1040

#define ULTIMATEGRPS            12
#define OTHERGRPS               8
#define SMALLGROUPS             2

#define PACDRIVENAME            "PacDrive"
#define UHIDNAME                "U-HID"
#define BLUEHIDNAME             "Blue-HID"
#define NANOLEDNAME             "Nano-LED"
#define PACLED64NAME            "PacLED64"
#define IPACULTIMATEIONAME      "I-Pac Ultimate I/O"

struct UltimarcData
{
    qint8 type;
    QString typeName;
    quint16 vendorID;
    QString vendorIDS;
    quint16 productID;
    QString productIDS;
    quint8 id;
    quint16 version;
    QString versionS;
    QString vendorName;
    QString productName;
    QString serialNumber;
    QString devicePath;
    quint8 deviceID;
    quint8 intensities[96];
    bool pins[96];
    bool valid = false;

    bool operator==(const UltimarcData& other) const
    {
        return (type == other.type) && (typeName == other.typeName) && (vendorID == other.vendorID) && (vendorIDS == other.vendorIDS) && (productID == other.productID)
        && (productIDS == other.productIDS) && (version == other.version) && (vendorName == other.vendorName) && (productName == other.productName)
            && (serialNumber == other.serialNumber)  && (devicePath == other.devicePath) && (versionS == other.versionS)  && (id == other.id) && (deviceID == other.deviceID);
    }
};

inline QString ULTIMARCTYPENAME[ULTIMARCTYPES] = {"Unknown", PACDRIVENAME, UHIDNAME, BLUEHIDNAME, NANOLEDNAME, PACLED64NAME, IPACULTIMATEIONAME};

inline quint8 ULTIMARCTYPELEDCOUNT[ULTIMARCTYPES] = {0, 16, 16, 16, 60, 64, 96};

#endif // GLOBAL_H
