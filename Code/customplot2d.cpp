#include "customplot2d.h"

CustomPlot2D::CustomPlot2D(QWidget *parent, SpectrumData *mainData, QVector< QVector<double> > *mainFSRS, QVector < QVector <double> > *auxFSRS) :
    QCustomPlot(parent),
    mData(mainData),
    mFSRS(mainFSRS),
    aFSRS(auxFSRS),
    isLog(true), showFit(false), showAux(false),
    sell_wl(mainData->Wavelength.first()), sell_pt(mainData->ProbeTime.first())
{
    CalculateLogTime();
    InitializeGraphs();
    InitializeGaussianLabels();
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(ContextMenuRequest(QPoint)));
    connect(this,SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(MouseHover(QMouseEvent*)));
    connect(this,SIGNAL(mouseDoubleClick(QMouseEvent*)),this,SLOT(GetMouseLocation(QMouseEvent*)));
}

CustomPlot2D::~CustomPlot2D()
{
    // Panaikinti visus Gaussian...?
}

void CustomPlot2D::GetMouseLocation(QMouseEvent *event)
{
    QVector < double > *PTR=GetPTPointer();
    double ax=xAxis->pixelToCoord(event->pos().x()),
           ay=yAxis->pixelToCoord(event->pos().y());
    if (ax>=mData->Wavelength.first() && ax<=mData->Wavelength.last() && ay>=(isLog ? -1 : PTR->first()) && ay<=PTR->last())
    {
        int pt, wl;
        hunt(&(mData->Wavelength),ax,&wl);
        hunt(PTR,ay,&pt);
        emit SendMouseLocation(pt,wl);
    }
}

void CustomPlot2D::CalculateLogTime()
{
    int i;
    for (i=0;i<mData->ProbeTime.size(); i++)
        LogProbeTime << linToLog(mData->ProbeTime[i],1.0);
    int logF=int(floor(LogProbeTime.last()));
    for (i=-1;i<=logF;i++)
    {
        LogTicks << double(i);
        LogTickLabels << (i < 1 ? QString::number(i) : QString::number(int(pow(10,i-1))));
    }
}

void CustomPlot2D::InitializeGaussianLabels()
{
    QColor whAlpha(Qt::white);
    whAlpha.setAlpha(175);

    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(whAlpha);

    QPen pen, curvePen;
    pen.setColor(whAlpha);

    curvePen.setWidth(1);
    curvePen.setColor(Qt::red);

    QCPAxisRect *r01 = static_cast<QCPAxisRect*>(plotLayout()->element(0,1));

    for (int i=0;i<3;i++)
    {
        if (i!=2)
        {
            GaussianLabels.push_back( new QCPItemText(this) );
            GaussianLabels[i]->position->setType(QCPItemPosition::ptPlotCoords);
            GaussianLabels[i]->setClipToAxisRect(true);
            GaussianLabels[i]->setClipAxisRect(r01);
            GaussianLabels[i]->position->setAxes(GetAxisPointer(0,1,QCPAxis::atRight),GetAxisPointer(0,1,QCPAxis::atBottom));
            GaussianLabels[i]->setPen(pen);
            GaussianLabels[i]->setBrush(brush);
            GaussianLabels[i]->setFont(QFont(font().family(), 10, QFont::Bold));
            GaussianLabels[i]->setColor(Qt::red);
            GaussianLabels[i]->setVisible(false);
            addItem(GaussianLabels[i]);
        }
        GaussianArrows.push_back( new QCPItemLine(this) );
        GaussianArrows[i]->setClipToAxisRect(true);
        GaussianArrows[i]->setClipAxisRect(r01);
        GaussianArrows[i]->start->setType(QCPItemPosition::ptPlotCoords);
        GaussianArrows[i]->start->setAxes(GetAxisPointer(0,1,QCPAxis::atRight),GetAxisPointer(0,1,QCPAxis::atBottom));
        GaussianArrows[i]->end->setType(QCPItemPosition::ptPlotCoords);
        GaussianArrows[i]->end->setAxes(GetAxisPointer(0,1,QCPAxis::atRight),GetAxisPointer(0,1,QCPAxis::atBottom));
        GaussianArrows[i]->setPen(curvePen);
        GaussianArrows[i]->setHead(QCPLineEnding::esSpikeArrow);
        GaussianArrows[i]->setVisible(false);
        addItem(GaussianArrows[i]);
    }
}

