#include <QCoreApplication>
#include "robot.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString str;
    if(!g_robot.init(str)){
        qDebug() << str;
    }else{
        qDebug() << " init all OK!";
    }
    return a.exec();
}

