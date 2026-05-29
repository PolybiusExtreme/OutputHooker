/*
 * Original Copyright (c) 2026 PolybiusExtreme
 *
 * Licensed under the GNU GPLv3.
 */

#include "EditorAddCmdWindow.h"
#include "ui_EditorAddCmdWindow.h"

#include "GuiUtilities.h"

int EditorAddCmdWindow::lastSelectedCategory = 0;
int EditorAddCmdWindow::lastSelectedFunction = 0;

EditorAddCmdWindow::EditorAddCmdWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::EditorAddCmdWindow)
{
    QWidget::setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);

    ui->setupUi(this);

    ui->horizontalLayout_Parameter->setAlignment(Qt::AlignLeft);

    ui->cbCategory->addItem("All Functions",                       CatAll);
    ui->cbCategory->addItem("COM Port",                            CatComPort);
    ui->cbCategory->addItem("Blamcon Lightgun",                    CatBlamcon);
    ui->cbCategory->addItem("Fusion Lightgun",                     CatFusion);
    ui->cbCategory->addItem("GUN4IR Lightgun",                     CatGun4ir);
    ui->cbCategory->addItem("OpenFIRE Lightgun",                   CatOpenFire);
    ui->cbCategory->addItem("Retro Shooter - MX24 Lightgun",       CatRsMX24);
    ui->cbCategory->addItem("Retro Shooter - RS3 Reaper Lightgun", CatRsReaper);
    ui->cbCategory->addItem("Sinden Lightgun",                     CatSinden);
    ui->cbCategory->addItem("X-Gunner Lightgun",                   CatXgunner);
    ui->cbCategory->addItem("Alien Positional Gun",                CatAlien);
    ui->cbCategory->addItem("LEDWiz Controller",                   CatLedWiz);
    ui->cbCategory->addItem("Ultimarc Controller",                 CatUltimarc);
    ui->cbCategory->addItem("Gamecontroller",                      CatSDL3FFB);
    ui->cbCategory->addItem("Generic HID",                         CatUsbHid);
    ui->cbCategory->addItem("Network (TCP/UDP/HTTP)",              CatNetwork);
    ui->cbCategory->addItem("External Application",                CatApplication);
    ui->cbCategory->addItem("Audio",                               CatAudio);
    ui->cbCategory->addItem("Miscellaneous",                       CatMisc);

    ui->cbCategory->setStyleSheet("QComboBox { combobox-popup: 0; }");
    ui->cbCategory->view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
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
    validator65535 = new QIntValidator(0, 65535, this);

    ui->lineEditCommand->setReadOnly(true);

    // Handle category ComboBox
    connect(ui->cbCategory, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorAddCmdWindow::handleCategoryChanged);

    // Handle function ComboBox
    connect(ui->cbFunction, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorAddCmdWindow::handleFunctionChanged);

    // Clear error style if text in lineEditParameter has changed
    connect(ui->lineEditParameter1, &QLineEdit::textChanged, this, &EditorAddCmdWindow::clearErrorStyle);
    connect(ui->lineEditParameter2, &QLineEdit::textChanged, this, &EditorAddCmdWindow::clearErrorStyle);
    connect(ui->lineEditParameter3, &QLineEdit::textChanged, this, &EditorAddCmdWindow::clearErrorStyle);
    connect(ui->lineEditParameter4, &QLineEdit::textChanged, this, &EditorAddCmdWindow::clearErrorStyle);
    connect(ui->lineEditParameter5, &QLineEdit::textChanged, this, &EditorAddCmdWindow::clearErrorStyle);

    // Update command text in lineEditCommand
    connect(ui->cbFunction, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorAddCmdWindow::updateCommandDisplay);

    // Update parameter 1 text in lineEditCommand
    connect(ui->cbParameter1, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorAddCmdWindow::updateCommandDisplay);
    connect(ui->lineEditParameter1, &QLineEdit::textChanged, this, &EditorAddCmdWindow::updateCommandDisplay);

    // Update parameter 2 text in lineEditCommand
    connect(ui->cbParameter2, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorAddCmdWindow::updateCommandDisplay);
    connect(ui->lineEditParameter2, &QLineEdit::textChanged, this, &EditorAddCmdWindow::updateCommandDisplay);

    // Update parameter 3 text in lineEditCommand
    connect(ui->cbParameter3, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorAddCmdWindow::updateCommandDisplay);
    connect(ui->lineEditParameter3, &QLineEdit::textChanged, this, &EditorAddCmdWindow::updateCommandDisplay);

    // Update parameter 4 text in lineEditCommand
    connect(ui->cbParameter4, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorAddCmdWindow::updateCommandDisplay);
    connect(ui->lineEditParameter4, &QLineEdit::textChanged, this, &EditorAddCmdWindow::updateCommandDisplay);

    // Update parameter 5 text in lineEditCommand
    connect(ui->cbParameter5, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorAddCmdWindow::updateCommandDisplay);
    connect(ui->lineEditParameter5, &QLineEdit::textChanged, this, &EditorAddCmdWindow::updateCommandDisplay);

    // Set last selected category and function
    ui->cbCategory->blockSignals(true);
    ui->cbFunction->blockSignals(true);

    int targetCategory = lastSelectedCategory;
    int targetFunction = lastSelectedFunction;

    ui->cbCategory->setCurrentIndex(targetCategory);
    handleCategoryChanged(targetCategory);

    if (targetFunction >= 0 && targetFunction < ui->cbFunction->count())
    {
        ui->cbFunction->setCurrentIndex(targetFunction);
    }
    else
    {
        ui->cbFunction->setCurrentIndex(0);
    }

    handleFunctionChanged(ui->cbFunction->currentIndex());

    ui->cbCategory->blockSignals(false);
    ui->cbFunction->blockSignals(false);

    // Update command display
    updateCommandDisplay();

    adjustSize();

    GuiUtilities::centerWidgetOnScreen(this);

    this->setFixedSize(this->size());
}

EditorAddCmdWindow::~EditorAddCmdWindow()
{
    delete ui;
}

// Get command
FunctionCommand EditorAddCmdWindow::getCommand() const
{
    FunctionCommand cmd;
    cmd.type = static_cast<CommandType>(ui->cbFunction->currentData().toInt());

    switch (cmd.type)
    {
    case CmdComOpen:
    case CmdBlamconCPO:
    case CmdFusionCPO:
    case CmdGun4irCPO:
    case CmdOpenFireCPO:
    case CmdRsMX24CPO:
    case CmdRsReaperCPO:
    case CmdXgunnerCPO:
        cmd.commandCode = COMPORTOPEN;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->lineEditParameter2->text();
        break;

    case CmdComClose:
    case CmdBlamconCPC:
    case CmdFusionCPC:
    case CmdGun4irCPC:
    case CmdOpenFireCPC:
    case CmdRsMX24CPC:
    case CmdRsReaperCPC:
    case CmdXgunnerCPC:
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

    case CmdBlamconSSM:
        cmd.commandCode = COMPORTWRITE;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentData().toString() + ui->cbParameter3->currentData().toString();
        break;

    case CmdBlamconESM:
    case CmdBlamconIM:
    case CmdBlamconOR:
    case CmdBlamconPM:
    case CmdBlamconAR:
    case CmdBlamconRM:
    case CmdBlamconCP:
        cmd.commandCode = COMPORTWRITE;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentData().toString();
        break;

    case CmdBlamconIGF:
        cmd.commandCode = COMPORTWRITE;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentData().toString();
        cmd.param3 = ui->cbParameter3->currentData().toString();
        cmd.param4 = ui->lineEditParameter4->text();
        cmd.param5 = ui->cbParameter5->currentData().toString();
        break;

    case CmdFusionSSM:
    case CmdFusionESM:
    case CmdFusionFM:
    case CmdFusionJM:
    case CmdFusionPM:
        cmd.commandCode = COMPORTWRITE;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentData().toString();
        break;

    case CmdFusionIGF:
        cmd.commandCode = COMPORTWRITE;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentData().toString();
        cmd.param3 = ui->cbParameter3->currentData().toString();
        cmd.param4 = ui->lineEditParameter4->text();
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

    case CmdGun4irIGF:
        cmd.commandCode = COMPORTWRITE;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentData().toString();
        cmd.param3 = ui->cbParameter3->currentData().toString();
        cmd.param4 = ui->lineEditParameter4->text();
        break;

    case CmdOpenFireSSM:
    case CmdOpenFireESM:
    case CmdOpenFireIM:
    case CmdOpenFireOR:
    case CmdOpenFirePM:
    case CmdOpenFireAR:
    case CmdOpenFireRO:
    case CmdOpenFireFA:
    case CmdOpenFireDM:
        cmd.commandCode = COMPORTWRITE;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentData().toString();
        break;

    case CmdOpenFirePO:
    case CmdOpenFireIGF:
        cmd.commandCode = COMPORTWRITE;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentData().toString();
        cmd.param3 = ui->cbParameter3->currentData().toString();
        cmd.param4 = ui->lineEditParameter4->text();
        break;

    case CmdRsMX24SSM:
    case CmdRsMX24ESM:
    case CmdRsMX24SCM:
    case CmdRsMX24IGF:
    case CmdRsReaperSSM:
    case CmdRsReaperESM:
    case CmdRsReaperIM:
    case CmdRsReaperOR:
    case CmdRsReaperAR:
    case CmdRsReaperLA:
        cmd.commandCode = COMPORTWRITE;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentData().toString();
        break;

    case CmdRsReaperIGF:
        cmd.commandCode = COMPORTWRITE;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentData().toString();
        cmd.param3 = ui->cbParameter3->currentData().toString();
        break;

    case CmdSindenIGF:
    case CmdSindenRM:
    case CmdSindenAP:
    case CmdSindenSS:
        cmd.commandCode = TCPSOCKETSEND;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentData().toString();
        cmd.param3 = ui->cbParameter3->currentData().toString();
        cmd.param4 = ui->lineEditParameter4->text();
        break;

    case CmdXgunnerSSM:
    case CmdXgunnerESM:
    case CmdXgunnerIM:
    case CmdXgunnerAR:
    case CmdXgunnerIGF:
        cmd.commandCode = COMPORTWRITE;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentData().toString();
        break;

    case CmdAlienLED:
        cmd.commandCode = USBHIDCMD;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->lineEditParameter2->text();
        cmd.param3 = ui->lineEditParameter3->text();
        cmd.param4 = ui->lineEditParameter4->text();
        cmd.param5 = ui->lineEditParameter5->text();
        break;

    case CmdAlienRecoil:
        cmd.commandCode = USBHIDCMD;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->lineEditParameter2->text();
        cmd.param3 = ui->lineEditParameter3->text();
        cmd.param4 = ui->lineEditParameter4->text();
        cmd.param5 = ui->cbParameter5->currentData().toString();
        break;

    case CmdAlienCounter:
        cmd.commandCode = USBHIDCMD;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->lineEditParameter2->text();
        cmd.param3 = ui->lineEditParameter3->text();
        cmd.param4 = ui->lineEditParameter4->text();
        cmd.param5 = ui->cbParameter4->currentData().toString() + ui->cbParameter5->currentData().toString();
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

    case CmdUltimarcFadeTime:
        cmd.commandCode = PACSETFADETIME;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->lineEditParameter2->text();
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

    case CmdSDL3FFB:
        cmd.commandCode = SDL3FFB;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentData().toString();
        break;

    case CmdSDL3FFA:
        cmd.commandCode = SDL3FFA;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->cbParameter2->currentData().toString();
        cmd.param3 = ui->lineEditParameter3->text();
        cmd.param4 = ui->lineEditParameter4->text();
        cmd.param5 = ui->lineEditParameter5->text();
        break;

    case CmdUsbHidSend:
        cmd.commandCode = USBHIDCMD;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->lineEditParameter2->text();
        cmd.param3 = ui->lineEditParameter3->text();
        cmd.param4 = ui->lineEditParameter4->text();
        cmd.param5 = ui->lineEditParameter5->text();
        break;

    case CmdTcpConnect:
    case CmdSindenTSC:
        cmd.commandCode = TCPSOCKETCONNECT;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->lineEditParameter2->text();
        cmd.param3 = ui->lineEditParameter3->text();
        break;

    case CmdTcpDisconnect:
    case CmdSindenTSD:
        cmd.commandCode = TCPSOCKETDISCONNECT;
        cmd.param1 = ui->cbParameter1->currentText();
        break;

    case CmdTcpSend:
        cmd.commandCode = TCPSOCKETSEND;
        cmd.param1 = ui->cbParameter1->currentText();
        cmd.param2 = ui->lineEditParameter2->text();
        break;

    case CmdUdpSend:
        cmd.commandCode = UDPSOCKETSEND;
        cmd.param1 = ui->cbParameter1->currentData().toString();
        cmd.param2 = ui->lineEditParameter2->text();
        cmd.param3 = ui->lineEditParameter3->text();
        cmd.param4 = ui->lineEditParameter4->text();
        break;

    case CmdHprSend:
        cmd.commandCode = HTTPPOSTREQUEST;
        cmd.param1 = ui->lineEditParameter1->text();
        cmd.param2 = ui->lineEditParameter2->text();
        cmd.param3 = ui->lineEditParameter3->text();
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

    case CmdNull:
        cmd.commandCode = NULLCMD;
        break;
    }

    return cmd;
}

