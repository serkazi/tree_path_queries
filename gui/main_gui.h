//
// Created by kazi on 2019-12-15.
//

#ifndef TREE_PATH_QUERIES_MAIN_GUI_H
#define TREE_PATH_QUERIES_MAIN_GUI_H


#include <QtWidgets/QMainWindow>
#include <QtWidgets/QActionGroup>
#include <QtWidgets/QLabel>
#include <QtWidgets//QStatusBar>
#include <QMenuBar>
#include <QContextMenuEvent>

class main_gui: public QMainWindow {
    Q_OBJECT
public:
    main_gui() ;
protected:
#ifndef QT_NO_CONTEXTMENU
    void contextMenuEvent(QContextMenuEvent *event) override;
#endif // QT_NO_CONTEXTMENU
private slots:
private:
    void createActions();
    void createMenus();
};


#endif //TREE_PATH_QUERIES_MAIN_GUI_H
