// Created by kazi on 2019-12-15.
#include <QApplication>
#include <QPushButton>
#include "tpq_gui.h"
#include "main_gui.h"
//#include <QQuickStyle>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    // QQuickStyle::setStyle("Material");
    //tpq_gui userControls;
    main_gui mainWindow;
    //mainWindow.setCentralWidget(&userControls);
    //main_gui mg;
    //userControls.show();
    mainWindow.show();
    //userControls.show();
    return app.exec();
}
