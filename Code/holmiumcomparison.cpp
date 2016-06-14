#include "holmiumcomparison.h"
#include "ui_holmiumcomparison.h"

HolmiumComparison::HolmiumComparison(QWidget *parent, QSettings *DirSettings, QVector < double > *mainGSx, QVector < double > *mainGSy) :
    QDialog(parent),
    ui(new Ui::HolmiumComparison), ActiveDir(DirSettings)
{
    ui->setupUi(this);
    ui->solvBox->setDirSettings(ActiveDir);

    InitializeGraphs();
    InitializeLabels();

    ui->spectrum->setInteraction(QCP::iRangeZoom, true);
    ui->spectrum->setInteraction(QCP::iRangeDrag, true);
    connect(ui->spectrum,SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(showPointToolTip(QMouseEvent*)));
    connect(ui->spectrum,SIGNAL(mousePress(QMouseEvent*)),this,SLOT(clickEvent(QMouseEvent*)));
    connect(ui->solvBox,SIGNAL(sendSolvent(QString)),this,SLOT(solventChanged(QString)));

    if (mainGSx->isEmpty() || mainGSy->isEmpty())
    {
        ReadSolventSpectrum(ui->solvBox->getMapValue("TestSpec"),false,&tempX,&tempY);
        GSx=&tempX;
        GSy=&tempY;
    }
    else
    {
        GSx=mainGSx;
        GSy=mainGSy;
    }

    v0=ui->vBox->value();
    ReadSolventSpectrum(ui->solvBox->getCurrentSolvent(),false,&solvX,&solvY);

    //mn=min(GSy);
    mx=max(GSy);
    replot();
}

HolmiumComparison::~HolmiumComparison()
{
    //Pats susitvarko atmintį?
    /*
    for (int i=0;i<nlabels;i++)
        delete [] peakLabels[i];
    delete [] peakLabels;
    */
    // Galbut reikia tik:
    //delete [] peakLabels;

    delete ui;
}

void HolmiumComparison::on_applyButton_clicked()
{
    emit sendCalibration(ui->kBox->value(),ui->bBox->value(),ui->vBox->value());
    close();
}

void HolmiumComparison::on_closeButton_clicked()
{
    close();
}

void HolmiumComparison::replot()
{
    int i,j;
    double mean,m1,m2,std;
    QVector<double> LambdaCM(GSx->size()), OD(GSy->size());

    /******************  Convolution ***********************/
    QVector < double > *solvYPoint, ref_temp;
    if (ui->convoBox->isChecked())
    {
        convolve(&ref_temp);
        solvYPoint=&ref_temp;
    }
    else
    {
        solvYPoint=&solvY;
    }
    /*******************************************************/


    for(i=0;i<GSx->size();i++)
    {
        LambdaCM[i]=1e7*(1/ui->vBox->value()-1/((*GSx)[i]*ui->kBox->value()+ui->bBox->value()));
        OD[i]=(GSy->at(i))/(mx)*(ui->mBox->value());
    }

    QVector < double > yVect(GSx->size()), statVect(GSx->size());
    interpolate(&solvX,solvYPoint,&LambdaCM,&yVect);
    for(i=0;i<GSx->size();i++)
    {
        statVect[i]=OD[i]-yVect[i];
        yVect[i]=statVect[i]*0.5+1.1;
    }
    min_and_max(&statVect,&m1,&m2);
    MeanAndSTD(&statVect,&mean,&std);

    ui->maxlabel->setText("Max: "+QString::number(MAX(fabs(m1),fabs(m2)),'f',6));
    ui->meanlabel->setText("Mean: "+QString::number(mean,'f',6));
    ui->stdlabel->setText("STD: "+QString::number(std,'f',6));

    ui->spectrum->graph(1)->setData(LambdaCM,OD);
    ui->spectrum->graph(2)->setData(LambdaCM,yVect);

    QVector < double > solvYScaled(solvY.size()), solvTemp;
    int l0,l1;
    hunt(&solvX,LambdaCM.first(),&l0);
    hunt(&solvX,LambdaCM.last() ,&l1);
    solvTemp=solvYPoint->mid(l0,l1-l0+1);
    mx_solv=max(&solvTemp);
    for(i=0;i<solvX.size();i++)
        solvYScaled[i]=((*solvYPoint)[i])/(mx_solv);
    ui->spectrum->graph(0)->setData(solvX,solvYScaled);

    /***********************************************************/
    /***********************  Peaks  ***************************/
    /***********************************************************/

    QVector < double > maxLam(ui->peakBox->value()), maxAmp(ui->peakBox->value());
    maxLam.fill(0.0);
    maxAmp.fill(0.0);
    for (i=1;i<LambdaCM.size()-1;i++)
    {
        if ((OD[i-1]<OD[i])&&(OD[i+1]<OD[i]))
        {
            if(OD[i]>min_index(&maxAmp,&j))
            {
                maxAmp[j]=OD[i];
                maxLam[j]=LambdaCM[i];
            }
        }
    }
    shell_sort_TwoArrays(&maxLam,&maxAmp);

    for(i=0;i<ui->peakBox->value();i++)
    {
        peakLabels[i]->position->setCoords(maxLam[i],maxAmp[i]+0.11);
        peakLabels[i]->setText(QString::number(maxLam[i],'f',2));
    }

    /***********************************************************/

    ui->spectrum->xAxis->setRange(below_min(&LambdaCM,1)-25,above_max(&LambdaCM,1)+25);
    ui->spectrum->replot();
}

