// Created by kazi on 2019-12-15.
#include <QApplication>
#include <QPushButton>
#include "UserControls.hpp"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    UserControls userControls;
    QPushButton button("Hello world !");
    userControls.show();
    return app.exec();
}
