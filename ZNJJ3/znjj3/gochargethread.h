#ifndef GOCHARGETHREAD_H
#define GOCHARGETHREAD_H

#include <QObject>

class GoChargeThread : public QObject
{
    Q_OBJECT
public:
    explicit GoChargeThread(QObject *parent = 0);

signals:
    void resultReady(const QString &result);        //结果
public slots:
    void doGoCharge();  //执行线程
};

#endif // GOCHARGETHREAD_H
