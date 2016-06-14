#include "dispersionplot.h"

DispersionPlot::DispersionPlot(QWidget *parent, const SpectrumData *mainData,
                               const QVector<HintCluster> *hntCluster, QMap<int, DispPoint> *mainMap,
                               const QVector<QVector<double> > *mainPPSpectra) :
    QCustomPlot(parent), mData(mainData),
    hCluster(hntCluster), mMap(mainMap),
    isLog(true), plotHints(false),
    sell_wl(mainData->Wavelength.first()), sell_pt(mainData->ProbeTime.first()),
    axisStart(-1.0), axisEnd(mData->ProbeTime.last()), mouseAtX(0),
    mPPSpectra(mainPPSpectra)
{
    CalculateLogTime();
    InitializeGraphs();
    connect(this,SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(MouseHover(QMouseEvent*)));
    connect(this,SIGNAL(mousePress(QMouseEvent*)),this,SLOT(MousePressed(QMouseEvent*)));
}

DispersionPlot::~DispersionPlot()
{

}

void DispersionPlot::SetCarpetPlottables()
{
    if(!mMap->isEmpty())
    {
        QVector < double > xs, ys;
        QMap<int, DispPoint>::iterator it;
        for (it = mMap->begin(); it != mMap->end(); ++it)
        {
            xs << mData->Wavelength.at(it.key());
            ys << (isLog ? linToLog(it.value().Time,1.0) : it.value().Time);
        }
        graph(7)->setData(xs,ys);
        emit AttemptToFit(true);
    }
    else if (!graph(7)->data()->isEmpty())
    {
        graph(7)->clearData();
    }
}

void DispersionPlot::SetKineticsPlottables(double *xPnt, double *yPnt)
{
    QVector < double > xs, ys;
    if ((xPnt!=NULL)&&(yPnt!=NULL))
    {
        xs << (*xPnt); ys << (*yPnt);
        graph(2)->setData(xs,ys);
    }
    else
    {
        int wl;
        hunt(&(mData->Wavelength),sell_wl,&wl);
        if (mMap->contains(wl))
        {
            xs << (isLog ? linToLog(mMap->value(wl).Time,1.0) : mMap->value(wl).Time);
            ys << mMap->value(wl).Amp;
            graph(2)->setData(xs,ys);
        }
    }
}

void DispersionPlot::MousePressed(QMouseEvent *event)
{
    double ax=xAxis->pixelToCoord(event->pos().x()),
           ay=yAxis->pixelToCoord(event->pos().y());
    if (ax>=mData->Wavelength.first() && ax<=mData->Wavelength.last() &&
        ay>=(isLog ? linToLog(axisStart,1.0) : axisStart) && ay<(isLog ? linToLog(axisEnd,1.0) : axisEnd))
    {
        int pt, wl;
        hunt(&(mData->Wavelength),ax,&wl);
        hunt(GetPTPointer(),ay,&pt);
        // Pasirinktu tasku piesimas desiniajame grafike:
        if (event->button()==Qt::LeftButton)
        {
            // Cia yra atliekamas tas pats skaiciavimas. Galbut imanoma kaip nors to isvengti.
            DispPoint temp;
            double k=((*mPPSpectra)[pt+1][wl]-(*mPPSpectra)[pt][wl])/(mData->ProbeTime[pt+1]-mData->ProbeTime[pt]),
                   b=(*mPPSpectra)[pt][wl]-mData->ProbeTime[pt]*k;
            temp.Time=(isLog ? logToLin(ay,1.0) : ay);
            temp.Amp=k*temp.Time+b;
            (*mMap)[wl]=temp;
            SetKineticsPlottables(&ay,&temp.Amp);
        }
        else if (event->button()==Qt::RightButton)
        {
            if(mMap->contains(wl)) mMap->remove(wl);
            graph(2)->clearData();
        }
        // Pasirinktu tasku piesimas ant kilimo:
        if((event->button()==Qt::LeftButton) || (event->button()==Qt::RightButton))
        {
            SetCarpetPlottables();
            replot();
        }
    }
}

void DispersionPlot::CalculateLogTime()
{
    for(int i=0;i<mData->ProbeTime.size();i++)
        LogProbeTime << linToLog(mData->ProbeTime.at(i),1.0);
    double logTemp=1.0, epsChecked;
    double logC=ceil(ceil(LogProbeTime.last())+EXTRA_LOG_TICKS_D*ceil(logTemp)),
           logF=floor(floor((LogProbeTime.first()-EXTRA_LOG_TICKS_D*ceil(logTemp)))/logTemp)*logTemp;
    for (double i=logF;i<=logC;i+=logTemp)
    {
        epsChecked=(fabs(i)>EPS_D ? i : 0.0);
        LogTicks << epsChecked;
        LogTickLabels << (epsChecked < logTemp ? QString::number(epsChecked) : QString::number(logToLin(epsChecked,logTemp)));
    }
}

