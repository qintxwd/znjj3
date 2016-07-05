#include "stargazermap.h"
#include <QDebug>
#include <QFile>
#include <QPair>
#include <QDomDocument>
#include "qyhtts.h"
#include <math.h>

StargazerMap::StargazerMap():
    status(STARGAZER_STATUS_UNKONW),
    originStarId(-1),
    isCreateMap(false)
{

}

StargazerMap::~StargazerMap()
{
}

bool StargazerMap::init(int portNumber,int buadrate)
{
    //timerToSyn.setInterval(300000);
    //1.load from xml file
    if(!g_stargazer.init(portNumber,buadrate))
        return false;

    connect(&g_stargazer,SIGNAL(getSerialRead(QString)),this,SLOT(getSerialRead(QString)));
    connect(this,SIGNAL(createMapEvent(int,bool)),this,SLOT(createMapEventFilter(int,bool)));
    //connect(&timerToSyn,SIGNAL(timeout()),this,SLOT(synOnTimer()));

    //timerToSyn.start();
    if(!load()){
        qDebug()<< "can not load data from xml";
        return false;
    }

    //check the origin haing been loaded!
    for(int i=0;i<stars.length();++i){
        if(stars.at(i).x==0&&stars.at(i).y ==0 && stars.at(i).theta==0){
            status = STARGAZER_STATUS_UPDATE_MAP;
        }
    }
    return true;
}

bool StargazerMap::startCreateMap()
{
    //isCreateMap = true;

    status = STARGAZER_STATUS_BEFORE_CREATE_MAP;
    curpos.theta = 0;
    curpos.x = 0;
    curpos.y = 0;
    originStarId = -1;
    mutexStars.lock();
    stars.clear();
    mutexStars.unlock();

    isCreateMap = true;

    g_stargazer.sendToStargazer(Enum_CalcStart);
    emit createMapEvent(EnumEvent_CalcStart,true);

    return true;

}

void StargazerMap::startCalc()
{
    g_stargazer.sendToStargazer(Enum_CalcStart);
}
void StargazerMap::stopCalc()
{
    g_stargazer.sendToStargazer(Enum_CalcStop);
}

void StargazerMap::finishCreateMap()
{
    syn();
    status = STARGAZER_STATUS_UPDATE_MAP;
}

