#ifndef CONFIGURE_H
#define CONFIGURE_H

#include <QObject>
#include <QDomDocument>
class Configure : public QObject
{
    Q_OBJECT
public:
    explicit Configure(QObject *parent = 0);

    virtual ~Configure();

    bool load(const QString &filePath);

    QString getValue(const QString &what);

signals:

public slots:

private:
    QDomDocument doc;
    QDomElement root;

};
#endif // CONFIGURE_H
