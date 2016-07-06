#ifndef MOTOR_H
#define MOTOR_H
#include <QPair>
#include <QObject>
#include <QVector>
#include "serial.h"


typedef enum{
    MOTOR_MOVE_TYPE_FORWARD,
    MOTOR_MOVE_TYPE_BACKWARD,
    MOTOR_MOVE_TYPE_TURNLEFT,
    MOTOR_MOVE_TYPE_TURNRIGHT,
    MOTOR_MOVE_TYPE_DIFFRUN,
    MOTOR_MOVE_TYPE_STOP,
    MOTOR_MOVE_TYPE_SLOWSTOP
}MOTOR_MOVE_TYPE;


class Motor : public QObject
{
    Q_OBJECT

public:

    friend class Robot;

    explicit Motor(QObject *parent = NULL);

    static void CALLBACK onComRead(void * pOwner,BYTE* buf,int bufLen);

    void waitForMoveEnd();

    bool isMoveFinished();

    void move(MOTOR_MOVE_TYPE moveType,QVector<int> params);


    bool init(int portNum, int buadrate);

signals:

    void motorReadCom(unsigned char *buf,int len);

    void moveEnd();

private:

    void processComRead(BYTE *buf,int bufLen);

    volatile bool moveFinished;

    Serial serial;

    volatile int waitHandredMs;
};

#endif // MOTOR_H
