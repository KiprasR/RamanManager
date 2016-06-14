#ifndef QCPSEMILOGAXIS_H
#define QCPSEMILOGAXIS_H

#include "qcustomplot.h"

class QCPSemiLogAxis : public QCPAxis
{
    Q_OBJECT

public:
    explicit QCPSemiLogAxis(QCPAxisRect *parent=0, AxisType type=QCPAxis::atBottom);
    void setLogSubTicks(bool on);
    void setLogStart(double logStart);

protected:
    double mLogStart;
    bool mLogSubTicks;
    virtual void setupTickVectors();

signals:


public slots:

};

#endif // QCPSEMILOGAXIS_H
