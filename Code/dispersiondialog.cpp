#include "dispersiondialog.h"
#include "ui_dispersiondialog.h"

DispersionDialog::DispersionDialog(QWidget *parent, SpectrumData *mainPointer) :
    QDialog(parent),
    ui(new Ui::DispersionDialog),
    mainData(mainPointer)
{
    ui->setupUi(this);
    InitializeBoxes();
    CalculatePP();
    InitializeCarpet(true);
    InitializeCurves();
    if (ui->hintBox->isChecked()) ReplotHint();
    connect(ui->dispPlot,SIGNAL(mousePress(QMouseEvent*)),this,SLOT(clickedGraph(QMouseEvent *)));
    connect(ui->dispPlot,SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(mouseHover(QMouseEvent *)));
}

DispersionDialog::~DispersionDialog()
{
    delete ui;
}

void DispersionDialog::InitializeBoxes()
{
    ui->fromSpinBox->setMinimum(mainData->ProbeTime.first());
    ui->fromSpinBox->setMaximum(mainData->ProbeTime.last());
    ui->toSpinBox->setMinimum(mainData->ProbeTime.first());
    ui->toSpinBox->setMaximum(mainData->ProbeTime.last());

    ui->everyBox->setMinimum(1);
    ui->everyBox->setMaximum(mainData->Wavelength.size()-1);

    ui->stopBox->setMinimum(mainData->ProbeTime.first());
    ui->stopBox->setMaximum(mainData->ProbeTime.last());
}

void DispersionDialog::mouseHover(QMouseEvent *event)
{
    float x,y;
    x = ui->dispPlot->xAxis->pixelToCoord(event->pos().x());
    y = ui->dispPlot->yAxis->pixelToCoord(event->pos().y());
    if ((x >= ui->dispPlot->xAxis->range().lower) &&  (x <= ui->dispPlot->xAxis->range().upper) &&
        (y >= ui->dispPlot->yAxis->range().lower) &&  (y <= ui->dispPlot->yAxis->range().upper))
    {
        int xi, yi;
        hunt(&(mainData->Wavelength),x,&xi);
        hunt(&(mainData->ProbeTime),y,&yi);
        QToolTip::showText(ui->dispPlot->mapToGlobal(event->pos()),
                           QString("%1 %2\n%3 ps\n%4 mOD").arg(x,0,'f',2).arg(QString::fromUtf8(" cm⁻¹")).arg(y,0,'f',2).arg(PP.at(yi).at(xi),0,'f',2));
    }
}

void DispersionDialog::clickedGraph(QMouseEvent *mouseEvent)
{
    double dx=ui->dispPlot->xAxis->pixelToCoord(mouseEvent->pos().x()),
           dy=ui->dispPlot->yAxis->pixelToCoord(mouseEvent->pos().y());
    if (mouseEvent->button()==Qt::LeftButton)
    {
        if(dotX.indexOf(dx)==-1)
        {
            dotX.append(dx);
            dotY.append(dy);
            shell_sort_TwoArrays(&dotX,&dotY);
            ReplotDots();
        }
    }
    else if (mouseEvent->button()==Qt::RightButton)
    {
        if (!dotX.isEmpty())
        {
            for (int i=0;i<dotX.size();i++)
                if( (fabs(dx-dotX[i])<DELTA) && (fabs(dy-dotY[i])<DELTA) )
                {
                    dotX.remove(i);
                    dotY.remove(i);
                    if (!dotX.isEmpty())
                        ReplotDots();
                    else
                    {
                        ui->dispPlot->graph(0)->clearData();
                        ui->dispPlot->graph(1)->clearData();
                        ui->dispPlot->replot();
                    }
                    break;
                }
        }
    }
}

void DispersionDialog::ReplotDots()
{
    if (!dotX.isEmpty())
    {
        ui->dispPlot->graph(0)->setData(dotX,dotY);
        ui->dispPlot->graph(0)->setLayer("main");
        if (dotX.size()>=ui->polyBox->value())
        {
            int i;
            double chisq;
            QVector < double > x(dotX.size()), yPoly(mainData->Wavelength.size()), coeffs;
            for (i=0;i<dotX.size();i++)
                x[i]=(dotX[i]-mainData->Wavelength.first())/(mainData->Wavelength.last()-mainData->Wavelength.first());
            svdfit_wrapper(x,dotY,coeffs,ui->polyBox->value(),&chisq);
            ui->chi2->setText(QString::number(chisq,'e',3));
            x.resize(mainData->Wavelength.size());
            for (i=0;i<mainData->Wavelength.size();i++)
                x[i]=(mainData->Wavelength[i]-mainData->Wavelength.first())/(mainData->Wavelength.last()-mainData->Wavelength.first());
            polynomialRecursionLin(&x,&coeffs,&yPoly);
            ui->dispPlot->graph(1)->setData(mainData->Wavelength,yPoly);
            ui->dispPlot->graph(1)->setLayer("main");
        }
        else
        {
            ui->dispPlot->graph(1)->clearData();
        }
        ui->dispPlot->replot();
    }
}

