#ifndef EYES_H
#define EYES_H

#include <QSerialPort>

class Eyes : public QSerialPort
{
    Q_OBJECT
public:
    Eyes(QObject *parent = 0);
    virtual ~Eyes();
    bool openCom(const QString& portName,int buadrate);
public slots:
    void startRead();
};

#endif // EYES_H
