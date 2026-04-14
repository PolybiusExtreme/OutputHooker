#include "TestOutputWindow.h"
#include "ui_TestOutputWindow.h"

#include "GuiUtilities.h"

int TestOutputWindow::lastSelectedFunction = 0;

TestOutputWindow::TestOutputWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TestOutputWindow)
{
    QWidget::setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);

    ui->setupUi(this);

    ui->horizontalLayout_Parameter->setAlignment(Qt::AlignLeft);

    ui->cbFunction->addItem("Choose a function...",         CmdNone);
    ui->cbFunction->addItem("COM Port - Open",              CmdComOpen);
    ui->cbFunction->addItem("COM Port - Close",             CmdComClose);
    ui->cbFunction->addItem("COM Port - Write",             CmdComWrite);
    ui->cbFunction->addItem("COM Port - Set Settings",      CmdComSet);
    ui->cbFunction->addItem("GUN4IR - Start Serial Mode",   CmdGun4irSSM);
    ui->cbFunction->addItem("GUN4IR - Exit Serial Mode",    CmdGun4irESM);
    ui->cbFunction->addItem("GUN4IR - Input",               CmdGun4irIM);
    ui->cbFunction->addItem("GUN4IR - Offscreen Reload",    CmdGun4irOR);
    ui->cbFunction->addItem("GUN4IR - Pedal",               CmdGun4irPM);
    ui->cbFunction->addItem("GUN4IR - Aspect Ratio",        CmdGun4irAR);
    ui->cbFunction->addItem("GUN4IR - Temperature Sensor",  CmdGun4irTS);
    ui->cbFunction->addItem("GUN4IR - Auto Reload",         CmdGun4irRL);
    ui->cbFunction->addItem("GUN4IR - Rumble Only",         CmdGun4irRO);
    ui->cbFunction->addItem("GUN4IR - Full Auto",           CmdGun4irFA);
    ui->cbFunction->addItem("LEDWiz - Set Pin State",       CmdLedWizState);
    ui->cbFunction->addItem("LEDWiz - Set Power Level",     CmdLedWizPower);
    ui->cbFunction->addItem("LEDWiz - Set RGB LED Color",   CmdLedWizColor);
    ui->cbFunction->addItem("LEDWiz - Set Pulse Rate",      CmdLedWizPulse);
    ui->cbFunction->addItem("LEDWiz - Kill All LEDs",       CmdLedWizKill);
    ui->cbFunction->addItem("Ultimarc - Set LED State",     CmdUltimarcState);
    ui->cbFunction->addItem("Ultimarc - Set LED Intensity", CmdUltimarcIntensity);
    ui->cbFunction->addItem("Ultimarc - Set RGB LED Color", CmdUltimarcColor);
    ui->cbFunction->addItem("Ultimarc - Kill All LEDs",     CmdUltimarcKill);
    ui->cbFunction->addItem("Launch Application",           CmdAppLaunch);
    ui->cbFunction->addItem("Close Application",            CmdAppClose);
    ui->cbFunction->addItem("Play WAV Audio File",          CmdPlayWav);

    ui->cbFunction->setStyleSheet("QComboBox { combobox-popup: 0; }");
    ui->cbFunction->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->cbParameter1->setStyleSheet("QComboBox { combobox-popup: 0; }");
    ui->cbParameter1->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->cbParameter2->setStyleSheet("QComboBox { combobox-popup: 0; }");
    ui->cbParameter2->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->cbParameter3->setStyleSheet("QComboBox { combobox-popup: 0; }");
    ui->cbParameter3->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->cbParameter4->setStyleSheet("QComboBox { combobox-popup: 0; }");
    ui->cbParameter4->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->cbParameter5->setStyleSheet("QComboBox { combobox-popup: 0; }");
    ui->cbParameter5->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    setParamLabelVisibility(false, false, false, false, false);
    setParamComboBoxVisibility(false, false, false, false, false);
    setParamLineEditVisibility(false, false, false, false, false);

    validator255 = new QIntValidator(0, 255, this);

    ui->lineEditCommand->setEnabled(false);

    //Set last selected function
    ui->cbFunction->setCurrentIndex(lastSelectedFunction);

    // Handle function ComboBox
    connect(ui->cbFunction, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TestOutputWindow::handleFunctionChanged);

    // Clear error style if text in lineEditParameter has changed
    connect(ui->lineEditParameter1, &QLineEdit::textChanged, this, &TestOutputWindow::clearErrorStyle);
    connect(ui->lineEditParameter2, &QLineEdit::textChanged, this, &TestOutputWindow::clearErrorStyle);
    connect(ui->lineEditParameter3, &QLineEdit::textChanged, this, &TestOutputWindow::clearErrorStyle);

    // Update command text in lineEditCommand
    connect(ui->cbFunction, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TestOutputWindow::updateCommandDisplay);

    // Update parameter 1 text in lineEditCommand
    connect(ui->cbParameter1, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TestOutputWindow::updateCommandDisplay);
    connect(ui->lineEditParameter1, &QLineEdit::textChanged, this, &TestOutputWindow::updateCommandDisplay);

    // Update parameter 2 text in lineEditCommand
    connect(ui->cbParameter2, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TestOutputWindow::updateCommandDisplay);
    connect(ui->lineEditParameter2, &QLineEdit::textChanged, this, &TestOutputWindow::updateCommandDisplay);

    // Update parameter 3 text in lineEditCommand
    connect(ui->cbParameter3, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TestOutputWindow::updateCommandDisplay);
    connect(ui->lineEditParameter3, &QLineEdit::textChanged, this, &TestOutputWindow::updateCommandDisplay);

    // Update parameter 4 text in lineEditCommand
    connect(ui->cbParameter4, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TestOutputWindow::updateCommandDisplay);
    connect(ui->lineEditParameter4, &QLineEdit::textChanged, this, &TestOutputWindow::updateCommandDisplay);

    // Update parameter 5 text in lineEditCommand
    connect(ui->cbParameter5, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TestOutputWindow::updateCommandDisplay);
    connect(ui->lineEditParameter5, &QLineEdit::textChanged, this, &TestOutputWindow::updateCommandDisplay);

    handleFunctionChanged(ui->cbFunction->currentIndex());

    updateCommandDisplay();

    adjustSize();

    GuiUtilities::centerWidgetOnScreen(this);

    this->setFixedSize(this->size());
}

