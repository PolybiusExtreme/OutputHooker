/*
 * Original Copyright (c) 2026 PolybiusExtreme
 *
 * Licensed under the GNU GPLv3.
 */

#ifndef DEVICEWINDOW_H
#define DEVICEWINDOW_H

#include <QDialog>
#include <QVBoxLayout>
#include <QSerialPortInfo>
#include <QList>
#include <QSet>

#include "../core/OutputHookerCore.h"
#include "../core/LedWizModule.h"
#include "../core/PacDriveModule.h"

namespace Ui {
class DeviceWindow;
}

class DeviceWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceWindow(QWidget *parent = nullptr);
    ~DeviceWindow();

public slots:
    // Check COM Ports
    void checkComPorts();

    // Update LEDWiz device list
    void updateLedWizList(const QList<LedWizData> &devices);

    // Update Ultimarc device list
    void updateUltimarcList(const QList<UltimarcData> &devices);

    // Update SDL device list
    void updateSdlList(const QList<SdlCtrlData> &devices);

private slots:
    // On pushButton Close clicked
    void on_pushButtonClose_clicked();

private:
    Ui::DeviceWindow *ui;

    // QSet - Known COM Ports
    QSet<QString> m_knownComPorts;

    // QList - COM Port
    QList<ComPortData> m_comPortDevices;

    // QList - LEDWiz
    QList<LedWizData> m_ledWizDevices;

    // QList - Ultimarc
    QList<UltimarcData> m_ultimarcDevices;

    // QList - SDL
    QList<SdlCtrlData> m_sdlDevices;

    // Update device list
    void updateDeviceList();
};

#endif // DEVICEWINDOW_H
