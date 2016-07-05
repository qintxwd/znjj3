#ifndef GOPOSITIONTHREAD_H
#define GOPOSITIONTHREAD_H

#include <QObject>

class GoPositionThread : public QObject
{
    Q_OBJECT
public:
    explicit GoPositionThread(QObject *parent = 0);

signals:
    void resultReady(const QString &result);        //结果
public slots:
    void doGoPosition(double x,double y,double theta);  //执行线程
};

#endif // GOPOSITIONTHREAD_H
