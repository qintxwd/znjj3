#include "usercommandtcpserver.h"

#include <QTextCodec>

#include "robot.h"

UserCommandTcpServer::UserCommandTcpServer(QObject *parent)
    :QTcpServer(parent),
      tcpConnectedUserCommand(NULL)
{

}
UserCommandTcpServer::~UserCommandTcpServer()
{
    if(tcpConnectedUserCommand){
        tcpConnectedUserCommand->close();
        tcpConnectedUserCommand = NULL;
    }
    close();
}

bool UserCommandTcpServer::init(int portNumber)
{
    if(!listen(QHostAddress::Any,portNumber))
        return false;

    connect(this,SIGNAL(newConnection()),this,SLOT(newConnectUser()));
    return true;
}

void UserCommandTcpServer::toBeSend(const char *str)
{
    mySend(str);
}

void UserCommandTcpServer::mySend(const char *str)
{
    if(tcpConnectedUserCommand)
    {
        qDebug() <<"send:"<<str;
        tcpConnectedUserCommand->write(str);
    }
}

void UserCommandTcpServer::newConnectUser()
{
    if(tcpConnectedUserCommand)
    {
        tcpConnectedUserCommand->close();
        tcpConnectedUserCommand = NULL;
    }
    tcpConnectedUserCommand = nextPendingConnection();
    connect(tcpConnectedUserCommand,SIGNAL(readyRead()),this,SLOT(readMessageUser()));
}

void UserCommandTcpServer::readMessageUser()
{
    QByteArray qba = tcpConnectedUserCommand->readAll();
    //this->ui->textBrowser__user_data->append(qba);
    QString qstr(qba);
    if(!qstr.startsWith("a:"))
        qDebug()<<"read from user:"<<qstr;
    QStringList qqsl = qstr.split(";");
    for(int i=0;i<qqsl.length();++i)
    {
        QStringList qqqsl = qqsl[i].split(":");
        if(qqqsl.length()<=0||qqqsl[0]!="zzjj")
            continue;
        //        1 停止和开启欢迎功能（停止说那句欢迎的话）
        //        zzjj:welcome:n;(n = 0关闭，n = 1开启)
        //        功能设置完成后反馈
        //        zzjjok;
        if(qqqsl.length()==3 &&qqqsl[1]=="welcome" ){
            if(qqqsl[2]=="0"){
                g_robot.canSayWelcom(false);
                mySend("zzjjok;");
            }else if(qqqsl[2]=="1"){
                g_robot.canSayWelcom(true);
                mySend("zzjjok;");
            }
        }

        //        2 播放音频
        //        zzjj:play:n;(n就是音频文件的名字)
        //        播放结束后反馈
        //        zzjjok;
        if(qqqsl.length()==3 &&qqqsl[1]=="play" ){
            int n = qqqsl[2].toInt();
            g_robot.ttsPlayReturn(n);
        }

        //        3 机器人移动
        //        zzjj:move:x:y:a;(x,y,a浮点数,分别是坐标和角度)
        //        移动完成后反馈
        //        zzjjok;
        if(qqqsl.length()==5 &&qqqsl[1]=="move" ){
            double x = qqqsl[2].toDouble();
            double y = qqqsl[3].toDouble();
            double theta = qqqsl[4].toDouble();
            g_robot.goPosition(x,y,theta);
        }

        //        4 机器人手臂
        //        zzjj:leftarm:a;(抬左臂，aa浮点数,代表角度)
        //        zzjj:rightarm:a;(抬右臂aa浮点数,代表角度)
        //        zzjj:botharm:a;(抬双臂aa浮点数,代表角度)
        //        移动完成后反馈
        //        zzjjok;
        if(qqqsl.length()==3 &&qqqsl[1]=="leftarm" ){
            double a = qqqsl[2].toDouble();
            g_robot.leftArm(a);
        }
        if(qqqsl.length()==3 &&qqqsl[1]=="rightarm" ){
            double a = qqqsl[2].toDouble();
            g_robot.rightArm(a);
        }
        if(qqqsl.length()==3 &&qqqsl[1]=="botharm" ){
            double a = qqqsl[2].toDouble();
            g_robot.leftArm(a);
            g_robot.rightArm(a);
        }
        if(qqqsl.length()==4 && qqqsl[1]=="forward" ){
            int mm = qqqsl[2].toInt();
            int speed = qqqsl[3].toInt();
            g_robot.forward(mm,speed);
        }
        if(qqqsl.length()==4 && qqqsl[1]=="backward" ){
            int mm = qqqsl[2].toInt();
            int speed = qqqsl[3].toInt();
            g_robot.backward(mm,speed);
        }
    }
}
