#include "robot.h"

const QString DEFAULT_CONFIG_FILE_NAME = "config.xml";

Robot::Robot(QObject *parent) : QObject(parent),obTimeInterval(-1),needReturnArm(false),needReturnTts(false),needReturnMotor(false)
{

}

bool Robot::init(QString &str)
{
    //载入配置文件
    if(!configure.load(DEFAULT_CONFIG_FILE_NAME))
    {
        str = QString("%1 not exist!").arg(DEFAULT_CONFIG_FILE_NAME);
        qDebug()<<"can not open config file:"<<DEFAULT_CONFIG_FILE_NAME;
        return false;
    }
    //初始化tts
    qyhtts.init();
    connect(&qyhtts,SIGNAL(playEnd()),this,SLOT(ttsPlayEnd()));
    qyhtts.playFile("9.wav");

    //打开电机(motor)控制串口
    QString motorSerialPortName = configure.getValue("motor/serial_port");
    QString motorSerialPortBuadrate = configure.getValue("motor/serial_buadrate");
    if(motorSerialPortName.length()>0&&motorSerialPortBuadrate.length()>0)
    {
        int portNum = motorSerialPortName.right(1).toInt();
        char c = motorSerialPortName.at(motorSerialPortName.length()-2).toLower().toLatin1();
        if(c>='0'&&c<='9'){
            portNum = motorSerialPortName.right(2).toInt();
        }
        if(!motor.init(portNum,motorSerialPortBuadrate.toInt()))
        {
            str = QString("can not open motor serial port:%1").arg(motorSerialPortName);
            return false;
        }
    }else{
        str = "no configure for motor in the config file";
        return false;
    }
	connect(&motor,SIGNAL(moveEnd()),this,SLOT(motorMoveEnd()));
    //打开stm32串口
    QString stm32SerialPortName = configure.getValue("stm32/serial_port");
    QString stm32SerialPortBuadrate = configure.getValue("stm32/serial_buadrate");
    if(stm32SerialPortName.length()>0&&stm32SerialPortBuadrate.length()>0)
    {
        if(!stm32.openCom(stm32SerialPortName,stm32SerialPortBuadrate.toInt())){
            qDebug()<<"can not open STM32 serial port:"<<stm32SerialPortName;
            return false;
        }
    }else{
        str = "no configure for STM32 in the config file";
        return false;
    }

    //初始化stargazer
    QString stargazerSerialPortName = configure.getValue("stargazer/serial_port");
    QString stargazerSerialPortBuadrate = configure.getValue("stargazer/serial_buadrate");
    if(stargazerSerialPortName.length()>0&&stargazerSerialPortBuadrate.length()>0)
    {
        int portNum = stargazerSerialPortName.right(1).toInt();
        char c = stargazerSerialPortName.at(stargazerSerialPortName.length()-2).toLower().toLatin1();
        if(c>='0'&&c<='9'){
            portNum = motorSerialPortName.right(2).toInt();
        }
        if(!stargazermap.init(portNum,stargazerSerialPortBuadrate.toInt())){
            str = QString("can not open stargazer serial port:%1").arg(stm32SerialPortName);
            return false;
        }
    }else{
        str = "no configure for stargazer in the config file";
        return false;
    }
    curpos = &(stargazermap.curpos);

    //打开手臂串口
    QString armSerialPortName = configure.getValue("arm/serial_port");
    QString armSerialPortBuadrate = configure.getValue("arm/serial_buadrate");
    if(armSerialPortName.length()>0&&armSerialPortBuadrate.length()>0)
    {
        if(!arm.openCom(armSerialPortName,armSerialPortBuadrate.toInt())){
            str = QString("can not open stargazer serial port:%1").arg(armSerialPortName);
            return false;
        }
    }else{
        str = "no configure for arm in the config file";
        return false;
    }
    connect(&arm,SIGNAL(moveEnd()),this,SLOT(armMoveEnd()));

    //打开眼睛灯板串口
    QString eyeSerialPortName = configure.getValue("eye/serial_port");
    QString eyeSerialPortBuadrate = configure.getValue("eye/serial_buadrate");
    if(eyeSerialPortName.length()>0&&eyeSerialPortBuadrate.length()>0)
    {
        if(!eyes.openCom(eyeSerialPortName,eyeSerialPortBuadrate.toInt())){
            str = QString("can not open eyes serial port:%1").arg(eyeSerialPortName);
            return false;
        }
    }else{
        str = "no configure for eyes in the config file";
        return false;
    }

    //打开胸灯控制串口
    QString chestSerialPortName = configure.getValue("chest/serial_port");
    QString chestSerialPortBuadrate = configure.getValue("chest/serial_buadrate");
    if(chestSerialPortName.length()>0&&chestSerialPortBuadrate.length()>0)
    {
        if(!chest.openCom(chestSerialPortName,chestSerialPortBuadrate.toInt())){
            str = QString("can not open chest serial port:%1").arg(eyeSerialPortName);
            return false;
        }
    }else{
        str = "no configure for chest in the config file";
        return false;
    }

    //打开用户控制tcpserver
    QString userCommandPort = configure.getValue("user/command_tcp_server_port");
    if(userCommandPort.length()>0)
    {
        if(!ucts.init(userCommandPort.toInt()))
        {
            str = QString("userCommandTcp can not open port:%1").arg(userCommandPort);
            return false;
        }
    }else{
        str = "no configure for userCommandTcp in the config file";
        return false;
    }

    //初始化两个线程：

    //1.goPosition
    workerGoPosition = new GoPositionThread;
    workerGoPosition->moveToThread(&goPositionThread);
    connect(&goPositionThread, &QThread::finished, workerGoPosition, &QObject::deleteLater);
    connect(this, SIGNAL(goPositionInThread(double,double,double)), workerGoPosition, SLOT(doGoPosition(double,double,double)));
    connect(workerGoPosition,SIGNAL(resultReady(QString)), this, SLOT(handleResultsGoPosition(QString)));
    goPositionThread.start();

    //2.gocharge
    workerGoCharge = new GoChargeThread;
    workerGoCharge->moveToThread(&goChargeThread);
    connect(&goChargeThread, &QThread::finished, workerGoCharge, &QObject::deleteLater);
    connect(this, SIGNAL(goChargeInThread()), workerGoCharge, SLOT(doGoCharge()));
    connect(workerGoCharge,SIGNAL(resultReady(QString)), this, SLOT(handleResultsGoCharge(QString)));
    goChargeThread.start();

    //启动几个定时器
    sendStatusTimer.setInterval(2000);
    powerLowTimer.setInterval(20000);
    isChargingTimer.setInterval(100);
    speakWelcome.setInterval(90000);
    showPositionTimer.setInterval(500);
    checkChargeFullTimer.setInterval(5000);

    connect(&showPositionTimer,SIGNAL(timeout()),this,SLOT(showPosition()));
    connect(&speakWelcome,SIGNAL(timeout()),this,SLOT(toSpeakWelcome()));
    connect(&sendStatusTimer,SIGNAL(timeout()),this,SLOT(sendStatus()));
    connect(&powerLowTimer,SIGNAL(timeout()),this,SLOT(checkPowerLow()));
    connect(&isChargingTimer,SIGNAL(timeout()),this,SLOT(checkIsCharging()));
    connect(&checkChargeFullTimer,SIGNAL(timeout()),this,SLOT(checkChargeFull()));

    sendStatusTimer.start();
    powerLowTimer.start();
    isChargingTimer.start();
    speakWelcome.start();
    showPositionTimer.start();
    checkChargeFullTimer.start();

    return true;
}

