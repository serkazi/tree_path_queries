//
// Created by kazi on 2019-12-15.
//

#include "tpq_gui.h"
#include "suites_modified_weights.cpp"

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

QRadioButton* tpq_gui::getSelectedQueryButton() {
    for (auto x: this->queryTypeButtons)
        if (x->isChecked())
            return x;
    return nullptr;
}

void tpq_gui::clickedSlot() {

    /*
    QString selectedQueryType= [&]() {
        for ( auto x: queryTypeButtons )
            if ( x->isChecked() )
                return x->text();
        return QString();
    }();
     */
    QString selectedQueryType= getSelectedQueryButton()->text();

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
    int num_queries= static_cast<int>(
            strtol(numQueriesQLineEdit->text().toStdString().c_str(),nullptr,10)
            );
    int K= [&]()->int {
        int i= 0;
        for ( ;i < querySizeButtons.size(); ++i )
            if ( querySizeButtons[i]->isChecked() )
                break ;
        assert( i < querySizeButtons.size() );
        std::regex r(R"(([0-9]+))");
        std::smatch m;
        std::string text= querySizeButtons[i]->text().toStdString();
        if ( std::regex_search(text,m,r) ) {
            return strtol(m[1].str().c_str(),nullptr,10);
        }
        return 1;
    }();

    QString method= getSelectedQueryButton()->text();
    QString dts= [this]()-> QString {
        std::regex r("([^/]+)$");
        std::smatch m;
        if ( std::regex_search(dataset_full_path,m,r) )
            return QString(tr(m.str(1).c_str()));
        return QString(tr("NO_DATASET"));
    }();

    QFileDialog dlg(nullptr,tr("Save result in..."),
                    method+QString(tr(" "))+dts,
                    tr("JSON type (*.json);;All Files (*)"));
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    result_filename = [&]() {
        /*
        dlg.getSaveFileName(this,
                            tr("Save result"), method+QString(tr(" "))+dts,
                            tr("JSON type (*.json);;All Files (*)"));
        */
        if ( dlg.exec() ) {
            QString fileName= dlg.selectedFiles().at(0);
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
        }
        else return QString(tr(""));
    }();

    if ( result_filename == QString(tr("")) )
        return ;

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
    // plot= plot_histogram_(pth);
    plot= plot_histogram_2(pth);
    QWidget *w= new QWidget();
    QGridLayout *grid= new QGridLayout();
    grid->addWidget(plot, 0, 0, 2, 2);
    w->setLayout(grid);
    w->setWindowTitle(tr("Histogram"));
    w->resize(480, 320);
    w->show();
}