// Update command text in lineEditCommand
void EditorAddCmdWindow::updateCommandDisplay()
{
    FunctionCommand cmd = getCommand();
    QString command;
    QStringList parts;

    if (cmd.type == CmdBlamconIGF)
    {
        command = cmd.commandCode + " " + cmd.param1 + " ";

        if (!cmd.param2.isEmpty()) parts << cmd.param2;
        if (!cmd.param3.isEmpty()) parts << cmd.param3;
        if (!cmd.param4.isEmpty()) parts << cmd.param4;
        if (!cmd.param5.isEmpty()) parts << cmd.param5;

        command += parts.join(".");
    }
    else if (cmd.type == CmdRsReaperIGF)
    {
        command = cmd.commandCode + " " + cmd.param1 + " ";

        if (!cmd.param2.isEmpty()) parts << cmd.param2;
        if (!cmd.param3.isEmpty()) parts << cmd.param3;

        command += parts.join("");
    }
    else if (cmd.type == CmdFusionIGF || cmd.type == CmdGun4irIGF || cmd.type == CmdOpenFirePO || cmd.type == CmdOpenFireIGF)
    {
        {
            command = cmd.commandCode + " " + cmd.param1 + " ";

            if (!cmd.param2.isEmpty()) parts << cmd.param2;
            if (!cmd.param3.isEmpty()) parts << cmd.param3;
            if (!cmd.param4.isEmpty()) parts << cmd.param4;

            command += parts.join("x");
        }
    }
    else if (cmd.type == CmdSindenIGF || cmd.type == CmdSindenRM || cmd.type == CmdSindenAP || cmd.type == CmdSindenSS)
    {
        command = cmd.commandCode + " " + cmd.param1 + " ";

        if (!cmd.param2.isEmpty()) parts << cmd.param2;
        if (!cmd.param3.isEmpty()) parts << cmd.param3;
        if (!cmd.param4.isEmpty()) parts << cmd.param4;

        command += parts.join("");
    }
    else
    {
        QString separator = " ";

        if (!cmd.commandCode.isEmpty()) parts << cmd.commandCode;
        if (!cmd.param1.isEmpty())      parts << cmd.param1;
        if (!cmd.param2.isEmpty())      parts << cmd.param2;
        if (!cmd.param3.isEmpty())      parts << cmd.param3;
        if (!cmd.param4.isEmpty())      parts << cmd.param4;
        if (!cmd.param5.isEmpty())      parts << cmd.param5;

        command = parts.join(separator);
    }

    ui->lineEditCommand->setText(command);
    ui->lineEditCommand->setCursorPosition(0);
}

// Clear error style
void EditorAddCmdWindow::clearErrorStyle()
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(sender());
    if (lineEdit)
        lineEdit->setStyleSheet("");
}

// On pushButton Add clicked
void EditorAddCmdWindow::on_pushButtonAdd_clicked()
{
    clearErrorStyle();

    FunctionCommand cmd = getCommand();

    if (checkCommand(cmd))
    {
        lastSelectedCategory = ui->cbCategory->currentIndex();
        lastSelectedFunction = ui->cbFunction->currentIndex();
        accept();
    }
}

// On pushButton Cancel clicked
void EditorAddCmdWindow::on_pushButtonCancel_clicked()
{
    lastSelectedCategory = ui->cbCategory->currentIndex();
    lastSelectedFunction = ui->cbFunction->currentIndex();
    reject();
}

// Event filter for Blamcon LED color setting
bool EditorAddCmdWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->lineEditParameter4 && event->type() == QEvent::MouseButtonDblClick)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        if (mouseEvent->button() == Qt::LeftButton)
        {
            bool ok;
            unsigned int currentColorVal = ui->lineEditParameter4->text().toUInt(&ok);
            QColor initialColor = ok ? QColor((currentColorVal >> 16) & 0xFF, (currentColorVal >> 8) & 0xFF, currentColorVal & 0xFF) : Qt::white;

            QColorDialog dialog(initialColor, this);
            dialog.setWindowTitle("Choose Blamcon LED color");

            GuiUtilities::centerWidgetOnScreen(&dialog);

            if (dialog.exec() == QColorDialog::Accepted)
            {
                QColor selectedColor = dialog.selectedColor();

                unsigned int colorValue = (selectedColor.red() << 16) | (selectedColor.green() << 8) | selectedColor.blue();

                ui->lineEditParameter4->setInputMask("");
                ui->lineEditParameter4->setText(QString::number(colorValue));

                QString style = QString("QLineEdit#%1 { background-color: %2; color: %3; }")
                .arg(ui->lineEditParameter4->objectName(), selectedColor.name(), selectedColor.lightness() > 128 ? "black" : "white");
                ui->lineEditParameter4->setStyleSheet(style);
                ui->lineEditParameter4->setInputMask("99999999");
            }
            ui->lineEditParameter4->clearFocus();
            return true;
        }
    }
    return QDialog::eventFilter(obj, event);
}

