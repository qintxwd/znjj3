#ifndef ROBOT_H
#define ROBOT_H

#include <QObject>
#include "arm.h"
#include "chest.h"
#include "eyes.h"
#include "motor.h"
#include "qyhtts.h"
#include "stargazermap.h"
#include "configure.h"
#include "stm32.h"
#include "gopositionthread.h"
#include "gochargethread.h"
#include "configure.h"
#include "usercommandtcpserver.h"

typedef enum{
    CONTROLLER_BUSINESS_IDLE,       //空闲
    CONTROLLER_BUSINESS_CHARGE,      //正在返回充电
    CONTROLLER_BUSINESS_CHARGING,       //充电中
} CONTROLLER_CURRENT_STATUS;

extern CONTROLLER_CURRENT_STATUS currentBusiness;

class Robot : public QObject
{
    Q_OBJECT
    GoPositionThread *workerGoPosition;
    QThread goPositionThread;           //依附线程

    GoChargeThread *workerGoCharge;
    QThread goChargeThread;             //依附线程

public:
    explicit Robot(QObject *parent = 0);
    bool init(QString &str);
    void stop();
    void slowStop();
    void circle();
    void forward(int mm,int speed);
    void backward(int mm,int speed);
    void turnLeft(int theta,int speed);
    void turnRight(int theta, int speed);
    void diffRun(int l,int sl,int r,int sr);
    void leftArm(int angle);//完成时发送返回值
    void rightArm(int angle);   //完成时发送返回值
    void bothArm(int angle);//完成时发送返回值
    void goPosition(double x,double y,double theta);
    void goCharge();
    volatile bool isGoPosition;
    MOTOR_MOVE_TYPE lastMoveType;

    void canSayWelcom(bool canOrNot);
    //以下为私有，请勿访问
signals:
    void sigIsCharge(bool isCharging);
    void goPositionInThread(double x,double y,double theta);
    void goChargeInThread();
public slots:
    void ttsPlay(int n);
    void ttsPlayReturn(int n);
    void ttsPlayEnd();
    void armMoveEnd();
    void motorMoveEnd();
    void handleResultsGoPosition(QString str);
    void handleResultsGoCharge(QString str);
public:
    StargazerMap stargazermap;
    void goAboutPosition(double x,double y,double theta);
    void waitForMoveEnd();
    void waitForGoPositionFinish();
    double turnToAngle(double angle);
    Stm32 stm32;
    Arm arm;
    Motor motor;
private:
    Chest chest;
    Eyes eyes;
    QyhTts qyhtts;
    Configure configure;
    SGPOSITION *curpos;

    bool needReturnTts;
    bool needReturnArm;
    bool needReturnMotor;

    //发送正在回充电 或者正在充电
    QTimer sendStatusTimer;
public slots:
    void sendStatus();
private:

    //检测电量低,如果低则返回充电
    QTimer powerLowTimer;
public slots:
    void checkPowerLow();
private:

    //检测是否正在充电
    QTimer isChargingTimer;
public slots:
    void checkIsCharging();
private:


    //空闲时间，定时说欢迎语
    QTimer speakWelcome;
public slots:
    void toSpeakWelcome();
private:


    //是否遇到障碍物
    int obTimeInterval;
public slots:
    void queryDis();
public:

    UserCommandTcpServer ucts;
};

extern Robot g_robot;
#endif // ROBOT_H