QCustomPlot *tpq_gui::plot_histogram_( std::string pth ) {

    std::ifstream input(pth);
    // first, read the incoming JSON
    nlohmann::json obj; input >> obj;
    nlohmann::json arr= obj["benchmarks"]; //FIXME: what if such a key is non-existent?

    // extract the dataset as the last entry of the "/"-separated path
    std::string dataset= [&pth]()-> std::string {
        std::regex r(".*([^/]+)$");
        std::smatch m;
        if ( std::regex_search(pth,m,r) )
            return m.str(1);
        return "";
    }();
    assert( not dataset.empty() );

    // secondly, parse the JSON and extract what we need
    QVector<QString> labels;
    QVector<double> ticks;
    QVector<double> data;
    QVector<double> tops;
    int num_queries= static_cast<int>(
            strtol(numQueriesQLineEdit->text().toStdString().c_str(),nullptr,10)
    );
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
        data.push_back(static_cast<double>(a["real_time"])/(1e+3)/num_queries);
        labels << tr(dsname.c_str());
        tops.push_back(static_cast<double>(a["real_time"])/(1e+3)/num_queries);
    }

    QFont fnt("Monospace");
    fnt.setStyleHint(QFont::TypeWriter);

    // finally, draw it
    QCustomPlot *customPlot = new QCustomPlot;
    customPlot->setBackground(QBrush(EconLighter));

    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->addTicks(ticks, labels);

    QFont xLabelsFont= fnt;
    xLabelsFont.setPointSize(9);
    customPlot->xAxis->setLabelFont(xLabelsFont);
    customPlot->xAxis->grid()->setVisible(false);
    customPlot->xAxis->setSubTicks(false);
    customPlot->xAxis->setTickLength(0, 4);
    customPlot->xAxis->setRange(0, ds.size());
    customPlot->xAxis->setTicker(textTicker);
    customPlot->xAxis->setTickLabelRotation(60);

    // customPlot->axisRect(0)->addAxis(QCPAxis::AxisType::atRight,EconomistStyleQCPAxis(customPlot->rect(),QCPAxis::AxisType::atRight));
    customPlot->yAxis->setRange(0, *(std::max_element(data.begin(),data.end()))*1.13);
    customPlot->yAxis->setLabelFont(QFont(fnt.family(),9));
    customPlot->yAxis->setPadding(5); // a bit more space to the left border
    customPlot->yAxis->setNumberFormat(tr("gb"));
    // customPlot->yAxis->setTickLabelRotation(60);
    customPlot->yAxis->setLabel("Average query time (microsec.)");
    customPlot->yAxis->setBasePen(QPen(Qt::black));
    customPlot->yAxis->setTickPen(QPen(Qt::black));
    customPlot->yAxis->setSubTickPen(QPen(Qt::black));
    customPlot->yAxis->grid()->setSubGridVisible(false);
    customPlot->yAxis->grid()->setPen(QPen(Shadow.lighter(227),0,Qt::SolidLine));

    QCPBars *bars= new QCPBars(customPlot->xAxis,customPlot->yAxis);
    //bars->setName("Time to answer all queries (in seconds)");
    bars->setData(ticks,data);
    bars->setPen(QPen(Shadow.lighter(170)));
    bars->setBrush(EconBlue);
    bars->setWidth(0.68);

    customPlot->addGraph();

    /*{
        QCPItemText *textLabel = new QCPItemText(customPlot);
        //customPlot->addItem(textLabel);
        textLabel->setClipToAxisRect(false);
        textLabel->position->setAxes(customPlot->xAxis, customPlot->yAxis);
        textLabel->position->setType(QCPItemPosition::ptPlotCoords);
        //placing the item over the bar with a spacing of 0.25

        // Qt::AlignTop|Qt::AlignRight);
        //textLabel->position->setCoords();
        //Customizing the item
        textLabel->setText(tr(dataset.c_str()));

        // textLabel->setFont(QFont(font().family(), 9));
        textLabel->setFont(QFont(fnt.family(), 9));
        textLabel->setPen(QPen(Qt::black));

        QCPLayoutElement *element= new QCPLayoutElement(customPlot);
        element->setLayer(textLabel->text());
        customPlot->plotLayout()->addElement(ds.size()+0.5,customPlot->yAxis->range().lower,element);
    }*/
    // setup legend:

    //customPlot->setWindowTitle(tr(dataset.c_str()));
    //customPlot->legend->setVisible(false);
    /*
    QCPAxisRect *ar = customPlot->axisRect(0);
    customPlot->plotLayout()->addElement(0, 1, ar);
    QCPLegend *legend= new QCPLegend();
    ar->insetLayout()->addElement(legend, Qt::AlignTop|Qt::AlignRight);
    legend->setParent(ar);
    legend->setLayer(tr(dataset.c_str()));
    customPlot->setAutoAddPlottableToLegend(false);
    */

    //customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignHCenter);
    customPlot->legend->setBrush(QColor(255, 255, 255, 100));
    customPlot->legend->setBorderPen(Qt::NoPen);
    // QFont legendFont = font();
    QFont legendFont = fnt;
    legendFont.setPointSize(10);
    customPlot->legend->setFont(legendFont);
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    QVector<QCPItemText *> items;
    //Creating and configuring an item
    double spacing= 300;
    for ( auto i= 0; i < data.size(); ++i ) {
        QCPItemText *textLabel = new QCPItemText(customPlot);
        //customPlot->addItem(textLabel);
        textLabel->setClipToAxisRect(false);
        textLabel->position->setAxes(customPlot->xAxis, customPlot->yAxis);
        textLabel->position->setType(QCPItemPosition::ptPlotCoords);
        //placing the item over the bar with a spacing of 0.25
        textLabel->position->setCoords(ticks[i],data[i]+spacing);
        //Customizing the item
        textLabel->setText(QString::number(data[i]));

        // textLabel->setFont(QFont(font().family(), 9));
        textLabel->setFont(QFont(fnt.family(), 9));
        textLabel->setPen(QPen(Qt::black));
        items.push_back(textLabel);
    }

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

