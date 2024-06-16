#include "widget.h"
#include "ui_widget.h"

#include <Qfile>
#include <QString>
#include <QThread>
#include <QTextStream>
#include <QMessageBox>

#include "ftd2xx.h"

#include "QDebug"

// add on 2024.04.25
#include <QMutex>
#include <QQueue>
#include <QWaitCondition>
#include <QByteArray>


FT_DEVICE_LIST_INFO_NODE *devInfo;
FT_STATUS ftStatus;
FT_HANDLE ftHandle;
DWORD numDevs;

bool bContinue = FALSE;

UCHAR Mask = 0xff;
//Single Channel Synchrocnous 245 FIFO Mode (FT2232H and FT232H devices only)
UCHAR Mode = 0x40;

QThread *rxthread = new QThread;
QThread *writethread = new QThread;

QMutex mutex;
QQueue<QByteArray> dataQueue;


void Widget::CheakNum()
{
    ui->comboBox->clear();
    ui->textBrowser->clear();

    ftStatus = FT_CreateDeviceInfoList(&numDevs);

    if (ftStatus == FT_OK) {

        ui->label->setNum((int)numDevs);

        // int i = 10;
        // QString dev = QString("Dev %1");
        // dev = dev.arg(i);
        // ui->textBrowser->append(dev);

        // ui->textBrowser->append("Dev");

        if (numDevs > 0)
        {
            devInfo =
                (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*numDevs);
            // get the device information list
            ftStatus = FT_GetDeviceInfoList(devInfo, &numDevs);
            if (ftStatus == FT_OK) {
                for (int i = 0; i < (int)numDevs; i++) {
                    // printf("Dev %d:\n", i);
                    // printf(" Flags=0x%x\n", devInfo[i].Flags);
                    // printf(" Type=0x%x\n", devInfo[i].Type);
                    // printf(" ID=0x%x\n", devInfo[i].ID);
                    // printf(" LocId=0x%x\n", devInfo[i].LocId);
                    // printf(" SerialNumber=%s\n", devInfo[i].SerialNumber);
                    // printf(" Description=%s\n", devInfo[i].Description);
                    // printf(" ftHandle=0x%x\n", devInfo[i].ftHandle);

                    QString dev = QString("Dev %1");
                    dev = dev.arg(i);
                    ui->textBrowser->append(dev);

                    QString SerialNumber = QString::fromLocal8Bit(devInfo[i].SerialNumber);
                    ui->textBrowser->append(" SerialNumber=");
                    ui->textBrowser->insertPlainText(SerialNumber);

                    QString Description = QString::fromLocal8Bit(devInfo[i].Description);
                    ui->textBrowser->append(" Description=");
                    ui->textBrowser->insertPlainText(Description);

                    QString str;
                    str = QString::fromLocal8Bit(devInfo[i].Description);

                    ui->comboBox->addItem(str);
                }
            }
        }
    }
    else {
        QMessageBox::warning(NULL, "test", "CheakNum ERROR!");
    }

    qDebug() << numDevs;

}

void Widget::OpenPort()
{
    if (numDevs > 0)
    {
        int iSelIndex = ui->comboBox->currentIndex();
        if (iSelIndex >= 0)
        {
            ftStatus = FT_Open(iSelIndex , &ftHandle);

            if (ftStatus == FT_OK)
            {
                ui->label->setText("connect!");
                bContinue = TRUE;
                ftStatus = FT_SetBitMode(ftHandle, Mask, Mode);
                if (ftStatus == FT_OK) {
                    // 0xff written to device
                }
                else {
                    // FT_SetBitMode FAILED!
                    QMessageBox::warning(NULL, "test", "FT_SetBitMode FAILED!");
                }

                //add timeout set by zeyu on 2024.04.19
                //set read timeout of 5sec, write timeout of 1sec
//                ftStatus = FT_SetTimeouts(ftHandle, 5000, 1000);
//                if(ftStatus == FT_OK){
//                    // FT_SetTimeouts OK
//                    qDebug() << "FT_SetTimeouts OK!";
//                }
//                else{
//                    // FT_SetTimeouts failed
//                    qDebug() << "FT_SetTimeouts failed!";
//                }

                rxthread->start();
                writethread->start();
            }
            else
            {
                QMessageBox::warning(NULL, "test", "OpenPort ERROR!");
            }
        }
    }
}

void Widget::ClosePort()
{
    if (ftHandle)
    {
        ftStatus = FT_Close(ftHandle);
        ftHandle = NULL;
        if (ftStatus == FT_OK)
        {
            ui->label->setText("no connect!");
            bContinue = FALSE;
        }
        else
        {
            QMessageBox::warning(NULL, "test", "ClosePort ERROR!");
        }
    }
}

