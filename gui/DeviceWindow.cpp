/*
 * Original Copyright (c) 2026 PolybiusExtreme
 *
 * Licensed under the GNU GPLv3.
 */

#include "DeviceWindow.h"
#include "ui_DeviceWindow.h"

#include "GuiUtilities.h"
#include "../OutputHooker.h"

DeviceWindow::DeviceWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DeviceWindow)
{
    QWidget::setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);

    ui->setupUi(this);

    ui->listWidgetDevices->setSelectionMode(QAbstractItemView::NoSelection);
    ui->listWidgetDevices->setFocusPolicy(Qt::NoFocus);

    GuiUtilities::centerWidgetOnScreen(this);

    checkComPorts();
}

DeviceWindow::~DeviceWindow()
{
    delete ui;
}

// Update LEDWiz device list
void DeviceWindow::updateLedWizList(const QList<LedWizData> &devices)
{
    m_ledWizDevices = devices;
    updateDeviceList();
}

// Update Ultimarc device list
void DeviceWindow::updateUltimarcList(const QList<UltimarcData> &devices)
{
    m_ultimarcDevices = devices;
    updateDeviceList();
}

// Update SDL device list
void DeviceWindow::updateSdlList(const QList<SdlCtrlData> &devices)
{
    m_sdlDevices = devices;
    updateDeviceList();
}

// Check COM Ports
void DeviceWindow::checkComPorts()
{
    const auto infos = QSerialPortInfo::availablePorts();
    QSet<QString> currentPortNames;
    QList<ComPortData> newPortList;

    for (const QSerialPortInfo &info : infos)
    {
        currentPortNames.insert(info.portName());
        newPortList.append({info.portName(), info.description()});
    }

    // Check for changes (added or removed)
    if (currentPortNames != m_knownComPorts)
    {
        m_knownComPorts = currentPortNames;
        m_comPortDevices = newPortList;
        updateDeviceList();
    }
}

// On pushButton Cancel clicked
void DeviceWindow::on_pushButtonClose_clicked()
{
    reject();
}

// Update device list
void DeviceWindow::updateDeviceList()
{
    ui->listWidgetDevices->clear();

    auto addHeader = [this](const QString &text)
    {
        QListWidgetItem *headerItem = new QListWidgetItem(text, ui->listWidgetDevices);
        QFont font = headerItem->font();
        font.setPointSize(11);
        font.setUnderline(true);
        headerItem->setFont(font);
        ui->listWidgetDevices->addItem(headerItem);
    };

    addHeader("COM Port device(s):");

    if (m_comPortDevices.isEmpty())
    {
        ui->listWidgetDevices->addItem("No COM Port devices found!");
    }
    else
    {
        for (const auto &port : std::as_const(m_comPortDevices))
        {
            ui->listWidgetDevices->addItem(QString("%1: %2").arg(port.portName, port.description));
        }
    }

    ui->listWidgetDevices->addItem("");

    addHeader("LEDWiz device(s):");

    if (m_ledWizDevices.isEmpty())
    {
        ui->listWidgetDevices->addItem("No LEDWiz devices found!");
    }
    else
    {
        for (const auto &device : std::as_const(m_ledWizDevices))
        {
            ui->listWidgetDevices->addItem(QString("ID%1: %2").arg(QString::number(device.id), device.name));
        }
    }

    ui->listWidgetDevices->addItem("");

    addHeader("Ultimarc device(s):");

    if (m_ultimarcDevices.isEmpty())
    {
        ui->listWidgetDevices->addItem("No Ultimarc devices found!");
    }
    else
    {
        int index = 1;
        for (const auto &device : std::as_const(m_ultimarcDevices))
        {
            ui->listWidgetDevices->addItem(QString("ID%1: %2").arg(QString::number(index), device.typeName));
            index++;
        }
    }

    ui->listWidgetDevices->addItem("");

    addHeader("Gamecontroller(s):");

    if (m_sdlDevices.isEmpty())
    {
        ui->listWidgetDevices->addItem("No Gamecontrollers found!");
    }
    else
    {
        for (const auto &device : std::as_const(m_sdlDevices))
        {
            ui->listWidgetDevices->addItem(QString("ID%1: %2").arg(QString::number(device.id), device.name));
        }
    }
}
