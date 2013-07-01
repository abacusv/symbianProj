/*
 * Copyright (c) 2011 Nokia Corporation.
 */

#ifndef FETCHMAP_H
#define FETCHMAP_H

#include <QObject>
#include <QtNetwork>
#include <QPixmap>
#include <QPointer>

class FetchMap: public QObject
{
Q_OBJECT

public:
    FetchMap(QObject *parent = 0);
    ~FetchMap();

    QUrl fetchMapImage(const QSize& size, qreal yourLong = 0, qreal yourLat = 0, qreal friendLong =
        0, qreal friendLat = 0);

public slots:
    void handleNetworkData(QNetworkReply*);

signals:
    void mapImageLoaded(QPixmap);
    void httpError();

private:
    QNetworkAccessManager m_manager;
    QUrl m_url;
    QNetworkSession* m_session;
};

#endif // FETCHMAP_H
