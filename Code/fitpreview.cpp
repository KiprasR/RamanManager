#include "fitpreview.h"
#include "ui_fitpreview.h"

FitPreview::FitPreview(QWidget *parent, QVector < double > *pT, QTableWidget *tBL, QSettings *mainSettings) :
    QDialog(parent),
    ui(new Ui::FitPreview),
    isLog(true), ProbeTime(pT),
    mSettings(mainSettings)
{
      ui->setupUi(this);

      ui->widget->plotLayout()->clear();

      QCPAxisRect *sub0 = new QCPSemiLogAxisRect(ui->widget);
      QCPAxisRect *sub1 = new QCPSemiLogAxisRect(ui->widget);
      QCPAxisRect *sub2 = new QCPSemiLogAxisRect(ui->widget);

      ui->widget->plotLayout()->addElement(0,0,sub0);
      ui->widget->plotLayout()->addElement(1,0,sub1);
      ui->widget->plotLayout()->addElement(2,0,sub2);

      x00=sub0->axis(QCPAxis::atBottom);
      x01=sub0->axis(QCPAxis::atTop);
      y0=sub0->axis(QCPAxis::atLeft);

      x10=sub1->axis(QCPAxis::atBottom);
      x11=sub1->axis(QCPAxis::atTop);
      y1=sub1->axis(QCPAxis::atLeft);

      x20=sub2->axis(QCPAxis::atBottom);
      x21=sub2->axis(QCPAxis::atTop);
      y2=sub2->axis(QCPAxis::atLeft);

      ui->widget->xAxis  = sub0->axis(QCPAxis::atBottom);
      ui->widget->xAxis2 = sub0->axis(QCPAxis::atTop);
      ui->widget->yAxis  = sub0->axis(QCPAxis::atLeft);
      ui->widget->yAxis2 = sub0->axis(QCPAxis::atRight);

      QCPMarginGroup *marginGroup = new QCPMarginGroup(ui->widget);
      sub0->setMarginGroup(QCP::msTop | QCP::msLeft | QCP::msRight, marginGroup);
      sub1->setMarginGroup(QCP::msLeft | QCP::msRight, marginGroup);
      sub2->setMarginGroup(QCP::msBottom | QCP::msLeft | QCP::msRight, marginGroup);

      ui->widget->setBackground(QColor(240,240,240));
      foreach (QCPAxisRect *rect, ui->widget->axisRects())
      {
        rect->setLayer("background");
        rect->setBackground(Qt::white);
        foreach (QCPAxis *axis, rect->axes())
        {
          axis->setLayer("axes");
          axis->grid()->setLayer("grid");
        }
      }

      QPen pen;
      pen.setWidth(3);
      for (int i=0,j=0;i<tBL->columnCount();i+=3,j++)
      {
          ui->widget->addGraph(sub0->axis(QCPAxis::atBottom), sub0->axis(QCPAxis::atLeft));
          ui->widget->addGraph(sub1->axis(QCPAxis::atBottom), sub1->axis(QCPAxis::atLeft));
          ui->widget->addGraph(sub2->axis(QCPAxis::atBottom), sub2->axis(QCPAxis::atLeft));

          pen.setColor(GetCurveColor(j));
          ui->widget->graph(i  )->setPen(pen);
          ui->widget->graph(i+1)->setPen(pen);
          ui->widget->graph(i+2)->setPen(pen);
      }

      CalculateLogTime();
      SetupSignals(tBL);
      ReplotSignals();
      RescaleLogAxes();
      RescaleYAxes();
      ui->widget->replot();

      ui->widget->setInteractions(QCP::iRangeZoom);
      sub0->setRangeZoom(Qt::Vertical);
      sub1->setRangeZoom(Qt::Vertical);
      sub2->setRangeZoom(Qt::Vertical);
      ui->widget->setInteraction(QCP::iRangeDrag);

      connect(ui->widget,SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(MouseHover(QMouseEvent*)));
      connect(ui->widget,SIGNAL(mousePress(QMouseEvent*)),this,SLOT(RescaleGraphs(QMouseEvent*)));
      connect(ui->widget,SIGNAL(mouseDoubleClick(QMouseEvent*)),this,SLOT(RescaleAxes(QMouseEvent*)));

      ui->widget->setContextMenuPolicy(Qt::CustomContextMenu);
      connect(ui->widget,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(ContextMenuRequest(QPoint)));
}

