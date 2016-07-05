#ifndef QYHTTSTHREAD_H
#define QYHTTSTHREAD_H

#include <QObject>
#include <QMediaPlayer>
#include <QThread>

class QyhTtsThread : public QObject
{
    Q_OBJECT
public:
    explicit QyhTtsThread(QObject *parent = 0);
signals:
    void resultReady(const QString &result);
public slots:
    void doPlay(const QString &playFileName, bool isMust);
    void playStatusChange(QMediaPlayer::State s);
    void stopPlay();
private:
    QMediaPlayer *player;
};

#endif // QYHTTSTHREAD_H
