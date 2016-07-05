#include "chest.h"

#include <QDebug>


const unsigned char CHEST_LED_ON[] = {0x2A,0x03,0x01,0x10,0x00,0x00,0x00,0x00,0x11,0x2A};
const unsigned char CHEST_LED_OFF[] = {0x2A,0x03,0x01,0x11,0x00,0x00,0x00,0x00,0x12,0x2A};
const unsigned char CHEST_LED_FLICLER[] = {0x2A,0x03,0x01,0x12,0x00,0x00,0x00,0x00,0x13,0x2A};
const unsigned char CHEST_LED_OFF_FLICLER[] = {0x2A,0x03,0x01,0x13,0x00,0x00,0x00,0x00,0x14,0x2A};

Chest::Chest(QObject *parent) : QSerialPort(parent)
{

}

Chest::~Chest()
{
    close();
}

bool Chest::openCom(const QString& portName,int buadrate)
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
        connect(this, SIGNAL(readyRead()), this, SLOT(startRead()));
        //上来先亮灯
        write((char *)CHEST_LED_ON,sizeof(CHEST_LED_ON));
    }else{
        return false;
    }
    return true;
}

void Chest::startRead()
{
    QByteArray qba = readAll();
    QString Str(qba);
    emit getRead(Str);
}

void Chest::ledon()
{
    if(isOpen())
        write((char *)CHEST_LED_ON,sizeof(CHEST_LED_ON));
}

void Chest::ledoff()
{
    if(isOpen())
        write((char *)CHEST_LED_OFF,sizeof(CHEST_LED_OFF));
}

void Chest::ledflicker()
{
    if(isOpen())
        write((char *)CHEST_LED_FLICLER,sizeof(CHEST_LED_FLICLER));
}

void Chest::ledoffflicker()
{
    if(isOpen())
        write((char *)CHEST_LED_OFF_FLICLER,sizeof(CHEST_LED_OFF_FLICLER));
}
