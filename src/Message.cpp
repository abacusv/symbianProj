/*
 * Copyright (c) 2011 Nokia Corporation.
 */

#include "Message.h"
#include <QMessageBox>
#include <QDebug>
#include <QTimerEvent>
#include <QTimer>

Message::Message(QObject *parent) :
    QObject(parent)
{
    // QMessageService class provides the interface for requesting messaging service operations
    m_service = new QMessageService(this);
    QObject::connect(m_service, SIGNAL(messagesFound(const QMessageIdList&)), this,
        SLOT(messagesFound(const QMessageIdList&)));

    // NOTE: Modest bugi in Maemo
    // Messaging is disabled with Qt Mobility 1.0.2
    // http://bugreports.qt.nokia.com/browse/QTMOBILITY-771
#ifndef MESSAGING_ENABLED
    return;
#endif

    // QMessageManager class represents the main interface for storage and 
    // retrieval of messages, folders and accounts in the system message store
    m_manager = new QMessageManager(this);

    QObject::connect(m_manager,
        SIGNAL(messageAdded(const QMessageId&,const QMessageManager::NotificationFilterIdSet&)),
        this, SLOT(messageAdded(const QMessageId&,const QMessageManager::NotificationFilterIdSet&)));

    // Register SMS in inbox folder notificationfilter
#ifdef Q_OS_SYMBIAN
#ifdef __WINS__
    m_notifFilterSet.insert(m_manager->registerNotificationFilter(QMessageFilter::byStandardFolder(
        QMessage::DraftsFolder)));
#else
    m_notifFilterSet.insert(m_manager->registerNotificationFilter(QMessageFilter::byStandardFolder(
                QMessage::InboxFolder)));
#endif    
#else    
    m_notifFilterSet.insert(m_manager->registerNotificationFilter(QMessageFilter::byStandardFolder(
                QMessage::InboxFolder)));
#endif

}

Message::~Message()
{
}

void Message::messageAdded(const QMessageId& id, const QMessageManager::NotificationFilterIdSet&)
{
    processIncomingSMS(id);
}

void Message::checkMessages()
{
#ifdef Q_OS_SYMBIAN
#ifdef __WINS__
    QMessageFilter folderFilter(QMessageFilter::byStandardFolder(QMessage::DraftsFolder));
#else
    QMessageFilter folderFilter(QMessageFilter::byStandardFolder(QMessage::InboxFolder));
#endif
#else
    QMessageFilter folderFilter(QMessageFilter::byStandardFolder(QMessage::InboxFolder));
#endif

    m_service->queryMessages(folderFilter);
    // Message::messagesFound() is called if SMS messages found
}

void Message::messagesFound(const QMessageIdList &ids)
{
    foreach(const QMessageId& id, ids)
        {
        processIncomingSMS(id);
        }
}

void Message::processIncomingSMS(const QMessageId& id)
{
    QMessage message = m_manager->message(id);

    // Handle only SMS messages
    if (message.type() != QMessage::Sms)
        return;

    QString txt;
    if (message.isContentAvailable())
        txt = message.textContent();

    if (txt.length() > 10) {
        if (txt.mid(0, 4).contains("RES:", Qt::CaseSensitive)) {
            // Your friend send his location to your
            QString s = txt.right(txt.length() - 4);
            int space = s.indexOf(" ");
            QString friend_latitudeStr = s.left(space);
            QString friend_longitudeStr = s.right(s.length() - space - 1);

            // Application to foreground
            emit raise();

            // SMS friend location received
            emit friendLocationSMSReceived(friend_latitudeStr, friend_longitudeStr);

            // Remove message from inbox
            m_manager->removeMessage(id);
        }
        else if (txt.mid(0, 4).contains("REQ:", Qt::CaseSensitive)) {
            // Your friend send location request to you
            // and
            // also his location
            QString s = txt.right(txt.length() - 4);
            int space = s.indexOf(" ");
            QString friend_latitudeStr = s.left(space);
            QString friend_longitudeStr = s.right(s.length() - space - 1);

            // Application to foreground
            emit raise();

            // Friend ask your location
            QMessageAddress from = message.from();
            if (from.type() == QMessageAddress::Phone) {
                emit friendAskLocationSMS(from.addressee());
            }

            // SMS friend location received
            emit friendLocationSMSReceived(friend_latitudeStr, friend_longitudeStr);

            // Remove message from inbox
            m_manager->removeMessage(id);
        }
    }
}

bool Message::sendLocationSMS(QString typeStr, QGeoPositionInfo& position, QString phoneNumber)
{
    // Send SMS
    if (!position.isValid()) {
        return false;
    }

    QGeoCoordinate coordinate = position.coordinate();

    QMessage smsMessage;
    smsMessage.setType(QMessage::Sms);
    smsMessage.setParentAccountId(QMessageAccount::defaultAccount(QMessage::Sms));    
    smsMessage.setTo(QMessageAddress(QMessageAddress::Phone, phoneNumber));

    QString bodyText;
    bodyText += typeStr;
    bodyText += QString().setNum(coordinate.latitude(),'g',8);
    bodyText += " ";
    bodyText += QString().setNum(coordinate.longitude(),'g',8);
    smsMessage.setBody(bodyText);

    // Send SMS
    return m_service->send(smsMessage);
}