void Robot::stop()
{
    QVector<int> p;
    motor.move(MOTOR_MOVE_TYPE_STOP,p);
}

void Robot::slowStop()
{
    QVector<int> p;
    motor.move(MOTOR_MOVE_TYPE_SLOWSTOP,p);
}

void Robot::circle()
{
    QVector<int> param;
    param<<0<<10<<0<<10;
    motor.move(MOTOR_MOVE_TYPE_DIFFRUN,param);
}

void Robot::forward(int mm,int speed)
{
	needReturnMotor = true;
    if(isGoPosition)
        return ;
    QVector<int> param;
    param<<mm<<speed;
    motor.move(MOTOR_MOVE_TYPE_FORWARD,param);
    lastMoveType = MOTOR_MOVE_TYPE_FORWARD;
}
void Robot::backward(int mm,int speed)
{
	needReturnMotor = true;
    if(isGoPosition)
        return ;
    QVector<int> param;
    param<<mm<<speed;
    motor.move(MOTOR_MOVE_TYPE_BACKWARD,param);
    lastMoveType = MOTOR_MOVE_TYPE_BACKWARD;
}
void Robot::turnLeft(int theta, int speed)
{
	needReturnMotor = true;
    if(isGoPosition)
        return ;
    QVector<int> param;
    param<<theta<<speed;
    motor.move(MOTOR_MOVE_TYPE_TURNLEFT,param);
    lastMoveType = MOTOR_MOVE_TYPE_TURNLEFT;
}
void Robot::turnRight(int theta,int speed)
{
	needReturnMotor = true;
    if(isGoPosition)
        return ;
    QVector<int> param;
    param<<theta<<speed;
    motor.move(MOTOR_MOVE_TYPE_TURNRIGHT,param);
    lastMoveType = MOTOR_MOVE_TYPE_TURNRIGHT;
}

