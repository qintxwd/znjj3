#ifndef CHEST_H
#define CHEST_H

#include <QSerialPort>

class Chest : public QSerialPort
{
    Q_OBJECT
public:
    Chest(QObject *parent = 0);
    virtual ~Chest();
    bool openCom(const QString& portName,int buadrate);
signals:
    void getRead(QString str);
public slots:
    void startRead();
public:
    void ledon();
    void ledoff();
    void ledflicker();
    void ledoffflicker();
};
#endif // CHEST_H
