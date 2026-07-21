/*
 * Original Copyright (c) 2026 PolybiusExtreme
 * Portions Copyright (c) 2026 6Bolt
 *
 * Licensed under the GNU GPLv3.
 */

#include "OutputHooker.h"

#include <QApplication>
#include <QSharedMemory>
#include <QSystemSemaphore>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString programName = QCoreApplication::applicationName();

    QString sharedMemoryName = programName+"2323POLYBIUS";
    QString semaphoreName = programName+"POLYBIUS2323";

    QSystemSemaphore semaphore(semaphoreName, 1);

    semaphore.acquire();

    QSharedMemory sharedMemory(sharedMemoryName);
    bool isRunning;

    if (sharedMemory.create(1))
        isRunning = false;
    else
        isRunning = true;

    semaphore.release();

    if(!isRunning)
    {
        if (argc > 1)
        {
            OutputHooker w;
            QStringList args;
            for (int i = 1; i < argc; ++i)
            {
                args << QString::fromLocal8Bit(argv[i]);
            }
            w.processCommandLineArgs(args);
            QTimer::singleShot(500, &a, &QCoreApplication::quit);
            return a.exec();
        }
        else
        {
            QApplication::setStyle(QStyleFactory::create("Fusion"));
            OutputHooker w;

            // Show the window unless OutputHooker is set to start in the system tray.
            // The command line branch above never shows it, as it only runs the given
            // commands and quits again
            if (!w.getStartMinimized())
                w.showMainWindow();

            return a.exec();
        }
    }
    else
    {
        if (argc > 1)
        {
            QLocalSocket socket;
            socket.connectToServer("OutputHooker_CommandLine_Server");
            if (socket.waitForConnected(1000))
            {
                QStringList args;
                for (int i = 1; i < argc; ++i)
                {
                    args << QString::fromLocal8Bit(argv[i]);
                }

                QDataStream out(&socket);
                out << args;
                socket.waitForBytesWritten(1000);
                socket.disconnectFromServer();
            }
        }
        return 0;
    }
}
