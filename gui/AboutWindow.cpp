#include "AboutWindow.h"
#include "ui_AboutWindow.h"

#include "../Global.h"

AboutWindow::AboutWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutWindow)
{
    ui->setupUi(this);

    this->setFixedSize(this->size());
    setWindowModality(Qt::ApplicationModal);
    move(pos()+(QGuiApplication::primaryScreen()->geometry().center()-geometry().center()));

    QString versionInfo = "Version: ";
    versionInfo.append(VERSION);
    ui->VersionLabel->setText(versionInfo);
}

AboutWindow::~AboutWindow()
{
    delete ui;
}
