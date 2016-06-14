#ifndef DISPERSIONPLOT_H
#define DISPERSIONPLOT_H

#define MAX_WH_D 1e5
#define MIN_WH_D 200
#define EPS_D 1.0e-8
#define EXTRA_LOG_TICKS_D 2

#include <QMap>
#include <QToolTip>
#include <QMouseEvent>
#include <QApplication>
#include "helpful_functions.h"
#include "qcpsemilogaxisrect.h"

struct DispPoint
{
    double Time;
    double Amp;
};

struct HintCluster
{
    DispPoint Max;
    DispPoint MaxHalf;
    DispPoint Min;
    DispPoint MinHalf;
};

class DispersionPlot : public QCustomPlot
{
    Q_OBJECT

public:
    explicit DispersionPlot(QWidget *parent=0, const SpectrumData *mainData=NULL,
                            const QVector < HintCluster > *hntCluster=NULL, QMap<int, DispPoint> *mainMap=NULL,
                            const QVector< QVector< double> > *mainPPSpectra=NULL);
    ~DispersionPlot();
    void ReplotCarpet();
    void SetHints(bool show);

signals:
    void AttemptToFit(bool selPoly);

private slots:
    void MouseHover(QMouseEvent *event);
    void MousePressed(QMouseEvent *event);
    void ReceiveAxisScale(bool axLog, double axStart, double axEnd);

private:
    QCPColorMap *cMap;
    const SpectrumData *mData;
    const QVector < HintCluster > *hCluster;
    QMap<int, DispPoint> *mMap;
    QVector < double > LogTicks;
    QVector < QString > LogTickLabels;
    bool isLog, plotHints;
    double sell_wl, sell_pt;
    double axisStart, axisEnd;
    int mouseAtX;
    const QVector < QVector < double > > *mPPSpectra;
    QVector < double > LogProbeTime;

    void RescaleLogAxes();
    void CalculateLogTime();
    void InitializeCarpet();
    void InitializeGraphs();
    void SetCarpetPlottables();
    void ShowHints(int *wl_ptr=NULL);
    void SetSpectralCuts(double *fsrs_val=NULL);
    void RescaleColorMap(double *mn_z=NULL, double *mx_z=NULL);
    void SetKineticsPlottables(double *xPnt=NULL, double *yPnt=NULL);

    // Inlines:
    inline QCPSemiLogAxis * GetAxisPointer(int row, int col, QCPAxis::AxisType type);
    inline const QVector < double > *GetPTPointer();
};

#endif // DISPERSIONPLOT_H
