#ifndef FITPREVIEW_H
#define FITPREVIEW_H

#include <QMenu>
#include <QDialog>
#include <QToolTip>
#include <QSettings>
#include <QMouseEvent>
#include <QFileDialog>
#include <QTableWidget>

#include "rescaleaxesdialog.h"
#include "helpful_functions.h"
#include "qcpsemilogaxisrect.h"

namespace Ui {
class FitPreview;
}

class FitPreview : public QDialog
{
    Q_OBJECT

public:
    explicit FitPreview(QWidget *parent = 0, QVector < double > *pT = NULL, QTableWidget *tBL = NULL, QSettings *mainSettings = NULL);
    ~FitPreview();

private:
    Ui::FitPreview *ui;
    QCPAxis *x00, *x01, *y0,
            *x10, *x11, *y1,
            *x20, *x21, *y2;
    double y0_high, y0_low,
           y1_high, y1_low,
           y2_high, y2_low;
    bool isLog;
    QVector < double > *ProbeTime,
                        LogProbeTime, LogTicks;
    QVector < QVector < double > > Fits;
    QVector < QString > LogTickLabels;
    QSettings *mSettings;

    void SetupSignals(QTableWidget *TablePtr);
    void SearchForMinMax();
    void CalculateLogTime();
    void RescaleLogAxes();
    void RescaleYAxes();
    void ReplotSignals();

private slots:
    void RescaleGraphs(QMouseEvent *event);
    void RescaleAxes(QMouseEvent *event);
    void MouseHover(QMouseEvent *event);
    void ContextMenuRequest(QPoint point);
    void ChangeAxesScale();
    void SaveData();
};

#endif // FITPREVIEW_H
