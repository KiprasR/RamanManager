#include "comparetraces.h"
#include "ui_comparetraces.h"

CompareTraces::CompareTraces(QWidget *parent, QVector< QVector< double > > *Data, const QVector< double > *Delays, int NScans, QVector< double > *Weights) :
    QDialog(parent), ui(new Ui::CompareTraces),
    FullData(Data), LinDelays(Delays), WeightsMain(Weights), NoOfPlots(NScans)
{
    ui->setupUi(this);

    ui->compPlot->plotLayout()->remove(ui->compPlot->axisRect());
    ui->compPlot->plotLayout()->addElement(0,0,new QCPSemiLogAxisRect(ui->compPlot));
    ui->compPlot->xAxis =ui->compPlot->axisRect()->axis(QCPAxis::atBottom);
    ui->compPlot->xAxis2=ui->compPlot->axisRect()->axis(QCPAxis::atTop);
    ui->compPlot->yAxis =ui->compPlot->axisRect()->axis(QCPAxis::atLeft);
    ui->compPlot->yAxis2=ui->compPlot->axisRect()->axis(QCPAxis::atRight);

    QCPLegend *arLegend = new QCPLegend;
    ui->compPlot->axisRect()->insetLayout()->addElement(arLegend, Qt::AlignTop | Qt::AlignRight);
    ui->compPlot->axisRect()->insetLayout()->setMargins(QMargins(12, 12, 12, 12));
    arLegend->setLayer("legend");
    ui->compPlot->setAutoAddPlottableToLegend(false);

    int i;
    ui->wlSlider->setMinimum(0);
    ui->wlSlider->setMaximum(FullData->at(0).size()-1);
    ui->tableWidget->setRowCount(1);
    ui->tableWidget->setColumnCount(NoOfPlots);
    for (i=0;i<NoOfPlots;i++)
        ui->tableWidget->setHorizontalHeaderItem(i,new QTableWidgetItem(QString::number(i+1)));

    disconnect(ui->tableWidget,SIGNAL(cellChanged(int,int)),this,SLOT(on_tableWidget_cellChanged(int,int)));
    ui->tableWidget->setVerticalHeaderItem(0,new QTableWidgetItem("Apply weights:"));
    for(i=0;i<NoOfPlots;i++)
    {
        ui->tableWidget->setItem(0,i,new QTableWidgetItem("1"));
        ui->tableWidget->item(0,i)->setTextAlignment(Qt::AlignCenter);
        ui->compPlot->addGraph();
        arLegend->addItem(new QCPPlottableLegendItem(arLegend, ui->compPlot->graph(i)));
    }
    connect(ui->tableWidget,SIGNAL(cellChanged(int,int)),this,SLOT(on_tableWidget_cellChanged(int,int)));
    ui->tableWidget->item(0,0)->setText("1");
    ui->compPlot->yAxis->grid()->setSubGridVisible(true);
    ui->compPlot->xAxis->grid()->setSubGridVisible(true);

    // Pasigaminame SemiLog:
    for(i=0;i<LinDelays->size();i++)
        LogDelays << linToLog((*LinDelays)[i],1.0);
    int logF=int(floor(LogDelays.last()));
    for (i=-1;i<=logF;i++)
    {
        LogTicks << double(i);
        LogTickLabels << (i < 1 ? QString::number(i) : QString::number(int(pow(10,i-1))));
    }

    if(LinDelays->first()>0.0)
        ui->linButton->setChecked(true);
    else
    {
        rescaleLogAxes();
        plot(ui->wlSlider->value());
    }
    ui->wlSlider->setFocus();
    ui->compPlot->setInteraction(QCP::iRangeDrag);

    // W10 crap:
    ui->compPlot->axisRect()->setLayer("background");
    ui->compPlot->setBackground(QColor(240,240,240));
    foreach (QCPAxis *axis, ui->compPlot->axisRect()->axes())
    {
        axis->setLayer("axes");
        axis->grid()->setLayer("grid");
    }
    ui->compPlot->axisRect()->setBackground(Qt::white);
}