void HolmiumComparison::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Down)&&(event->modifiers() == Qt::NoModifier))
    {
        ui->kBox->setValue(ui->kBox->value()-ui->kScalerBox->value());
        //replot();
    }
    else if ((event->key() == Qt::Key_Up)&&(event->modifiers() == Qt::NoModifier))
    {
        ui->kBox->setValue(ui->kBox->value()+ui->kScalerBox->value());
        //replot();
    }
    else if ((event->key() == Qt::Key_Right)&&(event->modifiers() == Qt::NoModifier))
    {
        ui->bBox->setValue(ui->bBox->value()-ui->bScalerBox->value());
        //replot();
    }
    else if ((event->key() == Qt::Key_Left)&&(event->modifiers() == Qt::NoModifier))
    {
        ui->bBox->setValue(ui->bBox->value()+ui->bScalerBox->value());
        //replot();
    }
    else if ((event->key() == Qt::Key_Comma)&&(event->modifiers() == Qt::NoModifier))
    {
        ui->mBox->setValue(ui->mBox->value()-ui->mScalerBox->value());
        //replot();
    }
    else if ((event->key() == Qt::Key_Period)&&(event->modifiers() == Qt::NoModifier))
    {
        ui->mBox->setValue(ui->mBox->value()+ui->mScalerBox->value());
        //replot();
    }
    else if ((event->key() == Qt::Key_Down)&&(event->modifiers() == Qt::ShiftModifier))
    {
        ui->kScalerBox->setValue( ui->kScalerBox->value()-0.001 );
    }
    else if ((event->key() == Qt::Key_Up)&&(event->modifiers() == Qt::ShiftModifier))
    {
        ui->kScalerBox->setValue( ui->kScalerBox->value()+0.001 );
    }
    else if ((event->key() == Qt::Key_Right)&&(event->modifiers() == Qt::ShiftModifier))
    {
        ui->bScalerBox->setValue( ui->bScalerBox->value()+0.01 );
    }
    else if ((event->key() == Qt::Key_Left)&&(event->modifiers() == Qt::ShiftModifier))
    {
        ui->bScalerBox->setValue( ui->bScalerBox->value()-0.01 );
    }
    else if (event->key() == Qt::Key_Less)
    {
        ui->mScalerBox->setValue( ui->mScalerBox->value()-0.01 );
    }
    else if (event->key() == Qt::Key_Greater)
    {
        ui->mScalerBox->setValue( ui->mScalerBox->value()+0.01 );
    }
}

void HolmiumComparison::ReadSolventSpectrum(QString solventFile, bool cmToNm, QVector < double > *x0, QVector <double> *y0)
{
    readXYFile(solventFile,x0,y0);
    if (cmToNm)
    {
        for (int i=0;i<x0->size();i++)
            (*x0)[i]=1/(1/v0-(*x0)[i]/1e7);
    }
}

void HolmiumComparison::InitializeGraphs()
{
    ui->spectrum->axisRect()->setupFullAxesBox(true);

    QPen pen;
    pen.setWidth(2);
    pen.setColor(Qt::black);

    ui->spectrum->addGraph();
    ui->spectrum->addGraph();
    ui->spectrum->addGraph();
    ui->spectrum->graph(0)->setPen(pen);

    pen.setColor(Qt::blue);
    ui->spectrum->graph(1)->setPen(pen);
    ui->spectrum->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle,4));

    pen.setColor(Qt::darkGray);
    ui->spectrum->graph(2)->setPen(pen);
    ui->spectrum->graph(2)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDiamond,4));

    ui->spectrum->addGraph();
    pen.setColor(Qt::red);
    ui->spectrum->graph(3)->setPen(pen);
    ui->spectrum->graph(3)->setLineStyle((QCPGraph::LineStyle)0);
    ui->spectrum->graph(3)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc,10));

    ui->spectrum->yAxis->setRange(-0.05,1.3);
}

void HolmiumComparison::on_vBox_valueChanged(double arg1)
{
    replot();
}

void HolmiumComparison::on_kBox_valueChanged(double arg1)
{
    replot();
}