void Robot::goPosition(double x,double y,double theta)
{
    isGoPosition = true;
    emit goPositionInThread(x,y,theta);
}
void Robot::goCharge()
{
    emit goChargeInThread();
}

void Robot::canSayWelcom(bool canOrNot)
{
    if(canOrNot){
        speakWelcome.start();
    }else{
        speakWelcome.stop();
    }
}
void Robot::diffRun(int l,int sl,int r,int sr)
{
    QVector<int> param;
    param<<l<<sl<<r<<sr;

    motor.move(MOTOR_MOVE_TYPE_DIFFRUN,param);
    lastMoveType = MOTOR_MOVE_TYPE_DIFFRUN;
}
void Robot::ttsPlay(int n)
{
    qyhtts.playIndex(n);
}

void Robot::ttsPlayReturn(int n)
{
    needReturnTts = true;
    qyhtts.playIndex(n,true);
}
void Robot::leftArm(int angle)//完成时发送返回值
{
    needReturnArm = true;
    arm.LEFT_UP(angle);
}
void Robot::rightArm(int angle)   //完成时发送返回值
{
    needReturnArm = true;
    arm.RIGHT_UP(angle);
}
void Robot::bothArm(int angle)//完成时发送返回值
{
    needReturnArm = true;
    arm.LEFT_UP(angle);
    arm.RIGHT_UP(angle);
}

void Robot::armMoveEnd()
{
    if(needReturnArm){
        ucts.mySend("zzjjok;");
        needReturnArm = false;
    }
}
void Robot::motorMoveEnd()
{
    if(needReturnMotor){
        ucts.mySend("zzjjok;");
        needReturnMotor = false;
    }
}
void Robot::ttsPlayEnd()
{
    if(needReturnTts){
        ucts.mySend("zzjjok;");
        needReturnTts = false;
    }
}

void Robot::handleResultsGoPosition(QString str)
{
    if(str=="OK"){
        ucts.mySend("zzjjok;");
        isGoPosition = false;
    }
}

void Robot::handleResultsGoCharge(QString str)
{
    if(str.length()>0){
        //TODO:返回充电结果
        qDebug() << "return to charge result:" <<str;
    }
}

