#include "stargazer.h"

#include <QDebug>
#include "qyhtts.h"

//zhiling:
const char ArrCmdStartGazer[][64] = {
    "Version",
    "Status",
    "ThrAlg",
    "ThrVal",
    "IDNum",
    "RefID",
    "MarkHeight",
    "MarkType",
    "MarkMode",
    "MapMode",
    "SetEnd",
    "Reset",
    "CalcStart",
    "CalcStop",
    "HeightCalc",
    "DeadZone"
};

Stargazer::Stargazer(QObject *parent) : QObject(parent)
{
}

Stargazer::~Stargazer()
{
    serial.ClosePort();
}

bool Stargazer::init(int portNumber,int buadrate)
{
    serial.m_OnSeriesRead = onComRead;

    if(!serial.OpenPort(this,portNumber,buadrate))
        return false;
    qDebug() <<"COM"<<portNumber<<" serial open OK!";
    return true;
}

void CALLBACK Stargazer::onComRead(void * pOwner,BYTE* buf,int bufLen)
{
    Stargazer *arg = (Stargazer *) pOwner;
    std::string s((char *)buf,bufLen);
    QString str = QString::fromStdString(s);
    arg->processComRead(str);
}

void Stargazer::processComRead(QString str)
{
    emit getSerialRead(str);
}

void Stargazer::sendToStargazer(int id)
{
    QString str ;
    if(id == Enum_MarkMode){
        str = QString("~#%1|%2`").arg(ArrCmdStartGazer[Enum_MarkMode]).arg("Alone");
    }else if(id == Enum_ThrAlg) {
        str = QString("~#%1|%2`").arg(ArrCmdStartGazer[Enum_ThrAlg]).arg("Auto");
    }else{
        str = QString("~#%1`").arg(ArrCmdStartGazer[id]);
    }

    qDebug()<<"write to stargazer com:"<<str;
    std::string sss = str.toStdString();
    for(unsigned int i=0;i<sss.length();++i){
        BYTE b = (BYTE)sss.at(i);
        serial.WriteSyncPort(&b,1);
        QThread::msleep(20);
    }
}