void StargazerMap::getSerialRead(QString readQStr)
{
    if(status ==STARGAZER_STATUS_UNKONW)
        return ;
    //qDebug() << readQStr;
    QStringList qsl = readQStr.split("`");
    for(int i=0;i<qsl.length();++i)
    {
        QString tempStr = qsl.at(i);
        if(tempStr.startsWith("~*")&&tempStr.contains("DeadZone")){
            qDebug() << tempStr;
            curpos.theta = 0;
            curpos.x = 0;
            curpos.y = 0;
        }else if(tempStr.contains("!")||tempStr.startsWith("$")||tempStr.contains("ark")||tempStr.contains("cal")||tempStr.contains("heig")||tempStr.contains("Thr"))
        {
            qDebug() << tempStr;
            tempStr = tempStr.right(tempStr.length() - 2);
            if(tempStr.contains("CalcStart")||tempStr.contains("CalcStar")||tempStr.contains("Start")||tempStr.contains("tart")){
                qDebug() << tempStr;
                //emit createMapEvent(EnumEvent_CalcStart,true);
            }else if(tempStr.contains("CalcStop")){
                qDebug() << tempStr;
                //emit createMapEvent(EnumEvent_CalcStop,true);
            }else if(tempStr.contains("ThrAlg")||tempStr.contains("hrAlg")||tempStr.contains("rAlg")||tempStr.contains("Alg")||tempStr.contains("rAg")){
                qDebug() << tempStr;
                if(tempStr.contains("Auto")||tempStr.contains("Ao")||tempStr.contains("ut")){
                    //emit createMapEvent(EnumEvent_ThrAlg,true);
                }else{
                    //emit createMapEvent(EnumEvent_ThrAlg,false);
                }
            }else if(tempStr.contains("MarkMode")){
                if(tempStr.contains("Alone"))
                {
                    qDebug() << tempStr;
                    //emit createMapEvent(EnumEvent_MarkMode,true);
                }else{
                    qDebug() << tempStr;
                    //emit createMapEvent(EnumEvent_MarkMode,false);
                }
            }else if(tempStr.contains("HeightCalc")){
                qDebug() << tempStr;
                //emit createMapEvent(EnumEvent_HeightCalc,true);
            }else if(tempStr.contains("ParameterUpdate")){
                qDebug() << tempStr;
                //emit createMapEvent(EnumEvent_SetEnd,true);
            }
        }else if(tempStr.startsWith("~^"))
        {
            if(status == STARGAZER_STATUS_BEFORE_CREATE_MAP)
                continue;
            if(tempStr.mid(2,1)=="z"||tempStr.mid(2,1)=="Z")
                continue;

            int staramount;;
            if((staramount = tempStr.mid(2,1).toInt())<=0)
                continue ;

            tempStr = tempStr.right(tempStr.length() - 3);
            QStringList tempQsl = tempStr.split("|");
            if(tempQsl.length()!=(4*staramount))
                continue;

            QList<STAR> findstar;
            for(int i=0;i<staramount;++i)
            {
                STAR sstar;
                sstar.theta = tempQsl.at(4*i).toDouble();
                sstar.x = tempQsl.at(4*i+1).toDouble();
                sstar.y = tempQsl.at(4*i+2).toDouble();
                sstar.id = tempQsl.at(4*i+3).toInt();
                //需要将贴的star贴片的ID写在下面，对于检测到的干扰直接忽略。
                if(sstar.id!=1460&&sstar.id!=9938&&sstar.id!=9952&&sstar.id!=9954){
                    continue;
                }else{
                    emit getStarPositions(sstar.id,sstar.x,sstar.y,sstar.theta);
                }
                findstar.append(sstar);
            }
            if(findstar.length()<=0)
                continue;

            if(status==STARGAZER_STATUS_CREATE_MAP && findstar.length()<=1)
                continue;

            //1.check if star has register
            int hasRegisterStarId = -1;
            STAR registerStar;
            STAR valueOfRegisterStar;
            STAR valueOfEvaluate;
            STAR starEvaluate;
            //分成两个部分，一个是更新地图，一个是创建地图
            if(status==STARGAZER_STATUS_CREATE_MAP)
            {
                if(findstar.length()<=1)
                    continue;

                for(int i=0;i<stars.length();++i)
                {
                    for(int j=0;j<findstar.length();++j)
                    {
                        if(stars.at(i).id == findstar.at(j).id)
                        {
                            if(hasRegisterStarId==-1)
                            {
                                hasRegisterStarId = findstar.at(j).id;
                                registerStar = stars.at(i);
                                valueOfRegisterStar = findstar.at(j);
                            }else if(stars.at(i).x ==0 &&stars.at(i).y ==0 &&stars.at(i).theta ==0){
                                //origin come first
                                hasRegisterStarId = findstar.at(j).id;
                                registerStar = stars.at(i);
                                valueOfRegisterStar = findstar.at(j);
                                break;
                            }
                        }
                    }
                }

                if(hasRegisterStarId==-1)//检测到点都未注册过
                {
                    if( originStarId != -1) //原地已经注册
                    {
                        continue;
                    }
                    //原地尚未注册，则该点注册为原点
                    STAR originStar ;
                    originStarId = findstar.at(0).id;
                    originStar.id = originStarId;
                    hasRegisterStarId = originStarId;
                    registerStar = originStar;
                    valueOfRegisterStar = findstar.at(0);
                    mutexStars.lock();
                    stars.append(originStar);
                    mutexStars.unlock();
                    //将后边的也一并注册了
                    for(int j = 1;j<findstar.length();++j)
                    {
                        valueOfEvaluate = findstar.at(j);
                        starEvaluate = calculate(registerStar,valueOfRegisterStar,valueOfEvaluate);
                        if(starEvaluate.id==-1)
                        {
                            mutexStars.lock();
                            stars.append(starEvaluate);
                            mutexStars.unlock();
                        }
                    }
                }else
                {
                    //其中有点已经注册了，或者全都注册过，则以最后一个注册过的ID为准，校准其他ID的值
                    for(int j = 0;j<findstar.length();++j)
                    {
                        valueOfEvaluate = findstar.at(j);
                        if(valueOfEvaluate.id == hasRegisterStarId)
                            continue;
                        starEvaluate = calculate(registerStar,valueOfRegisterStar,valueOfEvaluate);
                        if(starEvaluate.id==-1)
                            continue;
                        //检查是否已经存在，如果存在则滤波更新，否则直接保存
                        bool isExits = false;
                        mutexStars.lock();
                        for(int j=0;j<stars.length();++j){
                            if(stars.at(j).id == starEvaluate.id)
                            {
                                //已经存在，滤波更新
                                isExits = true;
                                qDebug()<<"old---> x="<<stars.at(j).x<<" y="<<stars.at(j).y<<" t="<<stars.at(j).theta<<" id="<<stars.at(j).id;
                                qDebug()<<"new---> x="<<starEvaluate.x<<" y="<<starEvaluate.y<<" t="<<starEvaluate.theta<<" id="<<starEvaluate.id;
                                //update
                                STAR newValue = kfStar(stars.at(j),starEvaluate);
                                if(newValue.id!=-1){
                                    stars.removeAt(j);
                                    stars.append(newValue);
                                    qDebug()<<"last---> x="<<newValue.x<<" y="<<newValue.y<<" t="<<newValue.theta<<" id="<<newValue.id;
                                }
                                break;
                            }
                        }
                        mutexStars.unlock();
                        //如果不存在，则直接保存
                        if(!isExits)
                        {
                            qDebug()<<"create it if it no eixts and status of now is create map";
                            //create it if it no eixts and status of now is create map
                            mutexStars.lock();
                            stars.append(starEvaluate);
                            mutexStars.unlock();
                        }
                    }
                }
            }else if(status==STARGAZER_STATUS_UPDATE_MAP){
                //计算当前位置信息，取平均值
                SGPOSITION temptemp;
                int tempCount = 0;
                for(int i = 0;i<findstar.length();++i)
                {
                    for(int j = 0;j<stars.length();++j)
                    {
                        if(findstar.at(i).id == stars.at(j).id)
                        {
                            double registerRadius = sqrt(pow(findstar.at(i).x,2)+pow(findstar.at(i).y,2));
                            double registerAngle = atan2(findstar.at(i).y,findstar.at(i).x)*180/PI + stars.at(j).theta;
                            while(registerAngle < -180)
                            {
                                registerAngle += 360;
                            }
                            while( registerAngle > 180)
                            {
                                registerAngle -= 360;
                            }
                            double xReInOrigin = registerRadius*cos(registerAngle*PI/180);
                            double yReInOrigin = registerRadius*sin(registerAngle*PI/180);

                            double tempTheta = findstar.at(i).theta - stars.at(j).theta;

                            while (tempTheta < -180 )
                            {
                                tempTheta += 360;
                            }
                            while( tempTheta > 180 )
                            {
                                tempTheta -= 360;
                            }
                            temptemp.x += xReInOrigin + stars.at(j).x;
                            temptemp.y += yReInOrigin + stars.at(j).y;
                            temptemp.theta += tempTheta;
                            ++tempCount;
                        }
                    }
                }
                if(tempCount>=1)
                {
                    temptemp.x /=tempCount;
                    temptemp.y /=tempCount;
                    temptemp.theta /=tempCount;
                    if(curpos.x ==0 && curpos.y ==0 && curpos.theta ==0){
                        curpos = temptemp;
                    }else if((temptemp.theta>140 && curpos.theta<-140) ||(curpos.theta>140 && temptemp.theta<-140)){
                        curpos = temptemp;
                    }else if(abs(temptemp.theta - curpos.theta )<120){
                        curpos = temptemp;
                    }
                }
            }
        }else{

        }
    }
}


