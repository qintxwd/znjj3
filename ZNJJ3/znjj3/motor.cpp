#include "motor.h"
#include <QDebug>
#include "qyhtts.h"

//1E00  -- > 3000mm

//1.使能1/2电机
//发送:	22 5c 00 01 01 00 00 00 aa
//返回:	01 22 5C 01 00 00 00 80 AA
//2.电机直行
//发送:22 50 00 00 50 02 00 00 aa   小车运行512*1.23=630mm
//返回:01 22 50 01 00 00 00 74 AA
//发送:22 50 00 00 50 01 00 00 aa   小车运行256*1.23=315mm
//返回:01 22 50 01 00 00 00 74 AA
//3.电机转弯（右转）
//发送:22 51 00 00 10 00 1e 00 aa   电机转动30度
//返回:01 22 51 01 00 00 00 75 AA
//发送:22 51 00 00 10 01 5e 00 aa   电机转动350度
//返回:01 22 51 01 00 00 00 75 AA
//3.电机转弯（左转）
//发送:22 51 00 01 10 00 1e 00 aa   电机转动30度
//返回:01 22 51 01 00 00 00 75 AA
//4. 设置原地旋转1度,轮子的移动距离
//发送:22 5c 2c 00 05 00 00 00 aa
//返回:01 22 5C 01 00 00 00 80 AA
//5. 查询原地旋转1度,轮子的移动距离
//发送:22 5c 2d 00 00 00 00 00 aa
//返回:01 5C 2D 00 05 00 00 8F AA
//6.设置电机1轮子周长
//发送:22 5c 28 01 f0 00 00 00 aa
//返回:01 22 5C 01 00 00 00 80 AA
//7.查询电机1轮子周长
//发送:22 5c 29 00 00 00 00 00 aa
//返回: 01 5C 29 01 F0 00 00 77 AA
//8.设置电机2轮子周长
//发送:22 5c 2a 01 f0 00 00 00 aa
//返回:01 22 5C 01 00 00 00 80 AA
//9.查询电机2轮子周长
//发送:22 5c 2b 00 00 00 00 00 aa
//返回:01 5C 2B 01 F0 00 00 79 AA
//10.设置电机1减数比
//发送:22 5c 0a 0e 10 00 00 00 aa
//返回: 01 22 5C 01 00 00 00 80 AA
//11.查询电机1减数比
//发送:22 5c 0b 00 00 00 00 00 aa
//返回:01 5C 0B 36 4C 00 00 EA AA
//12.设置电机2减数比
//发送:22 5c 0c 0e 10 00 00 00 aa
//返回:01 22 5C 01 00 00 00 80 AA
//13.查询电机2减数比
//发送:22 5c 0d 00 00 00 00 00 aa
//返回:01 22 5C 01 00 00 00 80 AA
//14.设置电机1保护电流
//发送:22 5c 18 07 d0 00 00 00 aa
//返回:01 22 5C 01 00 00 00 80 AA
//15.查询电机1保护电流
//发送:22 5c 19 00 00 00 00 00 aa
//返回:01 5C 19 07 D0 00 00 4D AA
//16.设置电机2保护电流
//发送:22 5c 1a 07 d0 00 00 00 aa
//返回: 01 22 5C 01 00 00 00 80 AA
//17.查询电机2保护电流
//发送:22 5c 1b 00 00 00 00 00 aa
//返回:01 5C 1B 07 D0 00 00 4F AA
//18.参数设置保存
//发送:22 5c 36 00 00 00 00 00 aa
//返回:01 22 5C 01 00 00 00 80 AA


unsigned char motor_enable[]     =   {0x22,0x5c,0x00,0x01,0x01,0x00,0x00,0x00,0xaa};
unsigned char motor_forward[]    =   {0x22,0x50,0x00,0x00,0x50,0x02,0x00,0x00,0xaa};
unsigned char motor_backward[]   =   {0x22,0x50,0x00,0x01,0x50,0x02,0x00,0x00,0xaa};
unsigned char motor_left[]       =   {0x22,0x51,0x00,0x01,0x10,0x01,0x67,0x00,0xaa};
unsigned char motor_right[]      =   {0x22,0x51,0x00,0x00,0x10,0x01,0x67,0x00,0xaa};
unsigned char motor_diff_run[]   =   {0x22,0x53,0x00,0x00,0x0A,0x01,0x1F,0x00,0xaa};
unsigned char motor_stop[]       =   {0x22,0x55,0x00,0x00,0x00,0x00,0x00,0x00,0xaa};
unsigned char motor_slow_stop[]  =   {0x22,0x54,0x00,0x00,0x00,0x00,0x00,0x00,0xaa};

Motor::Motor(QObject *parent)
    : QObject(parent),
      moveFinished(true),
      waitHandredMs(0)
{
}

bool Motor::init(int portNum, int buadrate)
{
    //1.open serial port
    serial.m_OnSeriesRead = onComRead;

    if(!serial.OpenPort(this,portNum,buadrate))
        return false;

    return true;
}

void CALLBACK Motor::onComRead(void * pOwner,BYTE* buf,int bufLen)
{
    Motor *thisMotor = (Motor *) pOwner;
    int lenlen = bufLen ;
    while(lenlen > 0)
    {
        thisMotor->processComRead(buf + bufLen - lenlen,(lenlen > 9) ? 9 : lenlen);
        lenlen -=9;
    }
}

