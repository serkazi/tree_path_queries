//
// Created by kazi on 2019-12-15.
//

#ifndef TREE_PATH_QUERIES_TPQ_GUI_H
#define TREE_PATH_QUERIES_TPQ_GUI_H
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <iostream>
#include "qcustomplot.h"
#include <sstream>
#include <fstream>
#include <iosfwd>
#include <ios>
#include <regex>
#include <string>
#include <cstdio>
#include <cassert>
#include "qcustomplot.h"

class tpq_gui: public QWidget {
    Q_OBJECT
private:
    QLineEdit *numQueriesQLineEdit;
    QVector<QRadioButton *> queryTypeButtons;
    QVector<QRadioButton *> querySizeButtons;
    QVector<QCheckBox *> boxes;
    const std::vector<std::string> ds= {"nv","nv_lca","nv_sct","hybrid",
                                        "tree_ext_ptr","wt_hpd_un","wt_hpd_rrr",
                                        "tree_ext_sct_un","tree_ext_sct_rrr"};
public:
    tpq_gui(QWidget *parent = nullptr) ;
    void set_dataset( std::string pth ) ;
    void plot_histogram( std::string pth ) ;
private:
    QCustomPlot *plot_histogram_( std::istream &input ) ;
    std::string dataset_full_path;
    QGroupBox *createFirstExclusiveGroup() ;
    QGroupBox *createSecondExclusiveGroup() ;
    QGroupBox *createNonExclusiveGroup() ;
    QGroupBox *createPushButtonGroup() ;
public slots:
    void clickedSlot() ;
    void saveToFileSlot() ;
    void loadFromFile() ;
};


#endif //TREE_PATH_QUERIES_TPQ_GUI_H