TestOutputWindow::~TestOutputWindow()
{
    delete ui;
}

// Get command
FunctionCommand TestOutputWindow::getCommand() const
{
    FunctionCommand cmd;
    cmd.type = static_cast<CommandType>(ui->cbFunction->currentData().toInt());

    switch (cmd.type)
    {
    case CmdComOpen:
        cmd.commandCode = COMPORTOPEN;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->lineEditParameter2->text();
        break;

    case CmdComClose:
        cmd.commandCode = COMPORTCLOSE;
        cmd.param1 = ui->cbParameter1->currentText();
        break;

    case CmdComWrite:
        cmd.commandCode = COMPORTWRITE;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->lineEditParameter2->text();
        break;

    case CmdComSet:
        cmd.commandCode = COMPORTSETTINGS;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->lineEditParameter2->text();
        break;

    case CmdGun4irSSM:
    case CmdGun4irESM:
    case CmdGun4irIM:
    case CmdGun4irOR:
    case CmdGun4irPM:
    case CmdGun4irAR:
    case CmdGun4irTS:
    case CmdGun4irRL:
    case CmdGun4irRO:
    case CmdGun4irFA:
        cmd.commandCode = COMPORTWRITE;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentData().toString();
        break;

    case CmdLedWizState:
        cmd.commandCode = LWSETSTATE;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentText();
        cmd.param3 = ui->cbParameter3->currentData().toString();
        break;

    case CmdLedWizPower:
        cmd.commandCode = LWSETPOWER;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentText();
        cmd.param3 = ui->cbParameter3->currentData().toString();
        break;

    case CmdLedWizColor:
        cmd.commandCode = LWSETCOLOR;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentText();
        cmd.param3 = ui->cbParameter3->currentData().toString();
        cmd.param4 = ui->cbParameter4->currentData().toString();
        cmd.param5 = ui->cbParameter5->currentData().toString();
        break;

    case CmdLedWizPulse:
        cmd.commandCode = LWSETPULSE;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentText();
        cmd.param3 = ui->cbParameter3->currentData().toString();
        break;

    case CmdLedWizKill:
        cmd.commandCode = LWKILLALLLEDS;
        cmd.param1 = ui->cbParameter1->currentText();
        break;

    case CmdUltimarcState:
        cmd.commandCode = PACSETSTATE;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentText();
        cmd.param3 = ui->cbParameter3->currentData().toString();
        break;

    case CmdUltimarcIntensity:
        cmd.commandCode = PACSETINTENSITY;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentText();
        cmd.param3 = ui->lineEditParameter3->text();
        break;

    case CmdUltimarcColor:
        cmd.commandCode = PACSETCOLOR;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentText();
        cmd.param3 = ui->cbParameter3->currentData().toString();
        cmd.param4 = ui->cbParameter4->currentData().toString();
        cmd.param5 = ui->cbParameter5->currentData().toString();
        break;

    case CmdUltimarcKill:
        cmd.commandCode = PACKILLALLLEDS;
        cmd.param1 = ui->cbParameter1->currentText();
        break;

    case CmdAppLaunch:
        cmd.commandCode = APPLAUNCH;
        cmd.param1 = ui->lineEditParameter1->text();
        cmd.param2 = ui->lineEditParameter2->text();
        cmd.param3 = ui->cbParameter3->currentData().toString();
        break;

    case CmdAppClose:
        cmd.commandCode = APPCLOSE;
        cmd.param1 = ui->lineEditParameter1->text();
        break;

    case CmdPlayWav:
        cmd.commandCode = PLAYWAVAUDIO;
        cmd.param1 = ui->lineEditParameter1->text();
        break;

    default:
        cmd.commandCode = "err";
        break;
    }

    return cmd;
}