void Motor::processComRead(BYTE *buf,int bufLen)
{
    char ss[256];
    int len = 0;
    char *ptr =ss;
    memset(ss,0,256);
    for(int i=0;i<bufLen;++i)
    {
        unsigned short c = static_cast<int>(buf[i]);
        sprintf(ptr,"0x%02X;",c&0x000000FF);
        ptr+=5;
        len+=5;
    }
    //qDebug() << (QString::fromStdString(std::string(ss,len)));

    if((buf[0]&0x000000ff) != 0x01||bufLen < 4)
        return ;
    if(0x22 == (buf[1]&0x000000ff))// && buf[3] == 1)
    {
        switch(buf[2]&0x000000ff)
        {
        case 0x50://前后执行结果
        case 0x51://左右执行结果
            if((buf[3]&0x000000ff) == 0)        //运动错误
            {
                moveFinished = true;
                waitHandredMs = 0;
                emit moveEnd();
                qDebug() <<"can not start move!!!!!!!";
            }
            else if((buf[3]&0x000000ff) == 1)   //运动OK
            {
                moveFinished = false;
                //qDebug() <<"start move!!!!!!!";
            }
            break;
        case 0x56:
            //这里是运动结束的
            //qDebug() <<"move finish!!!!!!!";
            moveFinished = true;
            waitHandredMs = 0;
            emit moveEnd();
            break;
        default:
            break;
        }
    }

    emit motorReadCom(buf,bufLen);
}

void Motor::move(MOTOR_MOVE_TYPE moveType,QVector<int> params)
{
    switch(moveType){
    case MOTOR_MOVE_TYPE_FORWARD:
        if(params.length()==2){
            int mm = params.at(0);
            int speed = params.at(1);
            //200 40 = 1second  ==> 10 * 100ms
            waitHandredMs =20+ mm*6/speed;
            mm *= 0x1E00;
            mm /= 3000;
            serial.WriteSyncPort(motor_enable,sizeof(motor_enable));
            serial.WriteSyncPort(motor_enable,sizeof(motor_enable));
            motor_forward[4] = speed &0x00ff;
            motor_forward[5] = (mm&0xff00)>>8;
            motor_forward[6] = (mm)&0x00ff;
            moveFinished = false;
            serial.WriteSyncPort(motor_forward,sizeof(motor_forward));
            QThread::msleep(100);
        }
        break;
    case MOTOR_MOVE_TYPE_BACKWARD:
        if(params.length()==2){
            int mm = params.at(0);
            int speed = params.at(1);
            waitHandredMs = 20+mm*6/speed;
            mm *= 0x1E00;
            mm /= 3000;
            serial.WriteSyncPort(motor_enable,sizeof(motor_enable));
            motor_backward[4] = speed &0x00ff;
            motor_backward[5] = (mm&0xff00)>>8;
            motor_backward[6] = (mm)&0x00ff;
            moveFinished = false;
            serial.WriteSyncPort(motor_backward,sizeof(motor_backward));
            QThread::msleep(100);
        }
        break;
    case MOTOR_MOVE_TYPE_TURNLEFT:
        if(params.length()==2){
            int theta = params.at(0);
            int speed = params.at(1);
            waitHandredMs = 10+theta*30/speed;
            theta*=10;
            serial.WriteSyncPort(motor_enable,sizeof(motor_enable));
            motor_left[4] = speed &0x00ff;
            motor_left[5] = (theta&0xff00)>>8;
            motor_left[6] = (theta)&0x00ff;
            moveFinished = false;
            serial.WriteSyncPort(motor_left,sizeof(motor_left));\
            QThread::msleep(100);
        }
        break;
    case MOTOR_MOVE_TYPE_TURNRIGHT:
        if(params.length()==2){
            int theta = params.at(0);
            int speed = params.at(1);
            waitHandredMs = 10+theta*30/speed;
            theta*=10;
            serial.WriteSyncPort(motor_enable,sizeof(motor_enable));
            motor_left[4] = speed &0x00ff;
            motor_right[5] = (theta>>8)&0xff;
            motor_right[6] = (theta)&0xff;
            moveFinished = false;
            serial.WriteSyncPort(motor_right,sizeof(motor_right));
            QThread::msleep(100);
        }
        break;
    case MOTOR_MOVE_TYPE_DIFFRUN:
        if(params.length()==4)
        {
            int leftDirect = params.at(0);
            int leftSpeed = params.at(1);
            int rightDirect = params.at(2);
            int rightSpeed = params.at(3);
            waitHandredMs = 99999999;
            serial.WriteSyncPort(motor_enable,sizeof(motor_enable));
            QThread::msleep(50);
            motor_diff_run[3] = leftDirect &0x00ff;
            motor_diff_run[4] = leftSpeed&0x00ff;
            motor_diff_run[5] = rightDirect&0x00ff;
            motor_diff_run[6] = rightSpeed&0x00ff;
            moveFinished = false;
            serial.WriteSyncPort(motor_diff_run,sizeof(motor_diff_run));
            QThread::msleep(100);
        }
        break;
    case MOTOR_MOVE_TYPE_STOP:
        moveFinished = true;
        waitHandredMs = 0;
        serial.WriteSyncPort(motor_stop,sizeof(motor_stop));
        QThread::msleep(100);
        break;
    case MOTOR_MOVE_TYPE_SLOWSTOP:
        moveFinished = true;
        waitHandredMs = 0;
        serial.WriteSyncPort(motor_slow_stop,sizeof(motor_slow_stop));
        QThread::msleep(100);
        break;
    }
}

void Motor::waitForMoveEnd()
{
    while(!moveFinished && (--waitHandredMs > 0))
    {
        QThread::msleep(100);
    }
}
bool Motor::isMoveFinished()
{
    return moveFinished;
}

