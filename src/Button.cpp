/*
 * Copyright (c) 2011 Nokia Corporation.
 */

#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include "Button.h"

Button::Button (ButtonId id, QSize size, const QPixmap& upPixmap, const QPixmap& downPixmap, QGraphicsItem *parent, Qt::WindowFlags wFlags) :
        QGraphicsWidget(parent,wFlags)
{
    m_id = id;
    // is btn up or down
    m_isUp = true;

    // button size
    m_size = size;
    resize(m_size);

    setCacheMode(DeviceCoordinateCache);

    // Button pixmaps
    m_upPixmap = upPixmap;
    m_downPixmap = downPixmap;
    m_upPixmap = m_upPixmap.scaled(m_size,Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_downPixmap = m_downPixmap.scaled(m_size,Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

Button::~Button()
{
}

void Button::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    m_isUp = false;
    update();
}

void Button::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    m_isUp = true;
    update();
    emit pressed();
}

void Button::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (m_isUp)
        painter->drawPixmap(0,0,m_upPixmap);
    else
        painter->drawPixmap(0,0,m_downPixmap);
}
