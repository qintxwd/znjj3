#include "configure.h"
#include <QFile>
#include <QDebug>

Configure::Configure(QObject *parent) : QObject(parent)
{

}

Configure::~Configure()
{
}

bool Configure::load(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
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

    root = doc.firstChildElement("config");

    return true;
}

QString Configure::getValue(const QString &what)
{
    QStringList qsl = what.split("/");
    if(qsl.length()==2){
        return root.firstChildElement(qsl.at(0)).attribute(qsl.at(1));
    }
    return "";
}

