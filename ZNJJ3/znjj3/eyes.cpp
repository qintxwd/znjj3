#include "eyes.h"

#include <QDebug>
#include "qyhtts.h"

unsigned char SYN_FRONT_EYES_COMMAND[] = {0x86,0x01,0x20,0x01,0x1e,0x01,0x00,0x00,0xaa};
unsigned char SYN_BACK_EYES_COMMAND[] =  {0x87,0x01,0x20,0x01,0x1e,0x01,0x00,0x00,0xaa};

Eyes::Eyes(QObject *parent) : QSerialPort(parent)
{

}
Eyes::~Eyes()
{
    close();
}

bool Eyes::openCom(const QString& portName,int buadrate)
{
    setPortName(portName);
    if(open(QIODevice::ReadWrite))
    {
        qDebug() << portName <<"serial open OK!";
        setBaudRate(buadrate);
        setParity(QSerialPort::NoParity);
        setDataBits(QSerialPort::Data8);
        setStopBits(QSerialPort::OneStop);
        setFlowControl(QSerialPort::NoFlowControl);
        setReadBufferSize(512);
        clearError();
        clear();
        this->write((char *)SYN_FRONT_EYES_COMMAND,sizeof(SYN_FRONT_EYES_COMMAND));
        QThread::msleep(100);
        this->write((char *)SYN_BACK_EYES_COMMAND,sizeof(SYN_BACK_EYES_COMMAND));
        connect(this, SIGNAL(readyRead()), this, SLOT(startRead()));
    }else{
        return false;
    }
    return true;
}

void Eyes::startRead()
{
    readAll();
}

