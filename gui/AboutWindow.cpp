#include "AboutWindow.h"
#include "ui_AboutWindow.h"

#include "../Global.h"

AboutWindow::AboutWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutWindow)
{
    QWidget::setWindowFlags(Qt::Dialog | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);

    ui->setupUi(this);

    this->setFixedSize(this->size());

    move(pos()+(QGuiApplication::primaryScreen()->geometry().center()-geometry().center()));

    QString versionInfo = "Version: ";
    versionInfo.append(VERSION);
    ui->VersionLabel->setText(versionInfo);
}

AboutWindow::~AboutWindow()
{
    delete ui;
}
