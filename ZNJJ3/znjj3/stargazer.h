#ifndef STARGAZER_H
#define STARGAZER_H

#include <QObject>
#include "serial.h"

#define PI  3.1415926535898
#define ZQx 1.15
#define ZQy 3.15
#define ZQtheta 2.15
#define DISTANCE_BETWEEN_CAMERA_AND_MOTOR       8

enum EnumCmdStartGazer
{
    Enum_Version = 0,
    Enum_Status,
    Enum_ThrAlg,
    Enum_ThrVal,
    Enum_IDNum,
    Enum_RefID,
    Enum_MarkHeight,
    Enum_MarkType,
    Enum_MarkMode,
    Enum_MapMode,
    Enum_SetEnd,
    Enum_Reset,
    Enum_CalcStart,
    Enum_CalcStop,
    Enum_CalcHeight,
    Enum_DeadZone,
    Enum_Length
};

struct SGPOSITION
{
    volatile double x;
    volatile double y;
    volatile double theta;
    SGPOSITION()
    {
        x=0;
        y=0;
        theta = 0;
    }
    void operator =(struct SGPOSITION &old)
    {
        x = old.x;
        y = old.y;
        theta = old.theta;
    }
};


struct STAR
{
    int id;
    double x;
    double y;
    double theta;
    double qx;
    double qy;
    double qtheta;
    STAR()
    {
        id=-1;
        x=0;
        y=0;
        theta = 0;
        qx = ZQx;
        qy = ZQy;
        qtheta = ZQtheta;
    }
    void operator =(const struct STAR &old)
    {
       x = old.x;
       y = old.y;
       theta = old.theta;
       id = old.id;
       qx = old.qx;
       qy = old.qy;
       qtheta = old.qtheta;
    }
};

class Stargazer : public QObject
{
    Q_OBJECT
public:
    explicit Stargazer(QObject *parent = 0);

    virtual ~Stargazer();

    bool init(int portNumber,int buadrate);

    static void CALLBACK onComRead(void * pOwner,BYTE* buf,int bufLen);

    void processComRead(QString str);

    void sendToStargazer(int id);

    double distanceBetweenMoveCenterAndCamera;
    double angleBetweenMotorAndCamera;
signals:
    void getSerialRead(QString str);
public slots:

private:
    Serial serial;

};

#endif // STARGAZER_H