// Update command text in lineEditCommand
void TestOutputWindow::updateCommandDisplay()
{
    FunctionCommand cmd = getCommand();

    if (cmd.type == CmdNone)
    {
        ui->lineEditCommand->clear();
        return;
    }

    QString separator = " ";
    QStringList parts;
    if (!cmd.commandCode.isEmpty()) parts << cmd.commandCode;
    if (!cmd.param1.isEmpty())      parts << cmd.param1;
    if (!cmd.param2.isEmpty())      parts << cmd.param2;
    if (!cmd.param3.isEmpty())      parts << cmd.param3;
    if (!cmd.param4.isEmpty())      parts << cmd.param4;
    if (!cmd.param5.isEmpty())      parts << cmd.param5;

    ui->lineEditCommand->setText(parts.join(separator));
}

// Clear error style
void TestOutputWindow::clearErrorStyle()
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(sender());
    if (lineEdit)
        lineEdit->setStyleSheet("");
}

// On pushButton Test clicked
void TestOutputWindow::on_pushButtonTest_clicked()
{
    clearErrorStyle();

    FunctionCommand cmd = getCommand();

    if (cmd.type == CmdNone || cmd.commandCode == "err")
        return;

    if (checkCommand(cmd))
    {
        emit sendTestCommand(cmd);
    }
}

// On pushButton Cancel clicked
void TestOutputWindow::on_pushButtonCancel_clicked()
{
    reject();
}

// Handle function ComboBox
void TestOutputWindow::handleFunctionChanged(int index)
{
    lastSelectedFunction = index;

    CommandType cmd = static_cast<CommandType>(ui->cbFunction->itemData(index).toInt());
    resetInputs();

    switch (cmd)
    {
    case CmdComOpen:
    case CmdComClose:
    case CmdComWrite:
    case CmdComSet:
        setupComPortUI(cmd);
        break;

    case CmdGun4irSSM:
    case CmdGun4irESM:
    case CmdGun4irIM:
    case CmdGun4irOR:
    case CmdGun4irPM:
    case CmdGun4irAR:
    case CmdGun4irTS:
    case CmdGun4irRL:
    case CmdGun4irRO:
    case CmdGun4irFA:
        setupGun4irUI(cmd);
        break;

    case CmdLedWizState:
    case CmdLedWizPower:
    case CmdLedWizColor:
    case CmdLedWizPulse:
    case CmdLedWizKill:
        setupLedWizUI(cmd);
        break;

    case CmdUltimarcState:
    case CmdUltimarcIntensity:
    case CmdUltimarcColor:
    case CmdUltimarcKill:
        setupUltimarcUI(cmd);
        break;

    case CmdAppLaunch:
    case CmdAppClose:
        setupAppUI(cmd);
        break;

    case CmdPlayWav:
        setupAudioUI();
        break;

    default:
        setParamLabelVisibility(false, false, false, false, false);
        setParamComboBoxVisibility(false, false, false, false, false);
        setParamLineEditVisibility(false, false, false, false, false);
        break;
    }

    if (layout())
        layout()->activate();

    adjustSize();
}

// Setup UI - COM Port functions
void TestOutputWindow::setupComPortUI(CommandType cmd)
{
    setParamLabelVisibility(true, false, false, false, false);
    setParamComboBoxVisibility(true, false, false, false, false);
    setParamLineEditVisibility(false, false, false, false, false);
    ui->labelParameter1->setFixedWidth(60);
    ui->labelParameter1->setText("Port:");
    ui->cbParameter1->setFixedWidth(60);
    for (int i = 1; i <= MAXCOMPORTS; ++i)
        ui->cbParameter1->addItem(QString::number(i));

    if (cmd == CmdComOpen || cmd == CmdComSet || cmd == CmdComWrite)
    {
        setParamLabelVisibility(true, true, false, false, false);
        setParamLineEditVisibility(false, true, false, false, false);

        if (cmd == CmdComWrite)
        {
            ui->labelParameter2->setText("Data:");
            ui->lineEditParameter2->setText("S6M3x0M4x1");
        }
        else
        {
            ui->labelParameter2->setText("Settings:");
            ui->lineEditParameter2->setText("baud=9600_parity=N_data=8_stop=1");
        }
    }
}

