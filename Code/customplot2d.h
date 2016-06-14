#ifndef CUSTOMPLOT2D_H
#define CUSTOMPLOT2D_H

#define MAX_WH 1e5
#define MIN_WH 150

#include <QMenu>
#include <QToolTip>
#include <QMouseEvent>

#include "fitmrq.h"
#include "qcpsemilogaxisrect.h"
#include "helpful_functions.h"

class CustomPlot2D : public QCustomPlot
{
    Q_OBJECT

public:
    explicit CustomPlot2D(QWidget *parent=0, SpectrumData *mainData=NULL, QVector< QVector< double > > *mainFSRS=NULL, QVector<QVector<double> > *auxFSRS=NULL);
    ~CustomPlot2D();
    void ReplotCarpet();
    void ReplotCarpet(double pt, double wl);
    // Galima sujungti.

signals:
    void SendMouseLocation(int, int);

private slots:
    void MouseHover(QMouseEvent *event);
    void ContextMenuRequest(QPoint pos);
    void ChangeAxesScale();
    void ChangeGaussStatus();
    void ChangeToAux();
    void GetMouseLocation(QMouseEvent *event);

private:
    QCPColorScale *colorScale;
    QCPColorMap *cMap;
    SpectrumData *mData;
    QVector < double > LogTicks;
    QVector < double > LogProbeTime;
    QVector < QVector < double > > *mFSRS, *aFSRS;
    QVector < QString > LogTickLabels;     // Galima ir nesaugoti;
    QVector < QCPItemText * > GaussianLabels;
    QVector < QCPItemLine * > GaussianArrows;
    bool isLog, showFit, showAux;
    double sell_wl, sell_pt;

    void HideLabels();
    void RescaleLogAxes();
    void ShowGaussianFit();
    void CalculateLogTime();
    void InitializeCarpet();
    void InitializeGraphs();
    void InitializeGaussianLabels();
    void SetSpectralCuts(double *fsrs_val=NULL);
    void ShowLabels(QVector < double > *fit);
    void ShowToolTip(QPoint mPoint, double fsrs_val);
    void RescaleColorMap(double *mn_z=NULL, double *mx_z=NULL);

    // Inlines:
    inline QCPSemiLogAxis * GetAxisPointer(int row, int col, QCPAxis::AxisType type);
    inline QVector < double > *GetPTPointer();
};

#endif // CUSTOMPLOT2D_H