void tpq_gui::save_plot_2( std::string pth ) {
    QString filename= QString(tr(pth.c_str()));
    mainPlot->saveJpg(filename,0,0,1.0,100,1200,QCP::ResolutionUnit::ruDotsPerInch);
}

QCustomPlot *tpq_gui::plot_histogram_2( std::string pth ) {

    QCPTextElement *titleElement= nullptr;

    std::ifstream input(pth);
    // first, read the incoming JSON
    nlohmann::json obj; input >> obj;
    nlohmann::json arr= obj["benchmarks"]; //FIXME: what if such a key is non-existent?

    // extract the dataset as the last entry of the "/"-separated path
    std::string dataset= [&pth]()-> std::string {
        std::regex r(".*([^/]+)$");
        std::smatch m;
        if ( std::regex_search(pth,m,r) )
            return m.str(1);
        return "";
    }();
    assert( not dataset.empty() );

    // secondly, parse the JSON and extract what we need
    QVector<QString> labels, ticklabels;
    QVector<double> ticks;
    QVector<double> data;
    QVector<double> tops;

    for ( auto i= 0; i < arr.size(); ++i ) {
        const auto &a= arr[i];
        auto num_queries=
                static_cast<size_t>(a["num_queries"]);
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
        ticks << i+1;
        data.push_back(static_cast<double>(a["real_time"])/(1e+3)/num_queries);
        // labels << tr(dsname.c_str());
        ticklabels << tr(dsname.c_str());
        labels.push_back(QString::number(i+1));
        tops.push_back(static_cast<double>(a["real_time"])/(1e+3)/num_queries);
    }

    QFont fnt("Monospace");
    fnt.setStyleHint(QFont::TypeWriter);

    QCustomPlot *customPlot = new QCustomPlot;
    mainPlot= customPlot;
    customPlot->setBackground(QBrush(EconLighter));

    if ( auto tmp= getSelectedQueryButton() ) {
        titleElement= new QCPTextElement(mainPlot);
        titleElement->setText(tmp->text());
        QFont titleFont= fnt;
        titleFont.setBold(true);
        titleElement->setFont(titleFont);
        // customPlot->axisRect(0)->insetLayout()->addElement(titleElement,Qt::AlignTop|Qt::AlignLeading);
    }

    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->addTicks(ticks, labels);

    QFont xLabelsFont= fnt;
    xLabelsFont.setPointSize(10);
    customPlot->xAxis->setLabelFont(xLabelsFont);
    customPlot->xAxis->grid()->setVisible(false);
    customPlot->xAxis->setSubTicks(false);
    customPlot->xAxis->setTickLength(0, 4);
    customPlot->xAxis->setRange(0, 1+labels.size());
    customPlot->xAxis->setTicker(textTicker);
    // customPlot->xAxis->setTickLabelRotation(60);

    // experimental features
    customPlot->axisRect(0)->removeAxis(customPlot->yAxis);
    customPlot->axisRect(0)->addAxis(QCPAxis::AxisType::atLeft,new EconomistStyleQCPAxis(customPlot->axisRect(),QCPAxis::AxisType::atLeft));
    customPlot->yAxis->setBasePen(QPen(Qt::NoPen));
    customPlot->yAxis->setTickPen(QPen(Qt::NoPen));
    customPlot->yAxis->setSubTickPen(QPen(Qt::NoPen));
    // end of experimental features

    customPlot->yAxis->setRange(0, *(std::max_element(data.begin(),data.end()))*1.13);
    customPlot->yAxis->setLabelPadding(-15);
    customPlot->yAxis->setLabelFont(xLabelsFont);
    customPlot->yAxis->setPadding(2); // a bit more space to the left border
    customPlot->yAxis->setNumberFormat(tr("gb"));
    // customPlot->yAxis->setTickLabelRotation(60);
    // customPlot->yAxis->setLabel("Time to complete (seconds)");
    //customPlot->yAxis->setOffset(-100);
    // customPlot->yAxis->setBasePen(QPen(Qt::black));
    // customPlot->yAxis->setTickPen(QPen(Qt::black));
    // customPlot->yAxis->setSubTickPen(QPen(Qt::black));
    customPlot->yAxis->setOffset(0);
    customPlot->yAxis->grid()->setSubGridVisible(false);
    customPlot->yAxis->grid()->setVisible(true);
    customPlot->yAxis->grid()->setPen(QPen(Shadow.lighter(227),0,Qt::SolidLine));

    QVector<QCPBars *> bars;
    for ( auto i= 0; i < arr.size(); ++i ) {
        QCPBars *bar = new QCPBars(customPlot->xAxis, customPlot->yAxis);
        //bars->setName("Time to answer all queries (in seconds)");
        QVector<double> keys= {i+1.00};
        QVector<double> values= {data[i]};
        bar->setData(keys,values,true);
        //bar->addData(i + 1, data[i]);
        bar->setPen(QPen(Shadow.lighter(170)));
        bar->setBrush(EconBlue);
        bar->setWidth(0.68);
        bar->setAntialiased(false);
        // bar->setName(tr((QString::number(i+1).toStdString()+labels[i].toStdString())).c_str());
        std::string myLabel= [&]()-> std::string {
            std::string x= labels[i].toStdString();
            std::string y= ticklabels[i].toStdString();
            return x+": "+y;
        }();
        bar->setName(tr(myLabel.c_str()));
        bar->setParent(customPlot);
        bars.push_back(bar);
    }


    /*{
        QCPItemText *textLabel = new QCPItemText(customPlot);
        //customPlot->addItem(textLabel);
        textLabel->setClipToAxisRect(false);
        textLabel->position->setAxes(customPlot->xAxis, customPlot->yAxis);
        textLabel->position->setType(QCPItemPosition::ptPlotCoords);
        //placing the item over the bar with a spacing of 0.25

        // Qt::AlignTop|Qt::AlignRight);
        //textLabel->position->setCoords();
        //Customizing the item
        textLabel->setText(tr(dataset.c_str()));

        // textLabel->setFont(QFont(font().family(), 9));
        textLabel->setFont(QFont(fnt.family(), 9));
        textLabel->setPen(QPen(Qt::black));

        QCPLayoutElement *element= new QCPLayoutElement(customPlot);
        element->setLayer(textLabel->text());
        customPlot->plotLayout()->addElement(ds.size()+0.5,customPlot->yAxis->range().lower,element);
    }*/
    // setup legend:

    //customPlot->setWindowTitle(tr(dataset.c_str()));
    //customPlot->legend->setVisible(false);
    /*
    QCPAxisRect *ar = customPlot->axisRect(0);
    customPlot->plotLayout()->addElement(0, 1, ar);
    QCPLegend *legend= new QCPLegend();
    ar->insetLayout()->addElement(legend, Qt::AlignTop|Qt::AlignRight);
    legend->setParent(ar);
    legend->setLayer(tr(dataset.c_str()));
    customPlot->setAutoAddPlottableToLegend(false);
    */

    //customPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignHCenter);
    //customPlot->legend->setVisible(true);

    //customPlot->legend->setBrush(QColor(255, 255, 255, 100));
    //customPlot->legend->setBorderPen(Qt::NoPen);
    // QFont legendFont = font();
    //QFont legendFont = fnt;
    //legendFont.setPointSize(10);
    //customPlot->legend->setFont(legendFont);

    QCPLegend *legend= new QCPLegend;
    legend->setParent(customPlot);
    legend->setBrush(QBrush(EconLighter));
    customPlot->axisRect(0)->insetLayout()->addElement(legend,Qt::AlignTop|Qt::AlignRight);
    legend->setLayer(QLatin1String("legend"));
    legend->setBorderPen(Qt::NoPen);
    for ( auto i= 0; i < labels.size(); ++i ) {
        PlainLegendItem *item= new PlainLegendItem(legend,bars[i]);
        item->setParent(customPlot);
        //std::string acc= labels[i].toStdString();
        //acc+= " ", acc+= ticklabels[i].toStdString();
        //item->setLayer(tr(acc.c_str()));
        legend->addItem(item);
    }

    customPlot->addGraph();

    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    QVector<QCPItemText *> items;
    //Creating and configuring an item
    double spacing= 300;
    for ( auto i= 0; i < data.size(); ++i ) {
        QCPItemText *textLabel = new QCPItemText(customPlot);
        //customPlot->addItem(textLabel);
        textLabel->setClipToAxisRect(false);
        textLabel->position->setAxes(customPlot->xAxis, customPlot->yAxis);
        textLabel->position->setType(QCPItemPosition::ptPlotCoords);
        //placing the item over the bar with a spacing of 0.25
        textLabel->position->setCoords(ticks[i],data[i]+spacing);
        //Customizing the item
        textLabel->setText(QString::number(data[i]));

        // textLabel->setFont(QFont(font().family(), 9));
        textLabel->setFont(QFont(fnt.family(), 9));
        textLabel->setPen(QPen(Qt::black));
        items.push_back(textLabel);
    }

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


    // QFont fnt("Monospace");
    // fnt.setStyleHint(QFont::TypeWriter);

    // form the title-string
    auto header= getSelectedQueryButton()->text();
    header.append(QString(tr(" queries for ")));
    std::string path_= [&]() {
        std::string x= obj["context"]["executable"];
        std::regex r("([^/]+)$");
        std::smatch m;
        if ( std::regex_search(x,m,r) ) {
            return std::regex_replace(m.str(1),std::regex(".txt$|.puu$"),""); //remove the extension
        }
        return std::string();
    }();
    header.append(QString(tr(path_.c_str())));

    // set title
    customPlot->plotLayout()->insertRow(0);
    auto textElement= new QCPTextElement(customPlot,header);
    QFont titleFont= QFont("Verdana",12);
    titleFont.setBold(true);
    textElement->setFont(titleFont);
    customPlot->plotLayout()->addElement(0,0,textElement);
    // customPlot->axisRect(0)->insetLayout()->addElement(textElement, Qt::AlignTop|Qt::AlignLeft);
    /*
    {
        int num_queries= static_cast<int>(
                strtol(numQueriesQLineEdit->text().toStdString().c_str(),nullptr,10)
        );
        char t[0x40];
        sprintf(t,"time to complete 10%s queries, sec.",num_queries==1000000?"\u2076":"\u2077");
        customPlot->plotLayout()->insertRow(1);
        QString subTitle(QString::fromUtf8(t));
        auto subtitleTextElement = new QCPTextElement(customPlot, subTitle);
        QFont subTitleFont = QFont(fnt.family(), 9);
        subTitleFont.setItalic(true);
        subtitleTextElement->setFont(subTitleFont);
        customPlot->plotLayout()->addElement(1, 0, subtitleTextElement);
    }
    */
    return customPlot;
}