void StargazerMap::createMapEventFilter(int eventId,bool isSuccess)
{
    switch(eventId)
    {
    case EnumEvent_CalcStart:
        if(isSuccess && isCreateMap)
        {
            QThread::msleep(3000);
            g_stargazer.sendToStargazer(Enum_ThrAlg);
            isCreateMap = false;

            //TODO:
            QThread::msleep(3000);
            g_stargazer.sendToStargazer(Enum_MarkMode);


            QThread::msleep(3000);
            g_stargazer.sendToStargazer(Enum_CalcHeight);

            QThread::msleep(10000);
            g_stargazer.sendToStargazer(Enum_SetEnd);

            QThread::msleep(300);
            g_stargazer.sendToStargazer(Enum_CalcStart);
            status = STARGAZER_STATUS_CREATE_MAP;
        }
        break;
    case EnumEvent_ThrAlg:
        if(isSuccess){
            QThread::msleep(300);
            g_stargazer.sendToStargazer(Enum_MarkMode);
        }
        break;
    case EnumEvent_MarkMode:
        if(isSuccess){
            QThread::msleep(300);
            g_stargazer.sendToStargazer(Enum_CalcHeight);
        }
        break;
    case EnumEvent_HeightCalc:
        if(isSuccess){
            QThread::msleep(800);
            g_stargazer.sendToStargazer(Enum_SetEnd);
        }
        break;
    case EnumEvent_SetEnd:
        if(isSuccess){
            QThread::msleep(300);
            g_stargazer.sendToStargazer(Enum_CalcStart);
            status = STARGAZER_STATUS_CREATE_MAP;
        }
        break;
    case EnumEvent_CalcStop:
        break;
    default:
        break;
    }
}

