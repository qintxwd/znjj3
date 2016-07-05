#include "stm32.h"

#include <QDebug>
#include <QFile>
#include "qyhtts.h"

//电压和百分比的对应值
const double voltageToPercent[100] =
{
    2.7568,2.69108,2.69095,2.68987,2.68941,2.68903,2.68881,2.68854,2.68829,2.68812,
    2.68811,2.68796,2.68791,2.68772,2.68768,2.68714,2.68704,2.68696,2.68644,2.68583,
    2.68533,2.6853,2.68523,2.68505,2.68494,2.68462,2.68452,2.68448,2.68407,2.68346,
    2.68292,2.68229,2.68223,2.68214,2.67995,2.6778,2.67636,2.67411,2.67343,2.67335,
    2.67236,2.67153,2.66741,2.66682,2.66547,2.66537,2.66537,2.66388,2.66342,2.66191,
    2.66146,2.66026,2.66024,2.65927,2.6586,2.65847,2.65844,2.65713,2.65585,2.65528,
    2.65253,2.65223,2.6511,2.64964,2.64903,2.64759,2.64714,2.64646,2.64557,2.64554,
    2.64381,2.64183,2.64011,2.63848,2.63711,2.6359,2.63474,2.63297,2.63111,2.63018,
    2.62838,2.62744,2.62594,2.62458,2.62458,2.62384,2.62346,2.62312,2.62247,2.61938,
    2.61882,2.61814,2.6181,2.61656,2.61339,2.60618,2.59892,2.58825,2.57839,2.56477
};

Stm32::Stm32(QObject *parent)
    : QSerialPort(parent),
      m_isCharging(false),
      fred(-2),
      u1(2000),
      battery(2.79)
{
    //recordChargeTimer.setInterval(30*1000);
    recentBattery[0] = battery;
    recentBattery[1] = battery;
    recentBattery[2] = battery;
    recentBattery[3] = battery;
    recentBattery[4] = battery;
    updateBatteryIndex = 0;
}

Stm32::~Stm32()
{
    close();
}


bool Stm32::openCom(const QString& portName,int buadrate)
{
    reopenTimer.setInterval(300); //如果300ms * 10还未收到stm32的消息,则重新打开
    setPortName(portName);
    //connect(&recordChargeTimer,SIGNAL(timeout()),this,SLOT(recordCharge()));
    connect(&reopenTimer,SIGNAL(timeout()),this,SLOT(isReopenNeed()));
    //connect(&reopenTimer,SIGNAL(timeout()),this,SLOT(reopen()));
    if(open(QIODevice::ReadWrite))
    {
        qDebug() << portName <<"serial open OK!";
        setBaudRate(buadrate);
        setParity(QSerialPort::NoParity);
        setDataBits(QSerialPort::Data8);
        setStopBits(QSerialPort::OneStop);
        setFlowControl(QSerialPort::NoFlowControl);
        setReadBufferSize(4096);
        clearError();
        clear();
        connect(this, SIGNAL(readyRead()), this, SLOT(startRead()));
        //recordChargeTimer.start();
        reopenTimer.start();
    }else{
        return false;
    }
    return true;
}

void Stm32::startRead()
{
    QByteArray qba = readAll();
    QString readStr(qba);
    kk = 0;
    parseQString(readStr);
}

void Stm32::parseQString(const QString &stm32Str)
{
    QStringList stm32qsl = stm32Str.split(";");
    for(int i = 0;i<stm32qsl.length();++i){
        QString t = stm32qsl.at(i);
        if(t==""||t.length()==0)
            continue;
        //qDebug() << "read from stm32: "<<t;
        if(t.startsWith("#")){
            QStringList tqsl = t.right(t.length()-1).split(":");
            if(tqsl.length()==2)
            {
                bool ok;
                //qDebug() << "tqsl.at(0) = "<<tqsl.at(0);
                if(tqsl[0]=="bettery")
                {
                    //qDebug() << "bettery = "<<tqsl.at(1);
                    float x = tqsl.at(1).toDouble(&ok);
                    //qDebug() << "ok = "<<ok <<" && x="<<x;
                    if(ok)
                    {
                        recentBattery[updateBatteryIndex++] = x ;
                        if(updateBatteryIndex>=5)
                            updateBatteryIndex = 0;
                        battery = (recentBattery[0]+recentBattery[1]+recentBattery[2]+recentBattery[3]+recentBattery[4])/5;
                    }

                }else if(tqsl[0]=="infrared"){
                    if(tqsl[1]=="UNKNOW"){
                        emit redLRM(QString("UNKNOW"));
                        fred = -2;
                    }else if(tqsl[1]=="RIGHT"){
                        emit redLRM(QString("RIGHT"));
                        fred = 1;
                    }else if(tqsl[1]=="MIDDLE"){
                        emit redLRM(QString("MIDDLE"));
                        fred = 0;
                    }else if(tqsl[1]=="LEFT"){
                        emit redLRM(QString("LEFT"));
                        fred = -1;
                    }
                }else if(tqsl[0]=="ultrasonic1"){
                    int x = tqsl[1].toInt(&ok);
                    if(ok){
                        u1 = x;
                        emit ulChange(QString("%1").arg(u1));
                    }
                }else if(tqsl[0]=="charge"){

                    int x = tqsl.at(1).toInt(&ok);

                    if(ok)
                    {
                        m_isCharging = (x ==1);
                        //qDebug() <<QString( "m_isCharging :" )<<m_isCharging;
                    }
                }
            }
        }
    }
}

int Stm32::getU1()
{
    return u1;
}

int Stm32::getFred()
{
    return fred;
}

double Stm32::getBattery(){
    return battery;
}

//int Stm32::getBatteryPercent()
//{
//    if(battery > voltageToPercent[0]){
//        return 100;
//    }else if(battery <voltageToPercent[99]){
//        return 0;
//    }
//    for(int i = 0;i<100;++i){
//        if(battery>voltageToPercent[i]){
//            return 100-i;
//        }
//    }
//    return 0;

//}

bool Stm32::isCharging()
{
    return  m_isCharging;
}
void Stm32::isReopenNeed()
{
    ++kk;
    if(kk>5){
        reopen();
    }
    //qDebug() << "kk = "<<kk;
    if(kk>50)
    {
        qDebug()<<"kk="<<kk;
        //90秒一直未受到STM32的消息
        //for(int i = 0;i<3;++i){
        emit ttsPlay(16);
        //g_qyhTts.playFile("16.wav");
        QThread::msleep(5000);
        //}
        qDebug()<<"chongqizhong.....";
        system("shutdown -r -t 0 ");
        kk = 0;
    }
}

void Stm32::reopen()
{
    qDebug() << "reopen stm32.....";
    close();
    if(open(QIODevice::ReadWrite))
    {
        //qDebug() << portName <<"serial open OK!";
        clearError();
        clear();
        connect(this, SIGNAL(readyRead()), this, SLOT(startRead()));
        //recordChargeTimer.start();
    }
}

