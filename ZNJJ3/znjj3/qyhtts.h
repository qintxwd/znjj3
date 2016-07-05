#ifndef QYHTTS_H
#define QYHTTS_H
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QObject>
#include <QThread>
#include "qyhttsthread.h"

class QyhTts : public QObject
{
    Q_OBJECT
    QThread playThread;
public:
    explicit QyhTts(QObject *parent = 0);
    void init();
    bool isPlayEnd();
    void playFile(QString fileName,bool isMust=false);
    void playIndex(int index,bool isMust=false);
signals:
    void play(QString fileName,bool isMust);
    void stop();
    void playEnd();
public slots:
    void handleResults(const QString &result);
private:
    volatile bool isPlaying;
    QyhTtsThread *worker;
};

#endif // QYHTTS_H
