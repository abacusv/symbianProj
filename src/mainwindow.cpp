/*
 * Copyright (c) 2011 Nokia Corporation.
 */

#include "mainwindow.h"
#include "PictureItem.h"
#include "FetchMap.h"
#include "ContactsDialog.h"
#include "QueryDialog.h"
#include "Message.h"
#include "Button.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    setWindowTitle("QWhoWhere");

    m_positionReadCounter = 0;
    m_gpsGraph = 0;
    m_splashScreenGraph = 0;
    m_background = 0;
    m_btnBackground = 0;
    m_btnBackgroundLogo = 0;
    m_btnUpdateMap = 0;
    m_btnSendMap = 0;
    m_btnAskMap = 0;
    m_MapGraph = 0;
    m_messageGraph = 0;
    
    m_friend_longitude = 0.0; // No friend position yet
    m_friend_latitude = 0.0;

    // Create QGraphicsScene and QGraphicsView
    m_scene = new QGraphicsScene(this);
    m_view = new QGraphicsView(m_scene, this);
    m_view->setCacheMode(QGraphicsView::CacheBackground);
    m_view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setBackgroundBrush(QBrush(Qt::black));

    // Set background colour to black
    QPalette p(palette());
    p.setColor(QPalette::Background, Qt::black);
    setPalette(p);

    // Google Map fetcher
    m_fetchmap = new FetchMap(this);
    QObject::connect(m_fetchmap, SIGNAL(mapImageLoaded(QPixmap)), this,
        SLOT(mapImageLoaded(QPixmap)));
    QObject::connect(m_fetchmap, SIGNAL(httpError()), this, SLOT(httpError()));
    m_yourMapPixmap = 0;

    // SMS message sending and receiving
    m_message = new Message(this);
    QObject::connect(m_message, SIGNAL(friendLocationSMSReceived(QString,QString)), this,
        SLOT(friendLocationSMSReceived(QString,QString)));
    QObject::connect(m_message, SIGNAL(friendAskLocationSMS(QString)), this,
        SLOT(friendAskLocationSMS(QString)));
#ifdef Q_OS_SYMBIAN
    QObject::connect(m_message, SIGNAL(raise()), this, SLOT(raise()));
#endif

    createMenu();

    setCentralWidget(m_view);

    // Remove context menu from the all widgets
#ifdef Q_OS_SYMBIAN
    QWidgetList widgets = QApplication::allWidgets();
    QWidget* w = 0;
    foreach(w,widgets)
        {
            w->setContextMenuPolicy(Qt::NoContextMenu);
        }
#endif    

    // Start showing splach screen
    QTimer::singleShot(500, this, SLOT(showSplashScreen()));
}

MainWindow::~MainWindow()
{
    delete m_splashScreenGraph;
    delete m_background;
    delete m_btnBackground;
    delete m_btnBackgroundLogo;
    delete m_btnUpdateMap;
    delete m_btnSendMap;
    delete m_btnAskMap;
    delete m_MapGraph;
    delete m_gpsGraph;
    delete m_btnExit;
    delete m_messageGraph;

    if (m_location)
        m_location->stopUpdates();

    delete m_contactManager;
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    // Update scene rect
    m_scene->setSceneRect(m_view->rect());
    int btnBackgroundWidth = size().width() / 3.1;
    m_mapSize = QSize(size().width()-btnBackgroundWidth+(btnBackgroundWidth/4),size().height());
}

void MainWindow::btnPressed()
{
    Button* button = (Button*) sender();
    int i = button->id();

    if (i == Button::EUpdateBtn) {
        updateMap();
    }
    else if (i == Button::ESendBtn) {
        sendYourLocation();
    }
    else if (i == Button::EAskBtn) {
        askFriendLocation();
    }
    else if (i == Button::EExitBtn) {
        qApp->exit(0);
    }
}

