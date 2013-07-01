/*
 * Copyright (c) 2011 Nokia Corporation.
 */

#include "QueryDialog.h"
#include "Button.h"
#include <QHBoxLayout>
#include <QDesktopWidget>
#include <QApplication>

QueryDialog::QueryDialog(QString message, QWidget *parent) :
    QDialog(parent)
{
    m_message = message;
    QHBoxLayout* l = new QHBoxLayout;

    // Transparent background
#ifdef Q_OS_SYMBIAN
    this->setAttribute(Qt::WA_NoSystemBackground);
#endif

    // Create QGraphicsScene and QGraphicsView
    m_scene = new QGraphicsScene(this);
    m_view = new QGraphicsView(m_scene, this);
    m_view->setCacheMode(QGraphicsView::CacheBackground);
    m_view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setBackgroundBrush(QBrush(Qt::transparent));
    m_view->setAttribute(Qt::WA_NoSystemBackground);

    // Background pixmap
    QPixmap pix(":/icons/message_background_noshadow.png");
    m_background = new QGraphicsPixmapItem(pix);
    m_background->setFlag(QGraphicsItem::ItemIsFocusable, false);
    m_background->setZValue(1);
    m_scene->addItem(m_background);

    // Message
    QFont f = QApplication::font();
#ifdef Q_OS_SYMBIAN
    f.setPointSize(8);
#else
    f.setPointSize(20);
#endif
    m_messageTextItem = new QGraphicsTextItem(m_message);
    m_messageTextItem->setDefaultTextColor(Qt::white);
    m_messageTextItem->setFlag(QGraphicsItem::ItemIsFocusable, false);
    m_messageTextItem->setFont(f);
    m_messageTextItem->setZValue(2);
    m_scene->addItem(m_messageTextItem);

    l->addWidget(m_view);

    // Exit button for Symbian
    QPixmap rejectPixmap(":/icons/close.png");
    QPixmap rejectPixmap2(":/icons/close_on.png");
    m_rejectButton = new Button(Button::ENone,rejectPixmap.size(), rejectPixmap, rejectPixmap2);
    QObject::connect(m_rejectButton, SIGNAL(pressed()), this, SLOT(reject()));
    m_scene->addItem(m_rejectButton);
    m_rejectButton->setZValue(3);

    // Send button
    QPixmap acceptPixmap(":/icons/messagebottle.png");
    QPixmap acceptPixmap2(":/icons/messagebottle_on.png");
    m_acceptButton = new Button(Button::ENone,acceptPixmap.size(), acceptPixmap, acceptPixmap2);
    QObject::connect(m_acceptButton, SIGNAL(pressed()), this, SLOT(accept()));
    m_scene->addItem(m_acceptButton);
    m_acceptButton->setZValue(4);
    

    // Remove context menu from the all widgets
#ifdef Q_OS_SYMBIAN
    QWidgetList widgets = QApplication::allWidgets();
    QWidget* w = 0;
    foreach(w,widgets)
        {
            w->setContextMenuPolicy(Qt::NoContextMenu);
        }
#endif

    // Set dialog title for the Maemo
#if defined Q_WS_HILDON || defined Q_WS_MAEMO_5
    this->setWindowTitle("Message received");
#endif

    // Use layout
    this->setLayout(l);
}

QueryDialog::~QueryDialog()
{
}


void QueryDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);

    // Background
    QSize s = size();
    s -= QSize(width() * 0.2, height() * 0.3);
    QPixmap scaledPix = m_background->pixmap().scaled(s, Qt::IgnoreAspectRatio,
        Qt::SmoothTransformation);
    m_background->setPixmap(scaledPix);
    m_background->setPos((size().width() - m_background->boundingRect().width()) / 2,
        (size().height() - m_background->boundingRect().height()) / 2);
    
    QRectF dialogRect = QRectF(m_background->pos(),QSizeF(s.width(),s.height()));
    
    int heightCap = s.height()*0.10;
    int widthCap = s.width()*0.10;
    
    // Message
    m_messageTextItem->setTextWidth(s.width()*0.6);
    m_messageTextItem->setPos(dialogRect.left()+(dialogRect.width()-m_messageTextItem->boundingRect().width())/2,
        dialogRect.top()+(dialogRect.height() - m_messageTextItem->boundingRect().height()) / 3);
    
    // Recect button
    m_rejectButton->setPos(dialogRect.right()-m_rejectButton->boundingRect().width() - widthCap,
        dialogRect.top()+ heightCap);

    // Accept button
    m_acceptButton->setPos(dialogRect.right()-m_acceptButton->boundingRect().width() - widthCap,
        dialogRect.bottom()-m_acceptButton->boundingRect().height() - heightCap);
  
}