void CustomPlot2D::ContextMenuRequest(QPoint pos)
{
    // Deja, nieko protingesnio nesugalvoju...
    sell_wl=xAxis->pixelToCoord(pos.x());
    sell_pt=yAxis->pixelToCoord(pos.y());
    if (isLog) sell_pt=logToLin(sell_pt,1.0);

    QMenu *menu = new QMenu(this);
    QString AxisStr=(isLog   ? "Linear temporal axis" : "Semi-log temporal axis"),
            FitStr =(showFit ? "Hide Gaussian fit" : "Show Gaussian fit");

    menu->addAction(AxisStr,this,SLOT(ChangeAxesScale()));
    menu->addAction(FitStr,this,SLOT(ChangeGaussStatus()));
    menu->setAttribute(Qt::WA_DeleteOnClose);
    if (aFSRS!=NULL)
    {
        QString AuxStr=(showAux ? "Hide Spectral fit" : "Show Spectral fit");
        menu->addAction(AuxStr,this,SLOT(ChangeToAux()));
    }
    menu->popup(mapToGlobal(pos));
}

void CustomPlot2D::ChangeToAux()
{
    showAux=!showAux;
    isLog=!isLog;

    QPen pen;
    pen.setWidth(3);
    pen.setColor(Qt::blue);
    pen.setStyle(Qt::SolidLine);
    if(showAux)
    {
        graph(3)->setPen(pen);
        graph(4)->setPen(pen);

        pen.setStyle(Qt::NoPen);
        graph(0)->setPen(pen);
        graph(1)->setPen(pen);
        graph(1)->setBrush(QBrush(QColor(0, 0, 255, 40)));
        graph(0)->setBrush(QBrush(QColor(0, 0, 255, 40)));
    }
    else
    {
        graph(0)->setPen(pen);
        graph(1)->setPen(pen);
        graph(0)->setBrush(Qt::NoBrush);
        graph(1)->setBrush(Qt::NoBrush);

        pen.setStyle(Qt::DashLine);
        pen.setColor(Qt::gray);
        graph(3)->setPen(pen);
        graph(4)->setPen(pen);
    }

    ChangeAxesScale();
}

void CustomPlot2D::MouseHover(QMouseEvent *event)
{
    QVector < double > *PTR=GetPTPointer();
    double ax=xAxis->pixelToCoord(event->pos().x()),
           ay=yAxis->pixelToCoord(event->pos().y());
    if (ax>=mData->Wavelength.first() && ax<=mData->Wavelength.last() && ay>=(isLog ? -1 : PTR->first()) && ay<=PTR->last())
    {
        /***************************************************/
        sell_wl=ax; sell_pt=(isLog ?  logToLin(ay,1.0) : ay);
        ShowGaussianFit();
        /***************************************************/
        double val;
        SetSpectralCuts(&val);
        ShowToolTip(mapToGlobal(event->pos()),val);
        replot();
    }
}

void CustomPlot2D::ShowToolTip(QPoint mPoint, double fsrs_val)
{
    // Nes reikia pozicijos, gaunamos is evento, ir mOD, kuri suskaiciuojama tik SetSpectralCuts() metu.
    QToolTip::showText(mPoint,   QString::number(sell_wl,'f',0) + QString::fromUtf8(" cm⁻¹;\n")
                               + QString::number(sell_pt,'f',2) + " ps\n"
                               + QString::number(fsrs_val,'f',2) + " mOD");
}