void DispersionPlot::MouseHover(QMouseEvent *event)
{
    double ax=xAxis->pixelToCoord(event->pos().x()),
           ay=yAxis->pixelToCoord(event->pos().y());
    if (ax>=mData->Wavelength.first() && ax<=mData->Wavelength.last() &&
        ay>=(isLog ? linToLog(axisStart,1.0) : axisStart) && ay<(isLog ? linToLog(axisEnd,1.0) : axisEnd))
    {
        // Taip nelieka nesklandumu del mappinimo:
        // Teoriskai galima sujungti sell_wl, sell_pt ir mouseAt, laikant ji atmintyje kaip QPoint;
        if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)==true)
        {
            QPoint p=event->globalPos();
            p.setX(mouseAtX);
            cursor().setPos(p);
        }
        else
        {
            mouseAtX=event->globalPos().x();
        }

        double val;
        sell_wl=ax; sell_pt=(isLog ?  logToLin(ay,1.0) : ay);
        SetSpectralCuts(&val);
        QToolTip::showText(mapToGlobal(event->pos()),  QString::number(sell_wl,'f',0) + " nm\n"
                                                     + QString::number(sell_pt,'f',2) + " ps\n"
                                                     + QString::number(val,'f',2) + " mOD");
        replot();
    }
}

void DispersionPlot::SetSpectralCuts(double *fsrs_val)
{
    int wl,pt;
    hunt(&mData->Wavelength,sell_wl,&wl);
    hunt(&mData->ProbeTime,sell_pt,&pt);
    QVector < double > specCut(LogProbeTime.size());
    for(int i=0;i<LogProbeTime.size();i++)
        specCut[i]=(*mPPSpectra).at(i).at(wl);

    double mmn, mmx;
    QCPSemiLogAxis *yAx=GetAxisPointer(0,1,QCPAxis::atBottom);
    below_min_and_above_max_proportional(&specCut,&mmn,&mmx,3.0);
    graph(0)->setData((*GetPTPointer()),specCut); // Kinetika
    yAx->setRange(mmn,mmx);
    if (pt==mData->ProbeTime.size()-1) pt--;
    double k=(specCut[pt+1]-specCut[pt])/(mData->ProbeTime[pt+1]-mData->ProbeTime[pt]),
           b=specCut[pt]-mData->ProbeTime[pt]*k;
    QVector <double > xs(1), ys(1);
    xs[0]=(isLog ? linToLog(sell_pt,1.0) : sell_pt);
    ys[0]=k*sell_pt+b;
    graph(1)->setData(xs,ys);
    if (mMap->contains(wl))
    {
        xs[0]=(isLog ? linToLog((*mMap)[wl].Time,1.0) : (*mMap)[wl].Time);
        ys[0]=(*mMap)[wl].Amp;
        graph(2)->setData(xs,ys);
    }
    else
    {
        graph(2)->clearData();
    }
    ShowHints(&wl);
    if (fsrs_val!=NULL) (*fsrs_val)=(*mPPSpectra).at(pt).at(wl);
}

