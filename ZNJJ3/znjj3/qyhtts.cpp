#include "qyhtts.h"
#include "qyhttsthread.h"

QyhTts::QyhTts(QObject *parent) : QObject(parent),isPlaying(false),worker(NULL)
{
}

void QyhTts::init(){
    worker = new QyhTtsThread;
    worker->moveToThread(&playThread);
    connect(&playThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, SIGNAL(play(QString,bool)), worker, SLOT(doPlay(QString,bool)));
    connect(this, SIGNAL(stop()), worker, SLOT(stopPlay()));
    connect(worker,SIGNAL(resultReady(QString)), this, SLOT(handleResults(QString)));
    playThread.start();
}

void QyhTts::playIndex(int index,bool isMust)
{
    emit play(QString("%1.wav").arg(index),isMust);
}

void QyhTts::playFile(QString fileName,bool isMust)
{
    emit play(fileName,isMust);
}

bool QyhTts::isPlayEnd()
{
    return !isPlaying;
}

void QyhTts::handleResults(const QString &result)
{
    isPlaying=(result=="playing");
    if(result!="playing")
    {
        emit playEnd();
    }
}