void CustomPlot2D::ShowGaussianFit()
{
    if (showFit)
    {
        int i,wl,mni,mxi;
        QVector < double > ptTrace(mData->ProbeTime.size()), ssig(mData->ProbeTime.size()), Fit(mData->ProbeTime.size()) ;

        // Skaiciuojama du kartus - kiek neefektyvu, bet bus griozdas, jeigu siuntinesiu rodykles ir tikrinsiu NULL'us.
        hunt(&(mData->Wavelength),sell_wl,&wl);
        for(i=0;i<mData->ProbeTime.size();i++)
        {
            ptTrace[i]=(*mFSRS)[i][wl];
            ssig[i]=1.0;
        }
        double mmn=min_index(&ptTrace,&mni), mmx=max_index(&ptTrace,&mxi);

        if (fabs(mmx-mmn)>1e-10)
        {
            QVector <double> coeffs;
            coeffs << (fabs(mmx)>fabs(mmn) ?  mmn : mmx)
                   << (mmx-mmn)*(fabs(mmx)>fabs(mmn) ? 1.0 : -1.0)
                   << (fabs(mmx)>fabs(mmn) ? mData->ProbeTime[mxi] : mData->ProbeTime[mni])
                   << 1.5;

            Fitmrq myMRQ(mData->ProbeTime,ptTrace,ssig,coeffs,fgauss2,1.0e-4);
            myMRQ.fit();
            myMRQ.a[3]=fabs(myMRQ.a[3]); // Kad laikas nebutu neigiamas.
            for (i=0;i<LogProbeTime.size();i++)
                Fit[i] = myMRQ.a[0]+myMRQ.a[1]*exp(-FWHM*SQR((mData->ProbeTime[i]-myMRQ.a[2])/myMRQ.a[3]));
            graph(2)->setData(*GetPTPointer(),Fit);
            ShowLabels(&myMRQ.a);
        }
        else
        {
            HideLabels();
        }
    }
    else
    {
        HideLabels();
    }
}

void CustomPlot2D::ShowLabels(QVector<double> *fit)
{
    double ptx= 0.5*(*fit)[3]+(*fit)[2],
           mtx=-0.5*(*fit)[3]+(*fit)[2],
           yfwhm=(*fit)[0]+(*fit)[1]/2,
           dx=(mData->ProbeTime.last()-mData->ProbeTime.first() > 10.0 ? 1.25 : 0.5),
           ddx=0.1;

    if((*fit)[1] >= 0.0)
    {
        GaussianLabels[0]->setPositionAlignment(Qt::AlignVCenter|Qt::AlignLeft);
        GaussianLabels[1]->setPositionAlignment(Qt::AlignVCenter|Qt::AlignRight);
        GaussianLabels[0]->setRotation(270);
    }
    else
    {
        GaussianLabels[0]->setPositionAlignment(Qt::AlignVCenter|Qt::AlignRight);
        GaussianLabels[1]->setPositionAlignment(Qt::AlignVCenter|Qt::AlignLeft);
        GaussianLabels[0]->setRotation(90);
    }

    GaussianLabels[0]->setText(QString::number((*fit)[3],'f',2));
    GaussianLabels[0]->position->setCoords((isLog ? linToLog(ptx+dx+ddx,1.0) : ptx+dx+ddx), yfwhm);

    GaussianLabels[1]->setText(QString::number((*fit)[2],'f',2));
    GaussianLabels[1]->position->setCoords((isLog ? linToLog((*fit)[2],1.0) : (*fit)[2]), (*fit)[0]+(*fit)[1]/4.0);

    GaussianArrows[0]->start->setCoords((isLog ? linToLog(mtx-dx,1.0) : mtx-dx),yfwhm);
    GaussianArrows[0]->end->setCoords((isLog ? linToLog(mtx-ddx,1.0) : mtx-ddx),yfwhm);

    GaussianArrows[1]->start->setCoords((isLog ? linToLog(ptx+dx,1.0) : ptx+dx),yfwhm);
    GaussianArrows[1]->end->setCoords((isLog ? linToLog(ptx+ddx,1.0) : ptx+ddx),yfwhm);

    GaussianArrows[2]->start->setCoords((isLog ? linToLog((*fit)[2],1.0) : (*fit)[2]), (*fit)[0]+(*fit)[1]/4.5);
    GaussianArrows[2]->end->setCoords((isLog ? linToLog((*fit)[2],1.0) : (*fit)[2]), 0.0);

    if (!GaussianLabels[0]->visible())
    {
        int i;
        for (i=0;i<2;i++) GaussianLabels[i]->setVisible(true);
        for (i=0;i<3;i++) GaussianArrows[i]->setVisible(true);
    }
}