void DispersionPlot::InitializeGraphs()
{
    plotLayout()->clear();

    QCPAxisRect *sub00 = new QCPSemiLogAxisRect(this);
    QCPAxisRect *sub01 = new QCPSemiLogAxisRect(this);

    plotLayout()->addElement(0, 0, sub00);
    plotLayout()->addElement(0, 1, sub01);

    sub01->setMaximumSize(MIN_WH_D, MAX_WH_D);

    xAxis  = sub00->axis(QCPAxis::atBottom);
    xAxis2 = sub00->axis(QCPAxis::atTop);
    yAxis  = sub00->axis(QCPAxis::atLeft);
    yAxis2 = sub00->axis(QCPAxis::atRight);

    sub01->axis(QCPAxis::atBottom)->setAutoTickCount(2);
    sub01->axis(QCPAxis::atTop)->setAutoTickCount(2);
    sub01->axis(QCPAxis::atBottom)->setRangeReversed(true);
    sub01->axis(QCPAxis::atTop)->setRangeReversed(true);
    sub01->axis(QCPAxis::atLeft)->setTickLabels(false);
    sub01->axis(QCPAxis::atRight)->setTickLabels(true);

    // Signalai atjungti, nes SetupFullAxesBox(true) juos sukuria (turbut?).
    //connect(sub01->axis(QCPAxis::atBottom),SIGNAL(rangeChanged(QCPRange)),sub01->axis(QCPAxis::atTop),SLOT(setRange(QCPRange)));
    //connect(yAxis,SIGNAL(rangeChanged(QCPRange)),yAxis2,SLOT(setRange(QCPRange)));
    //connect(yAxis,SIGNAL(rangeChanged(QCPRange)),sub01->axis(QCPAxis::atLeft),SLOT(setRange(QCPRange)));
    //connect(yAxis,SIGNAL(rangeChanged(QCPRange)),sub01->axis(QCPAxis::atRight),SLOT(setRange(QCPRange)));

    // Taip viskas susitvarko, kai nebera kvieciama rescaleAxes() funkcija funkcijoje RescaleColorMap():
    connect(yAxis,SIGNAL(rangeChanged(QCPRange)),sub01->axis(QCPAxis::atLeft),SLOT(setRange(QCPRange)));

    QCPMarginGroup *marginGroup = new QCPMarginGroup(this);
    sub00->setMarginGroup(QCP::msTop | QCP::msLeft, marginGroup);
    sub01->setMarginGroup(QCP::msTop | QCP::msRight, marginGroup);

    setBackground(QColor(240,240,240));
    foreach (QCPAxisRect *rect, axisRects())
    {
      rect->setLayer("background");
      rect->setBackground(Qt::white);
      foreach (QCPAxis *axis, rect->axes())
      {
        axis->setLayer("axes");
        axis->grid()->setLayer("grid");
      }
    }

    InitializeCarpet();

    QPen pen;
    pen.setWidth(3);
    pen.setColor(Qt::blue);

    // 0 - Kinetics
    addGraph(sub01->axis(QCPAxis::atRight), sub01->axis(QCPAxis::atBottom));
    graph(0)->setPen(pen);

    // 1 - Current selection on the kinetics curve
    addGraph(sub01->axis(QCPAxis::atRight), sub01->axis(QCPAxis::atBottom));
    pen.setColor(Qt::red);
    graph(1)->setLineStyle((QCPGraph::LineStyle)0);
    graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc,12));
    graph(1)->setPen(pen);

    // 2 - Saved selection on the kinetics curve
    addGraph(sub01->axis(QCPAxis::atRight), sub01->axis(QCPAxis::atBottom));
    pen.setColor(Qt::gray);
    graph(2)->setLineStyle((QCPGraph::LineStyle)0);
    graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross,12));
    graph(2)->setPen(pen);

    // 3 - Max
    addGraph(sub01->axis(QCPAxis::atRight), sub01->axis(QCPAxis::atBottom));
    pen.setColor(Qt::green);
    graph(3)->setLineStyle((QCPGraph::LineStyle)0);
    graph(3)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangle,12));
    graph(3)->setPen(pen);

    // 4 - Max/2
    addGraph(sub01->axis(QCPAxis::atRight), sub01->axis(QCPAxis::atBottom));
    pen.setColor(Qt::green);
    graph(4)->setLineStyle((QCPGraph::LineStyle)0);
    graph(4)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangle,6));
    graph(4)->setPen(pen);

    // 5 - Min
    addGraph(sub01->axis(QCPAxis::atRight), sub01->axis(QCPAxis::atBottom));
    pen.setColor(Qt::cyan);
    graph(5)->setLineStyle((QCPGraph::LineStyle)0);
    graph(5)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangleInverted,12));
    graph(5)->setPen(pen);

    // 6 - Min/2
    addGraph(sub01->axis(QCPAxis::atRight), sub01->axis(QCPAxis::atBottom));
    pen.setColor(Qt::cyan);
    graph(6)->setLineStyle((QCPGraph::LineStyle)0);
    graph(6)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssTriangleInverted,6));
    graph(6)->setPen(pen);

    // 7 - Selection on the carpet
    addGraph(xAxis, yAxis);
    pen.setColor(QColor(0,0,204,192));
    pen.setWidth(0);
    graph(7)->setPen(pen);
    graph(7)->setLineStyle((QCPGraph::LineStyle)0);
    graph(7)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc,10));
    graph(7)->setLayer("main");

    // 8 - Poly
    addGraph(xAxis, yAxis);
    pen.setColor(QColor(0,0,204,192));
    pen.setWidth(4);
    graph(8)->setPen(pen);
    graph(8)->setLayer("main");

    // 9 - Hints on the carpet
    addGraph(xAxis, yAxis);
    pen.setColor(QColor(204,0,0,128));
    pen.setWidth(4);
    graph(9)->setLineStyle((QCPGraph::LineStyle)0);
    graph(9)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCross,8));
    graph(9)->setPen(pen);
    graph(9)->setLayer("main");

    // 10 - Hint poly
    addGraph(xAxis, yAxis);
    pen.setColor(QColor(204,0,0,128));
    pen.setWidth(4);
    graph(10)->setPen(pen);
    graph(10)->setLayer("main");

    RescaleLogAxes();
}