// Handle category ComboBox
void EditorAddCmdWindow::handleCategoryChanged(int index)
{
    if (index < 0)
        return;

    lastSelectedCategory = index;

    int categoryData = ui->cbCategory->currentData().toInt();

    ui->cbFunction->blockSignals(true);
    ui->cbFunction->clear();

    auto addCmd = [this](const QString &text, CommandType type)
    {
        ui->cbFunction->addItem(text, type);
    };

    if (categoryData == CatAll || categoryData == CatComPort)
    {
        addCmd("COM Port - Open",                CmdComOpen);
        addCmd("COM Port - Close",               CmdComClose);
        addCmd("COM Port - Write",               CmdComWrite);
        addCmd("COM Port - Set Settings",        CmdComSet);
    }

    if (categoryData == CatAll || categoryData == CatBlamcon)
    {
        addCmd("Blamcon - Open COM Port",        CmdBlamconCPO);
        addCmd("Blamcon - Close COM Port",       CmdBlamconCPC);
        addCmd("Blamcon - Start Serial Mode",    CmdBlamconSSM);
        addCmd("Blamcon - Exit Serial Mode",     CmdBlamconESM);
        addCmd("Blamcon - Input Mode",           CmdBlamconIM);
        addCmd("Blamcon - Offscreen Reload",     CmdBlamconOR);
        addCmd("Blamcon - Pedal Mode",           CmdBlamconPM);
        addCmd("Blamcon - Aspect Ratio",         CmdBlamconAR);
        addCmd("Blamcon - Recoil Mode",          CmdBlamconRM);
        addCmd("Blamcon - Calibration Profile",  CmdBlamconCP);
        addCmd("Blamcon - In-Game Feedback",     CmdBlamconIGF);
    }

    if (categoryData == CatAll || categoryData == CatFusion)
    {
        addCmd("Fusion - Open COM Port",         CmdFusionCPO);
        addCmd("Fusion - Close COM Port",        CmdFusionCPC);
        addCmd("Fusion - Start Serial Mode",     CmdFusionSSM);
        addCmd("Fusion - Exit Serial Mode",      CmdFusionESM);
        addCmd("Fusion - Fire Mode",             CmdFusionFM);
        addCmd("Fusion - Joystick Mode",         CmdFusionJM);
        addCmd("Fusion - Player Mapping",        CmdFusionPM);
        addCmd("Fusion - In-Game Feedback",      CmdFusionIGF);
    }

    if (categoryData == CatAll || categoryData == CatGun4ir)
    {
        addCmd("GUN4IR - Open COM Port",         CmdGun4irCPO);
        addCmd("GUN4IR - Close COM Port",        CmdGun4irCPC);
        addCmd("GUN4IR - Start Serial Mode",     CmdGun4irSSM);
        addCmd("GUN4IR - Exit Serial Mode",      CmdGun4irESM);
        addCmd("GUN4IR - Input Mode",            CmdGun4irIM);
        addCmd("GUN4IR - Offscreen Reload",      CmdGun4irOR);
        addCmd("GUN4IR - Pedal Mode",            CmdGun4irPM);
        addCmd("GUN4IR - Aspect Ratio",          CmdGun4irAR);
        addCmd("GUN4IR - Temperature Sensor",    CmdGun4irTS);
        addCmd("GUN4IR - Auto Reload",           CmdGun4irRL);
        addCmd("GUN4IR - Rumble Only",           CmdGun4irRO);
        addCmd("GUN4IR - Full Auto",             CmdGun4irFA);
        addCmd("GUN4IR - In-Game Feedback",      CmdGun4irIGF);
    }

    if (categoryData == CatAll || categoryData == CatOpenFire)
    {
        addCmd("OpenFIRE - Open COM Port",       CmdOpenFireCPO);
        addCmd("OpenFIRE - Close COM Port",      CmdOpenFireCPC);
        addCmd("OpenFIRE - Start Serial Mode",   CmdOpenFireSSM);
        addCmd("OpenFIRE - Exit Serial Mode",    CmdOpenFireESM);
        addCmd("OpenFIRE - Input Mode",          CmdOpenFireIM);
        addCmd("OpenFIRE - Offscreen Reload",    CmdOpenFireOR);
        addCmd("OpenFIRE - Pedal Mode",          CmdOpenFirePM);
        addCmd("OpenFIRE - Aspect Ratio",        CmdOpenFireAR);
        addCmd("OpenFIRE - Rumble Only",         CmdOpenFireRO);
        addCmd("OpenFIRE - Full Auto",           CmdOpenFireFA);
        addCmd("OpenFIRE - Display Mode",        CmdOpenFireDM);
        addCmd("OpenFIRE - Pulse Override",      CmdOpenFirePO);
        addCmd("OpenFIRE - In-Game Feedback",    CmdOpenFireIGF);
    }

    if (categoryData == CatAll || categoryData == CatRsMX24)
    {
        addCmd("MX24 - Open COM Port",           CmdRsMX24CPO);
        addCmd("MX24 - Close COM Port",          CmdRsMX24CPC);
        addCmd("MX24 - Start Serial Mode",       CmdRsMX24SSM);
        addCmd("MX24 - Exit Serial Mode",        CmdRsMX24ESM);
        addCmd("MX24 - Self Control Mode",       CmdRsMX24SCM);
        addCmd("MX24 - In-Game Feedback",        CmdRsMX24IGF);
    }

    if (categoryData == CatAll || categoryData == CatRsReaper)
    {
        addCmd("RS3 Reaper - Open COM Port",     CmdRsReaperCPO);
        addCmd("RS3 Reaper - Close COM Port",    CmdRsReaperCPC);
        addCmd("RS3 Reaper - Start Serial Mode", CmdRsReaperSSM);
        addCmd("RS3 Reaper - Exit Serial Mode",  CmdRsReaperESM);
        addCmd("RS3 Reaper - Input Mode",        CmdRsReaperIM);
        addCmd("RS3 Reaper - Offscreen Reload",  CmdRsReaperOR);
        addCmd("RS3 Reaper - Aspect Ratio",      CmdRsReaperAR);
        addCmd("RS3 Reaper - LED Auto Control",  CmdRsReaperLA);
        addCmd("RS3 Reaper - In-Game Feedback",  CmdRsReaperIGF);
    }

    if (categoryData == CatAll || categoryData == CatSinden)
    {
        addCmd("Sinden - Connect",               CmdSindenTSC);
        addCmd("Sinden - Disconnect",            CmdSindenTSD);
        addCmd("Sinden - In-Game Feedback",      CmdSindenIGF);
        addCmd("Sinden - Set Recoil Mode",       CmdSindenRM);
        addCmd("Sinden - Set Automatic Preset",  CmdSindenAP);
        addCmd("Sinden - Set Settings",          CmdSindenSS);
    }

    if (categoryData == CatAll || categoryData == CatXgunner)
    {
        addCmd("X-Gunner - Open COM Port",       CmdXgunnerCPO);
        addCmd("X-Gunner - Close COM Port",      CmdXgunnerCPC);
        addCmd("X-Gunner - Start Serial Mode",   CmdXgunnerSSM);
        addCmd("X-Gunner - Exit Serial Mode",    CmdXgunnerESM);
        addCmd("X-Gunner - Input Mode",          CmdXgunnerIM);
        addCmd("X-Gunner - Aspect Ratio",        CmdXgunnerAR);
        addCmd("X-Gunner - In-Game Feedback",    CmdXgunnerIGF);
    }

    if (categoryData == CatAll || categoryData == CatAlien)
    {
        addCmd("Alien Gun - Enable LEDs",        CmdAlienLED);
        addCmd("Alien Gun - Recoil State",       CmdAlienRecoil);
        addCmd("Alien Gun - Digit Counter",      CmdAlienCounter);
    }

    if (categoryData == CatAll || categoryData == CatLedWiz)
    {
        addCmd("LEDWiz - Set Pin State",         CmdLedWizState);
        addCmd("LEDWiz - Set Power Level",       CmdLedWizPower);
        addCmd("LEDWiz - Set RGB LED Color",     CmdLedWizColor);
        addCmd("LEDWiz - Set Pulse Rate",        CmdLedWizPulse);
        addCmd("LEDWiz - Kill All LEDs",         CmdLedWizKill);
    }

    if (categoryData == CatAll || categoryData == CatUltimarc)
    {
        addCmd("Ultimarc - Set LED State",       CmdUltimarcState);
        addCmd("Ultimarc - Set LED Intensity",   CmdUltimarcIntensity);
        addCmd("Ultimarc - Set LED Fade Time",   CmdUltimarcFadeTime);
        addCmd("Ultimarc - Set RGB LED Color",   CmdUltimarcColor);
        addCmd("Ultimarc - Kill All LEDs",       CmdUltimarcKill);
    }

    if (categoryData == CatAll || categoryData == CatSDL3FFB)
    {
        addCmd("Force Feedback",                 CmdSDL3FFB);
        addCmd("Force Feedback Advanced",        CmdSDL3FFA);
    }

    if (categoryData == CatAll || categoryData == CatUsbHid)
    {
        addCmd("Generic HID - Send Data",        CmdUsbHidSend);
    }

    if (categoryData == CatAll || categoryData == CatNetwork)
    {
        addCmd("TCP - Connect",                  CmdTcpConnect);
        addCmd("TCP - Disconnect",               CmdTcpDisconnect);
        addCmd("TCP - Send Command",             CmdTcpSend);
        addCmd("UDP - Send Command",             CmdUdpSend);
        addCmd("HTTP POST - Send Request",       CmdHprSend);
    }

    if (categoryData == CatAll || categoryData == CatApplication)
    {
        addCmd("Launch Application",             CmdAppLaunch);
        addCmd("Close Application",              CmdAppClose);
    }

    if (categoryData == CatAll || categoryData == CatAudio)
    {
        addCmd("Play WAV Audio File",            CmdPlayWav);
    }

    if (categoryData == CatAll || categoryData == CatMisc)
    {
        addCmd("Null Command",                   CmdNull);
    }

    ui->cbFunction->blockSignals(false);

    if (ui->cbFunction->count() > 0)
    {
        handleFunctionChanged(ui->cbFunction->currentIndex());
    }
}