FitPreview::~FitPreview()
{
    delete ui;
}

void FitPreview::RescaleGraphs(QMouseEvent *event)
{
    if (event->button()==Qt::MiddleButton)
    {
        RescaleLogAxes();
        SearchForMinMax();
        RescaleYAxes();
        ui->widget->replot();
    }
}

void FitPreview::SearchForMinMax()
{
    double mx_y0, mx_y1, mx_y2,
           mn_y0, mn_y1, mn_y2,
           mmn, mmx;

    mx_y0=mx_y1=mx_y2=-1.0e6;
    mn_y0=mn_y1=mn_y2= 1.0e6;

    for (int i=0;i<Fits.size();i+=3)
    {
        min_and_max(&Fits[i],&mmn,&mmx);
        if (mmn<mn_y0) mn_y0=mmn;
        if (mmx>mx_y0) mx_y0=mmx;

        min_and_max(&Fits[i+1],&mmn,&mmx);
        if (mmn<mn_y1) mn_y1=mmn;
        if (mmx>mx_y1) mx_y1=mmx;

        min_and_max(&Fits[i+2],&mmn,&mmx);
        if (mmn<mn_y2) mn_y2=mmn;
        if (mmx>mx_y2) mx_y2=mmx;
    }

    double dy0=(mx_y0-mn_y0)*0.025,
           dy1=(mx_y1-mn_y1)*0.025,
           dy2=(mx_y2-mn_y2)*0.025;

    y0_high=mx_y0+dy0;
    y0_low =mn_y0-dy0;
    y1_high=mx_y1+dy1;
    y1_low =mn_y1-dy1;
    y2_high=mx_y2+dy2;
    y2_low =mn_y2-dy2;
}

void FitPreview::ReplotSignals()
{
    for (int i=0;i<Fits.size();i+=3)
    {
        ui->widget->graph(i  )->setData((isLog ? LogProbeTime : (*ProbeTime)),Fits[i  ]);
        ui->widget->graph(i+1)->setData((isLog ? LogProbeTime : (*ProbeTime)),Fits[i+1]);
        ui->widget->graph(i+2)->setData((isLog ? LogProbeTime : (*ProbeTime)),Fits[i+2]);
    }
}

void FitPreview::RescaleYAxes()
{
    y0->setRange(y0_low,y0_high);
    y1->setRange(y1_low,y1_high);
    y2->setRange(y2_low,y2_high);
}

void FitPreview::CalculateLogTime()
{
    int i;
    for (i=0;i<ProbeTime->size(); i++)
      LogProbeTime << linToLog((*ProbeTime)[i],1.0);
    int logF=int(floor(LogProbeTime.last()));
    for (i=-1;i<=logF;i++)
    {
      LogTicks << double(i);
      LogTickLabels << (i < 1 ? QString::number(i) : QString::number(int(pow(10,i-1))));
    }
}

void FitPreview::MouseHover(QMouseEvent *event)
{
    double xx0 = x00->pixelToCoord(event->pos().x()),
           yy0 = y0->pixelToCoord(event->pos().y()),
           xx1 = x10->pixelToCoord(event->pos().x()),
           yy1 = y1->pixelToCoord(event->pos().y()),
           xx2 = x20->pixelToCoord(event->pos().x()),
           yy2 = y2->pixelToCoord(event->pos().y());

    if (xx0 >= x00->range().lower &&  xx0 <= x00->range().upper && yy0 >= y0->range().lower &&  yy0 <= y0->range().upper )
      QToolTip::showText(ui->widget->mapToGlobal(event->pos()),QString("%1 , %2").arg((isLog ? logToLin(xx0,1.00) : xx0),0,'f',2).arg(yy0,0,'f',2));
    else if (xx1 >= x10->range().lower &&  xx1 <= x10->range().upper && yy1 >= y1->range().lower &&  yy1 <= y1->range().upper )
      QToolTip::showText(ui->widget->mapToGlobal(event->pos()),QString("%1 , %2").arg((isLog ? logToLin(xx1,1.00) : xx1),0,'f',2).arg(yy1,0,'f',2));
    else if (xx2 >= x20->range().lower &&  xx2 <= x20->range().upper && yy2 >= y2->range().lower &&  yy2 <= y2->range().upper )
      QToolTip::showText(ui->widget->mapToGlobal(event->pos()),QString("%1 , %2").arg((isLog ? logToLin(xx2,1.00) : xx2),0,'f',2).arg(yy2,0,'f',2));
}