void StargazerMap::syn()
{
    QDomDocument doc;
    QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(instruction);
    QDomElement root = doc.createElement("stargazer");
    doc.appendChild(root);
    QDomElement deCommon = doc.createElement("common");
    root.appendChild(deCommon);
    QDomElement deStars = doc.createElement("stars");
    root.appendChild(deStars);
    QDomElement dePositions = doc.createElement("positions");
    root.appendChild(dePositions);
    QDomElement deCenterPosition = doc.createElement("centerPosition");
    root.appendChild(deCenterPosition);

    QDomElement deGuard = doc.createElement("guard");
    root.appendChild(deGuard);
    QDomElement deCharge = doc.createElement("charge");
    root.appendChild(deCharge);
    QDomElement deCradle = doc.createElement("cradle");
    root.appendChild(deCradle);


    deCommon.setAttribute("distance_between_move_center_and_camera",g_stargazer.distanceBetweenMoveCenterAndCamera);
    deCommon.setAttribute("angle_between_motor_and_camera",g_stargazer.angleBetweenMotorAndCamera);

    mutexStars.lock();
    for(int i=0;i<stars.length();++i)
    {
        STAR s = stars.at(i);
        QDomElement deStar = doc.createElement("star");
        deStars.appendChild(deStar);
        deStar.setAttribute("x",s.x);
        deStar.setAttribute("y",s.y);
        deStar.setAttribute("theta",s.theta);
        deStar.setAttribute("id",s.id);
    }
    mutexStars.unlock();

    mutexChargeRoute.lock();
    for(int i=0;i<chargeRoute.length();++i)
    {
        QDomElement deP = doc.createElement("p");
        deCharge.appendChild(deP);
        deP.setAttribute("x",chargeRoute.at(i).position.x);
        deP.setAttribute("y",chargeRoute.at(i).position.y);
        deP.setAttribute("theta",chargeRoute.at(i).position.theta);
        deP.setAttribute("text",chargeRoute.at(i).textToSpeak);
        deP.setAttribute("leftHand",chargeRoute.at(i).leftHandUp?"1":"0");
        deP.setAttribute("rightHand",chargeRoute.at(i).rightHandUp?"1":"0");
    }
    mutexChargeRoute.unlock();

    deCenterPosition.setAttribute("x",centerPosition.x);
    deCenterPosition.setAttribute("y",centerPosition.y);
    deCenterPosition.setAttribute("theta",centerPosition.theta);

    QFile fileModify("stargazer.xml");
    if (!fileModify.open(QFile::WriteOnly | QFile::Text)){
        return ;
    }

    QTextStream out(&fileModify);
    doc.save(out,4);
    fileModify.close();
}

