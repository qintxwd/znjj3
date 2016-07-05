#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include <QObject>

class TcpThread : public QObject
{
    Q_OBJECT
public:
    explicit TcpThread(QObject *parent = 0);

signals:

public slots:
};

#endif // TCPTHREAD_H
