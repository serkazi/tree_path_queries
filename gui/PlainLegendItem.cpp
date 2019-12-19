//
// Created by kazi on 2019-12-19.
//
#include "PlainLegendItem.h"

void PlainLegendItem::draw(QCPPainter *painter) {
    //QCPPlottableLegendItem::draw(painter);
    if (!mPlottable) return;
    painter->setFont(getFont());
    painter->setPen(QPen(getTextColor()));
    QSizeF iconSize = mParentLegend->iconSize();
    QRectF textRect = painter->fontMetrics().boundingRect(0, 0, 0, iconSize.height(), Qt::TextDontClip, mPlottable->name());
    QRectF iconRect(mRect.topLeft(), iconSize);
    int textHeight = qMax(textRect.height(), iconSize.height());  // if text has smaller height than icon, center text vertically in icon height, else align tops
    painter->drawText(mRect.x()+iconSize.width()+mParentLegend->iconTextPadding(), mRect.y(), textRect.width(), textHeight, Qt::TextDontClip, mPlottable->name());
    // draw icon:
    painter->save();
    painter->setClipRect(iconRect, Qt::IntersectClip);
    //mPlottable->drawLegendIcon(painter, iconRect);
    painter->restore();
    // draw icon border:
    /*
    if (getIconBorderPen().style() != Qt::NoPen)
    {
        painter->setPen(getIconBorderPen());
        painter->setBrush(Qt::NoBrush);
        int halfPen = qCeil(painter->pen().widthF()*0.5)+1;
        painter->setClipRect(mOuterRect.adjusted(-halfPen, -halfPen, halfPen, halfPen)); // extend default clip rect so thicker pens (especially during selection) are not clipped
        painter->drawRect(iconRect);
    }
    */
}

PlainLegendItem::PlainLegendItem(QCPLegend *parent, QCPAbstractPlottable *plottable) : QCPPlottableLegendItem(parent,
                                                                                                              plottable) {}
