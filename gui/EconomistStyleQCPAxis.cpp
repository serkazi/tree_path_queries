//
// Created by kazi on 2019-12-18.
//

#include "EconomistStyleQCPAxis.h"

QPointF myQCPAxisPainterPrivate::getTickLabelDrawOffset(const QCPAxisPainterPrivate::TickLabelData &labelData) const {
    bool doRotation = !qFuzzyIsNull(tickLabelRotation);
    if (type == QCPAxis::atLeft && tickLabelSide == QCPAxis::lsOutside && !doRotation)
    {
        // double x = -labelData.totalBounds.width()+26;
        double x= 0;
        double y = -labelData.totalBounds.height();
        return QPointF(x, y);
    } else {
        return QCPAxisPainterPrivate::getTickLabelDrawOffset(labelData);
    }
}

myQCPAxisPainterPrivate::myQCPAxisPainterPrivate(QCustomPlot *parentPlot) : QCPAxisPainterPrivate(parentPlot) {
}

EconomistStyleQCPAxis::EconomistStyleQCPAxis(QCPAxisRect *parent, QCPAxis::AxisType type) : QCPAxis(parent, type) {
    mAxisPainter= new myQCPAxisPainterPrivate(parent->parentPlot());
}
