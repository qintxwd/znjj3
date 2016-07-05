#ifndef STM32_H
#define STM32_H
#include "serial.h"
#include <QSerialPort>
#include <QObject>
#include <QTimer>

#define STM32_BATTERY_CHARGE_LOWEST         2.75            //电压低于该值,自动返回充电
#define STM32_BATTERY_PERFORM_LOWEST        2.79            //电压高于该值,才可以离开充电做


class Stm32 : public QSerialPort
{
    Q_OBJECT

public:
    Stm32(QObject *parent = NULL);
    virtual ~Stm32();

    bool openCom(const QString& portName,int buadrate);

    int getU1();
    int getFred();
    double getBattery();
    //          int getBatteryPercent();
    bool isCharging();
signals:
    void redLRM(QString str);
    void ulChange(QString str);
    void ttsPlay(int n);
protected slots:
    void startRead();
    void isReopenNeed();
private:
    volatile int kk;
    void reopen();
private:
    Serial serial;

    void processComRead(BYTE *buf,int bufLen);

    void parseQString(const QString &stm32Str);
private:
    volatile bool m_isCharging;              //是否充电电压
    volatile double battery;             //电池电压
    volatile int u1;
    volatile int fred;
    QTimer reopenTimer;
private:
    //电压值在不停波动,故而采取取最近四次的值的平均值作为当前电压值
    double recentBattery[5];
    int updateBatteryIndex;
};


#endif // STM32_H