// Handle function ComboBox
void EditorAddCmdWindow::handleFunctionChanged(int index)
{
    if (index < 0)
        return;

    lastSelectedFunction = index;

    CommandType cmd = static_cast<CommandType>(ui->cbFunction->itemData(index).toInt());
    resetInputs();

    switch (cmd)
    {
    case CmdComOpen:
    case CmdComClose:
    case CmdComWrite:
    case CmdComSet:
    case CmdBlamconCPO:
    case CmdBlamconCPC:
    case CmdFusionCPO:
    case CmdFusionCPC:
    case CmdGun4irCPO:
    case CmdGun4irCPC:
    case CmdOpenFireCPO:
    case CmdOpenFireCPC:
    case CmdRsMX24CPO:
    case CmdRsMX24CPC:
    case CmdRsReaperCPO:
    case CmdRsReaperCPC:
    case CmdXgunnerCPO:
    case CmdXgunnerCPC:
        setupComPortUI(cmd);
        break;

    case CmdBlamconSSM:
    case CmdBlamconESM:
    case CmdBlamconIM:
    case CmdBlamconOR:
    case CmdBlamconPM:
    case CmdBlamconAR:
    case CmdBlamconRM:
    case CmdBlamconCP:
    case CmdBlamconIGF:
        setupBlamconUI(cmd);
        break;

    case CmdFusionSSM:
    case CmdFusionESM:
    case CmdFusionFM:
    case CmdFusionJM:
    case CmdFusionPM:
    case CmdFusionIGF:
        setupFusionUI(cmd);
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
    case CmdGun4irIGF:
        setupGun4irUI(cmd);
        break;

    case CmdOpenFireSSM:
    case CmdOpenFireESM:
    case CmdOpenFireIM:
    case CmdOpenFireOR:
    case CmdOpenFirePM:
    case CmdOpenFireAR:
    case CmdOpenFireRO:
    case CmdOpenFireFA:
    case CmdOpenFireDM:
    case CmdOpenFirePO:
    case CmdOpenFireIGF:
        setupOpenFireUI(cmd);
        break;

    case CmdRsMX24SSM:
    case CmdRsMX24ESM:
    case CmdRsMX24SCM:
    case CmdRsMX24IGF:
        setupRsMX24UI(cmd);
        break;

    case CmdRsReaperSSM:
    case CmdRsReaperESM:
    case CmdRsReaperIM:
    case CmdRsReaperOR:
    case CmdRsReaperAR:
    case CmdRsReaperLA:
    case CmdRsReaperIGF:
        setupRsReaperUI(cmd);
        break;

    case CmdSindenIGF:
    case CmdSindenRM:
    case CmdSindenAP:
    case CmdSindenSS:
        setupSindenUI(cmd);
        break;

    case CmdXgunnerSSM:
    case CmdXgunnerESM:
    case CmdXgunnerIM:
    case CmdXgunnerAR:
    case CmdXgunnerIGF:
        setupXgunnerUI(cmd);
        break;

    case CmdAlienLED:
    case CmdAlienRecoil:
    case CmdAlienCounter:
        setupAlienUI(cmd);
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
    case CmdUltimarcFadeTime:
    case CmdUltimarcColor:
    case CmdUltimarcKill:
        setupUltimarcUI(cmd);
        break;

    case CmdSDL3FFB:
    case CmdSDL3FFA:
        setupSDL3UI(cmd);
        break;

    case CmdUsbHidSend:
        setupUsbHidUI(cmd);
        break;

    case CmdTcpConnect:
    case CmdTcpDisconnect:
    case CmdTcpSend:
    case CmdSindenTSC:
    case CmdSindenTSD:
        setupTcpUI(cmd);
        break;

    case CmdUdpSend:
        setupUdpUI(cmd);
        break;

    case CmdHprSend:
        setupHttpUI(cmd);
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
void EditorAddCmdWindow::setupComPortUI(CommandType cmd)
{
    setParamLabelVisibility(true, false, false, false, false);
    setParamComboBoxVisibility(true, false, false, false, false);
    setParamLineEditVisibility(false, false, false, false, false);
    ui->labelParameter1->setFixedWidth(45);
    ui->labelParameter1->setText("Port:");
    ui->cbParameter1->setFixedWidth(45);
    for (int i = 1; i <= MAXCOMPORTS; ++i)
        ui->cbParameter1->addItem(QString::number(i));

    if (cmd == CmdComOpen || cmd == CmdComWrite || cmd == CmdComSet || cmd == CmdBlamconCPO || cmd == CmdFusionCPO || cmd == CmdGun4irCPO || cmd == CmdOpenFireCPO || cmd == CmdRsMX24CPO || cmd == CmdRsReaperCPO || cmd == CmdXgunnerCPO)
    {
        setParamLabelVisibility(true, true, false, false, false);
        setParamLineEditVisibility(false, true, false, false, false);

        if (cmd == CmdComOpen || cmd == CmdComSet || cmd == CmdBlamconCPO || cmd == CmdFusionCPO || cmd == CmdGun4irCPO || cmd == CmdOpenFireCPO || cmd == CmdXgunnerCPO)
        {
            ui->labelParameter2->setText("Settings:");
            ui->lineEditParameter2->setText("baud=9600_parity=N_data=8_stop=1");
        }
        else if (cmd == CmdRsMX24CPO || cmd == CmdRsReaperCPO)
        {
            ui->labelParameter2->setText("Settings:");
            ui->lineEditParameter2->setText("baud=115200_parity=N_data=8_stop=1");
        }
        else if (cmd == CmdComWrite)
        {
            ui->labelParameter2->setText("Data:");
            ui->lineEditParameter2->setText("S6M3x0M4x1");
        }
    }
}

// Setup UI - Blamcon functions
void EditorAddCmdWindow::setupBlamconUI(CommandType cmd)
{
    setParamLabelVisibility(true, true, false, false, false);
    setParamComboBoxVisibility(true, true, false, false, false);
    setParamLineEditVisibility(false, false, false, false, false);
    ui->labelParameter1->setFixedWidth(45);
    ui->labelParameter1->setText("Port:");
    ui->cbParameter1->setFixedWidth(45);
    for (int i = 1; i <= MAXCOMPORTS; ++i)
        ui->cbParameter1->addItem(QString::number(i));

    if (cmd == CmdBlamconIGF)
    {
        setParamLabelVisibility(true, true, true, false, false);
        setParamComboBoxVisibility(true, true, true, false, false);
        ui->labelParameter2->setFixedWidth(75);
        ui->labelParameter2->setText("Feedback:");
        ui->cbParameter2->setFixedWidth(75);
        ui->cbParameter2->addItem("Recoil", "FB.0");
        ui->cbParameter2->addItem("Rumble", "FB.1");
        ui->cbParameter2->addItem("LED", "FB.2");
        ui->labelParameter3->setFixedWidth(75);
        ui->labelParameter3->setText("State:");
        ui->cbParameter3->setFixedWidth(75);
        ui->cbParameter3->addItem("State", SIGNALDATAVARIABLE);
        ui->cbParameter3->addItem("Off", "0");
        ui->cbParameter3->addItem("On", "1");

        connect(ui->cbParameter2, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]()
        {
            ui->cbParameter3->blockSignals(true);
            ui->cbParameter3->clear();
            ui->cbParameter3->addItem("State", SIGNALDATAVARIABLE);
            ui->cbParameter3->addItem("Off", "0");
            ui->cbParameter3->addItem("On", "1");

            QString currentText = ui->cbParameter2->currentText();
            bool isLed = (currentText == "LED");

            if (currentText == "Recoil")
            {
                ui->cbParameter3->addItem("Pulse 2x", "1.2");
            }
            else if (currentText == "Rumble")
            {
                ui->cbParameter3->addItem("Pulse 2x", "2");
            }

            ui->cbParameter3->blockSignals(false);

            ui->labelParameter4->setVisible(isLed);
            ui->lineEditParameter4->setVisible(isLed);
            ui->labelParameter5->setVisible(isLed);
            ui->cbParameter5->setVisible(isLed);

            if (isLed)
            {
                ui->labelParameter4->setFixedWidth(70);
                ui->labelParameter4->setText("Color:");
                ui->lineEditParameter4->setFixedWidth(70);
                ui->lineEditParameter4->setInputMask("99999999");
                ui->lineEditParameter4->setToolTip("Double click -> Open color dialog");
                ui->lineEditParameter4->setText("16711680");
                ui->lineEditParameter4->installEventFilter(this);
                ui->labelParameter5->setFixedWidth(60);
                ui->labelParameter5->setText("Pulse:");
                ui->cbParameter5->setFixedWidth(60);
                ui->cbParameter5->blockSignals(true);
                ui->cbParameter5->clear();
                ui->cbParameter5->addItem("None");
                ui->cbParameter5->addItem("1x", "1");
                ui->cbParameter5->addItem("2x", "2");
                ui->cbParameter5->blockSignals(false);

                connect(ui->lineEditParameter4, &QLineEdit::textChanged, this, [this](const QString &text)
                {
                    bool ok;
                    unsigned int colorVal = text.toUInt(&ok);

                    if (ok)
                    {
                        QColor color(qRed(colorVal), qGreen(colorVal), qBlue(colorVal));

                        QString style = QString("QLineEdit#%1 { background-color: %2; color: %3; }")
                        .arg(ui->lineEditParameter4->objectName(), color.name(), (color.lightness() > 128) ? "black" : "white");
                        ui->lineEditParameter4->setStyleSheet(style);
                    }
                });

                ui->lineEditParameter4->textChanged(ui->lineEditParameter4->text());
            }
            else
            {
                ui->lineEditParameter4->clear();
                ui->cbParameter5->clear();
            }

            if (layout())
                layout()->activate();

            adjustSize();
        });

        ui->cbParameter2->currentIndexChanged(ui->cbParameter2->currentIndex());
    }
    else if (cmd == CmdBlamconSSM)
    {
        setParamLabelVisibility(true, true, true, false, false);
        setParamComboBoxVisibility(true, true, true, false, false);
        ui->labelParameter2->setFixedWidth(125);
        ui->labelParameter2->setText("Mode:");
        ui->cbParameter2->setFixedWidth(125);
        ui->cbParameter2->addItem("All Feedbacks", "SM.6.");
        ui->cbParameter2->addItem("Recoil", "SM.0.");
        ui->cbParameter2->addItem("Rumble", "SM.1.");
        ui->cbParameter2->addItem("LED", "SM.2.");
        ui->labelParameter3->setFixedWidth(120);
        ui->labelParameter3->setText("State:");
        ui->cbParameter3->setFixedWidth(120);
        ui->cbParameter3->addItem("Disabled", "0");
        ui->cbParameter3->addItem("Enabled", "1");
    }
    else
    {
        ui->labelParameter2->setFixedWidth(250);
        ui->labelParameter2->setText("Mode:");
        ui->cbParameter2->setFixedWidth(250);

        if (cmd == CmdBlamconESM)
        {
            ui->cbParameter2->addItem("Exit", "ES");
        }
        else if (cmd == CmdBlamconIM)
        {
            ui->cbParameter2->addItem("Mouse/Keyboard", "CM.0.0");
            ui->cbParameter2->addItem("Joystick", "CM.0.1");
        }
        else if (cmd == CmdBlamconOR)
        {
            ui->cbParameter2->addItem("Disabled", "CM.1.0");
            ui->cbParameter2->addItem("Move Offscreen", "CM.1.1");
            ui->cbParameter2->addItem("Shoot Offscreen", "CM.1.2");
        }
        else if (cmd == CmdBlamconPM)
        {
            ui->cbParameter2->addItem("Assignable Key", "CM.2.0");
            ui->cbParameter2->addItem("Right Mouse Button", "CM.2.1");
            ui->cbParameter2->addItem("Middle Mouse Button", "CM.2.2");
        }
        else if (cmd == CmdBlamconAR)
        {
            ui->cbParameter2->addItem("Fullscreen", "CM.3.0");
            ui->cbParameter2->addItem("4:3", "CM.3.1");
        }
        else if (cmd == CmdBlamconRM)
        {
            ui->cbParameter2->addItem("Disabled", "CM.8.0");
            ui->cbParameter2->addItem("Normal", "CM.8.1");
            ui->cbParameter2->addItem("Full Auto", "CM.8.2");
        }
        else if (cmd == CmdBlamconCP)
        {
            ui->cbParameter2->addItem("Profile 1", "CM.12.0");
            ui->cbParameter2->addItem("Profile 2", "CM.12.1");
            ui->cbParameter2->addItem("Profile 3", "CM.12.2");
            ui->cbParameter2->addItem("Profile 4", "CM.12.3");
        }
    }
}

// Setup UI - Fusion functions
void EditorAddCmdWindow::setupFusionUI(CommandType cmd)
{
    setParamLabelVisibility(true, true, false, false, false);
    setParamComboBoxVisibility(true, true, false, false, false);
    setParamLineEditVisibility(false, false, false, false, false);
    ui->labelParameter1->setFixedWidth(45);
    ui->labelParameter1->setText("Port:");
    ui->cbParameter1->setFixedWidth(45);
    for (int i = 1; i <= MAXCOMPORTS; ++i)
        ui->cbParameter1->addItem(QString::number(i));

    if (cmd == CmdFusionIGF)
    {
        setParamLabelVisibility(true, true, true, true, false);
        setParamComboBoxVisibility(true, true, true, false, false);
        setParamLineEditVisibility(false, false, false, true, false);
        ui->labelParameter2->setFixedWidth(90);
        ui->labelParameter2->setText("Feedback:");
        ui->cbParameter2->setFixedWidth(90);
        ui->cbParameter2->addItem("Recoil", "F0");
        ui->cbParameter2->addItem("Rumble", "F1");
        ui->cbParameter2->addItem("Red LED", "F2");
        ui->cbParameter2->addItem("Green LED", "F3");
        ui->cbParameter2->addItem("Blue LED", "F4");
        ui->labelParameter3->setFixedWidth(60);
        ui->labelParameter3->setText("State:");
        ui->cbParameter3->setFixedWidth(60);
        ui->cbParameter3->addItem("State", SIGNALDATAVARIABLE);
        ui->cbParameter3->addItem("Off", "0");
        ui->cbParameter3->addItem("On", "1");
        ui->cbParameter3->addItem("Pulse", "2");
        ui->labelParameter4->setText("Strength/Pulse:");
        ui->lineEditParameter4->setValidator(validator255);
        ui->lineEditParameter4->setToolTip("0 - 255");
        ui->lineEditParameter4->setText("255");
    }
    else
    {
        ui->labelParameter2->setFixedWidth(250);
        ui->labelParameter2->setText("Mode:");
        ui->cbParameter2->setFixedWidth(250);

        if (cmd == CmdFusionSSM)
        {
            ui->cbParameter2->addItem("Start", "S");
        }
        else if (cmd == CmdFusionESM)
        {
            ui->cbParameter2->addItem("Exit", "E");
        }
        else if (cmd == CmdFusionFM)
        {
            ui->cbParameter2->addItem("Single Fire", "MFx0");
            ui->cbParameter2->addItem("Burst Fire", "MFx1");
            ui->cbParameter2->addItem("Auto Fire", "MFx2");
        }
        else if (cmd == CmdFusionJM)
        {
            ui->cbParameter2->addItem("Left Stick", "XAL");
            ui->cbParameter2->addItem("Right Stick", "XAR");
        }
        else if (cmd == CmdFusionPM)
        {
            ui->cbParameter2->addItem("Player 1", "XR1");
            ui->cbParameter2->addItem("Player 2", "XR2");
            ui->cbParameter2->addItem("Player 3", "XR3");
            ui->cbParameter2->addItem("Player 4", "XR4");
        }
    }
}

// Setup UI - GUN4IR functions
void EditorAddCmdWindow::setupGun4irUI(CommandType cmd)
{
    setParamLabelVisibility(true, true, false, false, false);
    setParamComboBoxVisibility(true, true, false, false, false);
    setParamLineEditVisibility(false, false, false, false, false);
    ui->labelParameter1->setFixedWidth(45);
    ui->labelParameter1->setText("Port:");
    ui->cbParameter1->setFixedWidth(45);
    for (int i = 1; i <= MAXCOMPORTS; ++i)
        ui->cbParameter1->addItem(QString::number(i));

    if (cmd == CmdGun4irIGF)
    {
        setParamLabelVisibility(true, true, true, true, false);
        setParamComboBoxVisibility(true, true, true, false, false);
        setParamLineEditVisibility(false, false, false, true, false);
        ui->labelParameter2->setFixedWidth(90);
        ui->labelParameter2->setText("Feedback:");
        ui->cbParameter2->setFixedWidth(90);
        ui->cbParameter2->addItem("Recoil", "F0");
        ui->cbParameter2->addItem("Rumble", "F1");
        ui->cbParameter2->addItem("Red LED", "F2");
        ui->cbParameter2->addItem("Green LED", "F3");
        ui->cbParameter2->addItem("Blue LED", "F4");
        ui->labelParameter3->setFixedWidth(60);
        ui->labelParameter3->setText("State:");
        ui->cbParameter3->setFixedWidth(60);
        ui->cbParameter3->addItem("State", SIGNALDATAVARIABLE);
        ui->cbParameter3->addItem("Off", "0");
        ui->cbParameter3->addItem("On", "1");
        ui->cbParameter3->addItem("Pulse", "2");
        ui->labelParameter4->setText("Strength/Pulse:");
        ui->lineEditParameter4->setValidator(validator255);
        ui->lineEditParameter4->setToolTip("0 - 255");
        ui->lineEditParameter4->setText("255");
    }
    else
    {
        ui->labelParameter2->setFixedWidth(250);
        ui->labelParameter2->setText("Mode:");
        ui->cbParameter2->setFixedWidth(250);

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
            ui->cbParameter2->addItem("Joystick", "M0x1");
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
}

// Setup UI - OpenFIRE functions
void EditorAddCmdWindow::setupOpenFireUI(CommandType cmd)
{
    setParamLabelVisibility(true, true, false, false, false);
    setParamComboBoxVisibility(true, true, false, false, false);
    setParamLineEditVisibility(false, false, false, false, false);
    ui->labelParameter1->setFixedWidth(45);
    ui->labelParameter1->setText("Port:");
    ui->cbParameter1->setFixedWidth(45);
    for (int i = 1; i <= MAXCOMPORTS; ++i)
        ui->cbParameter1->addItem(QString::number(i));

    if (cmd == CmdOpenFirePO)
    {
        setParamLabelVisibility(true, true, true, true, false);
        setParamComboBoxVisibility(true, true, true, false, false);
        setParamLineEditVisibility(false, false, false, true, false);
        ui->labelParameter2->setFixedWidth(90);
        ui->labelParameter2->setText("Feedback:");
        ui->cbParameter2->setFixedWidth(90);
        ui->cbParameter2->addItem("Recoil", "R0");
        ui->cbParameter2->addItem("Rumble", "R1");
        ui->cbParameter2->addItem("Red LED", "R2");
        ui->cbParameter2->addItem("Green LED", "R3");
        ui->cbParameter2->addItem("Blue LED", "R4");
        ui->labelParameter3->setFixedWidth(80);
        ui->labelParameter3->setText("State:");
        ui->cbParameter3->setFixedWidth(80);
        ui->cbParameter3->addItem("Pulse On", "0");
        ui->cbParameter3->addItem("Pulse Off", "1");
        ui->labelParameter4->setText("Length:");
        ui->lineEditParameter4->setToolTip("Milliseconds");
        ui->lineEditParameter4->setText("100");
    }
    else if (cmd == CmdOpenFireIGF)
    {
        setParamLabelVisibility(true, true, true, true, false);
        setParamComboBoxVisibility(true, true, true, false, false);
        setParamLineEditVisibility(false, false, false, true, false);
        ui->labelParameter2->setFixedWidth(90);
        ui->labelParameter2->setText("Feedback:");
        ui->cbParameter2->setFixedWidth(90);
        ui->cbParameter2->addItem("Recoil", "F0");
        ui->cbParameter2->addItem("Rumble", "F1");
        ui->cbParameter2->addItem("Red LED", "F2");
        ui->cbParameter2->addItem("Green LED", "F3");
        ui->cbParameter2->addItem("Blue LED", "F4");
        ui->cbParameter2->addItem("Ammo Count", "FDA");
        ui->cbParameter2->addItem("Life Count", "FDL");
        ui->labelParameter3->setFixedWidth(60);
        ui->labelParameter3->setText("State:");
        ui->cbParameter3->setFixedWidth(60);
        ui->cbParameter3->addItem("State", SIGNALDATAVARIABLE);
        ui->cbParameter3->addItem("Off", "0");
        ui->cbParameter3->addItem("On", "1");
        ui->cbParameter3->addItem("Pulse", "2");
        ui->labelParameter4->setText("Strength/Pulse:");
        ui->lineEditParameter4->setValidator(validator255);
        ui->lineEditParameter4->setToolTip("0 - 255");
        ui->lineEditParameter4->setText("255");
    }
    else
    {
        ui->labelParameter2->setFixedWidth(250);
        ui->labelParameter2->setText("Mode:");
        ui->cbParameter2->setFixedWidth(250);

        if (cmd == CmdOpenFireSSM)
        {
            ui->cbParameter2->addItem("All Feedbacks", "S6");
            ui->cbParameter2->addItem("Recoil", "S0");
            ui->cbParameter2->addItem("Rumble", "S1");
            ui->cbParameter2->addItem("Red LED", "S2");
            ui->cbParameter2->addItem("Green LED", "S3");
            ui->cbParameter2->addItem("Blue LED", "S4");
        }
        else if (cmd == CmdOpenFireESM)
        {
            ui->cbParameter2->addItem("Exit", "E");
        }
        else if (cmd == CmdOpenFireIM)
        {
            ui->cbParameter2->addItem("Mouse/Keyboard", "M0x0");
            ui->cbParameter2->addItem("Joystick Left", "M0x1L");
            ui->cbParameter2->addItem("Joystick Right", "M0x1");
            ui->cbParameter2->addItem("Hybrid", "M0x2");
        }
        else if (cmd == CmdOpenFireOR)
        {
            ui->cbParameter2->addItem("Reload Button", "M1x2");
            ui->cbParameter2->addItem("Normal Shot", "M1x3");
        }
        else if (cmd == CmdOpenFirePM)
        {
            ui->cbParameter2->addItem("Separate Button", "M2x0");
            ui->cbParameter2->addItem("Right Mouse Button", "M2x1");
            ui->cbParameter2->addItem("Middle Mouse Button", "M2x2");
        }
        else if (cmd == CmdOpenFireAR)
        {
            ui->cbParameter2->addItem("Fullscreen", "M3x0");
            ui->cbParameter2->addItem("4:3", "M3x1");
        }
        else if (cmd == CmdOpenFireRO)
        {
            ui->cbParameter2->addItem("Disabled", "M6x0");
            ui->cbParameter2->addItem("Enabled", "M6x1");
        }
        else if (cmd == CmdOpenFireFA)
        {
            ui->cbParameter2->addItem("Disabled", "M8x0");
            ui->cbParameter2->addItem("Automatic", "M8x1");
            ui->cbParameter2->addItem("Always On", "M8x2");
        }
        else if (cmd == CmdOpenFireDM)
        {
            ui->cbParameter2->addItem("Life Only", "MDx1");
            ui->cbParameter2->addItem("Ammo Only", "MDx2");
            ui->cbParameter2->addItem("Life & Ammo", "MDx3");
            ui->cbParameter2->addItem("Life Bar", "MDx3B");
        }
    }
}

// Setup UI - Retro Shooter MX24 functions
void EditorAddCmdWindow::setupRsMX24UI(CommandType cmd)
{
    setParamLabelVisibility(true, true, false, false, false);
    setParamComboBoxVisibility(true, true, false, false, false);
    setParamLineEditVisibility(false, false, false, false, false);
    ui->labelParameter1->setFixedWidth(45);
    ui->labelParameter1->setText("Port:");
    ui->cbParameter1->setFixedWidth(45);
    for (int i = 1; i <= MAXCOMPORTS; ++i)
        ui->cbParameter1->addItem(QString::number(i));

    if (cmd == CmdRsMX24IGF)
    {
        ui->labelParameter2->setFixedWidth(250);
        ui->labelParameter2->setText("Feedback:");
        ui->cbParameter2->setFixedWidth(250);
        ui->cbParameter2->addItem("Recoil - Player 1/3", "0E");
        ui->cbParameter2->addItem("Recoil - Player 2/4", "aF");
    }
    else
    {
        ui->labelParameter2->setFixedWidth(250);
        ui->labelParameter2->setText("Mode:");
        ui->cbParameter2->setFixedWidth(250);

        if (cmd == CmdRsMX24SSM)
        {
            ui->cbParameter2->addItem("Player 1/3", "0E");
            ui->cbParameter2->addItem("Player 2/4", "aF");
        }
        else if (cmd == CmdRsMX24ESM)
        {
            ui->cbParameter2->addItem("Player 1/3 & Player 2/4", "0C");
            ui->cbParameter2->addItem("Player 1/3", "0A");
            ui->cbParameter2->addItem("Player 2/4", "aB");
        }
        else if (cmd == CmdRsMX24SCM)
        {
            ui->cbParameter2->addItem("Disable Player 1/3", "0I");
            ui->cbParameter2->addItem("Disable Player 2/4", "aJ");
        }
    }
}

// Setup UI - Retro Shooter Reaper functions
void EditorAddCmdWindow::setupRsReaperUI(CommandType cmd)
{
    setParamLabelVisibility(true, true, false, false, false);
    setParamComboBoxVisibility(true, true, false, false, false);
    setParamLineEditVisibility(false, false, false, false, false);
    ui->labelParameter1->setFixedWidth(45);
    ui->labelParameter1->setText("Port:");
    ui->cbParameter1->setFixedWidth(45);
    for (int i = 1; i <= MAXCOMPORTS; ++i)
        ui->cbParameter1->addItem(QString::number(i));

    if (cmd == CmdRsReaperIGF)
    {
        ui->labelParameter2->setFixedWidth(95);
        ui->labelParameter2->setText("Feedback:");
        ui->cbParameter2->setFixedWidth(95);
        ui->cbParameter2->addItem("Recoil/LEDs", "Z");
        ui->cbParameter2->addItem("Rumble", "ZZ");

        connect(ui->cbParameter2, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]()
        {
            QString currentText = ui->cbParameter2->currentText();
            bool isLedRecoil = (currentText == "Recoil/LEDs");

            ui->labelParameter3->setVisible(isLedRecoil);
            ui->cbParameter3->setVisible(isLedRecoil);

            if (isLedRecoil)
            {
                ui->labelParameter3->setFixedWidth(205);
                ui->labelParameter3->setText("State:");
                ui->cbParameter3->setFixedWidth(205);
                ui->cbParameter3->addItem("State", SIGNALDATAVARIABLE);
                ui->cbParameter3->addItem("Barrel slide back and stay", "0");
                ui->cbParameter3->addItem("1 Orange LED & 4 Red LEDs", "1");
                ui->cbParameter3->addItem("2 Orange LEDs & 3 Red LEDs", "2");
                ui->cbParameter3->addItem("3 Orange LEDs & 2 Red LEDs", "3");
                ui->cbParameter3->addItem("4 Orange LEDs & 1 Red LED", "4");
                ui->cbParameter3->addItem("5 Orange LEDs", "5");
                ui->cbParameter3->addItem("5 Orange LEDs & Barrel return", "6");
            }
            else
            {
                ui->cbParameter3->clear();
            }

            if (layout())
                layout()->activate();

            adjustSize();
        });

        ui->cbParameter2->currentIndexChanged(ui->cbParameter2->currentIndex());
    }
    else
    {
        ui->labelParameter2->setFixedWidth(250);
        ui->labelParameter2->setText("Mode:");
        ui->cbParameter2->setFixedWidth(250);

        if (cmd == CmdRsReaperSSM)
        {
            ui->cbParameter2->addItem("Start", "ZS");
        }
        else if (cmd == CmdRsReaperESM)
        {
            ui->cbParameter2->addItem("Exit", "ZX");
        }
        else if (cmd == CmdRsReaperIM)
        {
            ui->cbParameter2->addItem("Mouse/Keyboard", "ZM");
            ui->cbParameter2->addItem("Joystick", "ZJ");
        }
        else if (cmd == CmdRsReaperOR)
        {
            ui->cbParameter2->addItem("Enabled", "ZA");
            ui->cbParameter2->addItem("Disabled", "ZB");
        }
        else if (cmd == CmdRsReaperAR)
        {
            ui->cbParameter2->addItem("Fullscreen", "ZW");
            ui->cbParameter2->addItem("4:3", "ZN");
        }
        else if (cmd == CmdRsReaperLA)
        {
            ui->cbParameter2->addItem("Enabled", "ZR");
        }
    }
}

// Setup UI - Sinden functions
void EditorAddCmdWindow::setupSindenUI(CommandType cmd)
{
    setParamLabelVisibility(true, true, true, false, false);
    setParamComboBoxVisibility(true, true, true, false, false);
    setParamLineEditVisibility(false, false, false, false, false);
    ui->labelParameter1->setFixedWidth(45);
    ui->labelParameter1->setText("Socket:");
    ui->cbParameter1->setFixedWidth(45);
    ui->cbParameter1->addItem("1");
    ui->cbParameter1->addItem("2");
    ui->labelParameter2->setFixedWidth(60);
    ui->labelParameter2->setText("Player:");
    ui->cbParameter2->setFixedWidth(60);
    ui->cbParameter2->addItem("1", "1");
    ui->cbParameter2->addItem("2", "2");
    ui->cbParameter2->addItem("1 & 2", "B");
    ui->labelParameter3->setFixedWidth(185);
    ui->cbParameter3->setFixedWidth(185);

    if (cmd == CmdSindenIGF)
    {
        ui->labelParameter3->setText("Feedback:");
        ui->cbParameter3->addItem("Fire Single Recoil", "A");
        ui->cbParameter3->addItem("Start Repeat Recoil", "B");
        ui->cbParameter3->addItem("Stop Repeat Recoil", "C");
        ui->cbParameter3->addItem("Fire Pulse", "T");
        ui->cbParameter3->addItem("Fire Soft Single Recoil", "U");

        connect(ui->cbParameter3, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]()
        {
            QString currentData = ui->cbParameter3->currentData().toString();
            bool isT = (currentData == "T");
            bool isU = (currentData == "U");

            ui->labelParameter4->setVisible(isT || isU);
            ui->lineEditParameter4->setVisible(isT || isU);

            if (isT)
            {
                ui->labelParameter4->setText("Value:");
                ui->lineEditParameter4->setInputMask("9999");
                ui->lineEditParameter4->setToolTip("0 - 9999");
                ui->lineEditParameter4->setText("2200");
            }
            else if (isU)
            {
                ui->labelParameter4->setText("Strength:");
                ui->lineEditParameter4->setInputMask("99");
                ui->lineEditParameter4->setToolTip("0 - 10");
                ui->lineEditParameter4->setText("5");
            }
            else
            {
                ui->lineEditParameter4->clear();
            }

            if (layout())
                layout()->activate();

            adjustSize();
        });

        ui->cbParameter3->currentIndexChanged(ui->cbParameter3->currentIndex());
    }
    else if (cmd == CmdSindenRM)
    {
        ui->labelParameter3->setText("Mode:");
        ui->cbParameter3->addItem("Single", "D");
        ui->cbParameter3->addItem("Repeat", "E");
        ui->cbParameter3->addItem("Mixed", "F");
    }
    else if (cmd == CmdSindenAP)
    {
        ui->labelParameter3->setText("Preset:");
        ui->cbParameter3->addItem("Default", "G");
        ui->cbParameter3->addItem("Fast", "H");
        ui->cbParameter3->addItem("Strong", "I");
    }
    else if (cmd == CmdSindenSS)
    {
        ui->labelParameter3->setFixedWidth(240);
        ui->labelParameter3->setText("Setting:");
        ui->cbParameter3->setFixedWidth(240);
        ui->cbParameter3->addItem("Reset All Settings", "S");
        ui->cbParameter3->addItem("Disable Recoil", "J0");
        ui->cbParameter3->addItem("Enable Recoil", "J1");
        ui->cbParameter3->addItem("Disable Trigger Recoil", "K0");
        ui->cbParameter3->addItem("Enable Trigger Recoil", "K1");
        ui->cbParameter3->addItem("Disable Pump Action On Event", "L0");
        ui->cbParameter3->addItem("Enable Pump Action On Event", "L1");
        ui->cbParameter3->addItem("Disable Pump Action Off Event", "M0");
        ui->cbParameter3->addItem("Enable Pump Action Off Event", "M1");
        ui->cbParameter3->addItem("Set Single Shot Strength", "N");
        ui->cbParameter3->addItem("Set Automatic Pulse Length", "P");
        ui->cbParameter3->addItem("Set Automatic Delay Between Pulses", "Q");
        ui->cbParameter3->addItem("Set Automatic Delay After First Pulse", "R");

        connect(ui->cbParameter3, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]()
        {
            QString currentData = ui->cbParameter3->currentData().toString();
            bool isN = (currentData == "N");
            bool isP = (currentData == "P");
            bool isQ = (currentData == "Q");
            bool isR = (currentData == "R");

            ui->labelParameter4->setVisible(isN || isP || isQ || isR);
            ui->lineEditParameter4->setVisible(isN || isP || isQ || isR);

            if (isN)
            {
                ui->labelParameter4->setText("Value:");
                ui->lineEditParameter4->setInputMask("99");
                ui->lineEditParameter4->setToolTip("0 - 10");
                ui->lineEditParameter4->setText("5");
            }
            else if (isP)
            {
                ui->labelParameter4->setText("Value:");
                ui->lineEditParameter4->setInputMask("99");
                ui->lineEditParameter4->setToolTip("40 - 80");
                ui->lineEditParameter4->setText("60");
            }
            else if (isQ)
            {
                ui->labelParameter4->setText("Value:");
                ui->lineEditParameter4->setInputMask("99");
                ui->lineEditParameter4->setToolTip("0 - 50");
                ui->lineEditParameter4->setText("25");
            }
            else if (isR)
            {
                ui->labelParameter4->setText("Value:");
                ui->lineEditParameter4->setInputMask("99");
                ui->lineEditParameter4->setToolTip("0 - 16");
                ui->lineEditParameter4->setText("8");
            }
            else
            {
                ui->lineEditParameter4->clear();
            }

            if (layout())
                layout()->activate();

            adjustSize();
        });

        ui->cbParameter3->currentIndexChanged(ui->cbParameter3->currentIndex());
    }
}

