// Created by kazi on 2019-12-15.
#include <QApplication>
#include <QPushButton>
#include "tpq_gui.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    tpq_gui userControls;
    QPushButton button("Hello world !");
    userControls.show();
    return app.exec();
}
