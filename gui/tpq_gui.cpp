//
// Created by kazi on 2019-12-15.
//

#include "tpq_gui.h"
#include "suites.cpp"

tpq_gui::tpq_gui( QWidget *parent ): QWidget(parent) {
    textEdit= new QTextEdit;
    QGridLayout *grid = new QGridLayout();
    grid->addWidget(createFirstExclusiveGroup(), 0, 2);
    grid->addWidget(createSecondExclusiveGroup(), 1, 2);
    grid->addWidget(createNonExclusiveGroup(), 0, 3);
    grid->addWidget(createPushButtonGroup(), 1, 3);
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

    for ( result_filename= QString(tr("")); result_filename.isEmpty(); ) {
        result_filename = [this]() {
            QString fileName = QFileDialog::getSaveFileName(this,
                                                            tr("Save result"), "",
                                                            tr("JSON type (*.json);;All Files (*)"));
            if (fileName.isEmpty())
                return QString(tr(""));
            else {
                QFile file(fileName);
                if (!file.open(QIODevice::WriteOnly)) {
                    QMessageBox::information(this, tr("Unable to open file"),
                                             file.errorString());
                    return QString(tr(""));
                }
                // QDataStream out(&file);
                return file.fileName();
                //out.setVersion(QDataStream::Qt_4_5);
                //out << contacts;
            }
        }();
    }

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
        sprintf(argv[5], "%s", (std::string("--benchmark_out=")+result_filename.toStdString()).c_str());
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
                                                    tr("Save result"), "",
                                                    tr("JSON type (*.json);;All Files (*)"));
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
        //out.setVersion(QDataStream::Qt_4_5);
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

void tpq_gui::plot_histogram(std::string pth) {
    std::ifstream input(pth);
    plot= plot_histogram_(input);
    QWidget *w= new QWidget();
    QGridLayout *grid= new QGridLayout();
    grid->addWidget(plot, 0, 0, 2, 2);
    w->setLayout(grid);
    w->setWindowTitle(tr("Histogram"));
    w->resize(480, 320);
    w->show();
}

QCustomPlot *tpq_gui::plot_histogram_(std::istream &input) {

    // first, read the incoming JSON
    nlohmann::json obj; input >> obj;
    nlohmann::json arr= obj["benchmarks"]; //FIXME: what if such a key is non-existent?

    // secondly, parse the JSON and extract what we need
    QVector<QString> labels;
    QVector<double> ticks;
    QVector<double> data;
    for ( auto i= 0; i < arr.size(); ++i ) {
        const auto &a= arr[i];
        std::string dsname= [&a]()->std::string {
            std::string text= a["name"];
            auto pos= text.find('/');
            text= text.substr(pos+1,std::string::npos);
            auto rit= std::find_if(text.rbegin(),text.rend(),[](char ch) {
                return ch == '_';
            });
            text.resize(std::distance(std::next(rit),text.rend()));
            return text;
        }();
        ticks << i;
        data.push_back(static_cast<double>(a["seconds"]));
        labels << tr(dsname.c_str());
    }

    // finally, draw it
    QCustomPlot *customPlot = new QCustomPlot;
    customPlot->setBackground(QBrush(EconLighter));

    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->addTicks(ticks, labels);

    customPlot->xAxis->grid()->setVisible(false);
    customPlot->xAxis->setSubTicks(false);
    customPlot->xAxis->setTickLength(0, 4);
    customPlot->xAxis->setRange(0, ds.size());
    customPlot->xAxis->setTicker(textTicker);
    customPlot->xAxis->setTickLabelRotation(60);

    customPlot->yAxis->setRange(0, *(std::max_element(data.begin(),data.end()))+3);
    customPlot->yAxis->setPadding(5); // a bit more space to the left border
    customPlot->yAxis->setLabel("Seconds to complete");
    customPlot->yAxis->setBasePen(QPen(Qt::black));
    customPlot->yAxis->setTickPen(QPen(Qt::black));
    customPlot->yAxis->setSubTickPen(QPen(Qt::black));
    customPlot->yAxis->grid()->setSubGridVisible(false);
    customPlot->yAxis->grid()->setPen(QPen(Shadow.lighter(227),0,Qt::SolidLine));

    QCPBars *bars= new QCPBars(customPlot->xAxis,customPlot->yAxis);
    bars->setName("Time to answer all queries (in seconds)");
    bars->setData(ticks,data);
    bars->setPen(QPen(Shadow.lighter(170)));
    bars->setBrush(EconBlue);

    customPlot->addGraph();
    // setup legend:
    customPlot->legend->setVisible(false);
    customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignHCenter);
    customPlot->legend->setBrush(QColor(255, 255, 255, 100));
    customPlot->legend->setBorderPen(Qt::NoPen);
    QFont legendFont = font();
    legendFont.setPointSize(10);
    customPlot->legend->setFont(legendFont);
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    //==========================

//customPlot->xAxis->setBasePen(QPen(Qt::white));
//customPlot->xAxis->setTickPen(QPen(Qt::white));
// customPlot->xAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::DotLine));
// customPlot->xAxis->setTickLabelColor(Qt::white);
//customPlot->xAxis->setLabelColor(Qt::white);

// prepare y axis:

// make y-axis invisible
//customPlot->yAxis->setTickLabelColor(Qt::white);
//customPlot->yAxis->setLabelColor(Qt::white);
//customPlot->yAxis->grid()->setPen(QPen(QColor(130, 130, 130), 0, Qt::SolidLine));
//like Economist
//customPlot->yAxis->grid()->setSubGridPen(LightGrey) //QPen(QColor(130, 130,130), 0, Qt::DotLine)); There is no subgrid in Economist

// prepare the data
    return customPlot;
}

void tpq_gui::save_plot( std::string pth ) {
    // register the plot document object (only needed once, no matter how many plots will be in the QTextDocument):
    QCPDocumentObject *plotObjectHandler = new QCPDocumentObject(this);
    this->textEdit->document()->documentLayout()->registerHandler(QCPDocumentObject::PlotTextFormat, plotObjectHandler);
    QTextCursor cursor = this->textEdit->textCursor();
    // insert the current plot at the cursor position. QCPDocumentObject::generatePlotFormat creates a
    // vectorized snapshot of the passed plot (with the specified width and height) which gets inserted
    // into the text document.
    double width = 0;
    double height = 0;
    cursor.insertText(
            QString(QChar::ObjectReplacementCharacter),
            QCPDocumentObject::generatePlotFormat(this->plot, width, height));
    this->textEdit->setTextCursor(cursor);

    // @see: https://wiki.qt.io/Exporting_a_document_to_PDF
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPaperSize(QPrinter::A4);
    std::string filename= pth;
    printer.setOutputFileName(tr(filename.c_str()));

    //this->textEdit->setPageSize(printer.pageRect().size()); // This is necessary if you want to hide the page number
    this->textEdit->print(&printer);
}
