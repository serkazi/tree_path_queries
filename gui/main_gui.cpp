//
// Created by kazi on 2019-12-15.
//

#include <QtWidgets/QVBoxLayout>
#include "main_gui.h"

main_gui::main_gui()
{
    QWidget *widget = new QWidget;
    setCentralWidget(widget);

    QWidget *topFiller = new QWidget;
    topFiller->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QMainWindow::menuBar();

    setWindowTitle(tr("Menus"));
    setMinimumSize(160, 160);
    resize(480, 320);
}

void main_gui::createActions() {
}

void main_gui::createMenus()
{
}

#ifndef QT_NO_CONTEXTMENU
void main_gui::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.exec(event->globalPos());
}

#endif // QT_NO_CONTEXTMENU
