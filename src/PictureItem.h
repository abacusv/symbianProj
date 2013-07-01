/*
 * Copyright (c) 2011 Nokia Corporation.
 */

#ifndef PICTUREITEM_H
#define PICTUREITEM_H

#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>


class PictureItem: public QObject, public QGraphicsPixmapItem
{
Q_OBJECT
    // For Property animation:

    // Change picture position
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)

    // Rotating picture
    Q_PROPERTY(qreal rotationAngleY READ rotationAngleY WRITE setRotationAngleY)
    Q_PROPERTY(qreal rotationAngleZ READ rotationAngleZ WRITE setRotationAngleZ)

    // Fade picture
    Q_PROPERTY(int fade READ fade WRITE setFade)

    // Scale picture
    Q_PROPERTY(qreal scale READ scale WRITE setScale)

public:

    enum
    {
        PictureItemType = UserType + 2
    };

public:
    PictureItem(const QSizeF& size, const QPixmap& pixmap = 0, QObject* parent = 0);
    virtual ~PictureItem();

    QRectF boundingRect() const;

    int type() const
    {
        return PictureItemType;
    }

    void stop(QPropertyAnimation* anim);
    
    // For rotationAngleY property
    void setRotationAngleY(qreal angle);
    qreal rotationAngleY() const;

    // For rotationAngleZ property
    void setRotationAngleZ(qreal angle);
    qreal rotationAngleZ() const;
    
    // For fade property
    void setFade(int f);
    int fade() const;

    // Animate position of this class
    void animatePosition(const QPointF& start, const QPointF& end, int duration,const QEasingCurve& curve);

    // Animate angle of this class
    QPropertyAnimation* animateAngle(int startAngle, int endAngle, bool loop = false);

    // Animate position and angle
    void animatePosAndAngle(const QPointF& start, const QPointF& end,
                            int startAngle, int endAngle);

    // Animate fade effects for this class
    void animateFadeIn();
    void animateFadeOut();

    // Animate scale for this class
    void animateScale(qreal start, qreal end);
    
    void setHideWhenFinished();

public slots:
    void animationFinished();

protected:
    QSizeF m_size;
    qreal m_rotationAngleY;
    qreal m_rotationAngleZ;
    int m_fade;
    QPixmap m_originalPicForFade;
    bool m_hide;
};


#endif // PICTUREITEM_H
