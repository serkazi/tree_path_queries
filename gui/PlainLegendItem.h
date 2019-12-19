//
// Created by kazi on 2019-12-19.
//

#ifndef TREE_PATH_QUERIES_PLAINLEGENDITEM_H
#define TREE_PATH_QUERIES_PLAINLEGENDITEM_H
#include "qcustomplot.h"


class PlainLegendItem: public QCPPlottableLegendItem {
protected:
    void draw(QCPPainter *painter) override;
public:
    PlainLegendItem(QCPLegend *parent, QCPAbstractPlottable *plottable= nullptr);
};


#endif //TREE_PATH_QUERIES_PLAINLEGENDITEM_H