void CustomPlot2D::HideLabels()
{
    // Kad kiekviena karta nekeistu.
    if (!graph(2)->data()->isEmpty())
    {
        int i;
        graph(2)->clearData();
        for (i=0;i<2;i++) GaussianLabels[i]->setVisible(false);
        for (i=0;i<3;i++) GaussianArrows[i]->setVisible(false);
    }
}

void CustomPlot2D::ChangeGaussStatus()
{
    showFit=!showFit;
}

void CustomPlot2D::SetSpectralCuts(double *fsrs_val)
{
    int wl,pt;
    QVector < double > *pt_PTR=GetPTPointer();
    QCPSemiLogAxis *yAx1=GetAxisPointer(0,1,QCPAxis::atBottom),
                   *yAx2=GetAxisPointer(1,0,QCPAxis::atLeft);

    hunt(&mData->Wavelength,sell_wl,&wl);
    hunt(&mData->ProbeTime,sell_pt,&pt);

    QVector < double > specCut(LogProbeTime.size()), auxSpecCut(LogProbeTime.size());
    for(int i=0;i<LogProbeTime.size();i++)
    {
        specCut[i]=(*mFSRS)[i][wl];
        if (aFSRS!=NULL) auxSpecCut[i]=(*aFSRS)[i][wl];
    }

    // Laikinai idetas normavimas!!!
    if (aFSRS!=NULL)
    {
        double max_main=max(&specCut),max_aux=max(&auxSpecCut);
        if (fabs(max_main)<1e-15) max_main=1e-15;
        if (fabs(max_aux)<1e-15) max_aux=1e-15;
        for(int i=0;i<LogProbeTime.size();i++)
            auxSpecCut[i]*=(max_main/max_aux);
    }

    double mmn, mmx;
    below_min_and_above_max_proportional(&specCut,&mmn,&mmx,2.5);
    graph(0)->setData((*pt_PTR),specCut);
    yAx1->setRange(mmn,mmx);
    //graph(0)->rescaleValueAxis();

    below_min_and_above_max_proportional(&((*mFSRS)[pt]),&mmn,&mmx,2.5);
    graph(1)->setData(mData->Wavelength,(*mFSRS)[pt]);
    yAx2->setRange(mmn,mmx);
    //graph(1)->rescaleValueAxis();

    if (aFSRS!=NULL)
    {
        double mn, mx;
        min_and_max(&(specCut),&mn,&mx);
        if((mx-mn)>1.0e-14) graph(3)->setData((*pt_PTR),auxSpecCut);
        else graph(3)->clearData();

        min_and_max(&((*mFSRS)[pt]),&mn,&mx);
        if((mx-mn)>1.0e-14) graph(4)->setData(mData->Wavelength,(*aFSRS)[pt]);
        else graph(4)->clearData();

        if(!graph(3)->visible())
        {
            graph(3)->setVisible(true);
            graph(4)->setVisible(true);
        }
    }
    else
    {
        if (graph(3)->visible())
        {
            graph(3)->setVisible(false);
            graph(4)->setVisible(false);
        }
    }

    // Kiek durnai, bet taip bus paprasciausia:
    if (fsrs_val!=NULL)
        (*fsrs_val)=(*mFSRS)[pt][wl];
}