bool StargazerMap::load()
{
    QDomDocument doc("readxml");
    mutexStars.lock();
    stars.clear();
    mutexStars.unlock();

    QFile file("stargazer.xml");
    if (!file.open(QIODevice::ReadWrite))
        return false;
    QString errorMsg;
    int errorLine;
    int errorColumn;
    if (!doc.setContent(&file,&errorMsg,&errorLine,&errorColumn))
    {
        qDebug()<<"setcontent fail!"<<errorMsg<<" line:"<<errorLine<<" column:"<<errorColumn;
        file.close();
        return false;
    }
    file.close();



    QDomElement root = doc.firstChildElement("stargazer");
    QDomElement eltcommon = root.firstChildElement("common");
    QDomElement eltStars = root.firstChildElement("stars");
    QDomElement eltPositions = root.firstChildElement("positions");
    QDomElement eltCenterPosition = root.firstChildElement("centerPosition");

    QDomElement eltCharge = root.firstChildElement("charge");
    QDomElement eltGuard = root.firstChildElement("guard");
    QDomElement eltCradle = root.firstChildElement("cradle");
    //common
    g_stargazer.distanceBetweenMoveCenterAndCamera = eltcommon.toElement().attribute("distance_between_move_center_and_camera").toDouble();
    g_stargazer.angleBetweenMotorAndCamera = eltcommon.toElement().attribute("angle_between_motor_and_camera").toDouble();

    //read stars
    QDomNodeList domNodeStars = eltStars.childNodes();
    for(int i=0;i<domNodeStars.length();++i){
        QDomNode tdn = domNodeStars.at(i);
        if(tdn.toElement().tagName()=="star"){
            STAR tempstar;
            tempstar.x = tdn.toElement().attribute("x").toDouble();
            tempstar.y = tdn.toElement().attribute("y").toDouble();
            tempstar.theta = tdn.toElement().attribute("theta").toDouble();
            tempstar.id  = tdn.toElement().attribute("id").toInt();
            mutexStars.lock();
            stars.push_back(tempstar);
            mutexStars.unlock();
        }
    }

    //    //read positions
    //    QDomNodeList domNodePositions = eltPositions.childNodes();
    //    for(int i=0;i<domNodePositions.length();++i)
    //    {
    //        QDomNode tdn = domNodePositions.at(i);
    //        qDebug()<<tdn.toElement().tagName();
    //        if(tdn.toElement().tagName()=="position"){
    //            QString positionName = tdn.toElement().attribute("name");
    //            QList<SGPOSITION> ps;
    //            QDomNodeList domNodePs = tdn.childNodes();
    //            for(int j=0;j<domNodePs.length();++j){
    //                QDomNode ttdn = domNodePs.at(j);
    //                SGPOSITION p;
    //                p.x = ttdn.toElement().attribute("x").toDouble();
    //                p.y = ttdn.toElement().attribute("y").toDouble();
    //                p.theta = ttdn.toElement().attribute("theta").toDouble();
    //                ps.append(p);
    //            }

    //            mutexPositions.lock();
    //            positions.insert(positionName,ps);
    //            mutexPositions.unlock();
    //        }
    //    }

    //read charge
    QDomNodeList domNodes = eltCharge.childNodes();
    for(int i=0;i<domNodes.length();++i)
    {
        QDomNode tdn = domNodes.at(i);
        if(tdn.toElement().tagName()=="p")
        {
            ROUTE r;
            r.textToSpeak= tdn.toElement().attribute("text");
            //SGPOSITION p;
            r.position.x = tdn.toElement().attribute("x").toDouble();
            r.position.y = tdn.toElement().attribute("y").toDouble();
            r.position.theta = tdn.toElement().attribute("theta").toDouble();
            r.leftHandUp = tdn.toElement().attribute("leftHand").toInt() == 1;
            r.rightHandUp = tdn.toElement().attribute("rightHand").toInt() == 1;
            mutexChargeRoute.lock();
            chargeRoute.push_back(r);
            mutexChargeRoute.unlock();
        }
    }

    //read center position
    centerPosition.x = eltCenterPosition.toElement().attribute("x").toDouble();
    centerPosition.y = eltCenterPosition.toElement().attribute("y").toDouble();
    centerPosition.theta = eltCenterPosition.toElement().attribute("theta").toDouble();

    return true;
}

