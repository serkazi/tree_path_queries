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
#include "qcpdocumentobject.h"

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
    QColor Red = QColor(0xE3,0x12,0x0B),
            White = QColor(0xFA,0xFA,0xFA),
            DarkGray = QColor(0x4A,0x4A,0x4A),
            MidGreen = QColor(0x91,0xB8,0xBD),
            Turquoise = QColor(0xAC,0xC8,0xD4),
            BrightTurq = QColor(0x9A,0xE5,0xDE),
            LightGrey = QColor(0xD4,0xDD,0xDD),
            DarkGreen = QColor(0x24,0x47,0x47),
            Green = QColor(0x33,0x66,0x66),
            MidBlue = QColor(0x8A,0xBB,0xD0),
            Beige = QColor(0xEF,0xE8,0xD1),
            // 02. Subdued & Professional
            Shadow= QColor(0x2A,0x31,0x32),
            Stone= QColor(0x33,0x6B,0x87),
            AutumnFoliage= QColor(0x76,0x36,0x26),
            Mist= QColor(0x90,0xAF,0xC5),
            EconLighter= QColor(0xC4,0xD9,0xE5),
            EconDarker= QColor(0x23,0x53,0x6F),
            EconBlue= QColor(0x07,0x6D,0xA2),
            EconGrey= QColor(0xD8,0xD8,0xD8);

public:
    tpq_gui(QWidget *parent = nullptr) ;
    void set_dataset( std::string pth ) ;
    void plot_histogram( std::string pth ) ;
    void save_plot( std::string pth ) ;
private:
    QCustomPlot *plot_histogram_( std::istream &input ) ;
    std::string dataset_full_path;
    QTextEdit *textEdit;
    QString result_filename;
    QGroupBox *createFirstExclusiveGroup() ;
    QGroupBox *createSecondExclusiveGroup() ;
    QGroupBox *createNonExclusiveGroup() ;
    QGroupBox *createPushButtonGroup() ;
    QCustomPlot *plot;
public slots:
    void clickedSlot() ;
    void saveToFileSlot() ;
    void loadFromFile() ;
};


#endif //TREE_PATH_QUERIES_TPQ_GUI_H
