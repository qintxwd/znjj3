#ifndef USERCOMMANDTCPSERVER_H
#define USERCOMMANDTCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>

class UserCommandTcpServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit UserCommandTcpServer(QObject *parent = 0);
    virtual ~UserCommandTcpServer();
    bool init(int portNumber);
    void mySend(const char *str);
public slots:
    void toBeSend(const char *str);
signals:

protected slots:
    void newConnectUser();
    void readMessageUser();
private:
    QTcpSocket *tcpConnectedUserCommand;
};

#endif // USERCOMMANDTCPSERVER_H
