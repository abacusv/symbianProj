/*
 * Copyright (c) 2011 Nokia Corporation.
 */

#include "ContactsDialog.h"
#include "Button.h"
#include <QHBoxLayout>
#include <QDesktopWidget>
#include <QApplication>
#include <QTimer>
#include <QDebug>

ContactsDialog::ContactsDialog(QList<QContact>* contactCache, QContactManager* manager, bool ask, QWidget *parent) :
    QDialog(parent)
{
    m_askLocation = ask;

    m_contactCache = contactCache;
   
    m_contactManager = manager;
    
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

    // Title of the dialog
    QFont f = QApplication::font();
#ifdef Q_OS_SYMBIAN
    f.setPointSize(8);
#else
    f.setPointSize(20);
#endif
    if (m_askLocation){
        m_title = new QGraphicsSimpleTextItem("Ask for friend's location");
    } else {
        m_title = new QGraphicsSimpleTextItem("Send my location");
    }
    m_title->setBrush(QBrush(Qt::white));
    m_title->setFlag(QGraphicsItem::ItemIsFocusable, false);
    m_title->setFont(f);
    m_title->setZValue(1.1);
    m_scene->addItem(m_title);

    // Add QListWidget into QGraphicsScene
    m_listWidget = new QListWidget;
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_proxyToList = m_scene->addWidget(m_listWidget);
    m_proxyToList->setZValue(2);
    m_proxyToList->setFlag(QGraphicsItem::ItemIsFocusable, true);

    // Example how to defive transparent background to QListWidget using QPalette
    //QPalette listPalette = m_listWidget->palette();
    //listPalette.setBrush(QPalette::Base,QBrush(Qt::transparent));
    //m_listWidget->setPalette(listPalette);

    // We use stylesheets insted of QPalette in this example
    // See main.cpp where is stylesheet definition for the QListWidget and QListWidgetItem

    // Add widget into layout
    l->addWidget(m_view);

    // Exit button for Symbian
#ifdef Q_OS_SYMBIAN
    QPixmap closePixmap(":/icons/close.png");
    QPixmap closePixmap2(":/icons/close_on.png");
    m_closeButton = new Button(Button::EExitBtn, closePixmap.size(), closePixmap, closePixmap2);
    QObject::connect(m_closeButton, SIGNAL(pressed()), this, SLOT(reject()));
    m_scene->addItem(m_closeButton);
    m_closeButton->setZValue(100);
#endif

    // Send button
    QPixmap sendPixmap(":/icons/messagebottle.png");
    QPixmap sendPixmap2(":/icons/messagebottle_on.png");
    m_sendButton = new Button(Button::ESendBtn, sendPixmap.size(), sendPixmap, sendPixmap2);
    QObject::connect(m_sendButton, SIGNAL(pressed()), this, SLOT(selectContact()));
    m_scene->addItem(m_sendButton);
    m_sendButton->setZValue(100.1);
    
    // Array
    m_array = new QGraphicsPixmapItem(QPixmap(":/icons/arrow_down.png"));
    m_array->setFlag(QGraphicsItem::ItemIsFocusable, false);
    m_scene->addItem(m_array);
    m_array->setZValue(101);
    m_array->setVisible(false);

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
    this->setWindowTitle("Select boat");
#endif

    m_proxyToList->setFocus();
    
    // Use layout
    this->setLayout(l);

    // Create QContactManager and search contacts
    QTimer::singleShot(100,this,SLOT(findContacts()));
    
}

ContactsDialog::~ContactsDialog()
{
    delete m_listWidget;
}

void ContactsDialog::hideArray()
{
    m_array->setVisible(false);
}

void ContactsDialog::showArray()
{
    // If all items does not fit into list, show small array to user to tell that
    // there is more items coming when user scrolls the qlistwidget
    QListWidgetItem* item = m_listWidget->item(0);
    if (item) {
        QRect ir = m_listWidget->visualItemRect(item);
        if (ir.height() * m_listWidget->count() > m_listWidget->size().height()) {
            m_array->setVisible(true);
            QTimer::singleShot(2000, this, SLOT(hideArray()));
        }
    }
}

void ContactsDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);

    QSize s = size();
    s = s - QSize(width() * 0.1, height() * 0.1);
    QPixmap scaledPix = m_background->pixmap().scaled(s, Qt::IgnoreAspectRatio,
        Qt::SmoothTransformation);
    m_background->setPixmap(scaledPix);
    m_background->setPos((size().width() - m_background->boundingRect().width()) / 2,
        (size().height() - m_background->boundingRect().height()) / 2);

    QRect r = rect();
    r.adjust(width() * 0.3, height() * 0.3, width() * -0.3, height() * -0.20);
    m_proxyToList->setGeometry(r);

#ifdef Q_OS_SYMBIAN
    m_closeButton->setPos(QPointF(s.width() - m_closeButton->boundingRect().width() - 30,
        m_closeButton->boundingRect().height() + 30));

    m_sendButton->setPos(QPointF(s.width() - m_sendButton->boundingRect().width() - 20,
        s.height() - m_sendButton->boundingRect().height() - 10));

#else
    m_sendButton->setPos(QPointF(s.width() - m_sendButton->boundingRect().width() - 20,
        s.height() - m_sendButton->boundingRect().height() - 10));
#endif

    
    m_array->setPos(QPointF((size().width() - m_array->boundingRect().width()) / 2, size().height()
        - (m_array->boundingRect().height() * 2)));
    
    QPointF p = m_proxyToList->pos();
    p -= QPoint(0, 40);
    m_title->setPos(p);

}

void ContactsDialog::findContacts()
{
    QListWidgetItem *currItem = new QListWidgetItem;
    currItem->setData(Qt::DisplayRole, "Searching contacts...");
    m_listWidget->addItem(currItem);

    searchContact();
    showArray();
}

void ContactsDialog::searchContact()
{
    if (m_contactCache->count()>0) {
        m_listWidget->clear();
        foreach (QContact contact, *m_contactCache) {
            QListWidgetItem *currItem = new QListWidgetItem;
            QContactDisplayLabel dl = contact.detail(QContactDisplayLabel::DefinitionName);
            currItem->setData(Qt::DisplayRole, dl.label());
            currItem->setData(Qt::UserRole, contact.localId()); // also store the id of the contact
            m_listWidget->addItem(currItem);
        }
    } else {
        // Sort contacts by lastname
        QContactSortOrder sort;
        sort.setDirection(Qt::AscendingOrder);
        sort.setDetailDefinitionName(QContactName::DefinitionName, QContactName::FieldLastName);

        // Build QListWidget from the contact list
        QList<QContactLocalId> contactIds = m_contactManager->contactIds(sort);

        m_listWidget->clear();

        QContact currContact;
        foreach (const QContactLocalId& id, contactIds) {
            QListWidgetItem *currItem = new QListWidgetItem;
            currContact = m_contactManager->contact(id);
            QContactDisplayLabel dl = currContact.detail(QContactDisplayLabel::DefinitionName);
            currItem->setData(Qt::DisplayRole, dl.label());
            currItem->setData(Qt::UserRole, currContact.localId()); // also store the id of the contact
            m_listWidget->addItem(currItem);
            m_contactCache->append(currContact);
        }
    }
}

void ContactsDialog::selectContact()
{
    QList<QListWidgetItem*> items = m_listWidget->selectedItems();
    if (!items.isEmpty()) {
        itemClicked(items.first());
    }
}

void ContactsDialog::itemClicked(QListWidgetItem *item)
{
    QVariant data = item->data(Qt::UserRole);
    if (data.isValid()) {
        QContactLocalId id = data.toInt();
        QContact contact = m_contactManager->contact(id);
        QContactPhoneNumber cpn = contact.detail<QContactPhoneNumber> ();

        // Emit contact phonenumber
        if (!cpn.isEmpty()) {
            emit contactSelected(cpn.number());
            // Close dialog
            close();
        }
    }
}

