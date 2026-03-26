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
        QApplication::setStyle(QStyleFactory::create("Fusion"));
        OutputHooker w;
        return a.exec();
    }
    else
    {
        return 1;
    }
}