STAR StargazerMap::calculate(const STAR& registerStar,const STAR& valueOfRegisterStar,const STAR& valueOfEvaluate)
{
    STAR sstar;
    if(registerStar.id!=valueOfRegisterStar.id)
        return sstar;
    //formula:
    //(Tre - Tev = Tre - Tev)
    //(Xre + Xreinorigin = Xev + Xevinorigin)
    //(Yre + Yreinorigin = Yev + Yevinorigin)

    //1.
    double registerRadius = sqrt(pow(valueOfRegisterStar.x,2)+pow(valueOfRegisterStar.y,2));
    double registerAngle = atan2(valueOfRegisterStar.y,valueOfRegisterStar.x)*180/PI + registerStar.theta;
    if ( registerAngle < -180 )
    {
        registerAngle += 360;
    }else if ( registerAngle > 180 )
    {
        registerAngle -= 360;
    }
    double xReInOrigin = registerRadius*cos(registerAngle*PI/180);
    double yReInOrigin = registerRadius*sin(registerAngle*PI/180);

    curpos.theta = valueOfRegisterStar.theta - registerStar.theta;

    if (curpos.theta < -180 )
    {
        curpos.theta += 360;
    }else if ( curpos.theta > 180 )
    {
        curpos.theta -= 360;
    }
    curpos.x = xReInOrigin +registerStar.x;
    curpos.y = yReInOrigin +registerStar.y;

    //2.
    double evaluateRadius = sqrt(pow(valueOfEvaluate.x,2)+pow(valueOfEvaluate.y,2));
    double evaluateAngle = atan2(valueOfEvaluate.y,valueOfEvaluate.x)*180/PI + valueOfEvaluate.theta - valueOfRegisterStar.theta + registerStar.theta;
    if ( evaluateAngle < -180 )
    {
        evaluateAngle += 360;
    }else if ( evaluateAngle > 180 )
    {
        evaluateAngle -= 360;
    }
    double xEvInOrigin = evaluateRadius*cos(evaluateAngle*PI/180);
    double yEvInOrigin = evaluateRadius*sin(evaluateAngle*PI/180);

    //3.
    sstar.id = valueOfEvaluate.id;
    sstar.theta = valueOfEvaluate.theta - valueOfRegisterStar.theta + registerStar.theta;
    if ( sstar.theta < -180 )
    {
        sstar.theta += 360;
    }else if ( sstar.theta > 180 )
    {
        sstar.theta -= 360;
    }
    sstar.x = xReInOrigin - xEvInOrigin + registerStar.x;
    sstar.y = yReInOrigin - yEvInOrigin + registerStar.y;

    return sstar;
}

STAR StargazerMap::kfStar(const STAR& oldValue,const STAR& newValue)
{
    STAR sstar;
    if(oldValue.id!=newValue.id)
        return sstar;

    double kgx = sqrt(pow(oldValue.qx,2)/(pow(oldValue.qx,2)+pow(newValue.qx,2)));
    sstar.x = oldValue.x + kgx *(newValue.x - oldValue.x);
    sstar.qx = sqrt(pow(oldValue.qx,2)*(1 - kgx));

    double kgy = sqrt(pow(oldValue.qy,2)/(pow(oldValue.qy,2)+pow(newValue.qy,2)));
    sstar.y = oldValue.y + kgy *(newValue.y - oldValue.y);
    sstar.qy = sqrt(pow(oldValue.qy,2)*(1 - kgy));

    double kgtheta = sqrt(pow(oldValue.qtheta,2)/(pow(oldValue.qtheta,2)+pow(newValue.qtheta,2)));
    sstar.theta = oldValue.theta + kgtheta *(newValue.theta - oldValue.theta);
    sstar.qtheta = sqrt(pow(oldValue.qtheta,2)*(1 - kgtheta));
    if ( sstar.qtheta < -180 )
    {
        sstar.qtheta += 360;
    }else if ( sstar.qtheta > 180 )
    {
        sstar.qtheta -= 360;
    }
    return sstar;

}
