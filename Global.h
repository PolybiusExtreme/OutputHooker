/*
 * Original Copyright (c) 2026 PolybiusExtreme
 * Portions Copyright (c) 2026 6Bolt
 *
 * Licensed under the GNU GPLv3.
 */

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QObject>

#include <windows.h>

// Version info
#define VERSION                 "1.7.0"
#define VERSIONMAIN             1
#define VERSIONMID              7
#define VERSIONLAST             0

// Settings
#define SETTINGSFILE            "OutputHooker.ini"

// Autostart with Windows. The registry Run key is the only place this is stored, so
// removing the entry outside of OutputHooker is picked up the next time it starts
#define AUTOSTARTREGKEY         "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define AUTOSTARTVALUENAME      "OutputHooker"

// INI game & default files
#define INIFILEDIR              "ini"
#define ENDOFINIFILE            ".ini"
#define DEFAULTFILE             "default"
#define JUSTMAME                "Mame"
#define KEYSTATE                "KeyStates"
#define SIGNALSTATE             "Output"

// Display
#define GAMEINFO                "Game Info:"
#define GAMEINFODASHES          "-------------"
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
#define LWCMDSTART              "lw"
#define LWSETSTATE              "lws"
#define LWSETPOWER              "lwp"
#define LWSETCOLOR              "lwc"
#define LWSETPULSE              "lwr"
#define LWKILLALLLEDS           "lwk"
#define PACCMDSTART             "ul"
#define PACSETSTATE             "uls"
#define PACSETINTENSITY         "uli"
#define PACSETFADETIME          "ulf"
#define PACSETCOLOR             "ulc"
#define PACKILLALLLEDS          "ulk"
#define FFBCMDSTART             "ff"
#define SDL3FFB                 "ffb"
#define SDL3FFA                 "ffa"
#define USBHIDCMD               "ghd"
#define TCPCMDSTART             "ts"
#define TCPSOCKETCONNECT        "tsc"
#define TCPSOCKETDISCONNECT     "tsd"
#define TCPSOCKETSEND           "tss"
#define UDPSOCKETSEND           "udp"
#define HTTPPOSTREQUEST         "hpr"
#define APPCMDSTART             "ap"
#define APPLAUNCH               "apl"
#define APPCLOSE                "apc"
#define PLAYWAVAUDIO            "ply"
#define NULLCMD                 "nll"
#define SIGNALDATAVARIABLE      "%s%"

enum CategoryType
{
    CatAll,
    CatComPort,
    CatBlamcon,
    CatFusion,
    CatGun4ir,
    CatOpenFire,
    CatRsMX24,
    CatRsReaper,
    CatSinden,
    CatXgunner,
    CatAlien,
    CatLedWiz,
    CatUltimarc,
    CatSDL3FFB,
    CatUsbHid,
    CatNetwork,
    CatApplication,
    CatAudio,
    CatMisc
};

enum CommandType
{
    // COM Port
    CmdComOpen,
    CmdComClose,
    CmdComWrite,
    CmdComSet,
    // Blamcon Lightgun
    CmdBlamconCPO,
    CmdBlamconCPC,
    CmdBlamconSSM,
    CmdBlamconESM,
    CmdBlamconIM,
    CmdBlamconOR,
    CmdBlamconPM,
    CmdBlamconAR,
    CmdBlamconRM,
    CmdBlamconCP,
    CmdBlamconIGF,
    // Fusion Lightgun
    CmdFusionCPO,
    CmdFusionCPC,
    CmdFusionSSM,
    CmdFusionESM,
    CmdFusionFM,
    CmdFusionJM,
    CmdFusionPM,
    CmdFusionIGF,
    // GUN4IR Lightgun
    CmdGun4irCPO,
    CmdGun4irCPC,
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
    CmdGun4irIGF,
    // OpenFIRE Lightgun
    CmdOpenFireCPO,
    CmdOpenFireCPC,
    CmdOpenFireSSM,
    CmdOpenFireESM,
    CmdOpenFireIM,
    CmdOpenFireOR,
    CmdOpenFirePM,
    CmdOpenFireAR,
    CmdOpenFireRO,
    CmdOpenFireFA,
    CmdOpenFireDM,
    CmdOpenFirePO,
    CmdOpenFireIGF,
    // Retro Shooter MX24 Lightgun
    CmdRsMX24CPO,
    CmdRsMX24CPC,
    CmdRsMX24SSM,
    CmdRsMX24ESM,
    CmdRsMX24SCM,
    CmdRsMX24IGF,
    // Retro Shooter RS3 Reaper Lightgun
    CmdRsReaperCPO,
    CmdRsReaperCPC,
    CmdRsReaperSSM,
    CmdRsReaperESM,
    CmdRsReaperIM,
    CmdRsReaperOR,
    CmdRsReaperAR,
    CmdRsReaperLA,
    CmdRsReaperIGF,
    // Sinden Lightgun
    CmdSindenTSC,
    CmdSindenTSD,
    CmdSindenIGF,
    CmdSindenRM,
    CmdSindenAP,
    CmdSindenSS,
    // X-Gunner Lightgun
    CmdXgunnerCPO,
    CmdXgunnerCPC,
    CmdXgunnerSSM,
    CmdXgunnerESM,
    CmdXgunnerIM,
    CmdXgunnerAR,
    CmdXgunnerIGF,
    // Alien Positional Gun
    CmdAlienLED,
    CmdAlienRecoil,
    CmdAlienCounter,
    // LEDWiz
    CmdLedWizState,
    CmdLedWizPower,
    CmdLedWizColor,
    CmdLedWizPulse,
    CmdLedWizKill,
    // Ultimarc
    CmdUltimarcState,
    CmdUltimarcIntensity,
    CmdUltimarcFadeTime,
    CmdUltimarcColor,
    CmdUltimarcKill,
    // SDL3 Force Feedback
    CmdSDL3FFB,
    CmdSDL3FFA,
    // Generic HID
    CmdUsbHidSend,
    // Network (TCP/UDP)
    CmdTcpConnect,
    CmdTcpDisconnect,
    CmdTcpSend,
    CmdUdpSend,
    CmdHprSend,
    // External Application
    CmdAppLaunch,
    CmdAppClose,
    // Audio
    CmdPlayWav,
    // Null
    CmdNull
};