void DispersionDialog::InitializeCurves()
{
    QPen pen;

    // 0: Red dots;
    ui->dispPlot->addGraph();
    pen.setColor(Qt::lightGray);
    ui->dispPlot->graph(0)->setPen(pen);
    ui->dispPlot->graph(0)->setLineStyle((QCPGraph::LineStyle)0);
    ui->dispPlot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc,10));

    // 1: Fit curve;
    ui->dispPlot->addGraph();
    pen.setWidth(3);
    pen.setColor(Qt::lightGray);
    ui->dispPlot->graph(1)->setPen(pen);

    // 2: Hint dots;
    ui->dispPlot->addGraph();
    pen.setColor(Qt::darkYellow);
    ui->dispPlot->graph(2)->setPen(pen);
    ui->dispPlot->graph(2)->setLineStyle((QCPGraph::LineStyle)0);
    ui->dispPlot->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDiamond,5));

    // 3: Hint curve;
    ui->dispPlot->addGraph();
    pen.setColor(Qt::darkYellow);
    ui->dispPlot->graph(3)->setPen(pen);
}


void DispersionDialog::ReplotHint()
{
   QVector < double > x, y;
   get_dispersion_rise_Raman(mainData,&PP,ui->stopBox->value(),ui->everyBox->value(),&x,&y);
   ui->dispPlot->graph(2)->setData(x, y);
   ui->dispPlot->graph(2)->setLayer("main");

   if (x.size()>=ui->polyBox->value())
   {
       int i;
       double chisq;
       QVector < double > xScaled(x.size()),
                          yPoly(mainData->Wavelength.size()),
                          coeffs;
       for (i=0;i<xScaled.size();i++)
           xScaled[i]=(x[i]-mainData->Wavelength.first())/(mainData->Wavelength.last()-mainData->Wavelength.first());
       svdfit_wrapper(xScaled,y,coeffs,ui->polyBox->value(),&chisq);
       xScaled.resize(mainData->Wavelength.size());
       for (i=0;i<xScaled.size();i++)
           xScaled[i]=(mainData->Wavelength[i]-mainData->Wavelength.first())/(mainData->Wavelength.last()-mainData->Wavelength.first());
       polynomialRecursionLin(&xScaled,&coeffs,&yPoly);
       ui->chi->setText(QString::number(chisq,'e',3));
       ui->dispPlot->graph(3)->setData(mainData->Wavelength,yPoly);
       ui->dispPlot->graph(3)->setLayer("main");
   }
   else
   {
       ui->dispPlot->graph(3)->clearData();
   }
   ui->dispPlot->replot();
}

void DispersionDialog::InitializeCarpet(bool alloc)
{
    // configure axis rect:
    ui->dispPlot->axisRect()->setupFullAxesBox(true);
    // set up the QCPColorMap:
    if (!alloc) ui->dispPlot->removePlottable(cMap);
    QCPColorMap *colorMap = new QCPColorMap(ui->dispPlot->xAxis,ui->dispPlot->yAxis);
    cMap=colorMap;
    ui->dispPlot->addPlottable(colorMap);
    int nx = 300,
        ny = 300;
    colorMap->data()->setSize(nx,ny); // we want the color map to have nx * ny data points
    colorMap->data()->setRange(QCPRange(mainData->Wavelength.first(),mainData->Wavelength.last()),QCPRange(ui->fromSpinBox->value(),ui->toSpinBox->value())); // and span the coordinate range -4..4 in both key (x) and value (y) dimensions
    // now we assign some data, by accessing the QCPColorMapData instance of the color map:
    double x, y, z, mx_z, mn_z;
    int pt, wl;
    mx_z=-1e6;
    mn_z= 1e6;
    for (int xIndex=0; xIndex<nx; ++xIndex)
    {
      for (int yIndex=0; yIndex<ny; ++yIndex)
      {
        colorMap->data()->cellToCoord(xIndex, yIndex, &x, &y);
        hunt(&mainData->ProbeTime,y,&pt);
        hunt(&mainData->Wavelength,x,&wl);
        z=PP[pt][wl];
        //z=x*y;
        if (z>=mx_z) mx_z=z;
        if (z<=mn_z) mn_z=z;
        colorMap->data()->setCell(xIndex, yIndex, z);
      }
    }

    // add a color scale:
    // Galima efektyviau suzaisti su rodyklemis:
    if (!alloc) ui->dispPlot->plotLayout()->remove(cScale);
    QCPColorScale *colorScale = new QCPColorScale(ui->dispPlot);
    cScale=colorScale;
    colorScale->setDataRange(QCPRange(mn_z,mx_z));
    ui->dispPlot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorMap->setColorScale(colorScale); // associate the color map with the color scale

    // set the color gradient of the color map to one of the presets:
    colorMap->setGradient(QCPColorGradient::gpSpectrum);
    // we could have also created a QCPColorGradient instance and added own colors to
    // the gradient, see the documentation of QCPColorGradient for what's possible.

    // rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
    colorMap->rescaleDataRange();

    // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):

    QCPMarginGroup *marginGroup = new QCPMarginGroup(ui->dispPlot);
    ui->dispPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    // rescale the key (x) and value (y) axes so the whole color map is visible:
    ui->dispPlot->rescaleAxes();
    ui->dispPlot->yAxis->setRange(ui->fromSpinBox->value(),ui->toSpinBox->value());
    ui->dispPlot->replot();
}