void Robot::goAboutPosition(double x,double y,double theta)
{
	isGoPosition = true;
    //0.fix direction
    double aimX = x;
    double aimY = y;
    double aimTheta = theta;

    if(sqrt(pow(aimY-
                curpos->y,2)+pow(aimX-
                                 curpos->x,2))<20){
        //距离很近，直接旋转
        qDebug()<<"it is too near! no go but turn";
        turnToAngle(theta);
        return ;
    }

    //1.turn to direct  to go to aim
    double y2 = aimY +DISTANCE_BETWEEN_CAMERA_AND_MOTOR*sin(-1*aimTheta*PI/180);
    double x2 = aimX +DISTANCE_BETWEEN_CAMERA_AND_MOTOR*cos(-1*aimTheta*PI/180);

    double y1 = curpos->y+ DISTANCE_BETWEEN_CAMERA_AND_MOTOR*sin(-1*curpos->theta*PI/180);
    double x1 = curpos->x  + DISTANCE_BETWEEN_CAMERA_AND_MOTOR*cos(-1*curpos->theta*PI/180);

    double aimDirect =-1*atan2(y2-y1,x2-x1)*180/PI ;//+ angleBetweenMotorAndCamera;

    while(aimDirect>180){
        aimDirect-=360;
    }
    while(aimDirect<-180){
        aimDirect+=360;
    }
    //qDebug()<<"curpos x:"<<curpos->x<<" y:"<<curpos->y<<" t:"<<curpos->theta;
    //qDebug() <<"aimDirect:"<<aimDirect;
    turnToAngle(aimDirect);


    //qDebug() <<"turnToAngle aimDirect end!"<<aimDirect;

    //2. go to aim position
    y2 = aimY + DISTANCE_BETWEEN_CAMERA_AND_MOTOR*sin(-1*aimTheta*PI/180);
    x2 = aimX + DISTANCE_BETWEEN_CAMERA_AND_MOTOR*cos(-1*aimTheta*PI/180);

    y1 = curpos->y + DISTANCE_BETWEEN_CAMERA_AND_MOTOR*sin(-1*curpos->theta*PI/180);
    x1 = curpos->x + DISTANCE_BETWEEN_CAMERA_AND_MOTOR*cos(-1*curpos->theta*PI/180);

    double distance = sqrt(pow(y2-y1,2)+pow(x2-x1,2)) - DISTANCE_BETWEEN_CAMERA_AND_MOTOR;

    if(distance<0){
        distance = 0 - distance;
    }
    //qDebug() <<"go distance ="<<distance;

    QVector<int> param;
    param<<distance*10<<30;
    motor.move(MOTOR_MOVE_TYPE_FORWARD,param);
    lastMoveType = MOTOR_MOVE_TYPE_FORWARD;
    QThread::msleep(200);
    motor.waitForMoveEnd();
    //qDebug() <<"go distance end!";

    //3.fix current direction
    turnToAngle(aimTheta);
    //qDebug() <<"turnToAngle aimTheta end!"<<aimTheta;
	isGoPosition = false;
}

double Robot::turnToAngle(double angle)
{
    bool turnDirectLeft = true;     //turn left if true; turn right if false

    float changeValue = 0;          //how much we turn in this time
    //1.get current angle
    float angleCurrnet= curpos->theta;

    //2.calculate how much to turn
    //get how much we need to turn if we turn left and right
    float angleTurnLeft = angleCurrnet - angle;
    while(angleTurnLeft<0)
        angleTurnLeft+=360;

    float angleTurnRight = angle - angleCurrnet;
    while(angleTurnRight<0)
        angleTurnRight+=360;

    //choose to turn left or right
    changeValue = angleTurnLeft;
    if(angleTurnRight < angleTurnLeft ){
        //turn right
        turnDirectLeft = false;
        changeValue = angleTurnRight;
    }
    //3.go about aim
    QVector<int> param;
    param << changeValue << 30;
    if(turnDirectLeft){
        motor.move(MOTOR_MOVE_TYPE_TURNLEFT,param);
        lastMoveType = MOTOR_MOVE_TYPE_TURNLEFT;
    }else{
        motor.move(MOTOR_MOVE_TYPE_TURNRIGHT,param);
        lastMoveType = MOTOR_MOVE_TYPE_TURNRIGHT;
    }
    QThread::msleep(200);
    motor.waitForMoveEnd();

    //测试是否转到的要到达的位置
    double dd = curpos->theta - angle;
    if(dd>180){
        dd-=360;
    }
    if(dd<-180){
        dd+=360;
    }
    //qDebug() << "dd = "<<dd;

    if(changeValue > 30 && abs(dd)>changeValue*10/180){
        turnToAngle(angle);
    }

    return changeValue;
}