void DispersionPlot::RescaleLogAxes()
{
    QCPSemiLogAxis *y00 = GetAxisPointer(0,0,QCPAxis::atLeft),
                   *y01 = GetAxisPointer(0,0,QCPAxis::atRight),
                   *y10 = GetAxisPointer(0,1,QCPAxis::atRight),
                   *y11 = GetAxisPointer(0,1,QCPAxis::atLeft);
    if (isLog)
    {
        y00->setAutoTicks(false);
        y00->setAutoTickLabels(false);
        y00->setTickVector(LogTicks);
        y00->setTickVectorLabels(LogTickLabels);

        y01->setAutoTicks(false);
        y01->setAutoTickLabels(false);
        y01->setTickVector(LogTicks);

        y10->setAutoTicks(false);
        y10->setAutoTickLabels(false);
        y10->setTickVector(LogTicks);
        y10->setTickVectorLabels(LogTickLabels);

        y11->setAutoTicks(false);
        y11->setAutoTickLabels(false);
        y11->setTickVector(LogTicks);

        // Log subticks:
        y00->setAutoSubTicks(false);
        y00->setLogSubTicks(true);

        y01->setAutoSubTicks(false);
        y01->setLogSubTicks(true);

        y10->setAutoSubTicks(false);
        y10->setLogSubTicks(true);

        y11->setAutoSubTicks(false);
        y11->setLogSubTicks(true);

        // LogStart:
        y00->setLogStart(1.0);
        y01->setLogStart(1.0);
        y10->setLogStart(1.0);
        y11->setLogStart(1.0);
    }
    else
    {
        y00->setAutoTicks(true);
        y00->setAutoTickLabels(true);
        y00->setAutoSubTicks(true);

        y01->setAutoTicks(true);
        y01->setAutoTickLabels(true);
        y01->setAutoSubTicks(true);

        y10->setAutoTicks(true);
        y10->setAutoTickLabels(true);
        y10->setAutoSubTicks(true);

        y11->setAutoTicks(true);
        y11->setAutoTickLabels(true);
        y11->setAutoSubTicks(true);

        // Log subticks:
        y00->setLogSubTicks(false);
        y01->setLogSubTicks(false);
        y10->setLogSubTicks(false);
        y11->setLogSubTicks(false);
    }

    /* Niekas nebera nustatoma, nes viskas yra sutvakoma per signalus! */

    //y00->setRange((isLog ? linToLog(axisStart,1.0) : axisStart), (isLog ? linToLog(axisEnd,1.0) : axisEnd));
    //y01->setRange(y00->range());
    //y10->setRange(y00->range());
    //y11->setRange(y00->range());

    // Reikia nustatyti y11'a, nes jis yra tikroji yAxis (y10 yra yAxis2)!
    //y00->setRange((isLog ? linToLog(axisStart,1.0) : axisStart), (isLog ? linToLog(axisEnd,1.0) : axisEnd));
    //y11->setRange(y00->range());
}

void DispersionPlot::InitializeCarpet()
{
    double mn_z, mx_z;
    cMap = new QCPColorMap(xAxis, yAxis);
    addPlottable(cMap);

    RescaleColorMap(&mn_z,&mx_z);

    QCPColorScale *colorScale = new QCPColorScale(this);
    colorScale->setDataRange(QCPRange(mn_z,mx_z));
    cMap->setColorScale(colorScale); // associate the color map with the color scale

    //QCPColorGradient gr(QCPColorGradient::gpSpectrum);
    //gr.setLevelCount(32);
    //colorMap->setGradient(gr);
    cMap->setGradient(QCPColorGradient::gpSpectrum);
    cMap->setInterpolate(false);

    // rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
    //colorMap->rescaleDataRange();
    // rescale the key (x) and value (y) axes so the whole color map is visible:
    //rescaleAxes();
}