void HolmiumComparison::on_bBox_valueChanged(double arg1)
{
    replot();
}

void HolmiumComparison::on_mBox_valueChanged(double arg1)
{
    replot();
}

void HolmiumComparison::showPointToolTip(QMouseEvent *event)
{
    float x,y,v;
    x = ui->spectrum->xAxis->pixelToCoord(event->pos().x());
    y = ui->spectrum->yAxis->pixelToCoord(event->pos().y());
    if ((x >= ui->spectrum->xAxis->range().lower) &&  (x <= ui->spectrum->xAxis->range().upper) &&
        (y >= ui->spectrum->yAxis->range().lower) &&  (y <= ui->spectrum->yAxis->range().upper))
    {
        v = 1/(1/ui->vBox->value()-x/1e7);
        QToolTip::showText(ui->spectrum->mapToGlobal(event->pos()),
                           QString("%1 %2\n%3 nm\n%4").arg(x,0,'f',2).arg(QString::fromUtf8(" cm⁻¹")).arg(v,0,'f',2).arg(y,0,'f',2));
    }
}

void HolmiumComparison::clickEvent(QMouseEvent *event)
{
    if ((event->button()==Qt::RightButton)||(event->button()==Qt::MiddleButton))
    {
        ui->spectrum->yAxis->setRange(-0.05,1.3);
        replot();
    }
}

void HolmiumComparison::ReloadSampleSpectrum(QString solventName,bool fromFile)
{
    if (fromFile)
    {
        historyX.resize(GSx->size());
        historyY.resize(GSy->size());
        for (int i=0;i<GSx->size();i++)
        {
            historyX[i]=(*GSx)[i];
            historyY[i]=(*GSy)[i];
        }
        ReadSolventSpectrum(solventName,false,&tempX,&tempY);
        GSx=&tempX;
        GSy=&tempY;
        //mn=min(GSy);
        mx=max(GSy);
    }
    else
    {
        GSx=&historyX;
        GSy=&historyY;
        //mn=min(GSy);
        mx=max(GSy);
    }
}

void HolmiumComparison::InitializeLabels()
{
    /*
    peakLabels=new QCPItemText*[nlabels];
    for (int i=0;i<nlabels;i++)
    {
        peakLabels[i]=new QCPItemText(ui->spectrum);
        ui->spectrum->addItem(peakLabels[i]);
    }
    */

    QColor whAlpha(Qt::white);
    whAlpha.setAlpha(175);

    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(whAlpha);

    QPen pen;
    pen.setColor(whAlpha);

    for (int i=0;i<ui->peakBox->value();i++)
    {
        peakLabels.push_back( new QCPItemText(ui->spectrum) );
        ui->spectrum->addItem(peakLabels[i]);
        peakLabels[i]->setPen(pen);
        peakLabels[i]->setBrush(brush);
        peakLabels[i]->setFont(QFont(font().family(), 10, QFont::Bold));
        peakLabels[i]->setColor(Qt::blue);
        peakLabels[i]->setRotation(270);
    }
    nprev=ui->peakBox->value();
}

void HolmiumComparison::on_peakBox_valueChanged(int arg1)
{
    for (int i=0;i<nprev;i++)
        ui->spectrum->removeItem(peakLabels[i]);
    peakLabels.clear();
    InitializeLabels();
    replot();
}

void HolmiumComparison::on_autoButton_clicked()
{
    QVector <double> ps(2), dels(2), ans;
    ps[0]=ui->kBox->value();
    ps[1]=ui->bBox->value();
    dels[0]=dels[1]=1e-6;
    Amoeba HolmAmoeba(1e-10,this);

    ans=ps;
    for(int i=0;i<1000;i++)
    {
        ans=HolmAmoeba.minimize(ans,dels);
        //qDebug() << ps << ans << fabs(ans[0]-ps[0]) ;
        //qDebug() << "Before:" << get_STD(ps) << " After: " << get_STD(ans);
        if((fabs(ans[0]-ps[0])<1e-12) && (fabs(ans[1]-ps[1])<1e-12))
            break;
        ps=ans;
    }
    //ans=HolmAmoeba.minimize(ps,dels);

    ui->kBox->setValue(ans[0]);
    ui->bBox->setValue(ans[1]);
}

void HolmiumComparison::showSelectedPoints(QMouseEvent *event)
{
    int xi;
    double x,i;

    x=ui->spectrum->xAxis->pixelToCoord(event->pos().x());
    if (selCounter<2)
    {
        i=(1/(1/ui->vBox->value()-x/1e7)-ui->bBox->value())/ui->kBox->value();
        hunt(GSx,i,&xi);
        selI[selI.size()-1]=xi;
        selX[selX.size()-1]=1e7*(1/ui->vBox->value()-1/((*GSx)[xi]*ui->kBox->value()+ui->bBox->value()));
        selY[selY.size()-1]=((*GSy)[xi])/(mx);
    }
    else
    {
        hunt(&solvX,x,&xi);
        selI[selI.size()-1]=xi;
        selX[selX.size()-1]=solvX[xi];
        selY[selY.size()-1]=(solvY[xi])/(mx_solv);
    }
    ui->spectrum->graph(3)->setData(selX,selY);
    ui->spectrum->replot();
}

