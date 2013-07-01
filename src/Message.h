/*
 * Copyright (c) 2011 Nokia Corporation.
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include <QObject>

// QtMobility API headers
// Messaging
#include <QMessage>
#include <QMessageManager>
#include <QMessageService>

// Location
#include <QGeoPositionInfo>

// QtMobility namespace
QTM_USE_NAMESPACE

class Message: public QObject
{
Q_OBJECT

public:
    Message(QObject *parent = 0);
    ~Message();

    void checkMessages();
    bool sendLocationSMS(QString typeStr, QGeoPositionInfo& position, QString phoneNumber);
    
private:
    void processIncomingSMS(const QMessageId&);
    
public slots:
    // Listening QMessageService
    void messagesFound(const QMessageIdList &ids);
    
    // Listening QMessageManager
    void messageAdded(const QMessageId &id,
        const QMessageManager::NotificationFilterIdSet &matchingFilterIds);

signals:
    void friendLocationSMSReceived(QString latitude, QString longitude);
    void friendAskLocationSMS(QString friendPhoneNumber);
    void raise();

private:
    QMessageService* m_service;
    QMessageManager* m_manager;
    QMessageManager::NotificationFilterIdSet m_notifFilterSet;
};

#endif // MESSAGE_H