void DispersionPlot::ReceiveAxisScale(bool axLog, double axStart, double axEnd)
{
    isLog=axLog;
    axisStart=axStart;
    axisEnd=axEnd;
    RescaleColorMap();
    SetSpectralCuts();
    SetCarpetPlottables();
    SetKineticsPlottables();
    RescaleLogAxes();
    replot();
}

void DispersionPlot::RescaleColorMap(double *mn_z, double *mx_z)
{
    const QVector < double > *timePTR=GetPTPointer();
    int n1, n2, ny, nx = mData->Wavelength.size();
    hunt(&mData->ProbeTime,axisStart,&n1);
    hunt(&mData->ProbeTime,axisEnd,&n2);
    ny=n2-n1;

    double x, y, z;
    int pt, wl;

    if ((mn_z!=NULL)&&(mx_z!=NULL))
    {
        (*mn_z)= 1e6;
        (*mx_z)=-1e6;
    }

    cMap->data()->clear();
    cMap->data()->setSize(nx, ny);
    cMap->data()->setRange(QCPRange(mData->Wavelength.first(),mData->Wavelength.last()),
                           QCPRange((isLog ? linToLog(axisStart,1.0) : axisStart), (isLog ? linToLog(axisEnd,1.0) : axisEnd)));
    for (int xIndex=0; xIndex<nx; ++xIndex)
    {
      for (int yIndex=0; yIndex<ny; ++yIndex)
      {
        cMap->data()->cellToCoord(xIndex, yIndex, &x, &y);
        hunt(timePTR,y,&pt);
        hunt(&mData->Wavelength,x,&wl);
        z=(*mPPSpectra).at(pt).at(wl);
        if (mn_z != NULL)
        {
            if (z>=(*mx_z)) (*mx_z)=z;
            if (z<=(*mn_z)) (*mn_z)=z;
        }
        cMap->data()->setCell(xIndex, yIndex, z);
      }
    }
    cMap->rescaleDataRange();

    //rescaleAxes();
    // Tokiu budu rescalinant colormapa rescalinasi ir visa kita:
    xAxis->setRange(QCPRange(mData->Wavelength.first(),mData->Wavelength.last()));
    yAxis->setRange(QCPRange((isLog ? linToLog(axisStart,1.0) : axisStart), (isLog ? linToLog(axisEnd,1.0) : axisEnd)));
}

inline const QVector < double > * DispersionPlot::GetPTPointer()
{
    if (isLog) return &(LogProbeTime);
    else return &(mData->ProbeTime);
}

inline QCPSemiLogAxis * DispersionPlot::GetAxisPointer(int row, int col, QCPAxis::AxisType type)
{
    QCPAxisRect *r = static_cast<QCPAxisRect*>(plotLayout()->element(row,col));
    return static_cast<QCPSemiLogAxis*>(r->axis(type));
}

void DispersionPlot::ReplotCarpet()
{
    RescaleColorMap();
    SetSpectralCuts();
    RescaleLogAxes();
    replot();
}

void DispersionPlot::SetHints(bool show)
{
    plotHints=show;
    ShowHints();
}

void DispersionPlot::ShowHints(int *wl_ptr)
{
    if (plotHints)
    {
        int wl;
        if (wl_ptr!=NULL) wl=(*wl_ptr);
        else hunt(&mData->Wavelength,sell_wl,&wl);
        QVector < double > xs(1), ys(1);
        xs[0]=(isLog ? linToLog((*hCluster)[wl].Max.Time,1.0) : (*hCluster)[wl].Max.Time);
        ys[0]=(*hCluster)[wl].Max.Amp;
        graph(3)->setData(xs,ys);
        xs[0]=(isLog ? linToLog((*hCluster)[wl].MaxHalf.Time,1.0) : (*hCluster)[wl].MaxHalf.Time);
        ys[0]=(*hCluster)[wl].MaxHalf.Amp;
        graph(4)->setData(xs,ys);
        xs[0]=(isLog ? linToLog((*hCluster)[wl].Min.Time,1.0) : (*hCluster)[wl].Min.Time);
        ys[0]=(*hCluster)[wl].Min.Amp;
        graph(5)->setData(xs,ys);
        xs[0]=(isLog ? linToLog((*hCluster)[wl].MinHalf.Time,1.0) : (*hCluster)[wl].MinHalf.Time);
        ys[0]=(*hCluster)[wl].MinHalf.Amp;
        graph(6)->setData(xs,ys);
    }
    else
    {
        for (int i=3;i<=6;i++) graph(i)->clearData();
    }
}
