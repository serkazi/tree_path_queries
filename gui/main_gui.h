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
#include "tpq_gui.h"

class main_gui: public QMainWindow {
    Q_OBJECT
public:
    main_gui() ;
protected:
#ifndef QT_NO_CONTEXTMENU
    void contextMenuEvent(QContextMenuEvent *event) override;
#endif // QT_NO_CONTEXTMENU
private slots:
    void newFile();
    void open();
    void save();
    void print();
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void bold();
    void italic();
    void leftAlign();
    void rightAlign();
    void justify();
    void center();
    void setLineSpacing();
    void setParagraphSpacing();
    void about();
    void aboutQt();
    void plotHistogram();
private:
    tpq_gui *central_widget;
    void createActions();
    void createMenus();
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *formatMenu;
    QMenu *helpMenu;
    QActionGroup *alignmentGroup;
    QAction *histogramAct;
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *printAct;
    QAction *exitAct;
    QAction *undoAct;
    QAction *redoAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *boldAct;
    QAction *italicAct;
    QAction *leftAlignAct;
    QAction *rightAlignAct;
    QAction *justifyAct;
    QAction *centerAct;
    QAction *setLineSpacingAct;
    QAction *setParagraphSpacingAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
    QLabel *infoLabel;
};


#endif //TREE_PATH_QUERIES_MAIN_GUI_H