// Setup UI - GUN4IR functions
void TestOutputWindow::setupGun4irUI(CommandType cmd)
{
    setParamLabelVisibility(true, true, false, false, false);
    setParamComboBoxVisibility(true, true, false, false, false);
    setParamLineEditVisibility(false, false, false, false, false);

    ui->labelParameter1->setFixedWidth(60);
    ui->labelParameter1->setText("Port:");
    ui->cbParameter1->setFixedWidth(60);
    for (int i = 1; i <= MAXCOMPORTS; ++i)
        ui->cbParameter1->addItem(QString::number(i));

    ui->labelParameter2->setFixedWidth(150);
    ui->labelParameter2->setText("Mode:");
    ui->cbParameter2->setFixedWidth(150);

    if (cmd == CmdGun4irSSM)
    {
        ui->cbParameter2->addItem("All Feedbacks", "S6");
        ui->cbParameter2->addItem("Recoil", "S0");
        ui->cbParameter2->addItem("Rumble", "S1");
        ui->cbParameter2->addItem("Red LED", "S2");
        ui->cbParameter2->addItem("Green LED", "S3");
        ui->cbParameter2->addItem("Blue LED", "S4");
    }
    else if (cmd == CmdGun4irESM)
    {
        ui->cbParameter2->addItem("Exit", "E");
    }
    else if (cmd == CmdGun4irIM)
    {
        ui->cbParameter2->addItem("Mouse/Keyboard", "M0x0");
        ui->cbParameter2->addItem("Gamepad", "M0x1");
        ui->cbParameter2->addItem("Hybrid", "M0x2");
    }
    else if (cmd == CmdGun4irOR)
    {
        ui->cbParameter2->addItem("Disabled", "M1x0");
        ui->cbParameter2->addItem("Corner Shot", "M1x1");
        ui->cbParameter2->addItem("Reload Button", "M1x2");
        ui->cbParameter2->addItem("Normal Shot", "M1x3");
    }
    else if (cmd == CmdGun4irPM)
    {
        ui->cbParameter2->addItem("Separate Button", "M2x0");
        ui->cbParameter2->addItem("Reload Button", "M2x1");
    }
    else if (cmd == CmdGun4irAR)
    {
        ui->cbParameter2->addItem("Fullscreen", "M3x0");
        ui->cbParameter2->addItem("4:3", "M3x1");
    }
    else if (cmd == CmdGun4irTS)
    {
        ui->cbParameter2->addItem("Disabled", "M4x0");
        ui->cbParameter2->addItem("Enabled", "M4x1");
    }
    else if (cmd == CmdGun4irRL)
    {
        ui->cbParameter2->addItem("Disabled", "M5x0");
        ui->cbParameter2->addItem("Enabled", "M5x1");
    }
    else if (cmd == CmdGun4irRO)
    {
        ui->cbParameter2->addItem("Disabled", "M6x0");
        ui->cbParameter2->addItem("Enabled", "M6x1");
    }
    else if (cmd == CmdGun4irFA)
    {
        ui->cbParameter2->addItem("Disabled", "M8x0");
        ui->cbParameter2->addItem("Automatic", "M8x1");
        ui->cbParameter2->addItem("Always On", "M8x2");
    }
}

