//
// Created by kazi on 2019-12-15.
//

#include "tpq_gui.h"
#include "suites.cpp"

tpq_gui::tpq_gui( QWidget *parent ): QWidget(parent) {
    QGridLayout *grid = new QGridLayout();
    grid->addWidget(createFirstExclusiveGroup(), 0, 2);
    grid->addWidget(createSecondExclusiveGroup(), 1, 2);
    grid->addWidget(createNonExclusiveGroup(), 0, 3);
    grid->addWidget(createPushButtonGroup(), 1, 3);

    QCustomPlot *customPlot = new QCustomPlot;
    QVector<double> x(101), y(101); // initialize with entries 0..100
    for (int i = 0; i < 101; ++i) {
        x[i] = i / 50.0 - 1; // x goes from -1 to 1
        y[i] = x[i] * x[i]; // let's plot a quadratic function
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
    grid->addWidget(customPlot, 0, 0, 2, 2);
    setLayout(grid);
    setWindowTitle(tr("Group Boxes"));
    resize(960, 640);
}

QGroupBox *tpq_gui::createFirstExclusiveGroup() {
    auto *groupBox = new QGroupBox(tr("Query types"));
    queryTypeButtons= {new QRadioButton(tr("median")), new QRadioButton(tr("counting")), new QRadioButton(tr("reporting"))};
    QVBoxLayout *vbox = new QVBoxLayout;
    for ( auto x: queryTypeButtons ) {
        x->setCheckable(true);
        vbox->addWidget(x);
    }
    queryTypeButtons[0]->setChecked(true);
    vbox->addStretch(1);
    groupBox->setLayout(vbox);
    return groupBox;
}

QGroupBox *tpq_gui::createSecondExclusiveGroup() {
    QGroupBox *groupBox = new QGroupBox(tr("Configuration"));
    groupBox->setCheckable(true);
    groupBox->setChecked(false);
    querySizeButtons= {new QRadioButton(tr("large(K=1)")), new QRadioButton(tr("medium(K=10)")), new QRadioButton(tr("small(K=100)"))};
    queryTypeButtons.front()->setChecked(true);
    //QCheckBox *checkBox = new QCheckBox(tr("Ind&ependent checkbox"));
    //checkBox->setChecked(true);
    QVBoxLayout *vbox = new QVBoxLayout;
    for ( auto x: querySizeButtons )
        vbox->addWidget(x);
    querySizeButtons[0]->setChecked(true);
    //vbox->addWidget(checkBox);
    vbox->addStretch(1);
    groupBox->setLayout(vbox);
    return groupBox;
}

QGroupBox *tpq_gui::createNonExclusiveGroup() {
    QGroupBox *groupBox = new QGroupBox(tr("Data structures"));
    groupBox->setFlat(true);

    boxes.resize(ds.size());
    for ( auto i= 0; i < ds.size(); ++i ) {
        boxes[i] = new QCheckBox(tr(ds[i].c_str()));
        // TODO: set tooltips
        //boxes[i]->setToolTip(tr("<h2><b><font color='red'>ds[i]</font></b></h2>"
        //                             "<ol>"
        //                             "<li>First</li>"
        //                             "<li>Second</li>"
        //                             "<li>Third</li>"
        //                             "</ol>"));
    }
    // QCheckBox *tristateBox = new QCheckBox(tr("Tri-&state button"));
    // tristateBox->setTristate(true);
    QGridLayout *grid= new QGridLayout;
    grid->addWidget(boxes[0], 0, 0);
    grid->addWidget(boxes[1], 1, 0);
    grid->addWidget(boxes[2], 2, 0);
    grid->addWidget(boxes[3], 3, 0);
    grid->addWidget(boxes[4], 4, 0);

    grid->addWidget(boxes[5], 0, 1);
    grid->addWidget(boxes[6], 1, 1);
    grid->addWidget(boxes[7], 2, 1);
    grid->addWidget(boxes[8], 3, 1);

    groupBox->setLayout(grid);
    return groupBox;
}

QGroupBox *tpq_gui::createPushButtonGroup() {
    QGroupBox *groupBox = new QGroupBox(tr("&Push Buttons"));
    //groupBox->setCheckable(true);
    //groupBox->setChecked(true);
    QPushButton *pushButton = new QPushButton(tr("Execute"));

    QObject::connect(pushButton,SIGNAL(clicked()),this,SLOT(clickedSlot()));

    numQueriesQLineEdit= new QLineEdit;
    numQueriesQLineEdit->setValidator(new QIntValidator(numQueriesQLineEdit));
    numQueriesQLineEdit->setPlaceholderText("size of query-set");
    //QPushButton *toggleButton = new QPushButton(tr("&Toggle Button"));
    //toggleButton->setCheckable(true);
    //toggleButton->setChecked(true);
    //QPushButton *flatButton = new QPushButton(tr("&Flat Button"));
    //flatButton->setFlat(true);
    //QPushButton *popupButton = new QPushButton(tr("Pop&up Button"));
    //QMenu *menu = new QMenu(this);
    //menu->addAction(tr("&First Item"));
    //menu->addAction(tr("&Second Item"));
    //menu->addAction(tr("&Third Item"));
    //menu->addAction(tr("F&ourth Item"));
    //popupButton->setMenu(menu);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(numQueriesQLineEdit);
    vbox->addWidget(pushButton);
    //vbox->addWidget(toggleButton);
    //vbox->addWidget(flatButton);
    //vbox->addWidget(popupButton);
    //vbox->addStretch(1);
    groupBox->setLayout(vbox);
    return groupBox;
}

void tpq_gui::clickedSlot() {

    QString selectedQueryType= [&]() {
        for ( auto x: queryTypeButtons )
            if ( x->isChecked() )
                return x->text();
        return QString();
    }();

    std::stringstream str;
    bool is_first= true ;
    for ( auto i= 0; i < boxes.size(); ++i )
        if ( boxes[i]->isChecked() ) {
            if ( not is_first )
                str << "|";
            str << ".*" << ds[i] << "_" << selectedQueryType.toStdString();
            is_first= false;
        }

    std::cerr << "The regex is: " << str.str() << std::endl;
    // str now stores the regex with which we'll run benchmarks
    // the intention: --benchmark_filter=str.str()

    // The layout of our argv:
    // <dataset_full_path> <-- 0
    // <num_queries> <-- 1
    // <K> <-- 2
    // --benchmark_counters_tabular=true
    // --benchmark_format=json
    // --benchmark_out -- filename here; maybe to text area?
    int num_queries= static_cast<int>(strtol(numQueriesQLineEdit->text().toStdString().c_str(),nullptr,10));
    int K= [&]()->int {
        int i= 0;
        for ( ;i < querySizeButtons.size(); ++i )
            if ( querySizeButtons[i]->isChecked() )
                break ;
        assert( i < querySizeButtons.size() );
        std::regex r("(\\d+)");
        std::smatch m;
        std::string text= queryTypeButtons[i]->text().toStdString();
        if ( std::regex_search(text,m,r) ) {
            return strtol(m[1].str().c_str(),nullptr,10);
        }
        return 1;
    }();

    {
        int argc = 7;
        char **argv = new char *[argc];
        for (auto l = 0; l < argc; ++l) {
            argv[l] = new char[0x80];
            memset(argv[l], '\0', 0x80 * sizeof(*(argv[l])));
        }
        strcpy(argv[0], dataset_full_path.c_str());
        sprintf(argv[1], "%d", num_queries);
        sprintf(argv[2], "%d", K);
        sprintf(argv[3], "%s", "--benchmark_counters_tabular=true");
        sprintf(argv[4], "%s", "--benchmark_format=json");
        sprintf(argv[5], "%s", "--benchmark_out=sample_res.json");
        sprintf(argv[6], "%s", (std::string("--benchmark_filter=") + str.str()).c_str());
        RunAllGiven(argc, argv);

        //for (auto l = 0; l < argc; delete[] argv[l++]);
        //delete[] argv;
    }
    // /users/grad/kazi/CLionProjects/tree_path_queries/cmake-build-debug/src/benchmarking/utils/aggregate_bench
    // --benchmark_filter=.*nv_lca_counting\|.*tree_ext_ptr_counting
    // /users/grad/kazi/CLionProjects/tree_path_queries/data/datasets/eu.mst.osm.puu 1000000 1
    // --benchmark_counters_tabular=true --benchmark_format=json
    // --benchmark_out=/users/grad/kazi/CLionProjects/tree_path_queries/cmake-build-debug/src/benchmarking/utils/run_results.json
}

void tpq_gui::saveToFileSlot() {
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save image"), "",
                                                    tr("image type (*.jpg,*.png);;All Files (*)"));
    if (fileName.isEmpty())
        return;
    else {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),
                                     file.errorString());
            return;
        }
        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_4_5);
        //out << contacts;
    }
}

void tpq_gui::loadFromFile() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Weighted tree files"), "",
                                                    tr("Trees (*.puu,*.txt);;All Files (*)"));
    if (fileName.isEmpty())
        return;
    else {

        QFile file(fileName);

        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),
                                     file.errorString());
            return;
        }

        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_4_5);
        if (fileName.isEmpty())
            return;
        else {
            QFile file(fileName);
            if (!file.open(QIODevice::ReadOnly)) {
                QMessageBox::information(this, tr("Unable to open file"),
                                         file.errorString());
                return;
            }
            QDataStream in(&file);
            in.setVersion(QDataStream::Qt_4_5);
            /*
            if (contacts.isEmpty()) {
                QMessageBox::information(this, tr("No contacts in file"),
                                         tr("The file you are attempting to open contains no contacts."));
            } else {
                QMap<QString, QString>::iterator i = contacts.begin();
                nameLine->setText(i.key());
                addressText->setText(i.value());
            }
             */
        }
        //updateInterface(NavigationMode);
    }
}

void tpq_gui::set_dataset(std::string pth) {
    dataset_full_path= pth;
}