void Widget::ReadingProc()
{
    DWORD RxBytes;
    DWORD dwRXBytes;

//    uchar rx_char[99999];
    char rx_char[99999];


    while (bContinue)
    {
        FT_GetQueueStatus(ftHandle, &RxBytes);
        if ((ftStatus == FT_OK) && (RxBytes > 0))
        {
            ftStatus = FT_Read(ftHandle, &rx_char, RxBytes, &dwRXBytes);
            if (ftStatus == FT_OK)
            {
//                QByteArray data(reinterpret_cast<const char *>(rx_char), dwRXBytes);
                QByteArray data(rx_char, dwRXBytes);
                QMutexLocker locker(&mutex);
                dataQueue.enqueue(data);
            }
            else
            {
                QMessageBox::warning(NULL, "test", "ReadingProc ERROR!");
            }
        }
    }
}

void Widget::ReadingProc_old()
{
    DWORD RxBytes;
    DWORD dwRXBytes;

//    QChar rx_char;
    uchar rx_char[99999];
    rx_char[0] = 1;

    //从QT获取文件名称
    QString filepath;
    filepath = ui->PathEdit->text();
    QFile file(filepath);
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);

    while (bContinue)
    {
        FT_GetQueueStatus(ftHandle, &RxBytes);

        if ((ftStatus == FT_OK) && (RxBytes > 0)) //&& (RxBytes < 4000)
        {
//            ftStatus = FT_Read(ftHandle, &rx_char, 1, &dwRXBytes);
            ftStatus = FT_Read(ftHandle, &rx_char, RxBytes , &dwRXBytes);

            if (ftStatus == FT_OK)
            {
//                printf("%x\n",rx_char[0]);
//                printf("%x\n",rx_char[1]);
//                printf("%x\n",rx_char[2]);
//                printf("%x\n",rx_char[3]);
//                printf("%x\n",rx_char[4]);
//                printf("%x\n",rx_char[5]);

//                char data = rx_char[0].toLatin1();
//                QByteArray byteArray(&data, 1);
//                QString hexString = byteArray.toHex();
//                out << hexString << ' ';

                /*ui.textBrowser->insertPlainText(QString(rx_char));*/
                //out << hex << rx_char;

                //QByteArray byteArray(&rx_char, 1);
                //QString hexString = byteArray.toHex();

                //can use
//                char data = rx_char.toLatin1();
//                QByteArray byteArray(&data, 1);
//                QString hexString = byteArray.toHex();
//                out << hexString << ' ';

                //change
                for(DWORD i = 0 ; i < RxBytes ; i++){
//                    QString hexValue = QString::number(rx_char[i], 16);
//                    QString hexValue = QString::number(rx_char[i], 16).rightJustified(2, '0');
                    out << rx_char[i] << ' ';
                }

            }
            else
            {
                QMessageBox::warning(NULL, "test", "ReadingProc ERROR!");
            }
        }
    }
    file.close();
    // rxthread->quit();
}



Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    connect(rxthread, &QThread::started, [=]() {
        ReadingProc();
        rxthread->exit();
    });

    connect(writethread, &QThread::started, [=]() {
        qDebug() << "writethread start!";


        //从QT获取文件名称
        QString filepath;
        filepath = ui->PathEdit->text();
        QFile file(filepath);
        file.open(QIODevice::WriteOnly);
        QTextStream out(&file); // 使用QTextStream进行文件写入
        QByteArray data;

        while (true) {
            if(!bContinue && dataQueue.isEmpty()){
                break;
            }

            {
                QMutexLocker locker(&mutex);
                if (dataQueue.isEmpty())
                    continue;
                data = dataQueue.dequeue();
            }

            // 将QByteArray转换为16进制字符串
            QString hexData = data.toHex();
            qDebug() << "Hex Data:" << hexData;
            out << hexData; // 写入16进制字符串           
        }

        file.close(); // 关闭文件

        writethread->exit();
    });


    connect(rxthread, &QThread::finished, [=]() {
        qDebug() << "rxthread finish!";
    });

    connect(writethread, &QThread::finished, [=]() {
        qDebug() << "writethread finish!";
    });


    connect(ui->SearchButton , &QPushButton::clicked , [&](){
        CheakNum();
    });

    connect(ui->ConnectButton , &QPushButton::clicked , [&](){
        OpenPort();
    });

    connect(ui->CloseButton , &QPushButton::clicked , [&](){
        ClosePort();
        ui->textBrowser->clear();
        ui->textBrowser->append("rxthread finish!");
    });
}

Widget::~Widget()
{
    writethread->exit();
    delete ui;
}
