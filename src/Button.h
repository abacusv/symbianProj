/*
 * Copyright (c) 2011 Nokia Corporation.
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <QGraphicsWidget>

class Button : public QGraphicsWidget
{
Q_OBJECT

public:
    enum {
        ButtonType = UserType + 1
    };

    enum ButtonId {
        ENone = 0,
        EUpdateBtn,
        ESendBtn,
        EAskBtn,
        EExitBtn
    };

public:
    Button (ButtonId id,
            QSize size,
            const QPixmap& upPixmap,
            const QPixmap& downPixmap,
            QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~Button();

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    int type() const
    {
        return ButtonType;
    }

    int id() const
    {
        return m_id;
    }

signals:
    void pressed();

protected:
    QSize m_size;
    QPixmap m_upPixmap;
    QPixmap m_downPixmap;
    bool m_isUp;
    int m_id;
};

#endif // BUTTON_H