// Setup UI - X-Gunner functions
void EditorAddCmdWindow::setupXgunnerUI(CommandType cmd)
{
    setParamLabelVisibility(true, true, false, false, false);
    setParamComboBoxVisibility(true, true, false, false, false);
    setParamLineEditVisibility(false, false, false, false, false);
    ui->labelParameter1->setFixedWidth(45);
    ui->labelParameter1->setText("Port:");
    ui->cbParameter1->setFixedWidth(45);
    for (int i = 1; i <= MAXCOMPORTS; ++i)
        ui->cbParameter1->addItem(QString::number(i));
    ui->labelParameter2->setFixedWidth(250);
    ui->cbParameter2->setFixedWidth(250);

    if (cmd == CmdXgunnerIGF)
    {
        ui->labelParameter2->setText("Feedback:");
        ui->cbParameter2->addItem("Recoil", "F0");
        ui->cbParameter2->addItem("Rumble", "F1");
    }
    else
    {
        ui->labelParameter2->setText("Mode:");

        if (cmd == CmdXgunnerSSM)
        {
            ui->cbParameter2->addItem("Recoil & Rumble", "S6");
        }
        else if (cmd == CmdXgunnerESM)
        {
            ui->cbParameter2->addItem("Exit", "E");
        }
        else if (cmd == CmdXgunnerIM)
        {
            ui->cbParameter2->addItem("Mouse/Keyboard", "G");
            ui->cbParameter2->addItem("Joystick Left", "J");
            ui->cbParameter2->addItem("Joystick Right", "B");
        }
        else if (cmd == CmdXgunnerAR)
        {
            ui->cbParameter2->addItem("Fullscreen", "V");
            ui->cbParameter2->addItem("4:3", "Q");
        }
    }
}

