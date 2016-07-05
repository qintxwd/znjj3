#include "gopositionthread.h"
#include "robot.h"
GoPositionThread::GoPositionThread(QObject *parent) : QObject(parent)
{

}

void GoPositionThread::doGoPosition(double x,double y,double theta)  //执行线程
{
    g_robot.goAboutPosition(x,y,theta);
    emit resultReady("OK");
}