void MainWindow::createGraphics(const QSize& size)
{
    // Clean items, if exists
    QGraphicsItem* item = 0;
    foreach(item,m_scene->items())
        {
            m_scene->removeItem(item);
            delete item;
            item = 0;
        }

    // Background pixmap of the application
    m_background = new QGraphicsPixmapItem(QPixmap(":/icons/background_noshadow.png"));
    // Set center of the screen
    m_background->setPos((size.width() - m_background->boundingRect().width()) / 2, (size.height()
        - m_background->boundingRect().height()) / 2);
    m_background->setZValue(0.1);
    m_scene->addItem(m_background);

    // Satellite pixamp
    QPixmap pixmap(":/icons/rolling-compass_noshadow.png");
    m_gpsGraph = new PictureItem(QSize(120, 120), pixmap, this);
    m_gpsGraph->setZValue(98);
    m_gpsGraph->setVisible(false);
    m_scene->addItem(m_gpsGraph);
    m_gpsGraph->setPos(((size.width() - m_gpsGraph->boundingRect().width()) / 2)
        - m_gpsGraph->boundingRect().width(), (size.height() - m_gpsGraph->boundingRect().height())
        / 2);

    // Background for the buttons
    m_btnBackgroundWidth = size.width() / 3.1; // Calculate button backround width
    QPixmap btnBackgroundPixmap(":/icons/map_noshadow.png");
    btnBackgroundPixmap = btnBackgroundPixmap.scaled(size, Qt::IgnoreAspectRatio,
        Qt::SmoothTransformation);
    m_btnBackground = new QGraphicsPixmapItem(btnBackgroundPixmap);
    m_btnBackground->setZValue(50);
    m_btnBackground->setPos(QPointF(size.width() - m_btnBackgroundWidth, -5));
    m_scene->addItem(m_btnBackground);

    // Big logo
    QPixmap btnBackgroundLogoPixmap(":/icons/qww-logo_app_noshadow.png");
    // Calculate suitaple size for big logo
    btnBackgroundLogoPixmap = btnBackgroundLogoPixmap.scaled(QSize(m_btnBackgroundWidth * 0.7,
        m_btnBackgroundWidth * 0.7), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_btnBackgroundLogo = new QGraphicsPixmapItem(btnBackgroundLogoPixmap);
    m_btnBackgroundLogo->setZValue(51);
    m_btnBackgroundLogo->setPos(QPointF(size.width() - m_btnBackgroundLogo->boundingRect().width()
        - 10, size.height() * 0.03));
    m_scene->addItem(m_btnBackgroundLogo);

    // Update map button
#ifdef Q_OS_SYMBIAN
    int yPosBtn = size.height() * 0.39;
#else
    int yPosBtn = size.height()*0.42;
#endif
    m_btnUpdateMap = new Button(Button::EUpdateBtn, QSize(m_btnBackgroundWidth * 0.7, (m_btnBackgroundWidth * 0.7)
        / 3.1), QPixmap(":/icons/btn_updatemap.png"), QPixmap(":/icons/btn_updatemap_on.png"));
    m_btnUpdateMap->setZValue(52);
    m_scene->addItem(m_btnUpdateMap);
    m_btnUpdateMap->setPos(size.width() - m_btnUpdateMap->size().width(), yPosBtn);
    QObject::connect(m_btnUpdateMap, SIGNAL(pressed()), this, SLOT(btnPressed()));

    // Send location button
    m_btnSendMap = new Button(Button::ESendBtn, m_btnUpdateMap->size().toSize(),
        QPixmap(":/icons/btn_mylocation.png"), QPixmap(":/icons/btn_mylocation_on.png"));
    m_btnSendMap->setZValue(52.1);
    m_scene->addItem(m_btnSendMap);
    m_btnSendMap->setPos(m_btnUpdateMap->pos());
    m_btnSendMap->moveBy(0, m_btnUpdateMap->size().height() + 5);
    QObject::connect(m_btnSendMap, SIGNAL(pressed()), this, SLOT(btnPressed()));

    // Ask location button
    m_btnAskMap = new Button(Button::EAskBtn, m_btnUpdateMap->size().toSize(), QPixmap(
        ":/icons/btn_friendslocation.png"), QPixmap(":/icons/btn_friendslocation_on.png"));
    m_btnAskMap->setZValue(52.2);
    m_scene->addItem(m_btnAskMap);
    m_btnAskMap->setPos(m_btnSendMap->pos());
    m_btnAskMap->moveBy(0, m_btnSendMap->size().height() + 5);
    QObject::connect(m_btnAskMap, SIGNAL(pressed()), this, SLOT(btnPressed()));

    // Exit button
    m_btnExit = new Button(Button::EExitBtn, m_btnUpdateMap->size().toSize(), QPixmap(":/icons/btn_exit.png"),
        QPixmap(":/icons/btn_exit_on.png"));
#ifdef Q_OS_SYMBIAN
    m_btnExit->setZValue(52.3);
    m_scene->addItem(m_btnExit);
    m_btnExit->setPos(m_btnAskMap->pos());
    m_btnExit->moveBy(0, m_btnAskMap->size().height() + 5);
    QObject::connect(m_btnExit, SIGNAL(pressed()), this, SLOT(btnPressed()));
#endif
}

void MainWindow::showSplashScreen()
{
    // Clean items, if exists
    QGraphicsItem* item = 0;
    foreach(item,m_scene->items())
        {
            m_scene->removeItem(item);
            delete item;
            item = 0;
        }

    QSize size = this->size();

    // Splash screen graphcs on each other
    m_splashScreenGraph = new QGraphicsItemGroup;

    // Background
    QPixmap backgroundPixmap(":/icons/background_noshadow.png");
    QGraphicsPixmapItem* background =
        new QGraphicsPixmapItem(backgroundPixmap, m_splashScreenGraph);
    background->setPos((size.width() - background->boundingRect().width()) / 2, (size.height()
        - background->boundingRect().height()) / 2);
    m_splashScreenGraph->addToGroup(background);

    // Map
    QPixmap mapPixmap(":/icons/map_noshadow.png");
    mapPixmap = mapPixmap.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QGraphicsPixmapItem* map = new QGraphicsPixmapItem(mapPixmap, m_splashScreenGraph);
    map->setPos((size.width() - map->boundingRect().width()) / 2, (size.height()
        - map->boundingRect().height()) / 2);
    m_splashScreenGraph->addToGroup(map);

    // Logo
    QGraphicsPixmapItem* logo = new QGraphicsPixmapItem(QPixmap(
        ":/icons/qww-logo_splash_noshadow.png"), m_splashScreenGraph);
    logo->setPos((size.width() - logo->boundingRect().width()) / 2, (size.height()
        - logo->boundingRect().height()) / 2);
    m_splashScreenGraph->addToGroup(logo);

    // Add to scene
    m_scene->addItem(m_splashScreenGraph);
    m_splashScreenGraph->setZValue(99);
    m_splashScreenGraph->setPos(QPoint(0, 0));

    // Create contact manager
    createContactManager();

    // Hide splash screen after a while
    QTimer::singleShot(2000, this, SLOT(hideSplashScreen()));
}

void MainWindow::hideSplashScreen()
{
    // Hides the splash screen on applicatin startup when requested
    m_splashScreenGraph->hide();
    m_scene->removeItem(m_splashScreenGraph);
    delete m_splashScreenGraph;
    m_splashScreenGraph = 0;

    // Create rest of graphics
    createGraphics(size());

    // Set up bearer
    setupBearer();
}

void MainWindow::setupBearer()
{
    // use Bearer Management classes in QtNetwork module
    QNetworkConfigurationManager manager;
    QNetworkConfiguration cfg = manager.defaultConfiguration();
    // Show GPS rolling icon
    showSearchingGpsIcon(true);
    // Open session
    m_session = new QNetworkSession(cfg,this);
    m_session->open();
    m_session->waitForOpened(10000);
    // Start listening GPS
    QTimer::singleShot(1000,this,SLOT(startGps()));
}

void MainWindow::startGps()
{
    // QGeoPositionInfoSource: Start GPS and listen position changes 
    showSearchingGpsIcon(true);

    m_positionReadCounter = 0;

    if (!m_location) {
        m_location = QGeoPositionInfoSource::createDefaultSource(this);
        // System has some positioning source found
        if (m_location) {
            m_location->setUpdateInterval(10*1000);
            QObject::connect(m_location, SIGNAL(positionUpdated(QGeoPositionInfo)), this,
                SLOT(positionUpdated(QGeoPositionInfo)));
            QObject::connect(m_location, SIGNAL(updateTimeout()), this, SLOT(updateTimeout()));
            // Get last known GPS position
            m_lastKnownPosReaded = false;
            QGeoPositionInfo lastKnownPosition = m_location->lastKnownPosition();
            if (lastKnownPosition.isValid()) {
                m_lastKnownPosReaded = true;
                positionUpdated(lastKnownPosition);
            }
            // Start listening current GPS position
            m_location->startUpdates();
        }
        else {
            // System has not any positioning source
            // Could not found any positioning source. Close application.
            showSearchingGpsIcon(false);
            showMessage("There be no compass fer ye. Arrr!");
            return;
        }
    }
    else {
        // Start listening current GPS position
        m_location->startUpdates();
    }
}

void MainWindow::updateTimeout()
{
#ifndef Q_OS_SYMBIAN
    // Trying to get GPS position
    showMessage("Tryin to find compass fer ye, Captain!");
#endif
}

void MainWindow::positionUpdated(QGeoPositionInfo gpsPos)
{
    // Take your GPS position only once and search map for that location
    if (gpsPos.isValid() && !m_YougeoPosInfo.isValid()) {
        // Get first position
        // GPS position got
        m_positionReadCounter = 0;
        m_YougeoPosInfo = gpsPos;
        // Stop GPS, we have position
        if (!m_lastKnownPosReaded) {
            // Current position
            // If position was last known, then do not stop
            // let there come current position
            showMessage("Compass fer ye, Captain!");
            // Load map image
            m_fetchmap->fetchMapImage(m_mapSize, m_YougeoPosInfo.coordinate().longitude(),
            m_YougeoPosInfo.coordinate().latitude(), m_friend_longitude, m_friend_latitude);

        } else {
            // Last known position
            showMessage("Compass fer ye, Captain!");
            // Load map image
            m_fetchmap->fetchMapImage(m_mapSize, m_YougeoPosInfo.coordinate().longitude(),
            m_YougeoPosInfo.coordinate().latitude(), m_friend_longitude, m_friend_latitude);
            // Wait for current position...
        }
    }
    else if (gpsPos.isValid() && m_YougeoPosInfo.isValid()) {
        // Stop GPS, we have position
        m_positionReadCounter++;
        if (m_positionReadCounter > 2) {
            //m_location->stopUpdates(); // Keep reading GPS positions
            m_positionReadCounter = 0;
            return;
        }
        m_YougeoPosInfo = gpsPos;
        m_fetchmap->fetchMapImage(m_mapSize, m_YougeoPosInfo.coordinate().longitude(),
        m_YougeoPosInfo.coordinate().latitude(), m_friend_longitude, m_friend_latitude);
    }
}

void MainWindow::updateMap()
{
    // User wants to update his position to the map
    // Invalidate current position
    m_YougeoPosInfo.setCoordinate(QGeoCoordinate());

    // Remove old map picture
    if (m_MapGraph) {
        m_MapGraph->hide();
        m_scene->removeItem(m_MapGraph);
        delete m_MapGraph;
        m_MapGraph = 0;
    }

    // Searching new map image
    showMessage("Searchin fer map fer ye, Captain!");

    // Start GPS
    startGps();
}

void MainWindow::mapImageLoaded(QPixmap p)
{
    if (p.isNull()) {
        httpError();
        return;
    }

    showSearchingGpsIcon(false);

    // Remove old map picture
    if (m_MapGraph) {
        m_MapGraph->hide();
        m_scene->removeItem(m_MapGraph);
        delete m_MapGraph;
        m_MapGraph = 0;
    }

    // Show loaded map image
    m_yourMapPixmap = p;
    m_MapGraph = new PictureItem(m_yourMapPixmap.size(), m_yourMapPixmap, this);

    // to left
    QPointF pos(0, (rect().height() - m_MapGraph->boundingRect().height()) / 2);

    m_MapGraph->setZValue(2);
    m_scene->addItem(m_MapGraph);
    m_MapGraph->setPos(pos);

    // Start animation for the image
    //m_MapGraph->animateFadeOut();
    m_MapGraph->setVisible(true);
}

void MainWindow::httpError()
{
    showSearchingGpsIcon(false);

    // Could not load map image
    showMessage("Could not find map fer ye. Arrr!");
}

void MainWindow::showSearchingGpsIcon(bool searching)
{
    // Animate GPS logo or stop animation
    if (searching) {
        if (!m_gpsAnim && m_gpsGraph)
            m_gpsGraph->setVisible(true);
        m_gpsAnim = m_gpsGraph->animateAngle(0, 360, true);
    }
    else {
        if (m_gpsAnim && m_gpsGraph) {
            m_gpsGraph->stop(m_gpsAnim);
            m_gpsGraph->setVisible(false);
            m_gpsAnim = 0;
        }
    }
}

void MainWindow::createMenu()
{
    this->menuBar()->addAction("Update map", this, SLOT(updateMap()));
    this->menuBar()->addAction("Send your location", this, SLOT(sendYourLocation()));
    this->menuBar()->addAction("Ask friend location", this, SLOT(askFriendLocation()));
#ifdef Q_OS_SYMBIAN
    this->menuBar()->addAction("Exit", qApp, SLOT(quit()));
#endif
}

void MainWindow::askFriendLocation()
{
    askLocation = true;
    openDialog();
}

void MainWindow::sendYourLocation()
{
    askLocation = false;
    openDialog();
}

void MainWindow::openDialog()
{
    // Open dialog for selecting contact
    if (m_YougeoPosInfo.isValid()) {
        m_contactDialog = new ContactsDialog(&m_contactCache, m_contactManager, askLocation, this);
#if defined Q_WS_MAEMO_5
        m_contactDialog->setMinimumSize(size().width(),404);
#elif defined Q_OS_SYMBIAN
        m_contactDialog->showFullScreen();
#else
        m_contactDialog->showMaximized();
#endif

        QObject::connect(m_contactDialog, SIGNAL(contactSelected(QString)), this,
            SLOT(contactSelected(QString)));
        m_contactDialog->exec();
        QObject::disconnect(m_contactDialog, SIGNAL(contactSelected(QString)), this,
            SLOT(contactSelected(QString)));

        delete m_contactDialog;
        m_contactDialog = 0;
    }
    else {
        // Finding GPS location
        showMessage("Searchin fer compass fer ye, Captain!");
    }
}

void MainWindow::contactSelected(QString phoneNumber)
{
    // Contact selected
    QString type;
    if (askLocation)
        type = "REQ:";
    else
        type = "RES:";

#ifdef MESSAGING_ENABLED

    // Send SMS message
    bool ret = m_message->sendLocationSMS(type, m_YougeoPosInfo, phoneNumber);
    if (ret) {
        showMessage("Message sent, Captain!");
    }
    else {
        showMessage("HOOOH ! message was NOT sent, what now Captain?");
    }

#else
    showMessage("Messaging is disabled. Arrr!");
#endif
}

void MainWindow::friendLocationSMSReceived(QString lat, QString longi)
{
    // New location from the friend
    m_friend_longitude = longi.toDouble();
    m_friend_latitude = lat.toDouble();

    // Show you and friend location on the map
    if (m_YougeoPosInfo.isValid()) {
        // Friend location received
        if (!m_queryDialog) { // Dont show the message if dialog is open
            showMessage("Captain! Boat on the left!");
        }
        QUrl url = m_fetchmap->fetchMapImage(m_mapSize, m_YougeoPosInfo.coordinate().longitude(),
                    m_YougeoPosInfo.coordinate().latitude(), m_friend_longitude, m_friend_latitude);

    }
}

void MainWindow::friendAskLocationSMS(QString friendPhoneNumber)
{
    QString friendName;
    QString stringToDialog;

    // Search contact name
    QContact contact = findContact(friendPhoneNumber);
    if (contact.isEmpty()) {
        stringToDialog = friendPhoneNumber;
    }
    else {
        friendName = contact.displayLabel();
        stringToDialog = friendName;
    }

    QString question = QString(
        "Location received from %1. Do you want to send yours?").arg(stringToDialog);
    m_queryDialog = new QueryDialog(question, this);
#if defined Q_WS_HILDON || defined Q_WS_MAEMO_5
    m_queryDialog->setMinimumSize(size().width(),404);
#elif defined Q_OS_SYMBIAN
    m_queryDialog->showFullScreen();
#else
    m_queryDialog->showMaximized();
#endif

    int ret = m_queryDialog->exec();
    delete m_queryDialog;
    m_queryDialog = 0;
    if (ret == QDialog::Accepted) {
        if (!m_YougeoPosInfo.isValid()) {
            // You do not have GPS location, can not send it
            showMessage("Compass is missing, could not send the message!");
            return;
        }

        // Send SMS
        m_message->sendLocationSMS("RES:", m_YougeoPosInfo, friendPhoneNumber);
        showMessage("Message sent, Captain!");
    }
}

void MainWindow::showMessage(const QString& msg)
{
    // Show animated message to the user
    if (m_messageGraph) {
        m_scene->removeItem(m_messageGraph);
        delete m_messageGraph;
        m_messageGraph = 0;
    }

    QPixmap messagePixmap(":/icons/message_background_noshadow.png");
    messagePixmap = messagePixmap.scaled(QSize(size().width() / 2, size().height() / 3),
        Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QSize s = messagePixmap.size();
    QPainter painter(&messagePixmap);
    painter.setPen(Qt::black);
#ifdef Q_OS_SYMBIAN
    painter.setFont(QFont("Arial", 8));
#else
    painter.setFont(QFont("Arial", s.height()/7));
#endif

    QFontMetrics fm = painter.fontMetrics();
    QRect r = messagePixmap.rect();
    r.adjust(30, 0, -30, -0);
    painter.drawText(r, Qt::AlignCenter | Qt::TextWordWrap, msg);
    painter.end();

    // Animate created pixmap
    m_messageGraph = new PictureItem(s, messagePixmap, this);
    m_scene->addItem(m_messageGraph);
    m_messageGraph->setZValue(100);
    m_messageGraph->setHideWhenFinished();

    // Start animation
    QPointF from = QPointF((width() - s.width()) / 2 - 20, s.height() * -1);
    QPointF to = QPointF((width() - s.width()) / 2 - 20, height()+20);
    m_messageGraph->setPos(from);

    // Start animation for the image
    m_messageGraph->animatePosition(from, to, 3000, QEasingCurve::OutInCirc);
}

void MainWindow::createContactManager()
{
    if (!m_contactManager) {
    #if defined Q_WS_MAEMO_5
        m_contactManager = new QContactManager("maemo5");
    #elif defined Q_OS_SYMBIAN
        m_contactManager = new QContactManager("symbian");
    #endif
    }
}

QContact MainWindow::findContact(const QString phoneNumber)
{
    QContact contact;
    // Filter for search
    QContactDetailFilter phoneFilter;
    phoneFilter.setDetailDefinitionName(QContactPhoneNumber::DefinitionName,
        QContactPhoneNumber::FieldNumber);

#if defined Q_WS_MAEMO_5
    // Workaround for Maemo bug http://bugreports.qt.nokia.com/browse/QTMOBILITY-437
    phoneFilter.setValue(phoneNumber.right(7));
    phoneFilter.setMatchFlags(QContactFilter::MatchContains);
#else
    phoneFilter.setValue(phoneNumber);
    phoneFilter.setMatchFlags(QContactFilter::MatchPhoneNumber);
#endif


    // Find contacts
    QList<QContact> matchingContacts = m_contactManager->contacts(phoneFilter);
    if (matchingContacts.size() == 0) {
        return contact; // empty
    }
    else {
        contact = matchingContacts.at(0);
        return contact;
    }
}

