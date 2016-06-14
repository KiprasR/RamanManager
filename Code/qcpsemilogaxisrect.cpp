#include "qcpsemilogaxisrect.h"

QCPSemiLogAxisRect::QCPSemiLogAxisRect(QCustomPlot *parentPlot) :
    QCPAxisRect(parentPlot,false)
{
    addAxis(QCPAxis::atBottom, new QCPSemiLogAxis(this,QCPAxis::atBottom));
    addAxis(QCPAxis::atLeft,   new QCPSemiLogAxis(this,QCPAxis::atLeft));
    addAxis(QCPAxis::atTop,    new QCPSemiLogAxis(this,QCPAxis::atTop));
    addAxis(QCPAxis::atRight,  new QCPSemiLogAxis(this,QCPAxis::atRight));

    this->axis(QCPAxis::atTop)->setVisible(false);
    this->axis(QCPAxis::atRight)->setVisible(false);
    this->axis(QCPAxis::atBottom)->grid()->setVisible(true);
    this->axis(QCPAxis::atLeft)->grid()->setVisible(true);
    this->axis(QCPAxis::atBottom)->grid()->setSubGridVisible(true);
    this->axis(QCPAxis::atLeft)->grid()->setSubGridVisible(true);
    this->axis(QCPAxis::atTop)->grid()->setVisible(false);
    this->axis(QCPAxis::atRight)->grid()->setVisible(false);
    this->axis(QCPAxis::atTop)->grid()->setSubGridVisible(false);
    this->axis(QCPAxis::atRight)->grid()->setSubGridVisible(false);
    this->axis(QCPAxis::atTop)->grid()->setZeroLinePen(Qt::NoPen);
    this->axis(QCPAxis::atRight)->grid()->setZeroLinePen(Qt::NoPen);

    setRangeDragAxes(this->axis(QCPAxis::atBottom), this->axis(QCPAxis::atLeft));
    setRangeZoomAxes(this->axis(QCPAxis::atBottom), this->axis(QCPAxis::atLeft));

    setupFullAxesBox(true);

    foreach (QCPAxis *axis, axes())
    {
        axis->setLayer("axes");
        axis->grid()->setLayer("grid");
    }
}
