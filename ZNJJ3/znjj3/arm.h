#ifndef ARM_H
#define ARM_H

#include <QSerialPort>

class Arm : public QSerialPort
{
    Q_OBJECT
public:
    Arm(QObject *parent = 0);
    virtual ~Arm();
    bool openCom(const QString& portName,int buadrate);
signals:
    void readStr(QString str);
    void moveEnd();
public slots:
    void startRead();
    void LEFT_UP(int angle);
    void RIGHT_UP(int angle);
    void enable(bool isEnable = true);
private:
    bool isMoveEnd;
};
#endif // ARM_H
