/*
 * Copyright (c) 2011 Nokia Corporation.
 */

#include <QtGui/QApplication>
#include "mainwindow.h"

// Lock S60 orientation
#ifdef Q_OS_SYMBIAN
#include <eikenv.h>
#include <eikappui.h>
#include <aknenv.h>
#include <aknappui.h>
#endif


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    
    // Set style for the QListWidget and QListWidgetItem
    QString style;
    style += "QListWidget {background-color: transparent;}";
    style += "QListWidget::item {background-color: transparent;}";
    style += "QListWidget::item {selection-color: white;}";
    style += "QListWidget::item {color: black;}";

    a.setStyleSheet(style);

    // Lock S60 orientation
    #ifdef Q_OS_SYMBIAN
    CAknAppUi* appUi = dynamic_cast<CAknAppUi*> (CEikonEnv::Static()->AppUi());
    TRAP_IGNORE( 
    if (appUi) {
        appUi->SetOrientationL(CAknAppUi::EAppUiOrientationLandscape);
    }
    );
    #endif
    
    MainWindow w;
    
    #ifdef Q_OS_SYMBIAN
        w.showFullScreen();
    #else
        w.showMaximized();
    #endif
    return a.exec();
}

