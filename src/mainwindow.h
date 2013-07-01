/*
 * Copyright (c) 2011 Nokia Corporation.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QGraphicsItemGroup>

// NOTE: Bearer from QtNetwork in Qt 4.7.0 or newer
#include <QtNetwork>

// QtMobility API headers
// Location
#include <QGeoPositionInfoSource>
#include <QGeoPositionInfo>
// Contacts
#include <QContactManager>
#include <QContactSortOrder>
#include <QContactName>
#include <QContact>
#include <QContactDetailFilter>

// QtMobility namespace
QTM_USE_NAMESPACE

class PictureItem;
class SvgPictureItem;
class FetchMap;
class ContactsDialog;
class QueryDialog;
class Message;
class Button;
class MainWindow: public QMainWindow
{
Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void resizeEvent(QResizeEvent *);

private:
    void createGraphics(const QSize& size);
    void showSearchingGpsIcon(bool searching);
    void createMenu();
    void openDialog();
    void createContactManager();
    QContact findContact(const QString phoneNumber);

public slots:
    void showSplashScreen();
    void hideSplashScreen();

    void setupBearer();

    void startGps();
    void positionUpdated(QGeoPositionInfo gpsPos);
    void updateTimeout();

    void mapImageLoaded(QPixmap);
    void httpError();

    void askFriendLocation();
    void sendYourLocation();

    void contactSelected(QString phoneNumber);

    void friendLocationSMSReceived(QString latitude, QString longitude);
    void friendAskLocationSMS(QString friendPhoneNumber);

    void updateMap();

    void btnPressed();

    void showMessage(const QString& msg);

    
private:
    // Graphics view fw
    QGraphicsScene* m_scene;
    QGraphicsView* m_view;
    
    QNetworkSession* m_session;

    QPointer<QContactManager> m_contactManager;
    
    // Dialog for selecting contact
    QPointer<ContactsDialog> m_contactDialog;
    QPointer<QueryDialog> m_queryDialog;

    // Google Map API
    FetchMap* m_fetchmap;
    
    // Messaging
    Message* m_message;

    // Map pixmap
    QPixmap m_yourMapPixmap;

    // Gps icon animation properties
    QPointer<QPropertyAnimation> m_gpsAnim;

    // Location data
    QPointer<QGeoPositionInfoSource> m_location;
    bool m_lastKnownPosReaded;
    QGeoPositionInfo m_YougeoPosInfo;
    qreal m_friend_longitude;
    qreal m_friend_latitude;

    // Is user asking or requesting location
    bool askLocation;

    // Used graphics items
    QGraphicsItemGroup* m_splashScreenGraph;
    QGraphicsPixmapItem* m_background;
    QGraphicsPixmapItem* m_btnBackground;
    int m_btnBackgroundWidth;
    QGraphicsPixmapItem* m_btnBackgroundLogo;
    
    PictureItem* m_gpsGraph;
    PictureItem* m_messageGraph;
    PictureItem* m_MapGraph;

    // Buttons
    Button* m_btnUpdateMap;
    Button* m_btnSendMap;
    Button* m_btnAskMap;
    Button* m_btnExit;

    QSize m_mapSize;
    int m_positionReadCounter;

    QList<QContact> m_contactCache;
};

#endif // MAINWINDOW_H
