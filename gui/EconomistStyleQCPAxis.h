//
// Created by kazi on 2019-12-18.
//

#ifndef TREE_PATH_QUERIES_ECONOMISTSTYLEQCPAXIS_H
#define TREE_PATH_QUERIES_ECONOMISTSTYLEQCPAXIS_H
#include "qcustomplot.h"

class myQCPAxisPainterPrivate: public QCPAxisPainterPrivate {
public:
    explicit myQCPAxisPainterPrivate(QCustomPlot *parentPlot);
protected:
    QPointF getTickLabelDrawOffset(const TickLabelData &labelData) const override;
};

class EconomistStyleQCPAxis: public QCPAxis {
public:
    EconomistStyleQCPAxis( QCPAxisRect *parent, AxisType type ) ;
};


#endif //TREE_PATH_QUERIES_ECONOMISTSTYLEQCPAXIS_H