// Setup UI - LedWiz functions
void TestOutputWindow::setupLedWizUI(CommandType cmd)
{
    setParamLabelVisibility(true, false, false, false, false);
    setParamComboBoxVisibility(true, false, false, false, false);
    setParamLineEditVisibility(false, false, false, false, false);
    ui->labelParameter1->setFixedWidth(60);
    ui->labelParameter1->setText("Device:");
    ui->cbParameter1->setFixedWidth(60);
    for (int i = 1; i <= LEDWIZMAXDEVICES; ++i)
        ui->cbParameter1->addItem(QString::number(i));

    if (cmd == CmdLedWizState || cmd == CmdLedWizPower || cmd == CmdLedWizColor)
    {
        setParamLabelVisibility(true, true, true, false, false);
        setParamComboBoxVisibility(true, true, false, false, false);
        ui->labelParameter2->setFixedWidth(60);
        ui->labelParameter2->setText("Pin:");
        ui->cbParameter2->setFixedWidth(60);
        for (int i = 1; i <= 32; ++i)
            ui->cbParameter2->addItem(QString::number(i));

        if (cmd == CmdLedWizState)
        {
            setParamComboBoxVisibility(true, true, true, false, false);
            ui->labelParameter3->setText("State:");
            ui->cbParameter3->addItem("Off", "0");
            ui->cbParameter3->addItem("On", "1");
        }
        else if (cmd == CmdLedWizPower)
        {
            setParamComboBoxVisibility(true, true, true, false, false);
            ui->labelParameter3->setFixedWidth(95);
            ui->labelParameter3->setText("Power Level:");
            ui->cbParameter3->setFixedWidth(95);
            for (int i = 0; i <= 48; ++i)
                ui->cbParameter3->addItem(QString::number(i), QString::number(i));
            ui->cbParameter3->addItem("Fade in/out", "129");
            ui->cbParameter3->addItem("Blink", "130");
            ui->cbParameter3->addItem("Fade out", "131");
            ui->cbParameter3->addItem("Fade in", "132");
        }
        else if (cmd == CmdLedWizColor)
        {
            setParamLabelVisibility(true, true, true, true, true);
            setParamComboBoxVisibility(true, true, true, true, true);
            ui->labelParameter3->setFixedWidth(95);
            ui->labelParameter3->setText("Red Value:");
            ui->cbParameter3->setFixedWidth(95);
            for (int i = 0; i <= 48; ++i)
                ui->cbParameter3->addItem(QString::number(i), QString::number(i));
            ui->cbParameter3->addItem("Fade in/out", "129");
            ui->cbParameter3->addItem("Blink", "130");
            ui->cbParameter3->addItem("Fade out", "131");
            ui->cbParameter3->addItem("Fade in", "132");
            ui->labelParameter4->setFixedWidth(95);
            ui->labelParameter4->setText("Green Value:");
            ui->cbParameter4->setFixedWidth(95);
            for (int i = 0; i <= 48; ++i)
                ui->cbParameter4->addItem(QString::number(i), QString::number(i));
            ui->cbParameter4->addItem("Fade in/out", "129");
            ui->cbParameter4->addItem("Blink", "130");
            ui->cbParameter4->addItem("Fade out", "131");
            ui->cbParameter4->addItem("Fade in", "132");
            ui->labelParameter5->setFixedWidth(95);
            ui->labelParameter5->setText("Blue Value:");
            ui->cbParameter5->setFixedWidth(95);
            for (int i = 0; i <= 48; ++i)
                ui->cbParameter5->addItem(QString::number(i), QString::number(i));
            ui->cbParameter5->addItem("Fade in/out", "129");
            ui->cbParameter5->addItem("Blink", "130");
            ui->cbParameter5->addItem("Fade out", "131");
            ui->cbParameter5->addItem("Fade in", "132");
        }
    }
    else if (cmd == CmdLedWizPulse)
    {
        setParamLabelVisibility(true, true, false, false, false);
        setParamComboBoxVisibility(true, true, false, false, false);
        ui->labelParameter2->setFixedWidth(65);
        ui->labelParameter2->setText("Pulse Rate:");
        ui->cbParameter2->setFixedWidth(65);
        for (int i = 1; i <= 7; ++i)
            ui->cbParameter2->addItem(QString::number(i), QString::number(i));
    }
}

// Setup UI - Ultimarc functions
void TestOutputWindow::setupUltimarcUI(CommandType cmd)
{
    setParamLabelVisibility(true, false, false, false, false);
    setParamComboBoxVisibility(true, false, false, false, false);
    setParamLineEditVisibility(false, false, false, false, false);
    ui->labelParameter1->setFixedWidth(60);
    ui->labelParameter1->setText("Device:");
    ui->cbParameter1->setFixedWidth(60);
    for (int i = 1; i <= ULTIMARCMAXDEVICES; ++i)
        ui->cbParameter1->addItem(QString::number(i));

    if (cmd == CmdUltimarcState || cmd == CmdUltimarcIntensity || cmd == CmdUltimarcColor)
    {
        setParamLabelVisibility(true, true, true, false, false);
        setParamComboBoxVisibility(true, true, false, false, false);
        ui->labelParameter2->setFixedWidth(60);
        ui->labelParameter2->setText("Pin:");
        ui->cbParameter2->setFixedWidth(60);
        for (int i = 1; i <= 96; ++i)
            ui->cbParameter2->addItem(QString::number(i));

        if (cmd == CmdUltimarcState)
        {
            setParamComboBoxVisibility(true, true, true, false, false);
            ui->labelParameter3->setText("State:");
            ui->cbParameter3->addItem("Off", "0");
            ui->cbParameter3->addItem("On", "1");
        }
        else if (cmd == CmdUltimarcIntensity)
        {
            setParamLineEditVisibility(false, false, true, false, false);
            ui->labelParameter3->setText("Intensity:");
            ui->lineEditParameter3->setValidator(validator255);
            ui->lineEditParameter3->setText("255");
        }
        else if (cmd == CmdUltimarcColor)
        {
            setParamLabelVisibility(true, true, true, true, true);
            setParamComboBoxVisibility(true, true, true, true, true);
            ui->labelParameter3->setFixedWidth(75);
            ui->labelParameter3->setText("Red Value:");
            ui->cbParameter3->setFixedWidth(75);
            for (int i = 0; i <= 255; ++i)
                ui->cbParameter3->addItem(QString::number(i), QString::number(i));
            ui->labelParameter4->setFixedWidth(75);
            ui->labelParameter4->setText("Green Value:");
            ui->cbParameter4->setFixedWidth(75);
            for (int i = 0; i <= 255; ++i)
                ui->cbParameter4->addItem(QString::number(i), QString::number(i));
            ui->labelParameter5->setFixedWidth(75);
            ui->labelParameter5->setText("Blue Value:");
            ui->cbParameter5->setFixedWidth(75);
            for (int i = 0; i <= 255; ++i)
                ui->cbParameter5->addItem(QString::number(i), QString::number(i));
        }
    }
}