// Setup UI - Alien Positional Gun functions
void EditorAddCmdWindow::setupAlienUI(CommandType cmd)
{
    setParamLabelVisibility(true, false, false, false, false);
    setParamComboBoxVisibility(true, false, false, false, false);
    setParamLineEditVisibility(false, false, false, false, false);
    ui->labelParameter1->setFixedWidth(45);
    ui->labelParameter1->setText("Device:");
    ui->cbParameter1->setFixedWidth(45);
    for (int i = 1; i <= MAXPLAYERS; ++i)
        ui->cbParameter1->addItem(QString::number(i));
    ui->lineEditParameter2->setText("&H04B4");
    ui->lineEditParameter3->setText("&H6870");

    if (cmd == CmdAlienLED)
    {
        ui->lineEditParameter4->setText("8");
        ui->lineEditParameter5->setText("&h04:&h00:&h00:&h00:&h00:&h00:&h00:&h00");
    }
    else if (cmd == CmdAlienRecoil)
    {
        setParamLabelVisibility(true, false, false, false, true);
        setParamComboBoxVisibility(true, false, false, false, true);
        ui->lineEditParameter4->setText("2");
        ui->labelParameter5->setFixedWidth(60);
        ui->labelParameter5->setText("State:");
        ui->cbParameter5->setFixedWidth(60);
        ui->cbParameter5->addItem("State", "&h02:&h0%s%");
        ui->cbParameter5->addItem("Off", "&h02:&h00");
        ui->cbParameter5->addItem("On", "&h02:&h01");
    }
    else if (cmd == CmdAlienCounter)
    {
        setParamLabelVisibility(true, false, false, true, true);
        setParamComboBoxVisibility(true, false, false, true, true);
        ui->lineEditParameter4->setText("3");
        ui->labelParameter4->setFixedWidth(60);
        ui->labelParameter4->setText("Digit 1:");
        ui->cbParameter4->setFixedWidth(60);
        ui->cbParameter4->addItem("State", "&h03:&h0%s%:");
        ui->cbParameter4->addItem("0", "&h03:&h00:");
        ui->cbParameter4->addItem("1", "&h03:&h01:");
        ui->cbParameter4->addItem("2", "&h03:&h02:");
        ui->cbParameter4->addItem("3", "&h03:&h03:");
        ui->cbParameter4->addItem("4", "&h03:&h04:");
        ui->cbParameter4->addItem("5", "&h03:&h05:");
        ui->cbParameter4->addItem("6", "&h03:&h06:");
        ui->cbParameter4->addItem("7", "&h03:&h07:");
        ui->cbParameter4->addItem("8", "&h03:&h08:");
        ui->cbParameter4->addItem("9", "&h03:&h09:");
        ui->labelParameter5->setFixedWidth(60);
        ui->labelParameter5->setText("Digit 2:");
        ui->cbParameter5->setFixedWidth(60);
        ui->cbParameter5->addItem("State", "&h0%s%");
        ui->cbParameter5->addItem("0", "&h00");
        ui->cbParameter5->addItem("1", "&h01");
        ui->cbParameter5->addItem("2", "&h02");
        ui->cbParameter5->addItem("3", "&h03");
        ui->cbParameter5->addItem("4", "&h04");
        ui->cbParameter5->addItem("5", "&h05");
        ui->cbParameter5->addItem("6", "&h06");
        ui->cbParameter5->addItem("7", "&h07");
        ui->cbParameter5->addItem("8", "&h08");
        ui->cbParameter5->addItem("9", "&h09");
    }
}

