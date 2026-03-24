#ifndef GUIUTILITIES_H
#define GUIUTILITIES_H

#include <QWidget>
#include <QScreen>
#include <QCursor>
#include <QMessageBox>
#include <QGuiApplication>

class GuiUtilities
{
public:
    static void centerWidgetOnScreen(QWidget *widget)
    {
        if (!widget)
            return;

        QScreen *screen = QGuiApplication::screenAt(QCursor::pos());

        if (!screen) {
            screen = QGuiApplication::primaryScreen();
        }

        if (screen) {
            QRect screenGeometry = screen->geometry();
            widget->adjustSize();

            int x = screenGeometry.center().x() - (widget->width() / 2);
            int y = screenGeometry.center().y() - (widget->height() / 2);

            widget->move(x, y);
        }
    }

    static void showMessageBoxCentered(QWidget* parent, const QString& title, const QString& text, QMessageBox::Icon icon)
    {
        QMessageBox msgBox(icon, title, text, QMessageBox::Ok, parent);
        centerWidgetOnScreen(&msgBox);
        msgBox.exec();
    }
};

#endif // GUIUTILITIES_H
