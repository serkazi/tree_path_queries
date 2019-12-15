//
// Created by kazi on 2019-12-15.
//

#ifndef TREE_PATH_QUERIES_USERCONTROLS_H
#define TREE_PATH_QUERIES_USERCONTROLS_H

#include <QtWidgets/QGroupBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include "qcustomplot.h"

class UserControls: public QWidget {
// Q_OBJECT
public:
    UserControls(QWidget *parent = nullptr): QWidget(parent) {
        QGridLayout *grid = new QGridLayout();
        grid->addWidget(createFirstExclusiveGroup(), 0, 0);
        grid->addWidget(createSecondExclusiveGroup(), 1, 0);
        grid->addWidget(createNonExclusiveGroup(), 0, 1);
        grid->addWidget(createPushButtonGroup(), 1, 1);

        QCustomPlot *customPlot= new QCustomPlot;
        QVector<double> x(101), y(101); // initialize with entries 0..100
        for (int i=0; i<101; ++i)
        {
            x[i] = i/50.0 - 1; // x goes from -1 to 1
            y[i] = x[i]*x[i]; // let's plot a quadratic function
        }
        // create graph and assign data to it:
        customPlot->addGraph();
        customPlot->graph(0)->setData(x, y);
        // give the axes some labels:
        customPlot->xAxis->setLabel("x");
        customPlot->yAxis->setLabel("y");
        // set axes ranges, so we see all data:
        customPlot->xAxis->setRange(-1, 1);
        customPlot->yAxis->setRange(0, 1);
        customPlot->replot();
        grid->addWidget(customPlot,2,0,1,2);
        setLayout(grid);
        setWindowTitle(tr("Group Boxes"));
        resize(480, 320);
   }
private:
    QGroupBox *createFirstExclusiveGroup() {
        QGroupBox *groupBox = new QGroupBox(tr("Query types"));
        QRadioButton *radio1 = new QRadioButton(tr("Path median"));
        QRadioButton *radio2 = new QRadioButton(tr("Path counting"));
        QRadioButton *radio3 = new QRadioButton(tr("Path reporting"));
        radio1->setChecked(true);
        QVBoxLayout *vbox = new QVBoxLayout;
        vbox->addWidget(radio1);
        vbox->addWidget(radio2);
        vbox->addWidget(radio3);
        vbox->addStretch(1);
        groupBox->setLayout(vbox);
        return groupBox;
    }
    QGroupBox *createSecondExclusiveGroup() {
        QGroupBox *groupBox = new QGroupBox(tr("Configuration"));
        groupBox->setCheckable(true);
        groupBox->setChecked(false);
        QRadioButton *radio1 = new QRadioButton(tr("large"));
        QRadioButton *radio2 = new QRadioButton(tr("medium"));
        QRadioButton *radio3 = new QRadioButton(tr("small"));
        radio1->setChecked(true);
        QCheckBox *checkBox = new QCheckBox(tr("Ind&ependent checkbox"));
        checkBox->setChecked(true);
        QVBoxLayout *vbox = new QVBoxLayout;
        vbox->addWidget(radio1);
        vbox->addWidget(radio2);
        vbox->addWidget(radio3);
        vbox->addWidget(checkBox);
        vbox->addStretch(1);
        groupBox->setLayout(vbox);
        return groupBox;
    }
    QGroupBox *createNonExclusiveGroup() {
        QGroupBox *groupBox = new QGroupBox(tr("Data structures"));
        groupBox->setFlat(true);
        std::vector<std::string> ds=
                            {"naive","naive_lca","nsrs","hybrid","tree_ext_ptr",
                           "tree_ext_sct (un)","tree_ext_sct (rrr)",
                           "wt_hpd (un)", "wt_hpd (rrr)"};
        std::vector<QCheckBox *> checkBoxes(ds.size());
        for ( auto i= 0; i < ds.size(); ++i ) {
            checkBoxes[i] = new QCheckBox(tr(ds[i].c_str()));
            //checkBoxes[i]->setToolTip(tr("<h2><b><font color='red'>ds[i]</font></b></h2>"
            //                             "<ol>"
            //                             "<li>First</li>"
            //                             "<li>Second</li>"
            //                             "<li>Third</li>"
            //                             "</ol>"));
        }
        // QCheckBox *tristateBox = new QCheckBox(tr("Tri-&state button"));
        // tristateBox->setTristate(true);
        QGridLayout *grid= new QGridLayout;
        grid->addWidget(checkBoxes[0], 0, 0);
        grid->addWidget(checkBoxes[1], 1, 0);
        grid->addWidget(checkBoxes[2], 2, 0);
        grid->addWidget(checkBoxes[3], 3, 0);
        grid->addWidget(checkBoxes[4], 4, 0);

        grid->addWidget(checkBoxes[5], 0, 1);
        grid->addWidget(checkBoxes[6], 1, 1);
        grid->addWidget(checkBoxes[7], 2, 1);
        grid->addWidget(checkBoxes[8], 3, 1);

        groupBox->setLayout(grid);
        return groupBox;
    }

    QGroupBox *createPushButtonGroup() {
        QGroupBox *groupBox = new QGroupBox(tr("&Push Buttons"));
        groupBox->setCheckable(true);
        groupBox->setChecked(true);
        QPushButton *pushButton = new QPushButton(tr("&Normal Button"));
        QPushButton *toggleButton = new QPushButton(tr("&Toggle Button"));
        toggleButton->setCheckable(true);
        toggleButton->setChecked(true);
        QPushButton *flatButton = new QPushButton(tr("&Flat Button"));
        flatButton->setFlat(true);
        QPushButton *popupButton = new QPushButton(tr("Pop&up Button"));
        QMenu *menu = new QMenu(this);
        menu->addAction(tr("&First Item"));
        menu->addAction(tr("&Second Item"));
        menu->addAction(tr("&Third Item"));
        menu->addAction(tr("F&ourth Item"));
        popupButton->setMenu(menu);
        QVBoxLayout *vbox = new QVBoxLayout;
        vbox->addWidget(pushButton);
        vbox->addWidget(toggleButton);
        vbox->addWidget(flatButton);
        vbox->addWidget(popupButton);
        vbox->addStretch(1);
        groupBox->setLayout(vbox);
        return groupBox;
    }
};
#endif //TREE_PATH_QUERIES_USERCONTROLS_H