void DispersionDialog::CalculatePP()
{
    int i,j;
    PP.fill(QVector < double > (mainData->Wavelength.size()), mainData->ProbeTime.size());
    if(!mainData->WL00.isEmpty())
    {

        for (i=0;i<mainData->ProbeTime.size();i++)
            for (j=0;j<mainData->Wavelength.size();j++)
                PP[i][j]=-1000*log10(mainData->WL10[i][j]/mainData->WL00[i][j]);
    }
    else
    {
        for (i=0;i<mainData->ProbeTime.size();i++)
            for (j=0;j<mainData->Wavelength.size();j++)
                PP[i][j]=1000*log10(mainData->FSRS[i][j]);
    }
}

void DispersionDialog::on_hintBox_toggled(bool checked)
{
    ui->everyBox->setEnabled(checked);
    ui->stopBox->setEnabled(checked);
    ui->label_3->setEnabled(checked);
    ui->label_4->setEnabled(checked);
    if (checked)
        ReplotHint();
    else
    {
        ui->dispPlot->graph(2)->clearData();
        ui->dispPlot->graph(3)->clearData();
        ui->dispPlot->replot();
    }
}

void DispersionDialog::on_applyHintButton_clicked()
{
    int i;
    double chisq;
    QVector < double > x, y,
                       fitx, coeffs,
                       tempWave=mainData->Wavelength;
    get_dispersion_rise_Raman(mainData,&PP,ui->toSpinBox->value(),ui->everyBox->value(),&x,&y);
    fitx.resize(x.size());
    for (i=0;i<fitx.size();i++)
        fitx[i]=(x[i]-mainData->Wavelength.first())/(mainData->Wavelength.last()-mainData->Wavelength.first());
    svdfit_wrapper(fitx,y,coeffs,ui->polyBox->value(),&chisq);
    for (i=0;i<mainData->Wavelength.size();i++)
        mainData->Wavelength[i]=(mainData->Wavelength[i]-tempWave.first())/(tempWave.last()-tempWave.first());
    correctDispersion(mainData,&coeffs);
    mainData->Wavelength=tempWave;
    close();
}

void DispersionDialog::on_applyButton_clicked()
{
    if (dotX.size()>=ui->polyBox->value())
    {
        int i;
        double chisq;
        QVector < double > x(dotX.size()),coeffs,tempWave=mainData->Wavelength;;
        for (i=0;i<dotX.size();i++)
            x[i]=(dotX[i]-mainData->Wavelength.first())/(mainData->Wavelength.last()-mainData->Wavelength.first());
        svdfit_wrapper(x,dotY,coeffs,ui->polyBox->value(),&chisq);
        for (i=0;i<mainData->Wavelength.size();i++)
            mainData->Wavelength[i]=(mainData->Wavelength[i]-tempWave.first())/(tempWave.last()-tempWave.first());
        correctDispersion(mainData,&coeffs);
        mainData->Wavelength=tempWave;
        close();
    }
}

void DispersionDialog::on_cancelButton_clicked()
{
    close();
}

void DispersionDialog::on_everyBox_valueChanged(int arg1)
{
    if (ui->hintBox->isChecked()) ReplotHint();
    ReplotDots();
}

void DispersionDialog::on_stopBox_valueChanged(double arg1)
{
    if (ui->hintBox->isChecked()) ReplotHint();
    ReplotDots();
}

void DispersionDialog::on_polyBox_valueChanged(int arg1)
{
    if (ui->hintBox->isChecked()) ReplotHint();
    ReplotDots();
}

void DispersionDialog::on_fromSpinBox_valueChanged(double arg1)
{
    if (arg1<ui->toSpinBox->value())
    {
        //ui->dispPlot->clearPlottables();
        InitializeCarpet();
        if (ui->hintBox->isChecked()) ReplotHint();
        ReplotDots();
    }
    else
    {
        ui->fromSpinBox->setValue(ui->toSpinBox->value()-1.0);
    }
}

void DispersionDialog::on_toSpinBox_valueChanged(double arg1)
{
    if (arg1>ui->fromSpinBox->value())
    {
        //ui->dispPlot->clearPlottables();
        InitializeCarpet();
        if (ui->hintBox->isChecked()) ReplotHint();
        ReplotDots();
    }
    else
    {
        ui->toSpinBox->setValue(ui->fromSpinBox->value()+1.0);
    }
}

void DispersionDialog::on_clearButton_clicked()
{
    dotX.clear();
    dotY.clear();
    ui->dispPlot->graph(0)->clearData();
    ui->dispPlot->graph(1)->clearData();
    ui->dispPlot->replot();
}