void CustomPlot2D::InitializeGraphs()
{
    plotLayout()->clear();

    QCPAxisRect *sub00 = new QCPSemiLogAxisRect(this);
    QCPAxisRect *sub01 = new QCPSemiLogAxisRect(this);
    QCPAxisRect *sub10 = new QCPSemiLogAxisRect(this);
    QCPAxisRect *dummy = new QCPAxisRect(this,false);

    // Galima deti tiesiai i plotLayout(), nes visi plotai yra "tinklelyje".
    plotLayout()->addElement(0, 0, sub00);
    plotLayout()->addElement(0, 1, sub01);
    plotLayout()->addElement(1, 0, sub10);
    plotLayout()->addElement(1, 1, dummy);

    sub01->setMaximumSize(MIN_WH, MAX_WH);
    sub10->setMaximumSize(MAX_WH, MIN_WH);
    dummy->setMaximumSize(MIN_WH, MIN_WH);

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
    sub10->axis(QCPAxis::atLeft)->setAutoTickCount(2);
    sub10->axis(QCPAxis::atRight)->setAutoTickCount(2);

    connect(sub01->axis(QCPAxis::atBottom),SIGNAL(rangeChanged(QCPRange)),sub01->axis(QCPAxis::atTop),SLOT(setRange(QCPRange)));
    connect(sub10->axis(QCPAxis::atLeft),SIGNAL(rangeChanged(QCPRange)),sub10->axis(QCPAxis::atRight),SLOT(setRange(QCPRange)));
    connect(sub10->axis(QCPAxis::atBottom),SIGNAL(rangeChanged(QCPRange)),sub10->axis(QCPAxis::atTop),SLOT(setRange(QCPRange)));

    QCPMarginGroup *marginGroup = new QCPMarginGroup(this);
    sub00->setMarginGroup(QCP::msTop | QCP::msLeft, marginGroup);
    sub01->setMarginGroup(QCP::msTop | QCP::msRight, marginGroup);
    sub10->setMarginGroup(QCP::msBottom | QCP::msLeft, marginGroup);
    dummy->setMarginGroup(QCP::msBottom | QCP::msRight, marginGroup);

    setBackground(QColor(240,240,240));
    foreach (QCPAxisRect *rect, axisRects())
    {
      rect->setLayer("background");
      if (rect!=dummy)
        rect->setBackground(Qt::white);
      else
        rect->setBackground(QColor(240,240,240));
      foreach (QCPAxis *axis, rect->axes())
      {
        axis->setLayer("axes");
        axis->grid()->setLayer("grid");
      }
    }

    // Sub00 - graph(-1):
    InitializeCarpet();

    QPen pen;
    pen.setWidth(3);
    pen.setColor(Qt::blue);

    // Sub01 - graph(0):
    addGraph(sub01->axis(QCPAxis::atRight), sub01->axis(QCPAxis::atBottom));
    graph(0)->setPen(pen);

    // Sub10 - graph(1):
    addGraph(sub10->axis(QCPAxis::atBottom), sub10->axis(QCPAxis::atLeft));
    sub10->axis(QCPAxis::atBottom)->setRange(mData->Wavelength.first(),mData->Wavelength.last());
    graph(1)->setPen(pen);

    /*****************************************/

    pen.setWidth(2);
    pen.setColor(Qt::red);
    pen.setStyle(Qt::DashLine);
    addGraph(sub01->axis(QCPAxis::atRight), sub01->axis(QCPAxis::atBottom));
    graph(2)->setPen(pen);

    /*****************************************/

    pen.setColor(Qt::gray);
    //pen.setStyle(Qt::SolidLine);
    addGraph(sub01->axis(QCPAxis::atRight), sub01->axis(QCPAxis::atBottom));
    graph(3)->setPen(pen);
    graph(3)->setVisible(false);

    addGraph(sub10->axis(QCPAxis::atBottom), sub10->axis(QCPAxis::atLeft));
    graph(4)->setPen(pen);
    graph(4)->setVisible(false);

    /*****************************************/

    RescaleLogAxes();
}