// Setup UI - Launch Application function
void TestOutputWindow::setupAppUI(CommandType cmd)
{
    if (cmd == CmdAppLaunch)
    {
        setParamLabelVisibility(true, true, true, false, false);
        setParamComboBoxVisibility(false, false, true, false, false);
        setParamLineEditVisibility(true, true, false, false, false);

        ui->labelParameter1->setFixedWidth(115);
        ui->labelParameter1->setText("Path & Executable:");
        ui->lineEditParameter1->setFixedWidth(115);
        ui->lineEditParameter1->setText(R"("C:\app\app.exe")");
        ui->labelParameter2->setFixedWidth(80);
        ui->labelParameter2->setText("Parameters:");
        ui->lineEditParameter2->setFixedWidth(80);
        ui->lineEditParameter2->setText(R"("parameters")");
        ui->labelParameter3->setText("Mode:");
        ui->cbParameter3->addItem("Normal", "0");
        ui->cbParameter3->addItem("Hidden", "1");
        ui->cbParameter3->addItem("Minimized", "2");
        ui->cbParameter3->addItem("Maximized", "3");
    }
    else
    {
        setParamLabelVisibility(true, false, false, false, false);
        setParamComboBoxVisibility(false, false, false, false, false);
        setParamLineEditVisibility(true, false, false, false, false);

        ui->labelParameter1->setText("Executable:");
        ui->lineEditParameter1->setText(R"("app.exe")");
    }
}

// Setup UI - Play WAV audio file function
void TestOutputWindow::setupAudioUI()
{
    setParamLabelVisibility(true, false, false, false, false);
    setParamComboBoxVisibility(false, false, false, false, false);
    setParamLineEditVisibility(true, false, false, false, false);

    ui->labelParameter1->setText("WAV Audio File:");
    ui->lineEditParameter1->setText("test.wav");
}

// Check command
bool TestOutputWindow::checkCommand(const FunctionCommand &cmd)
{
    if (cmd.commandCode == COMPORTOPEN || cmd.commandCode == COMPORTSETTINGS)
    {
        return validateComPortValues(cmd);
    }

    if (cmd.commandCode == PACSETINTENSITY)
    {
        return validateLedIntensityValue(cmd);
    }

    if (cmd.commandCode == APPLAUNCH)
    {
        return validateLaunchApp(cmd);
    }

    if (cmd.commandCode == APPCLOSE)
    {
        return validateCloseApp(cmd);
    }

    if (cmd.commandCode == PLAYWAVAUDIO)
    {
        return validateWavAudioFile(cmd);
    }

    return true;
}

