/*
 * Copyright (c) 2011 Nokia Corporation.
 */

#ifndef CONTACTSDIALOG_H
#define CONTACTSDIALOG_H

#include <QtGui/QDialog>
#include <QListWidget>
#include <QPointer>
#include <QGraphicsPixmapItem>
#include <QGraphicsSimpleTextItem>

// QtMobility API headers
// Contacts
#include <QContactManager>
#include <QContactPhoneNumber>
#include <QContactSortOrder>
#include <QContact>
#include <QContactName>

// QtMobility namespace
QTM_USE_NAMESPACE

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsProxyWidget>

class Button;
class ContactsDialog: public QDialog
{
Q_OBJECT

public:
    ContactsDialog(QList<QContact>* contactCache, QContactManager* manager, bool ask, QWidget *parent = 0);
    virtual ~ContactsDialog();
    void resizeEvent(QResizeEvent *);
    
public slots:
    void findContacts();
    void itemClicked(QListWidgetItem *item);
    void selectContact();
    void showArray();
    void hideArray();

signals:
    void contactSelected(QString phoneNumber);

private:
    void searchContact();

protected:
    QPointer<QContactManager> m_contactManager;

    QGraphicsScene* m_scene;
    QGraphicsView* m_view;
    QGraphicsPixmapItem* m_background;
    QGraphicsPixmapItem* m_array;
    QGraphicsProxyWidget* m_proxyToList;
    QGraphicsSimpleTextItem* m_title;
    Button* m_closeButton;
    Button* m_sendButton;
    
    QPointer<QListWidget> m_listWidget;
    bool m_askLocation;

    QList<QContact>* m_contactCache;
};

#endif // CONTACTSDIALOG_H