void CustomPlot2D::RescaleLogAxes()
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

    QVector < double > *pt_PTR = GetPTPointer();
    y10->setRange((isLog ? -1 : pt_PTR->first()), pt_PTR->last());
    y11->setRange((isLog ? -1 : pt_PTR->first()), pt_PTR->last());
    y00->setRange((isLog ? -1 : pt_PTR->first()), pt_PTR->last());
    y01->setRange((isLog ? -1 : pt_PTR->first()), pt_PTR->last());
}

void CustomPlot2D::InitializeCarpet()
{
    double mn_z, mx_z;
    cMap = new QCPColorMap(xAxis, yAxis);
    addPlottable(cMap);

    RescaleColorMap(&mn_z,&mx_z);

    colorScale = new QCPColorScale(this);
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

void CustomPlot2D::ChangeAxesScale()
{
    // Keista, kad sita buvau praziopsojes.
    // SpectraManageryje sutvarkyta kiek dailiau...
    double mn_z, mx_z;

    isLog=!isLog;
    RescaleColorMap(&mn_z,&mx_z);
    colorScale->setDataRange(QCPRange(mn_z,mx_z));
    SetSpectralCuts();
    RescaleLogAxes();
    replot();
}

void CustomPlot2D::RescaleColorMap(double *mn_z, double *mx_z)
{
    QVector < double > *timePTR=GetPTPointer();
    QVector < QVector < double > > *dataPTR=(((aFSRS!=NULL) && showAux) ? aFSRS  : mFSRS);
    int ny, nx = mData->Wavelength.size();
    if(!isLog)
        ny=mData->ProbeTime.size();
    else
    {
        int npix;
        hunt(&LogProbeTime,-1.0,&npix);
        ny=LogProbeTime.size()-npix;
    }

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
                           QCPRange((isLog ? -1 : timePTR->first()), timePTR->last()));
    for (int xIndex=0; xIndex<nx; ++xIndex)
    {
      for (int yIndex=0; yIndex<ny; ++yIndex)
      {
        cMap->data()->cellToCoord(xIndex, yIndex, &x, &y);
        hunt(timePTR,y,&pt);
        hunt(&mData->Wavelength,x,&wl);
        z=(*dataPTR)[pt][wl];
        if (mn_z != NULL)
        {
            if (z>=(*mx_z)) (*mx_z)=z;
            if (z<=(*mn_z)) (*mn_z)=z;
        }
        cMap->data()->setCell(xIndex, yIndex, z);
      }
    }

    // Ar juos palikti cia? Del visa ko dar neistrinu is senesnio kodo.
    cMap->rescaleDataRange();
    rescaleAxes();
}

inline QVector < double > * CustomPlot2D::GetPTPointer()
{
    if (isLog) return &(LogProbeTime);
    else return &(mData->ProbeTime);
}

inline QCPSemiLogAxis * CustomPlot2D::GetAxisPointer(int row, int col, QCPAxis::AxisType type)
{
    QCPAxisRect *r = static_cast<QCPAxisRect*>(plotLayout()->element(row,col));
    return static_cast<QCPSemiLogAxis*>(r->axis(type));
}

void CustomPlot2D::ReplotCarpet()
{
    RescaleColorMap();
    SetSpectralCuts();
    RescaleLogAxes();
    replot();
}

void CustomPlot2D::ReplotCarpet(double pt, double wl)
{
    sell_wl=wl;
    sell_pt=pt;
    RescaleColorMap();
    SetSpectralCuts();
    RescaleLogAxes();  // Reikia kviesti del RescaleColorMap() esancio rescaleAxes();
    replot();
}