struct FunctionCommand
{
    CommandType type;
    QString commandCode;
    QString param1;
    QString param2;
    QString param3;
    QString param4;
    QString param5;
};

// Windows message system
#define WINMSGTIMERTIME         1000

// Output source priority
// Time the arbitration waits for the priority output source to report in, before
// the output source that reported first is allowed to claim the output stream
#define SOURCEARBITRATIONWAIT   300
#define PRIORITYNETWORKNAME     "Network"
#define PRIORITYWINMSGNAME      "Windows"

// How the two output streams are handled
#define METHODPRIORITYNAME      "Priority"
#define METHODEXCLUSIVENAME     "Exclusive"
#define METHODCONCURRENTNAME    "Concurrent"

// Which output source a game came from
enum OutputSource
{
    SourceNone,
    SourceWinMsg,
    SourceNetwork
};

// Priority  - the preferred output source wins, the other one is used if it does not report
// Exclusive - only the preferred output source is used, the other one is never started
// Concurrent- both output streams are processed at the same time
enum OutputProcessingMethod
{
    MethodPriority,
    MethodExclusive,
    MethodConcurrent
};

// TCP Socket
#define TCPHOSTPORT             8000
#define TCPTIMERTIME            2000

// Serial COM Port
#define MAXCOMPORTS             50
#define BEGINCOMPORTNAME        "COM"
#define COMPORTPATHFRONT        "\\\\.\\"
#define BAUD_NUMBER             8
#define WRITERETRYATTEMPTS      2
#define DATABITS_MAX            8
#define DATABITS_MIN            5

struct ComPortData
{
    QString portName;
    QString description;
};

struct ComPortStruct
{
    qint32 baud;
    quint8 data;
    quint8 parity;
    quint8 stop;
};

inline qint32 BAUDDATA_ARRAY[BAUD_NUMBER] = {115200, 57600, 38400, 19200, 9600, 4800, 2400, 1200};

// USB HID
#define MAXPLAYERS              4
#define FRONTPATHREM            26

struct HIDInfo
{
    QString     path;
    QString     displayPath;
    quint16     vendorID;
    QString     vendorIDString;
    quint16     productID;
    QString     productIDString;
    QString     serialNumber;
    quint16     releaseNumber;
    QString     releaseString;
    QString     manufacturer;
    QString     productDiscription;
    quint16     usagePage;
    quint16     usage;
    QString     usageString;
    qint8       interfaceNumber;

    bool operator==(const HIDInfo& other) const
    {
        return (path == other.path) && (displayPath == other.displayPath) && (vendorID == other.vendorID) && (vendorIDString == other.vendorIDString)
        && (productID == other.productID) && (productIDString == other.productIDString) && (serialNumber == other.serialNumber) && (releaseNumber == other.releaseNumber)
            && (releaseString == other.releaseString)  && (manufacturer == other.manufacturer) && (productDiscription == other.productDiscription)
            && (usagePage == other.usagePage) && (usage == other.usage) && (usageString == other.usageString) && (interfaceNumber == other.interfaceNumber);
    }
};

// SDL3
struct SdlCtrlData
{
    int id;
    QString name;
};

// LedWiz
// 32 LED channels with 48 Brightness levels
// A value of 1 to 48 sets the brightness of each LED using PWM
// A value of 129-132 indicates an automated pulse mode as follows:
// 129 = Ramp Up / Ramp Down
// 130 = On / Off
// 131 = On / Ramp Down
// 132 = Ramp Up / On
// The speed is controlled by the Global Pulse Speed parameter

#define LEDWIZMAXDEVICES        16
#define LEDWIZMAXPINS           32

struct LedWizData
{
    int id;
    int type;
    QString name;
};

struct LedWizCache
{
    BYTE sbaBanks[4];
    BYTE pbaLevels[32];
    BYTE globalPulseSpeed;
};

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