// Setup UI - LedWiz functions
void EditorAddCmdWindow::setupLedWizUI(CommandType cmd)
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
            ui->cbParameter3->addItem("State", SIGNALDATAVARIABLE);
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
void EditorAddCmdWindow::setupUltimarcUI(CommandType cmd)
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
            ui->cbParameter3->addItem("State", SIGNALDATAVARIABLE);
            ui->cbParameter3->addItem("Off", "0");
            ui->cbParameter3->addItem("On", "1");
        }
        else if (cmd == CmdUltimarcIntensity)
        {
            setParamLineEditVisibility(false, false, true, false, false);
            ui->labelParameter3->setText("Intensity:");
            ui->lineEditParameter3->setValidator(validator255);
            ui->lineEditParameter3->setToolTip("0 - 255");
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
    else if (cmd == CmdUltimarcFadeTime)
    {
        setParamLabelVisibility(true, true, false, false, false);
        setParamLineEditVisibility(false, true, false, false, false);
        ui->labelParameter2->setText("Fade Time:");
        ui->lineEditParameter2->setValidator(validator255);
        ui->lineEditParameter2->setToolTip("0 - 255");
        ui->lineEditParameter2->setText("255");
    }
}

// Setup UI - SDL3 Gamecontroller Force Feedback functions
void EditorAddCmdWindow::setupSDL3UI(CommandType cmd)
{
    setParamLabelVisibility(true, true, false, false, false);
    setParamComboBoxVisibility(true, true, false, false, false);
    setParamLineEditVisibility(false, false, false, false, false);
    ui->labelParameter1->setFixedWidth(45);
    ui->labelParameter1->setText("Device:");
    ui->cbParameter1->setFixedWidth(45);
    for (int i = 1; i <= 16; ++i)
        ui->cbParameter1->addItem(QString::number(i));
    ui->labelParameter2->setFixedWidth(55);
    ui->labelParameter2->setText("State:");
    ui->cbParameter2->setFixedWidth(55);
    ui->cbParameter2->addItem("State", SIGNALDATAVARIABLE);
    ui->cbParameter2->addItem("Off", "0");
    ui->cbParameter2->addItem("On", "1");

    if (cmd == CmdSDL3FFA)
    {
        setParamLabelVisibility(true, true, true, true, true);
        setParamLineEditVisibility(false, false, true, true, true);
        ui->labelParameter3->setFixedWidth(90);
        ui->labelParameter3->setText("Left Strength:");
        ui->lineEditParameter3->setFixedWidth(90);
        ui->lineEditParameter3->setValidator(validator65535);
        ui->lineEditParameter3->setToolTip("0 - 65535");
        ui->lineEditParameter3->setText("65535");
        ui->labelParameter4->setFixedWidth(90);
        ui->labelParameter4->setText("Right Strength:");
        ui->lineEditParameter4->setFixedWidth(90);
        ui->lineEditParameter4->setValidator(validator65535);
        ui->lineEditParameter4->setToolTip("0 - 65535");
        ui->lineEditParameter4->setText("65535");
        ui->labelParameter5->setFixedWidth(60);
        ui->labelParameter5->setText("Duration:");
        ui->lineEditParameter5->setFixedWidth(60);
        ui->lineEditParameter5->setInputMask("99999");
        ui->lineEditParameter5->setToolTip("Milliseconds");
        ui->lineEditParameter5->setText("10000");
    }
}

// Setup UI - Generic HID functions
void EditorAddCmdWindow::setupUsbHidUI(CommandType cmd)
{
    setParamLabelVisibility(true, true, true, true, true);
    setParamComboBoxVisibility(true, false, false, false, false);
    setParamLineEditVisibility(false, true, true, true, true);
    ui->labelParameter1->setFixedWidth(45);
    ui->labelParameter1->setText("Device:");
    ui->cbParameter1->setFixedWidth(45);
    for (int i = 1; i <= MAXPLAYERS; ++i)
        ui->cbParameter1->addItem(QString::number(i));
    ui->labelParameter2->setFixedWidth(65);
    ui->labelParameter2->setText("Vendor ID:");
    ui->lineEditParameter2->setFixedWidth(65);
    ui->lineEditParameter2->setText("&H04B4");
    ui->labelParameter3->setFixedWidth(65);
    ui->labelParameter3->setText("Product ID:");
    ui->lineEditParameter3->setFixedWidth(65);
    ui->lineEditParameter3->setText("&H6870");
    ui->labelParameter4->setFixedWidth(70);
    ui->labelParameter4->setText("Report Len:");
    ui->lineEditParameter4->setFixedWidth(70);
    ui->lineEditParameter4->setText("2");
    ui->labelParameter5->setFixedWidth(80);
    ui->labelParameter5->setText("Bytes:");
    ui->lineEditParameter5->setFixedWidth(80);
    ui->lineEditParameter5->setText("&h02:&h01");
}

// Setup UI - TCP functions
void EditorAddCmdWindow::setupTcpUI(CommandType cmd)
{
    setParamLabelVisibility(true, false, false, false, false);
    setParamComboBoxVisibility(true, false, false, false, false);
    setParamLineEditVisibility(false, false, false, false, false);
    ui->labelParameter1->setFixedWidth(45);
    ui->labelParameter1->setText("Socket:");
    ui->cbParameter1->setFixedWidth(45);
    ui->cbParameter1->addItem("1");
    ui->cbParameter1->addItem("2");

    if (cmd == CmdTcpConnect || cmd == CmdSindenTSC)
    {
        setParamLabelVisibility(true, true, true, false, false);
        setParamLineEditVisibility(false, true, true, false, false);
        ui->labelParameter2->setFixedWidth(120);
        ui->labelParameter2->setText("IP Address:");
        ui->lineEditParameter2->setFixedWidth(120);
        ui->lineEditParameter2->setText("127.0.0.1");
        ui->labelParameter3->setText("Port:");
        ui->lineEditParameter3->setValidator(validator65535);
        ui->lineEditParameter3->setToolTip("0 - 65535");

        if (cmd == CmdSindenTSC)
        {
            ui->lineEditParameter3->setText("13000");
        }
        else
        {
            ui->lineEditParameter3->setText("12345");
        }
    }
    else if (cmd == CmdTcpSend)
    {
        setParamLabelVisibility(true, true, false, false, false);
        setParamLineEditVisibility(false, true, false, false, false);
        ui->labelParameter2->setText("Command:");
        ui->lineEditParameter2->setText("Hello_World");
    }
}

// Setup UI - UDP function
void EditorAddCmdWindow::setupUdpUI(CommandType cmd)
{
    setParamLabelVisibility(true, true, true, true, false);
    setParamComboBoxVisibility(true, false, false, false, false);
    setParamLineEditVisibility(false, true, true, true, false);
    ui->labelParameter1->setFixedWidth(55);
    ui->labelParameter1->setText("Type:");
    ui->cbParameter1->setFixedWidth(55);
    ui->cbParameter1->addItem("ASCII", "1");
    ui->cbParameter1->addItem("Hex", "2");
    ui->labelParameter2->setFixedWidth(100);
    ui->labelParameter2->setText("IP Address:");
    ui->lineEditParameter2->setFixedWidth(100);
    ui->lineEditParameter2->setText("127.0.0.1");
    ui->labelParameter3->setFixedWidth(50);
    ui->labelParameter3->setText("Port:");
    ui->lineEditParameter3->setFixedWidth(50);
    ui->lineEditParameter3->setValidator(validator65535);
    ui->lineEditParameter3->setToolTip("0 - 65535");
    ui->lineEditParameter3->setText("12345");
    ui->labelParameter4->setText("Command:");

    connect(ui->cbParameter1, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]()
    {
        QString currentText = ui->cbParameter1->currentText();
        bool isASCII = (currentText == "ASCII");
        bool isHex = (currentText == "Hex");

        if (isASCII)
        {
            ui->lineEditParameter4->setText("Hello_World");
        }
        else if (isHex)
        {
            ui->lineEditParameter4->setText("02 01 FF 00");
        }
        else
        {
            ui->lineEditParameter4->clear();
        }

        if (layout())
            layout()->activate();

        adjustSize();
    });

    ui->cbParameter1->currentIndexChanged(ui->cbParameter1->currentIndex());
}

