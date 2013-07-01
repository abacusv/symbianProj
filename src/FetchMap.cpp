/*
 * Copyright (c) 2011 Nokia Corporation.
 */

#include "FetchMap.h"
#include <QImage>

// Ovi Maps Rendering API
// https://projects.forum.nokia.com/ovimapsrenderingapi

// Ovi Maps Rendering API
// http://m.ovi.me/?c=62.240,25.76&z=16&u=1h&w=2500&h=2500
const QString
    MAPS_STATICMAP_URL_TEMPLATE_You =
    "http://m.ovi.me/?c=%1,%2&z=16&u=1h&w=%3&h=%4";

// Ovi Maps Rendering API
// http://m.ovi.me/?w=360&h=360&poi=62.241,25.761,32.230,25.78
const QString
    MAPS_STATICMAP_URL_TEMPLATE_You_and_Friend =
        "http://m.ovi.me/?w=%3&h=%4&poi=%1,%2,%5,%6";

FetchMap::FetchMap(QObject *parent) :
    QObject(parent)
{
    QObject::connect(&m_manager, SIGNAL(finished(QNetworkReply*)),
    this, SLOT(handleNetworkData(QNetworkReply*)));
}

FetchMap::~FetchMap()
{
}

QUrl FetchMap::fetchMapImage(const QSize& size, qreal yourLong, qreal yourLat, qreal friendLong,
    qreal friendLat)
{
    QString url;
    if (friendLong == 0 || friendLat == 0) {
        url = MAPS_STATICMAP_URL_TEMPLATE_You .arg(QString::number(yourLat,'g',8)) .arg(QString::number(
            yourLong,'g',8)) .arg(QString::number(size.width())) .arg(QString::number(size.height()));
    }
    else {
        url = MAPS_STATICMAP_URL_TEMPLATE_You_and_Friend .arg(QString::number(yourLat,'g',8)) .arg(
            QString::number(yourLong,'g',8)) .arg(QString::number(size.width())) .arg(QString::number(
            size.height())) .arg(QString::number(friendLat,'g',8)) .arg(QString::number(friendLong,'g',8));
    }

    m_url = QUrl(url);

    QNetworkRequest request;
    request.setUrl(m_url);
    m_manager.get(request);
    return m_url;
}

void FetchMap::handleNetworkData(QNetworkReply *reply)
{
    // Get image from the reply
    QImage img;
    if (reply->error() == QNetworkReply::NoError && reply->isReadable()) {
        // No errors
        img.load(reply, 0);
        // Read image from the reply and emit it
        if (!img.isNull()) {
            QPixmap p = QPixmap::fromImage(img);
            emit mapImageLoaded(p);
        }

    } else {
        // Errors!
        emit httpError();
    }

    // Delete reply
    reply->deleteLater();
}
