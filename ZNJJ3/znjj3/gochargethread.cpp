#include "gochargethread.h"

#include "robot.h"

GoChargeThread::GoChargeThread(QObject *parent) : QObject(parent)
{

}

void GoChargeThread::doGoCharge()  //执行线程
{
    if(g_robot.stargazermap.chargeRoute.length()==0)
    {
        emit resultReady("noRoute");
        return ;
    }
    if(CONTROLLER_BUSINESS_CHARGE == currentBusiness ||
            CONTROLLER_BUSINESS_CHARGING == currentBusiness){
        qDebug() <<QStringLiteral("已经在充电了不用再回充了");
        emit resultReady("already");
        return ;
    }

    g_robot.goAboutPosition(g_robot.stargazermap.centerPosition.x,g_robot.stargazermap.centerPosition.y,g_robot.stargazermap.centerPosition.theta);
    g_robot.waitForGoPositionFinish();

    g_robot.ttsPlay(1);

    currentBusiness = CONTROLLER_BUSINESS_CHARGE;

    QList<ROUTE>  ps = g_robot.stargazermap.chargeRoute;
    for(int i=0;i<ps.length();++i)
    {
        g_robot.arm.LEFT_UP(0);
        g_robot.arm.RIGHT_UP(0);
        g_robot.goAboutPosition(ps.at(i).position.x,ps.at(i).position.y,ps.at(i).position.theta);
        g_robot.waitForGoPositionFinish();
        QThread::msleep(200);
    }

    while(true)
    {
        int forwardTimers = 0;

        //获取3个值:
        //1.红外充电左中右
        int lmr = g_robot.stm32.getFred();      //1 right 0 middle -1 left -2 unkonw
        if(lmr==0){
            //如果找到中间了，那么调整姿态，然后向前
            int xxx = g_robot.stm32.getU1();
            if(xxx > 280)
            {
                g_robot.turnToAngle(ps.last().position.theta);
                g_robot.waitForMoveEnd();
            }
            g_robot.diffRun(0,8,1,8);
            QThread::msleep(200);
        }else if(lmr==1){
            g_robot.diffRun(0,6,1,8);
            QThread::msleep(100);
        }else if(lmr==-1){
            g_robot.diffRun(0,8,1,6);
            QThread::msleep(100);
        }

        //2.充电状态，如果电池状态变为充电状态，则停止移动
        if(g_robot.stm32.isCharging())
        {
            g_robot.stop();
            currentBusiness = CONTROLLER_BUSINESS_CHARGING;
            break;
        }

        //3.距离，如果距离充电座的距离小于20 则停止移动
        int xxx = g_robot.stm32.getU1();
        if(xxx <= 92)
        {
            g_robot.stop();

            if(g_robot.stm32.isCharging())
            {
                g_robot.stop();
                //完成
                currentBusiness = CONTROLLER_BUSINESS_CHARGING;
                break;
            }

            if(g_robot.stm32.isCharging())
            {
                g_robot.stop();
                //完成
                currentBusiness = CONTROLLER_BUSINESS_CHARGING;
                break;
            }

            if(forwardTimers<15){

                //退回，然后从来
                QVector<int> param;
                param<<300<<30;
                g_robot.motor.move(MOTOR_MOVE_TYPE_BACKWARD,param);
                //g_robot.motor.backward(300,30);
                g_robot.waitForMoveEnd();
                g_robot.goAboutPosition(ps.last().position.x,ps.last().position.y,ps.last().position.theta);
                g_robot.waitForGoPositionFinish();
                //然后重试这回充电座
                ++forwardTimers;
            }else{
                //未找到充电座 我要休息了
                emit resultReady("fail");
                //休息咯
                g_robot.ttsPlay(5);// qyhTts.playFile("5.wav");
                system("shutdown -s -t 30");
            }
        }
        QThread::msleep(50);
    }
    emit resultReady("OK");
}