void FitPreview::RescaleAxes(QMouseEvent *event)
{
    QCPAxis *active=NULL;
    double xx0 = x00->pixelToCoord(event->pos().x()),
           yy0 = y0->pixelToCoord(event->pos().y()),
           xx1 = x10->pixelToCoord(event->pos().x()),
           yy1 = y1->pixelToCoord(event->pos().y()),
           xx2 = x20->pixelToCoord(event->pos().x()),
           yy2 = y2->pixelToCoord(event->pos().y());

    if (xx0 >= x00->range().lower &&  xx0 <= x00->range().upper && yy0 >= y0->range().lower &&  yy0 <= y0->range().upper )
        active=y0;
    else if (xx1 >= x10->range().lower &&  xx1 <= x10->range().upper && yy1 >= y1->range().lower &&  yy1 <= y1->range().upper )
        active=y1;
    else if (xx2 >= x20->range().lower &&  xx2 <= x20->range().upper && yy2 >= y2->range().lower &&  yy2 <= y2->range().upper )
        active=y2;

    if (active!=NULL)
    {
        double *yl, *yh;

        if      (active==y0) {yl=&y0_low; yh=&y0_high;}
        else if (active==y1) {yl=&y1_low; yh=&y1_high;}
        else if (active==y2) {yl=&y2_low; yh=&y2_high;}

        RescaleAxesDialog *axesDial;
        axesDial=new RescaleAxesDialog(this,yl,yh);
        axesDial->setAttribute(Qt::WA_DeleteOnClose,true);
        axesDial->setModal(true);
        axesDial->exec();
        RescaleYAxes();
        ui->widget->replot();
    }
}

void FitPreview::SetupSignals(QTableWidget *TablePtr)
{
    QVector < double > ys0(TablePtr->rowCount()), ys1(TablePtr->rowCount()), ys2(TablePtr->rowCount());
    for (int i=0;i<TablePtr->columnCount();i+=3)
    {
        for (int j=0;j<TablePtr->rowCount();j++)
        {
          ys0[ys0.size()-1-j]=TablePtr->item(j,i  )->text().toDouble();
          ys1[ys1.size()-1-j]=TablePtr->item(j,i+1)->text().toDouble();
          ys2[ys2.size()-1-j]=TablePtr->item(j,i+2)->text().toDouble();
        }
        Fits << ys0 << ys1 << ys2;
    }
    SearchForMinMax();
}

void FitPreview::ContextMenuRequest(QPoint point)
{
    QMenu *menu = new QMenu(this);
    QString AxisStr=(isLog ? "Linear temporal axis" : "Semi-log temporal axis");

    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->addAction(AxisStr,this,SLOT(ChangeAxesScale()));
    menu->addAction("Save data",this,SLOT(SaveData()));
    menu->popup(ui->widget->mapToGlobal(point));
}

void FitPreview::ChangeAxesScale()
{
    isLog=!isLog;
    ReplotSignals();
    RescaleLogAxes();
    ui->widget->replot();
}

