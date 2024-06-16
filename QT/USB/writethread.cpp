#include "writethread.h"
#include <QFile>
#include <QTextStream>

WriteThread::WriteThread(QMutex *mutex, QQueue<QByteArray> *dataQueue, QObject *parent)
    : QThread(parent), m_mutex(mutex), m_dataQueue(dataQueue)
{
}

void WriteThread::run() {
    while (true) {
        QByteArray data;
        {
            QMutexLocker locker(m_mutex);
            if (m_dataQueue->isEmpty())
                continue;
            data = m_dataQueue->dequeue();
        }
        QFile file("data.bin");
        if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
            file.write(data);
            file.close();
        }
    }
}
