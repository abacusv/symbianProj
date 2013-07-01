/*
 * Copyright (c) 2011 Nokia Corporation.
 */

#ifndef QUERYDIALOG_H
#define QUERYDIALOG_H

#include <QtGui/QDialog>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QGraphicsScene>
#include <QGraphicsView>

class Button;
class QueryDialog: public QDialog
{
Q_OBJECT

public:
    QueryDialog(QString message, QWidget *parent = 0);
    virtual ~QueryDialog();
    void resizeEvent(QResizeEvent *);

protected:
    QGraphicsScene* m_scene;
    QGraphicsView* m_view;
    QGraphicsPixmapItem* m_background;
    QGraphicsTextItem* m_messageTextItem;
    Button* m_acceptButton;
    Button* m_rejectButton;
    QString m_message;
};

#endif // QUERYDIALOG_H