void tpq_gui::plot_bars_group( QStringList filenames ) {

    QCPTextElement *titleElement= nullptr;

    auto *customPlot = new QCustomPlot;
    mainPlot= customPlot;
    customPlot->setBackground(QBrush(EconLighter));

    customPlot->axisRect(0)->removeAxis(customPlot->yAxis);
    customPlot->axisRect(0)->addAxis(QCPAxis::AxisType::atLeft,new EconomistStyleQCPAxis(customPlot->axisRect(),QCPAxis::AxisType::atLeft));
    customPlot->yAxis->setBasePen(QPen(Qt::NoPen));
    customPlot->yAxis->setTickPen(QPen(Qt::NoPen));
    customPlot->yAxis->setSubTickPen(QPen(Qt::NoPen));
    customPlot->yAxis->setLabelPadding(-15);
    //customPlot->yAxis->setLabelFont(xLabelsFont);
    customPlot->yAxis->setPadding(2); // a bit more space to the left border
    customPlot->yAxis->setNumberFormat(tr("gb"));
    // customPlot->yAxis->setTickLabelRotation(60);
    // customPlot->yAxis->setLabel("Time to complete (seconds)");
    //customPlot->yAxis->setOffset(-100);
    // customPlot->yAxis->setBasePen(QPen(Qt::black));
    // customPlot->yAxis->setTickPen(QPen(Qt::black));
    // customPlot->yAxis->setSubTickPen(QPen(Qt::black));
    customPlot->yAxis->setOffset(0);
    customPlot->yAxis->grid()->setSubGridVisible(false);
    customPlot->yAxis->grid()->setVisible(true);
    customPlot->yAxis->grid()->setPen(QPen(Shadow.lighter(227),0,Qt::SolidLine));

    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    double low= std::numeric_limits<double>::max(),
            high= std::numeric_limits<double>::min(),
            key_max= std::numeric_limits<double>::min();

    auto *group = new QCPBarsGroup(customPlot);
    QVector<Qt::BrushStyle> styles= {Qt::SolidPattern,Qt::Dense1Pattern,Qt::Dense2Pattern,Qt::Dense3Pattern};
    QVector<QString> qnames= {QString(tr("large")),QString(tr("medium")),QString(tr("small"))};
    //FIXME: this assumes the files come in correct order;
    // we need to find out on the basis of the files themselves which one contains which K
    std::vector<QCPBars *> brs(filenames.size());
    for ( auto i= 0; i < filenames.size(); ++i ) {
        int kk;
        auto *bar= prepareBars(filenames[i],customPlot,&kk);
        int idx= kk==1?0:(kk==10?1:2);
        auto qcprng= bar->dataValueRange(0);
        low= std::min(low,qcprng.lower);
        high= std::max(high,qcprng.upper);
        bar->setWidth(0.15);
        QBrush brush(EconBlue,styles[idx]);
        bar->setName(qnames[idx]);
        bar->setBrush(brush);
        //group->append(bar);
        brs[idx]= bar;
        key_max= std::max(key_max, bar->dataCount()+0.00);
        // customPlot->addGraph();
    }
    std::cerr << high << std::endl;
    customPlot->yAxis->setRangeUpper(300);
    for ( auto &br: brs )
        group->append(br);

    QVector<QString> labels;
    QVector<double> ticks;
    for ( auto i= 0; i < key_max; ++i ) {
        ticks << i+1;
        labels.push_back(QString::number(i+1));
    }

    QFont fnt("Monospace");
    fnt.setStyleHint(QFont::TypeWriter);

    if ( auto tmp= getSelectedQueryButton() ) {
        titleElement= new QCPTextElement(mainPlot);
        titleElement->setText(tmp->text());
        QFont titleFont= fnt;
        titleFont.setBold(true);
        titleElement->setFont(titleFont);
        // customPlot->axisRect(0)->insetLayout()->addElement(titleElement,Qt::AlignTop|Qt::AlignLeading);
    }

    QFont xLabelsFont= fnt;
    xLabelsFont.setPointSize(10);
    customPlot->xAxis->setLabelFont(xLabelsFont);
    customPlot->xAxis->grid()->setVisible(false);
    customPlot->xAxis->setSubTicks(false);
    customPlot->xAxis->setTickLength(0, 4);
    customPlot->xAxis->setRange(0,key_max+2);
    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->addTicks(ticks, labels);
    customPlot->xAxis->setTicker(textTicker);

    // customPlot->yAxis->setRange(0.00,high*1.13);
    customPlot->yAxis->setLabelFont(xLabelsFont);
    customPlot->legend->setVisible(true);
    customPlot->legend->setFont(xLabelsFont);

    auto header= getSelectedQueryButton()->text();
    header.append(QString(tr(" queries for ")));
    std::ifstream input(filenames[0].toStdString());
    nlohmann::json obj; input >> obj;
    std::string path_= [&]() {
        std::string x= obj["context"]["executable"];
        std::regex r("([^/]+)$");
        std::smatch m;
        if ( std::regex_search(x,m,r) ) {
            return std::regex_replace(m.str(1),std::regex(".txt$|.puu$"),""); //remove the extension
        }
        return std::string();
    }();
    header.append(QString(tr(path_.c_str())));

    // set title
    customPlot->plotLayout()->insertRow(0);
    auto textElement= new QCPTextElement(customPlot,header);
    QFont titleFont= QFont("Verdana",12);
    titleFont.setBold(true);
    textElement->setFont(titleFont);
    customPlot->plotLayout()->addElement(0,0,textElement);
    // customPlot->axisRect(0)->insetLayout()->addElement(textElement, Qt::AlignTop|Qt::AlignLeft);
    /*
    {
        int num_queries= static_cast<int>(
                strtol(numQueriesQLineEdit->text().toStdString().c_str(),nullptr,10)
        );
        char t[0x40];
        sprintf(t,"time to complete 10%s queries, sec.",num_queries==1000000?"\u2076":"\u2077");
        customPlot->plotLayout()->insertRow(1);
        QString subTitle(QString::fromUtf8(t));
        auto subtitleTextElement = new QCPTextElement(customPlot, subTitle);
        QFont subTitleFont = QFont(fnt.family(), 9);
        subTitleFont.setItalic(true);
        subtitleTextElement->setFont(subTitleFont);
        customPlot->plotLayout()->addElement(1, 0, subtitleTextElement);
    }
    */

    auto *w= new QWidget();
    auto *grid= new QGridLayout();
    grid->addWidget(customPlot, 0, 0, 2, 2);
    w->setLayout(grid);
    w->setWindowTitle(tr("Bars group"));
    w->resize(480, 320);
    w->show();
}