void FitPreview::SaveData()
{
    QString fileName=QFileDialog::getSaveFileName(this, tr("Save Spectrum Data"),mSettings->value("CURRENT_RAMAN_DIR").toString());
    if(!fileName.isEmpty())
    {
        mSettings->setValue("CURRENT_RAMAN_DIR",GetCurrentDir(fileName));

        int i,j;
        FILE *outFile;
        outFile=fopen(fileName.toLatin1(),"w+");

        fprintf(outFile,"%16s\t","Probetime");
        for(j=0,i=1;j<Fits.size();j+=3,i++)
            fprintf(outFile,"%15s%1d\t","A_",i);
        for(j=0,i=1;j<Fits.size();j+=3,i++)
            fprintf(outFile,"%15s%1d\t","nu0_",i);
        for(j=0,i=1;j<Fits.size();j+=3,i++)
            fprintf(outFile,"%15s%1d\t","dnu_",i);
        fprintf(outFile,"\n");

        for (i=0;i<ProbeTime->size();i++)
        {
            fprintf(outFile,"%16.8e\t",(*ProbeTime)[i]);
            for(j=0;j<Fits.size();j+=3)
                fprintf(outFile,"%16.8e\t",Fits[j  ][i]);
            for(j=0;j<Fits.size();j+=3)
                fprintf(outFile,"%16.8e\t",Fits[j+1][i]);
            for(j=0;j<Fits.size();j+=3)
                fprintf(outFile,"%16.8e\t",Fits[j+2][i]);
            fprintf(outFile,"\n");
        }
        fclose(outFile);
    }
}

void FitPreview::RescaleLogAxes()
{
    QCPSemiLogAxis *ax00 = static_cast<QCPSemiLogAxis*>(x00),
                   *ax01 = static_cast<QCPSemiLogAxis*>(x01),
                   *ax10 = static_cast<QCPSemiLogAxis*>(x10),
                   *ax11 = static_cast<QCPSemiLogAxis*>(x11),
                   *ax20 = static_cast<QCPSemiLogAxis*>(x20),
                   *ax21 = static_cast<QCPSemiLogAxis*>(x21);
    if (isLog)
    {
      ax00->setAutoTicks(false);
      ax00->setAutoTickLabels(false);
      ax00->setTickVector(LogTicks);
      ax00->setTickVectorLabels(LogTickLabels);
      ax00->setAutoSubTicks(false);
      ax00->setLogSubTicks(true);

      ax01->setAutoTicks(false);
      ax01->setTickVector(LogTicks);
      ax01->setAutoSubTicks(false);
      ax01->setLogSubTicks(true);

      ax10->setAutoTicks(false);
      ax10->setAutoTickLabels(false);
      ax10->setTickVector(LogTicks);
      ax10->setTickVectorLabels(LogTickLabels);
      ax10->setAutoSubTicks(false);
      ax10->setLogSubTicks(true);

      ax11->setAutoTicks(false);
      ax11->setTickVector(LogTicks);
      ax11->setAutoSubTicks(false);
      ax11->setLogSubTicks(true);

      ax20->setAutoTicks(false);
      ax20->setAutoTickLabels(false);
      ax20->setTickVector(LogTicks);
      ax20->setTickVectorLabels(LogTickLabels);
      ax20->setAutoSubTicks(false);
      ax20->setLogSubTicks(true);

      ax21->setAutoTicks(false);
      ax21->setTickVector(LogTicks);
      ax21->setAutoSubTicks(false);
      ax21->setLogSubTicks(true);
    }
    else
    {
      ax00->setAutoTicks(true);
      ax00->setAutoTickLabels(true);
      ax00->setAutoSubTicks(true);
      ax00->setLogSubTicks(false);

      ax01->setAutoTicks(true);
      ax01->setAutoSubTicks(true);
      ax01->setLogSubTicks(false);

      ax10->setAutoTicks(true);
      ax10->setAutoTickLabels(true);
      ax10->setAutoSubTicks(true);
      ax10->setLogSubTicks(false);

      ax11->setAutoTicks(true);
      ax11->setAutoSubTicks(true);
      ax11->setLogSubTicks(false);

      ax20->setAutoTicks(true);
      ax20->setAutoTickLabels(true);
      ax20->setAutoSubTicks(true);
      ax20->setLogSubTicks(false);

      ax21->setAutoTicks(true);
      ax21->setAutoSubTicks(true);
      ax21->setLogSubTicks(false);
    }

    QVector < double > *pt_PTR = (isLog ? &LogProbeTime : ProbeTime);
    double start = (isLog ? -1 : pt_PTR->first()),
           end   = pt_PTR->last();
    ax00->setRange(start,end);
    ax01->setRange(start,end);
    ax10->setRange(start,end);
    ax11->setRange(start,end);
    ax20->setRange(start,end);
    ax21->setRange(start,end);
}
