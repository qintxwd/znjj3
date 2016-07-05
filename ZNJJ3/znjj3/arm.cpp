#include "arm.h"
#include <QThread>
#include <QDebug>
#include <QCoreApplication>

unsigned char ARM_MOTOR_ENABLE[] = {0x22,0x5C,0x00,0x01,0x01,0x00,0x00,0x00,0xAA};
unsigned char ARM_MOTOR_DISABLE[] = {0x22,0x5C,0x00,0x00,0x00,0x00,0x00,0x00,0xAA};

unsigned char ARM_LEFT_MOTOR_ZERO[] = {0x22,0x5d,0x01,0x05,0x01,0x00,0x00,0x00,0xaa};
unsigned char ARM_LEFT_MOTOR_UP[] = {0x22,0x5A,0x01,0x1E,0x5A,0x00,0x00,0x00,0xAA};     //其中0x1E是速度 1-100 0x5A是角度值 (F0代表170度 5A代表着100度)

unsigned char ARM_RIGHT_MOTOR_ZERO[] = {0x22,0x5d,0x02,0x05,0x00,0x00,0x00,0x00,0xaa};
unsigned char ARM_RIGHT_MOTOR_UP[] = {0x22,0x5A,0x02,0x1E,0x5A,0x00,0x00,0x00,0xAA};

Arm::Arm(QObject *parent) : QSerialPort(parent),isMoveEnd(true)
{
}

Arm::~Arm()
{
    close();
}

bool Arm::openCom(const QString& portName,int buadrate)
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
        //上来先归零
        write((char *)ARM_MOTOR_ENABLE,sizeof(ARM_MOTOR_ENABLE));
        QThread::msleep(200);
        write((char *)ARM_LEFT_MOTOR_ZERO,sizeof(ARM_LEFT_MOTOR_ZERO));
        QThread::msleep(200);
        write((char *)ARM_RIGHT_MOTOR_ZERO,sizeof(ARM_RIGHT_MOTOR_ZERO));
    }else{
        return false;
    }
    return true;
}

void Arm::enable(bool isEnable)
{
    if(!this->isOpen())
        return ;
    if(isEnable){
        write((char *)ARM_MOTOR_ENABLE,sizeof(ARM_MOTOR_ENABLE));
    }else{

    }

}

void Arm::startRead()
{
    QByteArray buf = readAll();
    if(buf.length()>3)
    {
        //char *s = buf.toStdString().c_str();
        if(0x22 == (buf[1]&0x000000ff))// && buf[3] == 1)
        {
            switch(buf[2]&0x000000ff)
            {
            case 0x50://前后执行结果
            case 0x51://左右执行结果
                if((buf[3]&0x000000ff) == 0)        //运动错误
                {
                    isMoveEnd = true;
                    emit moveEnd();
                    qDebug() <<"can not start move!!!!!!!";
                }
                else if((buf[3]&0x000000ff) == 1)   //运动OK
                {
                    isMoveEnd = false;
                }
                break;
            case 0x56:
                //
                isMoveEnd = true;
                emit moveEnd();
                break;
            default:
                break;
            }
        }
    }
    QString Str(buf);
    emit readStr(Str);
}

void Arm::LEFT_UP(int angle)
{
    if(!this->isOpen())
        return ;
    if(angle >170){
        angle = 170;
    }else if(angle<=0){
        angle = 0;
    }
    ARM_LEFT_MOTOR_UP[4] = 0xff & ( angle * 0xF0 / 170 );
    isMoveEnd = false;
    write((char *)ARM_LEFT_MOTOR_UP,sizeof(ARM_LEFT_MOTOR_UP));
}

void Arm::RIGHT_UP(int angle)
{
    if(!this->isOpen())
        return ;
    if(angle >170){
        angle = 170;
    }else if(angle<=0){
        angle = 0;
    }
    //F0代表170度
    ARM_RIGHT_MOTOR_UP[4] = 0xff & ( angle * 0xF0 / 170 );
    isMoveEnd = false;
    write((char *)ARM_RIGHT_MOTOR_UP,sizeof(ARM_RIGHT_MOTOR_UP));
}