CompareTraces::~CompareTraces()
{
    delete ui;
}

void CompareTraces::on_wlSlider_valueChanged(int value)
{
    plot(value);
}

void CompareTraces::on_tableWidget_cellChanged(int row, int column)
{
    bool ok;
    ui->tableWidget->item(row,column)->text().toDouble(&ok);
    if(!ok) ui->tableWidget->item(row,column)->setText("1");
    (*WeightsMain)[column]=ui->tableWidget->item(row,column)->text().toDouble();
}

void CompareTraces::on_linButton_toggled(bool checked)
{
    rescaleLogAxes();
    plot(ui->wlSlider->value());
}

void CompareTraces::rescaleLogAxes()
{
    //Galbūt galima ašis sinchronizuoti?

    QCPSemiLogAxis *ax1, *ax2;
    ax1 = static_cast<QCPSemiLogAxis*>(ui->compPlot->axisRect()->axis(QCPAxis::atBottom));
    ax2 = static_cast<QCPSemiLogAxis*>(ui->compPlot->axisRect()->axis(QCPAxis::atTop));
    if(ui->logButton->isChecked())
    {
        ax1->setRange(-1.0,LogDelays.last());
        ax1->setAutoTicks(false);
        ax1->setAutoTickLabels(false);
        ax1->setTickVector(LogTicks);
        ax1->setTickVectorLabels(LogTickLabels);
        ax1->setAutoSubTicks(false);
        ax1->setSubTickCount(4);
        ax1->setLogSubTicks(true);

        ax2->setRange(-1.0,LogDelays.last());
        ax2->setAutoTicks(false);
        ax2->setAutoTickLabels(false);
        ax2->setTickVector(LogTicks);
        ax2->setTickVectorLabels(LogTickLabels);
        ax2->setAutoSubTicks(false);
        ax2->setSubTickCount(4);
        ax2->setLogSubTicks(true);
    }
    else
    {
        ax1->setRange(LinDelays->first(),LinDelays->last());
        ax1->setAutoTicks(true);
        ax1->setAutoTickLabels(true);
        ax1->setAutoSubTicks(true);
        ax1->setLogSubTicks(false);

        ax2->setRange(LinDelays->first(),LinDelays->last());
        ax2->setAutoTicks(true);
        ax2->setAutoTickLabels(true);
        ax2->setAutoSubTicks(true);
        ax2->setLogSubTicks(false);
    }
}

void CompareTraces::plot(int value)
{
    int i,j,k=0;
    int d=FullData->size()/NoOfPlots;
    double MinZ=1e6,MaxZ=-1e6;
    QVector<double> tempVect(LinDelays->size());
    QPen pen;
    for(i=0;i<NoOfPlots;i++)
    {
        for(j=0;j<LinDelays->size();j++)
        {
            tempVect[j]=(*FullData)[j+k][value];
            if (tempVect[j]>=MaxZ) MaxZ=tempVect[j];
            if (tempVect[j]<=MinZ) MinZ=tempVect[j];
        }
        ui->compPlot->graph(i)->setData(ui->logButton->isChecked() ? LogDelays : (*LinDelays),tempVect);
        pen.setColor(GetCurveColor(i));
        if (ui->tableWidget->item(0,i)->isSelected())
        {
            pen.setWidth(4);
            pen.setStyle(Qt::DashLine);
        }
        else
        {
            pen.setWidth(2);
            pen.setStyle(Qt::SolidLine);
        }
        ui->compPlot->graph(i)->setPen(pen);
        ui->compPlot->graph(i)->setName(QString::number(i+1));
        k+=d;
    }
    ui->compPlot->yAxis->setRange(MinZ-(MaxZ-MinZ)*0.025,MaxZ+(MaxZ-MinZ)*0.025);
    ui->wlLabel->setText(QString::number(value+1));
    ui->compPlot->replot();
}

void CompareTraces::on_tableWidget_itemSelectionChanged()
{
    plot(ui->wlSlider->value());
}