// Setup UI - HTTP POST Request function
void EditorAddCmdWindow::setupHttpUI(CommandType cmd)
{
    setParamLabelVisibility(true, true, true, false, false);
    setParamComboBoxVisibility(false, false, false, false, false);
    setParamLineEditVisibility(true, true, true, false, false);
    ui->labelParameter1->setFixedWidth(150);
    ui->labelParameter1->setText("URL:");
    ui->lineEditParameter1->setFixedWidth(150);
    ui->lineEditParameter1->setText("http://192.168.0.150/json");
    ui->labelParameter2->setFixedWidth(100);
    ui->labelParameter2->setText("Content-Type:");
    ui->lineEditParameter2->setFixedWidth(100);
    ui->lineEditParameter2->setText("application/json");
    ui->labelParameter3->setFixedWidth(120);
    ui->labelParameter3->setText("Request:");
    ui->lineEditParameter3->setFixedWidth(120);
    ui->lineEditParameter3->setText(R"({"on":true,"bri":255})");
}

// Setup UI - External Application functions
void EditorAddCmdWindow::setupAppUI(CommandType cmd)
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
        ui->labelParameter2->setFixedWidth(85);
        ui->labelParameter2->setText("Parameters:");
        ui->lineEditParameter2->setFixedWidth(85);
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
void EditorAddCmdWindow::setupAudioUI()
{
    setParamLabelVisibility(true, false, false, false, false);
    setParamComboBoxVisibility(false, false, false, false, false);
    setParamLineEditVisibility(true, false, false, false, false);
    ui->labelParameter1->setText("WAV Audio File:");
    ui->lineEditParameter1->setText("test.wav");
}

// Check command
bool EditorAddCmdWindow::checkCommand(const FunctionCommand &cmd)
{
    if (cmd.commandCode == COMPORTOPEN || cmd.commandCode == COMPORTSETTINGS)
    {
        return validateComPortValues(cmd);
    }

    if (cmd.commandCode == PACSETINTENSITY)
    {
        return validateLedIntensityValue(cmd);
    }

    if (cmd.commandCode == PACSETFADETIME)
    {
        return validateLedFadeTimeValue(cmd);
    }

    if (cmd.commandCode == SDL3FFA)
    {
        return validateRumbleValues(cmd);
    }

    if (cmd.commandCode == TCPSOCKETCONNECT || cmd.commandCode == UDPSOCKETSEND)
    {
        return validateTcpUdpPort(cmd);
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

    switch (cmd.type)
    {
        case CmdFusionIGF:
        case CmdGun4irIGF:
        case CmdOpenFireIGF:
            return validateStrengthPulseValue(cmd);
            break;
    }

    return true;
}

// Validate COM Port values
bool EditorAddCmdWindow::validateComPortValues(const FunctionCommand &cmd)
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

// Validate strength/pulse value
bool EditorAddCmdWindow::validateStrengthPulseValue(const FunctionCommand &cmd)
{
    const QString errorStyle = "border: 1px solid red;";

    if (cmd.param4.isEmpty())
    {
        return true;
    }

    QString text = ui->lineEditParameter4->text();
    int pos = 0;

    if (ui->lineEditParameter4->validator()->validate(text, pos) != QValidator::Acceptable)
    {
        ui->lineEditParameter4->setStyleSheet(errorStyle);
        QString errorMsg = "The strength/pulse value is not in range [0-255]!\nStrength/Pulse value: " + cmd.param4;
        GuiUtilities::showMessageBoxCentered(this, "Strength/Pulse Value - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    return true;
}

// Validate Ultimarc LED intensity value
bool EditorAddCmdWindow::validateLedIntensityValue(const FunctionCommand &cmd)
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

// Validate Ultimarc LED fade time value
bool EditorAddCmdWindow::validateLedFadeTimeValue(const FunctionCommand &cmd)
{
    const QString errorStyle = "border: 1px solid red;";

    if (cmd.param2.isEmpty())
    {
        ui->lineEditParameter2->setStyleSheet(errorStyle);
        QString errorMsg = "The fade time value field is empty!";
        GuiUtilities::showMessageBoxCentered(this, "Ultimarc - Set LED Fade Time - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    QString text = ui->lineEditParameter2->text();
    int pos = 0;

    if (ui->lineEditParameter2->validator()->validate(text, pos) != QValidator::Acceptable)
    {
        ui->lineEditParameter2->setStyleSheet(errorStyle);
        QString errorMsg = "The fade time value is not in range [0-255]!\nIntensity value: " + cmd.param2;
        GuiUtilities::showMessageBoxCentered(this, "Ultimarc - Set LED Intensity - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    return true;
}

// Validate rumble values
bool EditorAddCmdWindow::validateRumbleValues(const FunctionCommand &cmd)
{
    const QString errorStyle = "border: 1px solid red;";

    if (cmd.param3.isEmpty())
    {
        ui->lineEditParameter3->setStyleSheet(errorStyle);
        QString errorMsg = "The left strength value field is empty!";
        GuiUtilities::showMessageBoxCentered(this, "Gamecontroller - Force Feedback - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    if (cmd.param4.isEmpty())
    {
        ui->lineEditParameter4->setStyleSheet(errorStyle);
        QString errorMsg = "The right strength value field is empty!";
        GuiUtilities::showMessageBoxCentered(this, "Gamecontroller - Force Feedback - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    if (cmd.param5.isEmpty())
    {
        ui->lineEditParameter5->setStyleSheet(errorStyle);
        QString errorMsg = "The duration value field is empty!";
        GuiUtilities::showMessageBoxCentered(this, "Gamecontroller - Force Feedback - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    QString text1 = ui->lineEditParameter3->text();
    QString text2 = ui->lineEditParameter4->text();
    int pos = 0;

    if (ui->lineEditParameter3->validator()->validate(text1, pos) != QValidator::Acceptable)
    {
        ui->lineEditParameter3->setStyleSheet(errorStyle);
        QString errorMsg = "The left strength value is not in range [0-65535]!\nStrength value: " + cmd.param3;
        GuiUtilities::showMessageBoxCentered(this, "Gamecontroller - Force Feedback - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    if (ui->lineEditParameter4->validator()->validate(text2, pos) != QValidator::Acceptable)
    {
        ui->lineEditParameter4->setStyleSheet(errorStyle);
        QString errorMsg = "The right strength value is not in range [0-65535]!\nStrength value: " + cmd.param4;
        GuiUtilities::showMessageBoxCentered(this, "Gamecontroller - Force Feedback - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    return true;
}

// Validate TCP/UDP port value
bool EditorAddCmdWindow::validateTcpUdpPort(const FunctionCommand &cmd)
{
    const QString errorStyle = "border: 1px solid red;";

    if (cmd.param3.isEmpty())
    {
        ui->lineEditParameter3->setStyleSheet(errorStyle);
        QString errorMsg = "The port field is empty!";
        GuiUtilities::showMessageBoxCentered(this, "TCP/UDP Port - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    QString text = ui->lineEditParameter3->text();
    int pos = 0;

    if (ui->lineEditParameter3->validator()->validate(text, pos) != QValidator::Acceptable)
    {
        ui->lineEditParameter3->setStyleSheet(errorStyle);
        QString errorMsg = "The port is not in range [0-65535]!\nPort: " + cmd.param3;
        GuiUtilities::showMessageBoxCentered(this, "TCP/UDP Port - Error", errorMsg, QMessageBox::Critical);
        return false;
    }

    return true;
}

// Validate "Launch Application"
bool EditorAddCmdWindow::validateLaunchApp(const FunctionCommand &cmd)
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
bool EditorAddCmdWindow::validateCloseApp(const FunctionCommand &cmd)
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
bool EditorAddCmdWindow::validateWavAudioFile(const FunctionCommand &cmd)
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
void EditorAddCmdWindow::setParamLabelVisibility(bool l1, bool l2, bool l3, bool l4, bool l5)
{
    ui->labelParameter1->setVisible(l1);
    ui->labelParameter2->setVisible(l2);
    ui->labelParameter3->setVisible(l3);
    ui->labelParameter4->setVisible(l4);
    ui->labelParameter5->setVisible(l5);
}

// Set parameter ComboBox visibility
void EditorAddCmdWindow::setParamComboBoxVisibility(bool cb1, bool cb2, bool cb3, bool cb4, bool cb5)
{
    ui->cbParameter1->setVisible(cb1);
    ui->cbParameter2->setVisible(cb2);
    ui->cbParameter3->setVisible(cb3);
    ui->cbParameter4->setVisible(cb4);
    ui->cbParameter5->setVisible(cb5);
}

// Set parameter LineEdit visibility
void EditorAddCmdWindow::setParamLineEditVisibility(bool le1, bool le2, bool le3, bool le4, bool le5)
{
    ui->lineEditParameter1->setVisible(le1);
    ui->lineEditParameter2->setVisible(le2);
    ui->lineEditParameter3->setVisible(le3);
    ui->lineEditParameter4->setVisible(le4);
    ui->lineEditParameter5->setVisible(le5);
}

// Reset all inputs
void EditorAddCmdWindow::resetInputs()
{
    ui->cbParameter1->disconnect(SIGNAL(currentIndexChanged(int)));
    connect(ui->cbParameter1, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorAddCmdWindow::updateCommandDisplay);

    ui->cbParameter2->disconnect(SIGNAL(currentIndexChanged(int)));
    connect(ui->cbParameter2, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorAddCmdWindow::updateCommandDisplay);

    ui->cbParameter3->disconnect(SIGNAL(currentIndexChanged(int)));
    connect(ui->cbParameter3, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorAddCmdWindow::updateCommandDisplay);

    ui->lineEditParameter4->disconnect(SIGNAL(textChanged(QString)));
    connect(ui->lineEditParameter4, &QLineEdit::textChanged, this, &EditorAddCmdWindow::clearErrorStyle);
    connect(ui->lineEditParameter4, &QLineEdit::textChanged, this, &EditorAddCmdWindow::updateCommandDisplay);

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
    ui->lineEditParameter4->removeEventFilter(this);
}