void Robot::waitForMoveEnd()
{
    motor.waitForMoveEnd();
}

void Robot::waitForGoPositionFinish()
{
    int i= 0;
    while(isGoPosition && ++i<100)
    {
        QThread::msleep(500);
    }
}
void Robot::toSpeakWelcome()
{
    if(currentBusiness == CONTROLLER_BUSINESS_IDLE)
        ttsPlay(3);
}

void Robot::showPosition()
{
    qDebug()<<QString("current position:%1   %2    %3").arg(curpos->x).arg(curpos->y).arg(curpos->theta);
}

void Robot::sendStatus()
{
    //qDebug() << "currentBusiness = "<<currentBusiness;
    if(currentBusiness == CONTROLLER_BUSINESS_CHARGE){
        //正在返回充电座
        ucts.mySend("goCharging;");
        //emit send("goCharging;");
    }else if(currentBusiness == CONTROLLER_BUSINESS_CHARGING ){
        //正在充电座上
        ucts.mySend("charging;");
        //emit send("charging;");
    }
    //发送心跳
    ucts.mySend("keepalive;");
    //emit send("keepalive;");
}

void Robot::checkIsCharging()
{
    if(stm32.isCharging())
    {
        currentBusiness = CONTROLLER_BUSINESS_CHARGING;

        chest.ledoffflicker();
        QThread::msleep(100);
        chest.ledon();
        emit sigIsCharge(true);
    }else{
        if(currentBusiness == CONTROLLER_BUSINESS_CHARGING){
            currentBusiness = CONTROLLER_BUSINESS_IDLE;
        }
        emit sigIsCharge(false);
    }
}
void Robot::queryDis()
{
    if(obTimeInterval >= 0){
        ++obTimeInterval;
    }
    if(obTimeInterval > 100){
        //够就了，又可以说话了 200ms * 100 = 20second
        obTimeInterval = -1;
    }
    if(currentBusiness == CONTROLLER_BUSINESS_CHARGE || currentBusiness == CONTROLLER_BUSINESS_CHARGING )
        return ;
    if(stm32.getU1()< 200)
    {
        //不能向前
        if(g_robot.lastMoveType == MOTOR_MOVE_TYPE_FORWARD)
        {
            g_robot.stop();
        }

        //ucts.mySend("obstruction;");
        //emit send("obstruction;");
        //遇到障碍物
        if(currentBusiness != CONTROLLER_BUSINESS_CHARGE && currentBusiness != CONTROLLER_BUSINESS_CHARGING)
        {
            if(obTimeInterval==-1)
            {
                ttsPlay(15);
                obTimeInterval = 0;
            }
        }
    }
}

void Robot::checkPowerLow()
{
    if(stm32.getBattery() <= STM32_BATTERY_CHARGE_LOWEST && currentBusiness != CONTROLLER_BUSINESS_CHARGING)
    {
        chest.ledflicker();
    }
    else if(stm32.getBattery() >= STM32_BATTERY_CHARGE_LOWEST){
        chest.ledoffflicker();
        QThread::msleep(100);
        chest.ledon();
    }

    if(stm32.getBattery() <= STM32_BATTERY_CHARGE_LOWEST)
    {
        if(currentBusiness == CONTROLLER_BUSINESS_CHARGE ||currentBusiness == CONTROLLER_BUSINESS_CHARGING)
        {
            return ;
        }
        qDebug() <<("check find:power is low!");

        //
        if(currentBusiness!=CONTROLLER_BUSINESS_IDLE){
            return ;
        }
        emit goChargeInThread();
    }
}

void Robot::checkChargeFull()
{
    if(currentBusiness==CONTROLLER_BUSINESS_CHARGING){//正在充电,检查是否充满
        if(stm32.getBattery()>STM32_BATTERY_PERFORM_LOWEST)
        {
            QVector<int> p;
            p<<500<<30;
            motor.move(MOTOR_MOVE_TYPE_BACKWARD,p);
            motor.waitForMoveEnd();
        }
    }
}

Robot g_robot;
CONTROLLER_CURRENT_STATUS currentBusiness;