QCPBars *tpq_gui::prepareBars( QString filename, QCustomPlot *customPlot, int *kk ) {
    QCPTextElement *titleElement= nullptr;

    std::ifstream input(filename.toStdString());
    // first, read the incoming JSON
    nlohmann::json obj; input >> obj;
    nlohmann::json arr= obj["benchmarks"]; //FIXME: what if such a key is non-existent?

    // parse the JSON and extract what we need
    QVector<QString> labels, ticklabels;
    QVector<double> ticks;
    QVector<double> data;
    QVector<double> tops;

    for ( auto i= 0; i < arr.size(); ++i ) {
        const auto &a= arr[i];
        auto num_queries= static_cast<size_t>(a["num_queries"]);
        if ( kk ) *kk= static_cast<int>(a["K"]);
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
        ticks << i+1;
        data.push_back(static_cast<double>(a["real_time"])/num_queries/(1e+3));
        // labels << tr(dsname.c_str());
        ticklabels << tr(dsname.c_str());
        labels.push_back(QString::number(i+1));
        tops.push_back(static_cast<double>(a["real_time"])/num_queries/(1e+3));
    }

    QFont fnt("Monospace");
    fnt.setStyleHint(QFont::TypeWriter);

    auto *bars = new QCPBars(customPlot->xAxis, customPlot->yAxis);
    for ( auto i= 0; i < arr.size(); ++i ) {
        bars->addData(i+1.00,data[i]);
        bars->setParent(customPlot);
    }
    return bars;
}