// Validate COM Port values
bool TestOutputWindow::validateComPortValues(const FunctionCommand &cmd)
{
    const QString errorStyle = "border: 1px solid red;";
    ComPortStruct portTemp;
    bool isNumber;

    if (cmd.param2.isEmpty())
    {
        QString errorMsg = "The settings field is empty!";
        GuiUtilities::showMessageBoxCentered(this, "COM Port Open - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    // Split the settings into 4 strings = 1: Baud  2: Parity  3: Data  4: Stop
    QStringList settings = cmd.param2.split('_', Qt::SkipEmptyParts);
    if (settings.size() < 4)
    {
        ui->lineEditParameter2->setStyleSheet(errorStyle);
        return false;
    }

    settings[0].remove(BAUDREMOVE);
    settings[1].remove(PARITYREMOVE);
    settings[2].remove(DATAREMOVE);
    settings[3].remove(STOPREMOVE);

    if (settings[0].isEmpty())
    {
        ui->lineEditParameter2->setStyleSheet(errorStyle);
        QString errorMsg = "Baud rate value is empty!";
        GuiUtilities::showMessageBoxCentered(this, "COM Port Open - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    portTemp.baud = settings[0].toUInt(&isNumber);

    if (!isNumber)
    {
        ui->lineEditParameter2->setStyleSheet(errorStyle);
        QString errorMsg = "Baud rate is not a number!\nBaud rate: " + settings[0];
        GuiUtilities::showMessageBoxCentered(this, "COM Port Open - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    bool chkSetting = false;

    for (quint8 i = 0; i < BAUD_NUMBER; i++)
    {
        if (portTemp.baud == BAUDDATA_ARRAY[i])
            chkSetting = true;
    }

    if (!chkSetting)
    {
        ui->lineEditParameter2->setStyleSheet(errorStyle);
        QString errorMsg = "Baud rate is not a correct rate!\nBaud rate: " + settings[0];
        GuiUtilities::showMessageBoxCentered(this, "COM Port Open - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    if (settings[1].isEmpty())
    {
        ui->lineEditParameter2->setStyleSheet(errorStyle);
        QString errorMsg = "Parity value is empty!";
        GuiUtilities::showMessageBoxCentered(this, "COM Port Open - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    if (settings[1] == "N") portTemp.parity = 0;
    else if (settings[1] == "E") portTemp.parity = 2;
    else if (settings[1] == "O") portTemp.parity = 3;
    else if (settings[1] == "S") portTemp.parity = 4;
    else if (settings[1] == "M") portTemp.parity = 5;
    else
    {
        ui->lineEditParameter2->setStyleSheet(errorStyle);
        QString errorMsg = "Parity is not a correct char [N,E,O,S,M]!\nParity char: " + settings[1];
        GuiUtilities::showMessageBoxCentered(this, "COM Port Open - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    if (settings[2].isEmpty())
    {
        ui->lineEditParameter2->setStyleSheet(errorStyle);
        QString errorMsg = "Data bits value is empty!";
        GuiUtilities::showMessageBoxCentered(this, "COM Port Open - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    portTemp.data = settings[2].toUInt(&isNumber);

    if (!isNumber)
    {
        ui->lineEditParameter2->setStyleSheet(errorStyle);
        QString errorMsg = "Data bits is not a number!\nData bits: " + settings[2];
        GuiUtilities::showMessageBoxCentered(this, "COM Port Open - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    // Can be 5-8
    if (portTemp.data < 5 || portTemp.data > 8)
    {
        ui->lineEditParameter2->setStyleSheet(errorStyle);
        QString errorMsg = "Data bits is not in range [5-8]!\nData bits: " + settings[2];
        GuiUtilities::showMessageBoxCentered(this, "COM Port Open - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    if (settings[3].isEmpty())
    {
        ui->lineEditParameter2->setStyleSheet(errorStyle);
        QString errorMsg = "Stop bits value is empty!";
        GuiUtilities::showMessageBoxCentered(this, "COM Port Open - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    if (settings[3] == "1.5")
        settings[3] = "3";

    portTemp.stop = settings[3].toUInt(&isNumber);

    if (!isNumber)
    {
        ui->lineEditParameter2->setStyleSheet(errorStyle);
        QString errorMsg = "Stop bits is not a number!\nStop bits: " + settings[3];
        GuiUtilities::showMessageBoxCentered(this, "COM Port Open - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    // Can be 1-3
    if (portTemp.stop == 0 || portTemp.stop > 3)
    {
        ui->lineEditParameter2->setStyleSheet(errorStyle);
        QString errorMsg = "Stop bits is not in range [1,1.5,2]!\nData bits: " + settings[3];
        GuiUtilities::showMessageBoxCentered(this, "COM Port Open - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    return true;
}

// Validate Ultimarc LED intensity value
bool TestOutputWindow::validateLedIntensityValue(const FunctionCommand &cmd)
{
    const QString errorStyle = "border: 1px solid red;";

    if (cmd.param3.isEmpty())
    {
        ui->lineEditParameter3->setStyleSheet(errorStyle);
        QString errorMsg = "The intensity value field is empty!";
        GuiUtilities::showMessageBoxCentered(this, "Ultimarc - Set LED Intensity - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    QString text = ui->lineEditParameter3->text();
    int pos = 0;

    if (ui->lineEditParameter3->validator()->validate(text, pos) != QValidator::Acceptable)
    {
        ui->lineEditParameter3->setStyleSheet(errorStyle);
        QString errorMsg = "The intensity value is not in range [0-255]!\nIntensity value: " + cmd.param3;
        GuiUtilities::showMessageBoxCentered(this, "Ultimarc - Set LED Intensity - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    return true;
}

// Validate "Launch Application"
bool TestOutputWindow::validateLaunchApp(const FunctionCommand &cmd)
{
    const QString errorStyle = "border: 1px solid red;";

    if (cmd.param1.isEmpty())
    {
        ui->lineEditParameter1->setStyleSheet(errorStyle);
        QString errorMsg = "The path & executable field is empty!";
        GuiUtilities::showMessageBoxCentered(this, "Launch Application - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    QString executableFilePath = cmd.param1;
    executableFilePath.remove('"');

    if (!QFile::exists(executableFilePath))
    {
        ui->lineEditParameter1->setStyleSheet(errorStyle);
        QString errorMsg = executableFilePath + " not found!";
        GuiUtilities::showMessageBoxCentered(this, "Launch Application - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    return true;
}

// Validate "Close Application"
bool TestOutputWindow::validateCloseApp(const FunctionCommand &cmd)
{
    const QString errorStyle = "border: 1px solid red;";

    if (cmd.param1.isEmpty())
    {
        ui->lineEditParameter1->setStyleSheet(errorStyle);
        QString errorMsg = "The executable field is empty!";
        GuiUtilities::showMessageBoxCentered(this, "Close Application - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    return true;
}

// Validate WAV audio file
bool TestOutputWindow::validateWavAudioFile(const FunctionCommand &cmd)
{
    const QString errorStyle = "border: 1px solid red;";

    if (cmd.param1.isEmpty())
    {
        ui->lineEditParameter1->setStyleSheet(errorStyle);
        QString errorMsg = "The file field is empty!";
        GuiUtilities::showMessageBoxCentered(this, "Play WAV Audio File - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    QString wavFile = cmd.param1;
    QString wavFilePath = QApplication::applicationDirPath() + "/sounds/" + wavFile;

    if (!QFile::exists(wavFilePath))
    {
        ui->lineEditParameter1->setStyleSheet(errorStyle);
        QString errorMsg = wavFile + " not found!";
        GuiUtilities::showMessageBoxCentered(this, "Play WAV Audio File - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    return true;
}

// Set parameter Label visibility
void TestOutputWindow::setParamLabelVisibility(bool l1, bool l2, bool l3, bool l4, bool l5)
{
    ui->labelParameter1->setVisible(l1);
    ui->labelParameter2->setVisible(l2);
    ui->labelParameter3->setVisible(l3);
    ui->labelParameter4->setVisible(l4);
    ui->labelParameter5->setVisible(l5);
}

// Set parameter ComboBox visibility
void TestOutputWindow::setParamComboBoxVisibility(bool cb1, bool cb2, bool cb3, bool cb4, bool cb5)
{
    ui->cbParameter1->setVisible(cb1);
    ui->cbParameter2->setVisible(cb2);
    ui->cbParameter3->setVisible(cb3);
    ui->cbParameter4->setVisible(cb4);
    ui->cbParameter5->setVisible(cb5);
}

// Set parameter LineEdit visibility
void TestOutputWindow::setParamLineEditVisibility(bool le1, bool le2, bool le3, bool le4, bool le5)
{
    ui->lineEditParameter1->setVisible(le1);
    ui->lineEditParameter2->setVisible(le2);
    ui->lineEditParameter3->setVisible(le3);
    ui->lineEditParameter4->setVisible(le4);
    ui->lineEditParameter5->setVisible(le5);
}

// Reset all inputs
void TestOutputWindow::resetInputs()
{
    ui->labelParameter1->setMinimumWidth(0);
    ui->labelParameter2->setMinimumWidth(0);
    ui->labelParameter3->setMinimumWidth(0);
    ui->labelParameter4->setMinimumWidth(0);
    ui->labelParameter5->setMinimumWidth(0);
    ui->labelParameter1->setMaximumWidth(16777215);
    ui->labelParameter2->setMaximumWidth(16777215);
    ui->labelParameter3->setMaximumWidth(16777215);
    ui->labelParameter4->setMaximumWidth(16777215);
    ui->labelParameter5->setMaximumWidth(16777215);
    ui->cbParameter1->clear();
    ui->cbParameter2->clear();
    ui->cbParameter3->clear();
    ui->cbParameter4->clear();
    ui->cbParameter5->clear();
    ui->cbParameter1->setMinimumWidth(0);
    ui->cbParameter2->setMinimumWidth(0);
    ui->cbParameter3->setMinimumWidth(0);
    ui->cbParameter4->setMinimumWidth(0);
    ui->cbParameter5->setMinimumWidth(0);
    ui->cbParameter1->setMaximumWidth(16777215);
    ui->cbParameter2->setMaximumWidth(16777215);
    ui->cbParameter3->setMaximumWidth(16777215);
    ui->cbParameter4->setMaximumWidth(16777215);
    ui->cbParameter5->setMaximumWidth(16777215);
    ui->lineEditParameter1->clear();
    ui->lineEditParameter2->clear();
    ui->lineEditParameter3->clear();
    ui->lineEditParameter4->clear();
    ui->lineEditParameter5->clear();
    ui->lineEditParameter1->setToolTip("");
    ui->lineEditParameter2->setToolTip("");
    ui->lineEditParameter3->setToolTip("");
    ui->lineEditParameter4->setToolTip("");
    ui->lineEditParameter5->setToolTip("");
    ui->lineEditParameter1->setInputMask("");
    ui->lineEditParameter2->setInputMask("");
    ui->lineEditParameter3->setInputMask("");
    ui->lineEditParameter4->setInputMask("");
    ui->lineEditParameter5->setInputMask("");
    ui->lineEditParameter1->setValidator(nullptr);
    ui->lineEditParameter2->setValidator(nullptr);
    ui->lineEditParameter3->setValidator(nullptr);
    ui->lineEditParameter4->setValidator(nullptr);
    ui->lineEditParameter5->setValidator(nullptr);
    ui->lineEditParameter1->setMinimumWidth(0);
    ui->lineEditParameter2->setMinimumWidth(0);
    ui->lineEditParameter3->setMinimumWidth(0);
    ui->lineEditParameter4->setMinimumWidth(0);
    ui->lineEditParameter5->setMinimumWidth(0);
    ui->lineEditParameter1->setMaximumWidth(16777215);
    ui->lineEditParameter2->setMaximumWidth(16777215);
    ui->lineEditParameter3->setMaximumWidth(16777215);
    ui->lineEditParameter4->setMaximumWidth(16777215);
    ui->lineEditParameter5->setMaximumWidth(16777215);
}
