#include "qyhttsthread.h"

QyhTtsThread::QyhTtsThread(QObject *parent) : QObject(parent)
{
    player = new QMediaPlayer;
    connect(player,SIGNAL(stateChanged(QMediaPlayer::State)),this,SLOT(playStatusChange(QMediaPlayer::State)));
}

void QyhTtsThread::doPlay(const QString &playFileName,bool isMust)
{
    if(!isMust){
        //非必须的播放，必须在其他播放结束的时候才可以播放
        if(player->state() == QMediaPlayer::PlayingState){
            return ;
        }
    }
    player->stop();
    player->setMedia(QUrl::fromLocalFile("media/"+playFileName));
    player->setVolume(100);
    player->play();
}

void QyhTtsThread::stopPlay()
{
    player->stop();
}

void QyhTtsThread::playStatusChange(QMediaPlayer::State s)
{
    //qDebug()<<"current status:"<<s;
    if(s==QMediaPlayer::PlayingState){
        emit resultReady("playing");
    }else{
        emit resultReady("notplaying");
    }
}
