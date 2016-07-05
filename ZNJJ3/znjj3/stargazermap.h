#ifndef STARGAZERMAP_H
#define STARGAZERMAP_H
#include <QMutex>
#include <QThread>
#include <QMap>
//#include <QTimer>
#include "stargazer.h"

typedef enum stargazer_status{
    STARGAZER_STATUS_UNKONW = 0,
    STARGAZER_STATUS_BEFORE_CREATE_MAP,
    STARGAZER_STATUS_CREATE_MAP,
    STARGAZER_STATUS_UPDATE_MAP,
}STARGAZER_STATUS;

enum EnumEventStarGazer
{
    EnumEvent_CalcStart = 0,
    EnumEvent_ThrAlg,
    EnumEvent_MarkMode,
    EnumEvent_HeightCalc,
    EnumEvent_SetEnd,
    EnumEvent_CalcStop,
};

typedef struct route{
    QString textToSpeak;
    SGPOSITION position;
    bool leftHandUp;
    bool rightHandUp;
}ROUTE;

class StargazerMap : public QObject
{
    Q_OBJECT
public:
    StargazerMap();
    virtual ~StargazerMap();
    bool init(int portNumber, int buadrate);

    bool startCreateMap();
    void finishCreateMap();

    void startCalc();
    void stopCalc();

    void syn();

    //当前位置，实时更新
    SGPOSITION curpos;

    //charge 路径
    QList<ROUTE> chargeRoute;

    //默认位置(执行完一个指令后自动回到该位置)
    SGPOSITION centerPosition;
signals:
    void createMapEvent(int eventId,bool isSuccess);
    void getStarPositions(int id,double x,double y,double theta);
public slots:
    void getSerialRead(QString readQStr);
    void createMapEventFilter(int eventId,bool isSuccess);
protected:

private:
    bool load();

    QList<STAR> stars;

    QMutex mutexStars;

    QMutex mutexChargeRoute;

    STAR calculate(const STAR& registerStar,const STAR& valueOfRegisterStar,const STAR& valueOfEvaluate);

    STAR kfStar(const STAR& oldValue,const STAR& newValue);

    STARGAZER_STATUS status;

    bool isCreateMap;

    int originStarId;

    Stargazer g_stargazer;
};

#endif // STARGAZERMAP_H