void HolmiumComparison::addSelectedPoints(QMouseEvent *event)
{
    if (event->button()==Qt::LeftButton)
    {
        //patikrinimas...
        selI.push_back(0);
        selX.push_back(0.0);
        selY.push_back(0.0);
        selCounter++;
        ui->spectrum->replot();

        if (selCounter==4)
        {
            double k, b, l0, l1;
            if (selI[0]>selI[1]) SWAP(selI[0],selI[1]);
            if (selI[2]>selI[3]) SWAP(selI[2],selI[3]);

            l0=1/(1/ui->vBox->value()-solvX[selI[2]]/1e7);
            l1=1/(1/ui->vBox->value()-solvX[selI[3]]/1e7);
            k=(l0-l1)/((*GSx)[selI[0]]-(*GSx)[selI[1]]);
            b=l0-k*(*GSx)[selI[0]];
            ui->kBox->setValue(k);
            ui->bBox->setValue(b);

            ui->spectrum->graph(3)->clearData();
            disconnect(ui->spectrum,SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(showSelectedPoints(QMouseEvent*)));
            disconnect(ui->spectrum,SIGNAL(mousePress(QMouseEvent*)),this,SLOT(addSelectedPoints(QMouseEvent*)));
            selI.clear();
            selX.clear();
            selY.clear();
            selCounter=0;
        }

    }
}

void HolmiumComparison::solventChanged(QString solvent)
{
    //qDebug() << "Arg: " << arg1;
    ReadSolventSpectrum(solvent,false,&solvX,&solvY);
    replot();
}


void HolmiumComparison::on_fsrsBox_currentIndexChanged(const QString &arg1)
{
    if (QString::compare(arg1,"GS FSRS")==0)
    {
        ReloadSampleSpectrum("",false);
        replot();
    }
    else
    {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open Raman Dataset (PDP Type)"),ActiveDir->value("CURRENT_RAMAN_DIR").toString());
        if(!fileName.isEmpty())
        {
            ActiveDir->setValue("CURRENT_RAMAN_DIR",GetCurrentDir(fileName));
            ReloadSampleSpectrum(fileName,true);
            replot();
        }
    }
}

double HolmiumComparison::get_STD(QVector<double> &z)
{
    int i;
    double mean,std;
    QVector < double > LambdaCM(GSx->size()),OD(GSy->size());
    QVector < double > yVect(GSx->size()), statVect(GSx->size());

    for(i=0;i<GSx->size();i++)
    {
        LambdaCM[i]=1e7*(1/ui->vBox->value()-1/((*GSx)[i]*z[0]+z[1]));
        OD[i]=(GSy->at(i))/(mx)*(ui->mBox->value());
    }

    /******************  Convolution ***********************/

    QVector < double > *solvYPoint;
    QVector <double> ref_temp;
    if (ui->convoBox->isChecked())
    {
        convolve(&ref_temp);
        solvYPoint=&ref_temp;
    }
    else
    {
        solvYPoint=&solvY;
    }

    /*******************************************************/

    interpolate(&solvX,solvYPoint,&LambdaCM,&yVect);
    for(i=0;i<GSx->size();i++)
        statVect[i]=OD[i]-yVect[i];
    MeanAndSTD(&statVect,&mean, &std);
    return std;
}

void HolmiumComparison::on_clickButton_clicked()
{
    replot();

    QMessageBox box1;
    box1.setWindowTitle("Point selection");
    box1.setText("Select two furthest separated spectral points.\nMake sure that Raman pump is set correctly.");
    box1.exec();

    selCounter=0;
    selI.clear();
    selX.clear();
    selY.clear();
    selI.push_back(0);
    selX.push_back(0.0);
    selY.push_back(0.0);

    connect(ui->spectrum,SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(showSelectedPoints(QMouseEvent*)));
    connect(ui->spectrum,SIGNAL(mousePress(QMouseEvent*)),this,SLOT(addSelectedPoints(QMouseEvent*)));
}

void HolmiumComparison::on_convoBox_clicked(bool checked)
{
    replot();
}


void HolmiumComparison::on_convoValBox_valueChanged(double arg1)
{
    if (ui->convoBox->isChecked())
        replot();
}

void HolmiumComparison::convolve(QVector<double> *in)
{
    fft_gaussian_convolution(&solvX,&solvY,ui->convoValBox->value(),&solvX,in);
}
