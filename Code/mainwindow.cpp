#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    Spectrum(new SpectrumData)
{
    ui->setupUi(this);
    ui->solvBox->setDirSettings(&DirSettings);
    connect(ui->solvBox,SIGNAL(sendSolvent(QString)),this,SLOT(LoadReferenceSolvent(QString)));
    InitializeGraphs();
}

void MainWindow::LoadReferenceSolvent(QString solvFile)
{
    if (!Spectrum->ProbeTime.isEmpty())
    {
        QVector < double > x0, y0;
        readXYFile(solvFile,&x0,&y0);
        if (ui->convoBox->isChecked())
            fft_gaussian_convolution(&x0,&y0,ui->convoSpinBox->value(),&Spectrum->Wavelength,&Solvent);
        else
            interpolate(&x0,&y0,&Spectrum->Wavelength,&Solvent);
        replot_some(false,true,false);
    }
}

void MainWindow::InitializeGraphs()
{

    ui->specPlot->plotLayout()->clear(); // clear default axis rect so we can start from scratch

    QCPLayoutGrid *upperLayout = new QCPLayoutGrid;
    QCPLayoutGrid *lowerLayout = new QCPLayoutGrid;

    QCPAxisRect *subFSRS     = new QCPAxisRect(ui->specPlot, true);
    QCPAxisRect *subGS       = new QCPAxisRect(ui->specPlot, true);// false means to not setup default axes
    QCPAxisRect *subDiffFSRS = new QCPAxisRect(ui->specPlot, true);

    upperLayout->addElement(0, 0, subFSRS);
    upperLayout->addElement(0, 1, subGS);
    lowerLayout->addElement(0, 0, subDiffFSRS);

    ui->specPlot->plotLayout()->addElement(0, 0, upperLayout); // insert axis rect in first row
    ui->specPlot->plotLayout()->addElement(1, 0, lowerLayout); // sub layout in second row (grid layout will grow accordingly)

    subFSRS->setupFullAxesBox(true);
    subGS->setupFullAxesBox(true);
    subDiffFSRS->setupFullAxesBox(true);

    subFSRS->axis(QCPAxis::atBottom)->grid()->setVisible(true);
    subFSRS->axis(QCPAxis::atRight)->grid()->setVisible(true);
    subFSRS->axis(QCPAxis::atBottom)->grid()->setSubGridVisible(true);
    subFSRS->axis(QCPAxis::atRight)->grid()->setSubGridVisible(true);

    subGS->axis(QCPAxis::atBottom)->grid()->setVisible(true);
    subGS->axis(QCPAxis::atLeft)->grid()->setVisible(true);
    subGS->axis(QCPAxis::atBottom)->grid()->setSubGridVisible(true);
    subGS->axis(QCPAxis::atLeft)->grid()->setSubGridVisible(true);

    subDiffFSRS->axis(QCPAxis::atBottom)->grid()->setVisible(true);
    subDiffFSRS->axis(QCPAxis::atLeft)->grid()->setVisible(true);
    subDiffFSRS->axis(QCPAxis::atBottom)->grid()->setSubGridVisible(true);
    subDiffFSRS->axis(QCPAxis::atLeft)->grid()->setSubGridVisible(true);

    // synchronize the left and right margins of the top and bottom axis rects:
    QCPMarginGroup *marginGroup = new QCPMarginGroup(ui->specPlot);
    subFSRS->setMarginGroup(QCP::msTop | QCP::msLeft, marginGroup);
    subGS->setMarginGroup(QCP::msTop | QCP::msRight, marginGroup);
    subDiffFSRS->setMarginGroup(QCP::msBottom | QCP::msLeft | QCP::msRight, marginGroup);

    // move newly created axes on "axes" layer and grids on "grid" layer:
    ui->specPlot->setBackground(QColor(240,240,240));
    foreach (QCPAxisRect *rect, ui->specPlot->axisRects())
    {
      rect->setLayer("background");
      rect->setBackground(Qt::white);
      foreach (QCPAxis *axis, rect->axes())
      {
        axis->setLayer("axes");
        axis->grid()->setLayer("grid");
      }
    }

    /****************************
     *  0 - FSRS Curve (black)  *
     *  1 - Red Dots            *
     *  2 - Fit Curve (gray)    *
     ****************************/
    ui->specPlot->addGraph(subFSRS->axis(QCPAxis::atBottom), subFSRS->axis(QCPAxis::atLeft));
    ui->specPlot->addGraph(subFSRS->axis(QCPAxis::atBottom), subFSRS->axis(QCPAxis::atLeft));
    ui->specPlot->addGraph(subFSRS->axis(QCPAxis::atBottom), subFSRS->axis(QCPAxis::atLeft));

    QPen pen;
    pen.setWidth(2);
    pen.setColor(Qt::black);
    ui->specPlot->graph(0)->setPen(pen);

    pen.setColor(Qt::red);
    ui->specPlot->graph(1)->setPen(pen);
    ui->specPlot->graph(1)->setLineStyle((QCPGraph::LineStyle)0);
    ui->specPlot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 7));

    pen.setColor(Qt::gray);
    pen.setStyle(Qt::DashLine);
    ui->specPlot->graph(2)->setLineStyle((QCPGraph::LineStyle)1);
    ui->specPlot->graph(2)->setPen(pen);

    /********************************
     *  3 - FSRS corrected (black)  *
     *  4 - GS FSRS Correctd (gray) *
     *******************************/
    ui->specPlot->addGraph(subDiffFSRS->axis(QCPAxis::atBottom), subDiffFSRS->axis(QCPAxis::atLeft));
    pen.setColor(Qt::black);
    pen.setStyle(Qt::SolidLine);
    ui->specPlot->graph(3)->setPen(pen);

    ui->specPlot->addGraph(subDiffFSRS->axis(QCPAxis::atBottom), subDiffFSRS->axis(QCPAxis::atLeft));
    pen.setColor(Qt::gray);
    pen.setStyle(Qt::DashDotLine);
    ui->specPlot->graph(4)->setLineStyle((QCPGraph::LineStyle)1);
    ui->specPlot->graph(4)->setPen(pen);

    /**************************
     *  5 - GS FSRS (black)   *
     *  6 - Red Dots          *
     *  7 - Fit Curve (gray)  *
     *************************/
    ui->specPlot->addGraph(subGS->axis(QCPAxis::atBottom), subGS->axis(QCPAxis::atLeft));
    ui->specPlot->addGraph(subGS->axis(QCPAxis::atBottom), subGS->axis(QCPAxis::atLeft));
    ui->specPlot->addGraph(subGS->axis(QCPAxis::atBottom), subGS->axis(QCPAxis::atLeft));

    pen.setColor(Qt::black);
    pen.setStyle(Qt::SolidLine);
    ui->specPlot->graph(5)->setPen(pen);

    pen.setColor(Qt::red);
    ui->specPlot->graph(6)->setPen(pen);
    ui->specPlot->graph(6)->setLineStyle((QCPGraph::LineStyle)0);
    ui->specPlot->graph(6)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 7));

    pen.setColor(Qt::gray);
    pen.setStyle(Qt::DashLine);
    ui->specPlot->graph(7)->setLineStyle((QCPGraph::LineStyle)1);
    ui->specPlot->graph(7)->setPen(pen);

    x1=subFSRS->axis(QCPAxis::atBottom);
    y1=subFSRS->axis(QCPAxis::atLeft);
    x2=subDiffFSRS->axis(QCPAxis::atBottom);
    y2=subDiffFSRS->axis(QCPAxis::atLeft);
    x3=subGS->axis(QCPAxis::atBottom);
    y3=subGS->axis(QCPAxis::atLeft);

    /**************************
     *  8 - Aux FSRS          *
     *  9 - Aux GS FSRS       *
     *************************/
    ui->specPlot->addGraph(subFSRS->axis(QCPAxis::atBottom), subFSRS->axis(QCPAxis::atLeft));
    ui->specPlot->addGraph(subGS->axis(QCPAxis::atBottom), subGS->axis(QCPAxis::atLeft));
    pen.setColor(Qt::darkRed);
    pen.setStyle(Qt::DashDotLine);
    ui->specPlot->graph(8)->setLineStyle((QCPGraph::LineStyle)1);
    ui->specPlot->graph(8)->setPen(pen);
    ui->specPlot->graph(9)->setLineStyle((QCPGraph::LineStyle)1);
    ui->specPlot->graph(9)->setPen(pen);

    /**************************
     *  10 - Aux FSRS Dots    *
     *  11 - Aux FSRS Fit     *
     *  12 - Aux GS FSRS Dots *
     *  13 - Aux GS FSRS Fit  *
     *************************/

    ui->specPlot->addGraph(subFSRS->axis(QCPAxis::atBottom), subFSRS->axis(QCPAxis::atLeft));
    ui->specPlot->addGraph(subFSRS->axis(QCPAxis::atBottom), subFSRS->axis(QCPAxis::atLeft));
    ui->specPlot->addGraph(subGS->axis(QCPAxis::atBottom), subGS->axis(QCPAxis::atLeft));
    ui->specPlot->addGraph(subGS->axis(QCPAxis::atBottom), subGS->axis(QCPAxis::atLeft));

    QColor redAlpha(Qt::red);
    redAlpha.setAlpha(125);

    pen.setColor(redAlpha);
    pen.setStyle(Qt::SolidLine);
    ui->specPlot->graph(10)->setPen(pen);
    ui->specPlot->graph(10)->setLineStyle((QCPGraph::LineStyle)0);
    ui->specPlot->graph(10)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 7));
    ui->specPlot->graph(12)->setPen(pen);
    ui->specPlot->graph(12)->setLineStyle((QCPGraph::LineStyle)0);
    ui->specPlot->graph(12)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 7));

    pen.setColor(Qt::gray);
    pen.setStyle(Qt::DashLine);
    ui->specPlot->graph(11)->setLineStyle((QCPGraph::LineStyle)1);
    ui->specPlot->graph(11)->setPen(pen);
    ui->specPlot->graph(13)->setLineStyle((QCPGraph::LineStyle)1);
    ui->specPlot->graph(13)->setPen(pen);

    connect(ui->specPlot,SIGNAL(mousePress(QMouseEvent*)),this,SLOT(clickedGraph(QMouseEvent*)));
    connect(ui->specPlot,SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(showPointToolTip(QMouseEvent*)));
    ui->specPlot->setInteractions(QCP::iRangeZoom | QCP::iRangeDrag);
}

void MainWindow::showPointToolTip(QMouseEvent *event)
{
    double xx1 = x1->pixelToCoord(event->pos().x()),
           yy1 = y1->pixelToCoord(event->pos().y()),
           xx2 = x2->pixelToCoord(event->pos().x()),
           yy2 = y2->pixelToCoord(event->pos().y()),
           xx3 = x3->pixelToCoord(event->pos().x()),
           yy3 = y3->pixelToCoord(event->pos().y());
    if (xx1 >= x1->range().lower &&  xx1 <= x1->range().upper && yy1 >= y1->range().lower &&  yy1 <= y1->range().upper )
        QToolTip::showText(ui->specPlot->mapToGlobal(event->pos()), QString("%1 , %2").arg(xx1,0,'f',2).arg(yy1,0,'f',2));
    else if (xx2 >= x2->range().lower &&  xx2 <= x2->range().upper && yy2 >= y2->range().lower &&  yy2 <= y2->range().upper )
        QToolTip::showText(ui->specPlot->mapToGlobal(event->pos()), QString("%1 , %2").arg(xx2,0,'f',2).arg(yy2,0,'f',2));
    else if (xx3 >= x3->range().lower &&  xx3 <= x3->range().upper && yy3 >= y3->range().lower &&  yy3 <= y3->range().upper )
        QToolTip::showText(ui->specPlot->mapToGlobal(event->pos()), QString("%1 , %2").arg(xx3,0,'f',2).arg(yy3,0,'f',2));
}

MainWindow::~MainWindow()
{
    delete Spectrum;
    delete ui;
}

void MainWindow::on_actionOpen_PDP_Dataset_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Raman Dataset (PDP Type)"),DirSettings.value("CURRENT_RAMAN_DIR").toString());
    if(!fileName.isEmpty())
    {
         DirSettings.setValue("CURRENT_RAMAN_DIR",GetCurrentDir(fileName));

         FILE *pFile;
         QString Header;
         char mystring[MAXLINELENGTH];

         pFile=fopen(fileName.toLatin1(),"r");
         fgets(mystring,MAXLINELENGTH,pFile);
         Header=mystring;

         if ( Header.contains("Pump-Dump-Probe",Qt::CaseInsensitive) || Header.contains("PumpDump-Probe",Qt::CaseInsensitive) ||
              Header.contains("PumpDumpProbe",Qt::CaseInsensitive) )
         {
             bool Ref;
             PDPTempData TempData;
             QVector< double > bksmp00,bksmp01,bksmp10,bksmp11;
             QVector< double > PumpTimes,DumpTimes,TV_00,TV_01,TV_10,TV_11;
             QVector< QVector < double > > WL00,WL01,WL10,WL11;
             QVector < QString > NansAndZeros;

             fpos_t position;
             QString CurrType;
             int i,j,k,CurrScanNo,n=0,HowMany=1,npix=256;
             double CurrDelayPump,CurrDelayDump;

             Ref = (!Header.contains("Not referenced",Qt::CaseInsensitive));

             Header = "Checking...";
             while(!Header.contains("scan 1",Qt::CaseInsensitive))
             {
                 fgetpos(pFile,&position);
                 fgets(mystring,MAXLINELENGTH,pFile);
                 Header=mystring;
             }

             /**************/
             /* Pixel hack */
             /**************/
             fgets(mystring,MAXLINELENGTH,pFile);
             fgets(mystring,MAXLINELENGTH,pFile);
             Header=mystring;
             npix=Header.count("\t")+1;
             // Kvaila, bet efektyvu.
             // QStringList count_list = Header.split('\t',QString::SkipEmptyParts);
             // npix = count_list.size();
             fsetpos(pFile,&position);

             /**********************/
             /* Resize all vectors */
             /**********************/
             if(Ref)
             {
                 bksmp00.fill(0.0,npix);
                 bksmp01.fill(0.0,npix);
                 bksmp10.fill(0.0,npix);
                 bksmp11.fill(0.0,npix);
             }
             TempData.smp00.resize(npix);
             TempData.smp01.resize(npix);
             TempData.smp10.resize(npix);
             TempData.smp11.resize(npix);
             TV_00.resize(npix);
             TV_01.resize(npix);
             TV_10.resize(npix);
             TV_11.resize(npix);
             Spectrum->ClearSpectra();
             Spectrum->Wavelength.resize(npix);

             ReadPDPPoint(pFile,&TempData,&NansAndZeros,npix,Ref);
             CurrType=TempData.NextType;
             CurrScanNo=TempData.NextScanNo;
             CurrDelayPump=TempData.NextDelayPump;
             CurrDelayDump=TempData.NextDelayDump;

             while(!TempData.FileEnd)
             {
                 ReadPDPPoint(pFile,&TempData,&NansAndZeros,npix,Ref);

                 if(TempData.FileEnd)
                 {
                     WL00.push_back( QVector<double>(npix) );
                     WL01.push_back( QVector<double>(npix) );
                     WL10.push_back( QVector<double>(npix) );
                     WL11.push_back( QVector<double>(npix) );
                     for(i=0;i<npix;i++)
                     {
                         WL00[n][i]=TempData.smp00[i]-bksmp00[i];
                         WL01[n][i]=TempData.smp01[i]-bksmp01[i];
                         WL10[n][i]=TempData.smp10[i]-bksmp10[i];
                         WL11[n][i]=TempData.smp11[i]-bksmp11[i];
                         Spectrum->Wavelength[i]=i+1;
                     }
                     PumpTimes.push_back(CurrDelayPump);
                     DumpTimes.push_back(CurrDelayDump);
                     n++;
                     if(CurrScanNo<2) HowMany=n;
                     break;
                 }
                 if(CurrType.compare("Background")==0)
                 {
                     bksmp00=TempData.smp00;
                     bksmp01=TempData.smp01;
                     bksmp10=TempData.smp10;
                     bksmp11=TempData.smp11;
                 }
                 if(CurrType.compare("Measurement")==0)
                 {
                     WL00.push_back( QVector<double>(npix) );
                     WL01.push_back( QVector<double>(npix) );
                     WL10.push_back( QVector<double>(npix) );
                     WL11.push_back( QVector<double>(npix) );
                     for(i=0;i<npix;i++)
                     {
                         WL00[n][i]=TempData.smp00[i]-bksmp00[i];
                         WL01[n][i]=TempData.smp01[i]-bksmp01[i];
                         WL10[n][i]=TempData.smp10[i]-bksmp10[i];
                         WL11[n][i]=TempData.smp11[i]-bksmp11[i];
                     }
                     PumpTimes.push_back(CurrDelayPump);
                     DumpTimes.push_back(CurrDelayDump);
                     n++;
                 }
                 if(CurrScanNo<2) HowMany=n;
                 CurrType=TempData.NextType;
                 CurrScanNo=TempData.NextScanNo;
                 CurrDelayPump=TempData.NextDelayPump;
                 CurrDelayDump=TempData.NextDelayDump;
             }
             fclose(pFile);

             // Dabar galima ideti ir Pumptimes
             for (i=0;i<n;i++)
             {
                 for (j=0;j<npix;j++)
                 {
                     if (WL00[i][j]<1e-20)
                     {
                         NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(DumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                         WL00[i][j]=1e-20;
                     }
                     if (WL01[i][j]<1e-20)
                     {
                         NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(DumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                         WL01[i][j]=1e-20;
                     }
                     if (WL10[i][j]<1e-20)
                     {
                         NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(DumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                         WL10[i][j]=1e-20;
                     }
                     if (WL11[i][j]<1e-20)
                     {
                         NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(DumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                         WL11[i][j]=1e-20;
                     }
                 }
             }

             bool Unfinished=(n!=CurrScanNo*HowMany);
             if(Unfinished)
             {
                 for(i=0;i<n-(CurrScanNo-1)*HowMany;i++)
                 {
                     PumpTimes.pop_back();
                     DumpTimes.pop_back();
                     WL00.pop_back();
                     WL01.pop_back();
                     WL10.pop_back();
                     WL11.pop_back();
                 }
                 CurrScanNo--;
             }
             if (Unfinished||(!NansAndZeros.isEmpty()))
             {
                 QString UnfMeas="Unfinished measurement. Displaying only the completed scans.";
                 if (NansAndZeros.isEmpty())
                 {
                     QMessageBox msgBox;
                     msgBox.setText(UnfMeas);
                     msgBox.exec();
                 }
                 else
                 {
                     if (Unfinished) NansAndZeros.push_front(UnfMeas);
                     NaNReportDialog *nanReport;
                     nanReport=new NaNReportDialog(this,&NansAndZeros);
                     nanReport->setAttribute(Qt::WA_DeleteOnClose,true);
                     nanReport->setModal(true);
                     nanReport->exec();
                 }
             }

             /********************************/
             /****** Delay line selection ****/
             /********************************/

             double TempPT, TempDT, Mean_PT, Mean_DT, STD_PT, STD_DT;
             bksmp00.resize(HowMany); // Pumpdelays
             bksmp01.resize(HowMany); // Dumpdelays
             for(j=0;j<HowMany;j++)
             {
                 TempPT=TempDT=0.0;
                 for(i=0;i<CurrScanNo;i++)
                 {
                     TempPT+=PumpTimes[j+i*HowMany];
                     TempDT-=DumpTimes[j+i*HowMany];
                 }
                 bksmp00[j]=TempPT/CurrScanNo;
                 bksmp01[j]=TempDT/CurrScanNo;
             }
             MeanAndSTD(&bksmp00,&Mean_PT,&STD_PT);
             MeanAndSTD(&bksmp01,&Mean_DT,&STD_DT);
             if(STD_PT>=STD_DT)
                 Spectrum->ProbeTime=bksmp00;
             else
                 Spectrum->ProbeTime=bksmp01;

             /********************************/
             /********************************/
             /********************************/

             shell_sort_Raman(Spectrum->ProbeTime,WL00,WL01,WL10,WL11,CurrScanNo);

             //qDebug() << t.elapsed();

             ////////////////////////////
             //// Palyginimo langas: ////
             ////////////////////////////

             QVector < double > Weights;
             double summ=0.0;
             Weights.fill(1,CurrScanNo);

             QVector < QVector < double > > LogGain=WL11;
             for(i=0;i<LogGain.size();i++)
                 for(j=0;j<npix;j++)
                     LogGain[i][j]=-1000.0*log10(LogGain[i][j]/WL01[i][j]);

             CompareTraces *compWindow;
             compWindow=new CompareTraces(this,&LogGain,&Spectrum->ProbeTime,CurrScanNo,&Weights);
             compWindow->setAttribute(Qt::WA_DeleteOnClose,true);
             compWindow->setModal(true);
             compWindow->exec();

             ////////////////////////////
             ////////////////////////////
             ////////////////////////////

             for(i=0;i<CurrScanNo;i++)
                 summ += Weights[i];
             if (summ==0) summ=1;

             for(j=0;j<HowMany;j++)
             {
                 TV_00.fill(0.0);
                 TV_01.fill(0.0);
                 TV_10.fill(0.0);
                 TV_11.fill(0.0);
                 for(i=0;i<CurrScanNo;i++)
                 {
                     for(k=0;k<npix;k++)
                     {
                         TV_00[k]+=WL00[j+i*HowMany][k]*Weights[i]/summ;
                         TV_01[k]+=WL01[j+i*HowMany][k]*Weights[i]/summ;
                         TV_10[k]+=WL10[j+i*HowMany][k]*Weights[i]/summ;
                         TV_11[k]+=WL11[j+i*HowMany][k]*Weights[i]/summ;
                     }
                 }
                 Spectrum->WL00.push_back(TV_00);
                 Spectrum->WL01.push_back(TV_01);
                 Spectrum->WL10.push_back(TV_10);
                 Spectrum->WL11.push_back(TV_11);
             }

             Spectrum->FSRS=Spectrum->WL11;
             for(i=0;i<Spectrum->ProbeTime.size();i++)
                 for(j=0;j<Spectrum->Wavelength.size();j++)
                     Spectrum->FSRS[i][j]=1000.0*log10(Spectrum->WL11[i][j]*Spectrum->WL00[i][j]/Spectrum->WL01[i][j]/Spectrum->WL10[i][j]);

             Spectrum->Type=PDPRaw;

             ui->delaySlider->setMinimum(0);
             ui->delaySlider->setMaximum(Spectrum->ProbeTime.size()-1);
             ui->zeroSpinBox->setMinimum(Spectrum->ProbeTime.first());
             ui->zeroSpinBox->setMaximum(Spectrum->ProbeTime.last());
             setWindowTitle(fileName);
             if (!Ref) reverse_wavenumber();
             replot_some();
         }
         else
         {
             fclose(pFile);
             QMessageBox msgBox;
             msgBox.setText("Invalid file.");
             msgBox.exec();
         }
    }
}

void MainWindow::ReadPDPPoint(FILE* pFile, PDPTempData *Temp, QVector < QString > *NaNs, int npix, bool Reference)
{
    QString Str;
    QStringList List;
    bool Unfinished=true;
    int i, n00, n01, n10, n11;
    double point;
    char LongLine[MAXLINELENGTH];

    Temp->smp00.fill(0);
    Temp->smp01.fill(0);
    Temp->smp10.fill(0);
    Temp->smp11.fill(0);
    n00=n01=n10=n11=0;

    while(Unfinished)
    {
        Temp->FileEnd=false;
        if(fgets(LongLine,MAXLINELENGTH,pFile)==NULL)
        {
            Temp->FileEnd=true;
            break;
        }
        if(strncmp(LongLine,"Pump",2)==0)
        {
            fscanf(pFile,"%lf",&point);
            if (!_isnan(point))
            {
                Temp->smp00[0]+=point;
                for(i=1;i<npix;i++)
                {
                    fscanf(pFile,"%lf",&point);
                    Temp->smp00[i]+=point;
                }
                n00++;
            }
            else
            {
                NaNs->append("NaNs in scan " + QString::number(Temp->NextScanNo) + " @ " + QString::number(Temp->NextDelayDump,'f',2) + " ps");
            }
            if (!Reference)  fgets(LongLine,MAXLINELENGTH,pFile);
            fgets(LongLine,MAXLINELENGTH,pFile);

            fscanf(pFile,"%lf",&point);
            if (!_isnan(point))
            {
                Temp->smp01[0]+=point;
                for(i=1;i<npix;i++)
                {
                    fscanf(pFile,"%lf",&point);
                    Temp->smp01[i]+=point;
                }
                n01++;
            }
            else
            {
                NaNs->append("NaNs in scan " + QString::number(Temp->NextScanNo) + " @ " + QString::number(Temp->NextDelayDump,'f',2) + " ps");
            }
            if (!Reference)  fgets(LongLine,MAXLINELENGTH,pFile);
            fgets(LongLine,MAXLINELENGTH,pFile);

            fscanf(pFile,"%lf",&point);
            if (!_isnan(point))
            {
                Temp->smp10[0]+=point;
                for(i=1;i<npix;i++)
                {
                    fscanf(pFile,"%lf",&point);
                    Temp->smp10[i]+=point;
                }
                n10++;
            }
            else
            {
                NaNs->append("NaNs in scan " + QString::number(Temp->NextScanNo) + " @ " + QString::number(Temp->NextDelayDump,'f',2) + " ps");
            }
            if (!Reference)  fgets(LongLine,MAXLINELENGTH,pFile);
            fgets(LongLine,MAXLINELENGTH,pFile);

            fscanf(pFile,"%lf",&point);
            if (!_isnan(point))
            {
                Temp->smp11[0]+=point;
                for(i=1;i<npix;i++)
                {
                    fscanf(pFile,"%lf",&point);
                    Temp->smp11[i]+=point;
                }
                n11++;
            }
            else
            {
                NaNs->append("NaNs in scan " + QString::number(Temp->NextScanNo) + " @ " + QString::number(Temp->NextDelayDump,'f',2) + " ps");
            }
            if (!Reference)  fgets(LongLine,MAXLINELENGTH,pFile);
            fgets(LongLine,MAXLINELENGTH,pFile);
        }
        else
        {
            Unfinished=false;
            Str=LongLine;
            Str.remove(",");
            List=Str.split(" ",QString::SkipEmptyParts);
            Temp->NextType=List.at(0);
            Temp->NextScanNo=List.at(2).toInt();
            if (List.size()>6)
            {
                // Musu PDP failams:
                Temp->NextDelayPump=List.at(4).toDouble();
                Temp->NextDelayDump=List.at(6).toDouble();
            }
            else
            {
                // Miko PDP failams:
                Temp->NextDelayPump=List.at(4).toDouble();
                Temp->NextDelayDump=List.at(5).toDouble();
            }
        }
    }

    for(i=0;i<npix;i++)
    {
        Temp->smp00[i]/=n00;
        Temp->smp01[i]/=n01;
        Temp->smp10[i]/=n10;
        Temp->smp11[i]/=n11;
    }
}

inline int MainWindow::my_isinf(double x)
{
   volatile double temp = x;
   if ((temp == x) && ((temp - x) != 0.0))
      return (x < 0.0 ? -1 : 1);
   else return 0;
}

void MainWindow::on_delaySlider_valueChanged(int value)
{
    replot_some();
}

void MainWindow::replot_some(bool rscFSRS,bool rscCFSRS,bool rscGS)
{
    if (!Spectrum->ProbeTime.isEmpty())
    {
        QVector <double> Raman,aux_Raman;
        bool aux_exists=((Spectrum->Type == PDPRawAux) || (Spectrum->Type == PPRawAux));
        double mn_lim,mx_lim,dlim=(Spectrum->Wavelength.last()-Spectrum->Wavelength.first())*REPLOT_PERC;

        ui->delayLabel->setText(QString::number(Spectrum->ProbeTime[ui->delaySlider->value()],'f',2));

        replot_gs(rscGS);
        get_FSRS(true,ui->delaySlider->value(),&Raman,(aux_exists ? &aux_Raman : NULL));

        ui->specPlot->graph(0)->setData(Spectrum->Wavelength,Raman);
        if (aux_exists)
            ui->specPlot->graph(8)->setData(Spectrum->Wavelength,aux_Raman);
        else
            ui->specPlot->graph(8)->clearData();

        if (rscFSRS)
        {
            below_min_and_above_max_proportional(&Raman,&mn_lim,&mx_lim,4.0);
            x1->setRange(Spectrum->Wavelength.first()-dlim,Spectrum->Wavelength.last()+dlim);
            if (aux_exists)
            {
                double mn_lim_a, mx_lim_a;
                below_min_and_above_max_proportional(&aux_Raman,&mn_lim_a,&mx_lim_a,4.0);
                mn_lim = (mn_lim < mn_lim_a ? mn_lim : mn_lim_a);
                mx_lim = (mx_lim > mx_lim_a ? mx_lim : mx_lim_a);
            }
            y1->setRange(mn_lim,mx_lim);
        }

        ui->specPlot->graph(3)->setData(Spectrum->Wavelength,ExcState);
        if (rscCFSRS)
        {
            below_min_and_above_max_proportional(&ExcState,&mn_lim,&mx_lim,4.0);
            x2->setRange(Spectrum->Wavelength.first()-dlim,Spectrum->Wavelength.last()+dlim);
            y2->setRange(mn_lim,mx_lim);
        }

        ui->specPlot->replot();
    }
}

inline void MainWindow::get_FSRS_spectrum(int delay, QVector < double > *FSRS, QVector < double > *AUX)
{
    // Kol kas patikrinimai reikalingi, nes get_FSRS gali buti kvieciama be AUX spektro (pvz difference file).
    bool auxExist=(AUX!=NULL);
    for (int i=0;i<Spectrum->Wavelength.size();i++)
    {
        switch (Spectrum->Type)
        {
            case 1:
            {
                (*FSRS)[i]=Spectrum->FSRS[delay][i];
                if (ui->linButton->isChecked()) (*FSRS)[i]=pow(10.0,(*FSRS)[i]/1000);
                break;
            }
            case 3:
            {
                if(auxExist)
                {
                    (*AUX)[i]=Spectrum->AUX01[delay][i]/Spectrum->WL00[delay][i];
                    if (ui->logButton->isChecked()) (*AUX)[i]=1000.0*log10((*AUX)[i]);
                }
            }
            case 2:
            {
                (*FSRS)[i]=Spectrum->WL01[delay][i]/Spectrum->WL00[delay][i];
                if (ui->logButton->isChecked()) (*FSRS)[i]=1000.0*log10((*FSRS)[i]);
                break;
            }
            case 6:
            {
                if(auxExist)
                {
                    (*AUX)[i]=Spectrum->AUX11[delay][i]*Spectrum->WL00[delay][i]/Spectrum->AUX01[delay][i]/Spectrum->WL10[delay][i];
                    if (ui->logButton->isChecked()) (*AUX)[i]=1000.0*log10((*AUX)[i]);
                }
            }
            case 4:
            {
                (*FSRS)[i]=Spectrum->WL11[delay][i]*Spectrum->WL00[delay][i]/Spectrum->WL01[delay][i]/Spectrum->WL10[delay][i];
                if (ui->logButton->isChecked()) (*FSRS)[i]=1000.0*log10((*FSRS)[i]);
                break;
            }
            case 8:
            default: break;
        }
    }
}

inline void MainWindow::get_GS_FSRS_spectrum(int delay, QVector < double > *FSRS, QVector < double > *AUX)
{
    bool auxExist=(AUX!=NULL);
    for (int i=0;i<Spectrum->Wavelength.size();i++)
    {
        switch (Spectrum->Type)
        {
            case 1:
            {
                (*FSRS)[i]=Spectrum->FSRS[delay][i];
                if (ui->linButton->isChecked()) (*FSRS)[i]=pow(10.0,(*FSRS)[i]/1000);
                break;
            }
            case 3:
            {
                if(auxExist)
                {
                    (*AUX)[i]=Spectrum->AUX01[delay][i]/Spectrum->WL00[delay][i];
                    if (ui->logButton->isChecked()) (*AUX)[i]=1000.0*log10((*AUX)[i]);
                }
            }
            case 2:
            {
                (*FSRS)[i]=Spectrum->WL01[delay][i]/Spectrum->WL00[delay][i];
                if (ui->logButton->isChecked()) (*FSRS)[i]=1000.0*log10((*FSRS)[i]);
                break;
            }
            case 6:
            {
                if (auxExist)
                {
                    (*AUX)[i]=Spectrum->AUX01[delay][i]/Spectrum->WL00[delay][i];
                    if (ui->logButton->isChecked()) (*AUX)[i]=1000.0*log10((*AUX)[i]);
                }
            }
            case 4:
            {
                (*FSRS)[i]=Spectrum->WL01[delay][i]/Spectrum->WL00[delay][i];
                if (ui->logButton->isChecked()) (*FSRS)[i]=1000.0*log10((*FSRS)[i]);
                break;
            }
            case 8:
            default: break;
        }
    }
}

inline void MainWindow::get_zero_spectrum(QVector < double > *FSRS, QVector < double > *AUX)
{
    int st;
    double temp,zero,zeroAux;
    bool auxExists=(AUX!=NULL);

    locate(&Spectrum->ProbeTime,ui->zeroSpinBox->value(),&st);
    if (st==0) st=1;

    for (int i=0;i<Spectrum->Wavelength.size();i++)
    {
        zero=0.0; zeroAux=0.0;
        for(int j=0;j<st;j++)
        {
            switch (Spectrum->Type)
            {
                case 1:
                {
                    zero+=(ui->logButton->isChecked() ? Spectrum->FSRS[j][i] : pow(10.0,Spectrum->FSRS[j][i]/1000.0));
                    break;
                }
                case 3:
                {
                    if(auxExists)
                    {
                        temp = Spectrum->AUX01[j][i]*Spectrum->WL00[j][i];
                        zeroAux+=(ui->logButton->isChecked() ? 1000.0*log10(temp) : temp);
                    }
                }
                case 2:
                {
                    temp = Spectrum->WL01[j][i]/Spectrum->WL00[j][i];
                    zero+=(ui->logButton->isChecked() ? 1000.0*log10(temp) : temp);
                    break;
                }
                case 6:
                {
                    if(auxExists)
                    {
                        temp = Spectrum->AUX11[j][i]*Spectrum->WL00[j][i]/Spectrum->AUX01[j][i]/Spectrum->WL10[j][i];
                        zeroAux+=(ui->logButton->isChecked() ? 1000.0*log10(temp) : temp);
                    }
                }
                case 4:
                {
                    temp = Spectrum->WL11[j][i]*Spectrum->WL00[j][i]/Spectrum->WL01[j][i]/Spectrum->WL10[j][i];
                    zero+=(ui->logButton->isChecked() ? 1000.0*log10(temp) : temp);
                    break;
                }
                case 8:
                default: break;
            }
        }
        (*FSRS)[i]-=(zero/double(st));
        if (auxExists) (*AUX)[i]-=(zeroAux/double(st));
        if(ui->linButton->isChecked())
        {
            (*FSRS)[i]++;
            if (auxExists) (*AUX)[i]++;
        }
    }
}

inline void MainWindow::get_poly(QVector < double > *FSRS, QVector < double > *c_FSRS, bool plot, bool excited, bool AUX)
{
    int i,j;
    double chi;
    bool polyfit,isolated, splineCheck = (excited ? ui->splineFSRSBox->isChecked() : ui->splineGSBox->isChecked());
    QVector < int > *indexes = (excited ? &zero_indexes : &gs_zero_indexes);
    QVector < double > ScatterX, ScatterY, scaled_xs, coeffs,norm_x(Spectrum->Wavelength.size());
    int g1, g2, g_code = (excited ? 20 : 10) + (AUX ? 1 : 2);
    switch (g_code)
    {
        case 11: {g1=12;g2=13; break;}
        case 12: {g1= 6;g2= 7; break;}
        case 21: {g1=10;g2=11; break;}
        case 22: {g1= 1;g2= 2; break;}
        default:  break;
    }

    if (!indexes->isEmpty())
    {
        ScatterX.resize(indexes->size());
        ScatterY.resize(indexes->size());
        scaled_xs.resize(indexes->size());
        for(i=0;i<indexes->size();i++)
        {
            ScatterX[i]=Spectrum->Wavelength[(*indexes)[i]];
            ScatterY[i]=(*FSRS)[(*indexes)[i]];
            scaled_xs[i]=(Spectrum->Wavelength[(*indexes)[i]]-Spectrum->Wavelength.first())/(Spectrum->Wavelength.last()-Spectrum->Wavelength.first());
        }
        if (plot) ui->specPlot->graph(g1)->setData(ScatterX,ScatterY);
    }
    else
    {
        if (plot) ui->specPlot->graph(g1)->clearData();
    }

    // Griozdas, bet veikia...
    polyfit=(excited ?
              (((!ui->splineFSRSBox->isChecked()) && (zero_indexes.size()>=ui->polyBox->value())) ||
               ((ui->splineFSRSBox->isChecked())  && (zero_indexes.size()>=4)) ||
               ((ui->isolateFSRSBox->isChecked()) && (zero_indexes.size()>=2) && (zero_indexes.size()%2==0))) :
              ((gs_zero_indexes.size()>=ui->polyBox->value()) ||
               ((ui->splineGSBox->isChecked())  && (gs_zero_indexes.size()>=4))  ||
               ((ui->isolateGSBox->isChecked()) && (gs_zero_indexes.size()%2==0) && (!gs_zero_indexes.isEmpty()))) );
    isolated=(excited ?
               ui->isolateFSRSBox->isChecked() && (zero_indexes.size()%2==0) && (!zero_indexes.isEmpty()) :
               (ui->isolateGSBox->isChecked()) && (gs_zero_indexes.size()%2==0) && (!gs_zero_indexes.isEmpty()) );
    // Aproksimuojame, jeigu viskas yra tvarkoje:
    if(polyfit)
    {
        // Izoliuotu piku rezimas.
        // Truputi neefektyvu del masyvu perrasymo, bet gudriau kol kas nesugalvoju.
        if (isolated)
        {
            bool add;
            int k=0;
            scaled_xs.clear();
            ScatterY.clear();
            for(i=0;i<Spectrum->Wavelength.size();i++)
            {
                add=true;
                for(j=0;j<(indexes->size()/2);j++)
                {
                    if((i>=(*indexes)[2*j])&&(i<=(*indexes)[2*j+1]))
                    {
                        add=false;
                        break;
                    }
                }
                if(add)
                {
                    if(k%ui->isoBox->value()==0)
                    {
                        scaled_xs.push_back((Spectrum->Wavelength[i]-Spectrum->Wavelength.first())/(Spectrum->Wavelength.last()-Spectrum->Wavelength.first()));
                        ScatterY.push_back((*FSRS)[i]);
                    }
                    k++;
                }
            }
        }

        for(i=0;i<Spectrum->Wavelength.size();i++)
            norm_x[i]=(Spectrum->Wavelength[i]-Spectrum->Wavelength.first())/(Spectrum->Wavelength.last()-Spectrum->Wavelength.first());

        if(splineCheck)
        {
            QVector < double > SplineY(Spectrum->Wavelength.size());
            Spline_interp spl(scaled_xs,ScatterY);
            for (i=0;i<Spectrum->Wavelength.size();i++)
                SplineY[i]=spl.interp(norm_x[i]);
            if (plot) ui->specPlot->graph(g2)->setData(Spectrum->Wavelength,SplineY);
            for(i=0;i<Spectrum->Wavelength.size();i++)
                (*c_FSRS)[i]=(*FSRS)[i]-SplineY[i];
        }
        else
        {
            svdfit_wrapper(scaled_xs,ScatterY,coeffs,ui->polyBox->value(),&chi);
            ScatterY.clear();
            polynomialRecursionLin(&norm_x,&coeffs,&ScatterY);
            if (plot) ui->specPlot->graph(g2)->setData(Spectrum->Wavelength,ScatterY);
            for(i=0;i<Spectrum->Wavelength.size();i++)
                (*c_FSRS)[i]=(*FSRS)[i]-ScatterY[i];
        }
    }
    else
    {
        // Jeigu nefitiname, tai tiesiog nupiesiame (jei piesiame) nekoreguota signala:
        if (plot) ui->specPlot->graph(g2)->clearData();
        (*c_FSRS)=(*FSRS);
    }
}


void MainWindow::get_FSRS(bool plot, int delay, QVector<double> *FSRS, QVector<double> *AUX)
{
    int i,j;
    QVector <double> temp;
    bool auxExists=(AUX!=NULL);

    // Vektoriu dydziu perskirstymas:
    if(FSRS->size()!=Spectrum->Wavelength.size()) FSRS->resize(Spectrum->Wavelength.size());
    if(ExcState.size()!=Spectrum->Wavelength.size()) ExcState.resize(Spectrum->Wavelength.size());
    if (auxExists)
    {
        if (AUX->size()!=Spectrum->Wavelength.size()) AUX->resize(Spectrum->Wavelength.size());
        if (AuxExcState.size()!=Spectrum->Wavelength.size()) AuxExcState.resize(Spectrum->Wavelength.size());
    }

    // FSRS duomenu paskaiciavimas ties tam tikru velinimu.
    get_FSRS_spectrum(delay,FSRS,AUX);

    // Spektro pries t=0 atemimas is paskaiciuotojo spektro.
    if (ui->zeroBox->isChecked()) get_zero_spectrum(FSRS,AUX);

    // Pagraziname duomenis:
    if(ui->smoothBox->isChecked())
    {
        (*FSRS)=savgolfilter(FSRS,ui->windowSpinBox->value(),ui->windowSpinBox->value(),0,ui->degSpinBox->value());
        if (auxExists) (*AUX)=savgolfilter(AUX,ui->windowSpinBox->value(),ui->windowSpinBox->value(),0,ui->degSpinBox->value());
    }

    // Randame polinoma:
    get_poly(FSRS,&ExcState,plot,true);
    if (auxExists) get_poly(AUX,&AuxExcState,plot,true,true);

    //Padauginame is svoriu:
    if(!GaussianMults.isEmpty())
    {
        for(i=0;i<GaussianMults.size();i+=2)
            for (j=0;j<Spectrum->Wavelength.size();j++)
            {
                ExcState[j]=ExcState[j]*(1.0-exp(-SQR((Spectrum->Wavelength[j]-GaussianMults[i])/GaussianMults[i+1])));
                if (auxExists) AuxExcState[j]=AuxExcState[j]*(1.0-exp(-SQR((Spectrum->Wavelength[j]-GaussianMults[i])/GaussianMults[i+1])));
            }
    }

    double mult,nonzero_div=max(&GroundState);
    if (fabs(nonzero_div)<1.0e-100) nonzero_div=max(&GroundState);
    if (fabs(nonzero_div)<1.0e-100)
    {
        nonzero_div=min(&GroundState);
        if(fabs(nonzero_div)<1.0e-100) nonzero_div=1.0e-50;
    }
    if((ui->subGSBox->isChecked())&&(!GroundState.isEmpty()))
    {
        mult=ExcState[max_ind]/nonzero_div ;
        for(i=0;i<ExcState.size();i++)
            ExcState[i]-=mult*GroundState[i];

    }
    else if (ui->refBox->isChecked())
    {
        if (Solvent.isEmpty()) LoadReferenceSolvent(ui->solvBox->getCurrentSolvent());
        int mi;
        double mx_slv=max_index(&Solvent,&mi),
               mx_fsrs=ExcState[mi];
        if(fabs(mx_slv)<1.0e-50) mx_slv=1.0e-50;
        QVector < double > temp(Solvent.size());
        for(int i=0;i<ExcState.size();i++)
        {
            temp[i]=Solvent[i]/mx_slv*mx_fsrs;
            ExcState[i]-=temp[i];
        }
        if (plot) ui->specPlot->graph(4)->setData(Spectrum->Wavelength,temp);
    }
    else if (plot)
    {
        temp.resize(ExcState.size());
        mult=ExcState[max_ind]/nonzero_div;
        for(i=0;i<ExcState.size();i++)
            temp[i]=mult*GroundState[i];
        ui->specPlot->graph(4)->setData(Spectrum->Wavelength,temp);
    }
}

void MainWindow::get_GS_FSRS(bool plot, int delay, QVector<double> *GS_FSRS, QVector<double> *AUX_GS_FSRS)
{
    if (!Spectrum->ProbeTime.isEmpty())
    {        
        bool auxExists=(AUX_GS_FSRS!=NULL);

        // Vektoriu dydziu perskirstymas:
        if (GroundState.size()!=Spectrum->Wavelength.size()) GroundState.resize(Spectrum->Wavelength.size());
        if(GS_FSRS->size()!=Spectrum->Wavelength.size()) GS_FSRS->resize(Spectrum->Wavelength.size());
        if (auxExists)
        {
            if (AuxGroundState.size()!=Spectrum->Wavelength.size()) AuxGroundState.resize(Spectrum->Wavelength.size());
            if (AUX_GS_FSRS->size()!=Spectrum->Wavelength.size()) AUX_GS_FSRS->resize(Spectrum->Wavelength.size());
        }

        get_GS_FSRS_spectrum(delay,GS_FSRS,AUX_GS_FSRS);

        if(ui->smoothBox->isChecked())
        {
            (*GS_FSRS)=savgolfilter(GS_FSRS,ui->windowSpinBox->value(),ui->windowSpinBox->value(),0,ui->degSpinBox->value());
            if (auxExists) (*AUX_GS_FSRS)=savgolfilter(AUX_GS_FSRS,ui->windowSpinBox->value(),ui->windowSpinBox->value(),0,ui->degSpinBox->value());
        }

        get_poly(GS_FSRS,&GroundState,plot,false,false);
        if (auxExists) get_poly(AUX_GS_FSRS,&AuxGroundState,plot,false,true);

        max_index(&GroundState,&max_ind);
    }
}

void MainWindow::replot_gs(bool rscGS)
{
    if (!Spectrum->ProbeTime.isEmpty())
    {
        QVector < double > raw_GS_FSRS, aux_GS_FSRS;
        bool aux_exists = (Spectrum->Type==PPRawAux || Spectrum->Type==PDPRawAux);

        get_GS_FSRS(true,ui->delaySlider->value(),&raw_GS_FSRS,(aux_exists ? &aux_GS_FSRS : NULL));

        ui->specPlot->graph(5)->setData(Spectrum->Wavelength,raw_GS_FSRS);

        if (aux_exists)
            ui->specPlot->graph(9)->setData(Spectrum->Wavelength,aux_GS_FSRS);
        else
            ui->specPlot->graph(9)->clearData();

        if (rscGS)
        {
            double mn_lim, mx_lim;
            double dlim=(Spectrum->Wavelength.last()-Spectrum->Wavelength.first())*REPLOT_PERC;
            below_min_and_above_max_proportional(&raw_GS_FSRS,&mn_lim,&mx_lim,4.0);
            if (aux_exists)
            {
                double mn_lim_a, mx_lim_a;
                below_min_and_above_max_proportional(&aux_GS_FSRS,&mn_lim_a,&mx_lim_a,4.0);
                mn_lim = (mn_lim < mn_lim_a ? mn_lim : mn_lim_a);
                mx_lim = (mx_lim > mx_lim_a ? mx_lim : mx_lim_a);
            }
            x3->setRange(Spectrum->Wavelength.first()-dlim,Spectrum->Wavelength.last()+dlim);
            y3->setRange(mn_lim,mx_lim);
        }
        ui->specPlot->replot();
    }
}

void MainWindow::on_actionSave_Raw_WL_File_triggered()
{
    if((!Spectrum->ProbeTime.isEmpty()) && (Spectrum->Type>=2) && (Spectrum->Type<=8))
    {
        QString fileName=QFileDialog::getSaveFileName(this, tr("Save Raw WL File (PDP Type)"),DirSettings.value("CURRENT_RAMAN_DIR").toString(),tr("Data files (*.dat);;Text files (*.txt);;Any files (*)"));
        if(!fileName.isEmpty())
        {
            DirSettings.setValue("CURRENT_RAMAN_DIR",GetCurrentDir(fileName));

            int i,j,k;
            FILE *outFile;

            // Griozdiskas, tikrai galima geriau...
            QVector < int > choices;
            switch (Spectrum->Type)
            {
                case PPRawAux:
                    choices << 0 << 1 << 4;
                    break;
                case PDPRawAux:
                    choices << 4 << 5;
                case PDPRaw:
                    choices << 2 << 3;
                case PPRaw:
                    choices << 0 << 1;
                    break;
                default:
                    break;
            }
            qSort(choices);

            outFile=fopen(fileName.toLatin1(),"w+");
            fprintf(outFile,"%18i\t",Spectrum->ProbeTime.size());
            for(i=0;i<Spectrum->Wavelength.size();i++)
                fprintf(outFile,"%18.10e\t",Spectrum->Wavelength[i]);
            fprintf(outFile,"\n");
            for(k=0;k<choices.size();k++)
            {
                for(i=0;i<Spectrum->ProbeTime.size();i++)
                {
                    fprintf(outFile,"%18.10e\t",Spectrum->ProbeTime[i]);
                    for(j=0;j<Spectrum->Wavelength.size();j++)
                    {
                        switch(choices[k])
                        {
                            case 0:
                                fprintf(outFile,"%18.10e\t",Spectrum->WL00[i][j]);
                                break;
                            case 1:
                                fprintf(outFile,"%18.10e\t",Spectrum->WL01[i][j]);
                                break;
                            case 2:
                                fprintf(outFile,"%18.10e\t",Spectrum->WL10[i][j]);
                                break;
                            case 3:
                                fprintf(outFile,"%18.10e\t",Spectrum->WL11[i][j]);
                                break;
                            case 4:
                                fprintf(outFile,"%18.10e\t",Spectrum->AUX01[i][j]);
                                break;
                            case 5:
                                fprintf(outFile,"%18.10e\t",Spectrum->AUX11[i][j]);
                                break;
                            default:
                                break;
                        }
                    }
                    fprintf(outFile,"\n");
                }
            }
            fclose(outFile);
        }
    }
}

void MainWindow::on_actionOpen_Raw_WL_File_triggered()
{
    QString fileName=QFileDialog::getOpenFileName(this,tr("Open Raw WL File (PDP Type)"),DirSettings.value("CURRENT_RAMAN_DIR").toString());
    if(!fileName.isEmpty())
    {
        DirSettings.setValue("CURRENT_RAMAN_DIR",GetCurrentDir(fileName));

        FILE *csFile;
        char *pch;
        char delimiters[]=" \t\n\v\f\r";
        char TempLine[MAXLINELENGTH];
        double Point;
        int PDPDelays;

        // Neefektyvu, bet taip isvengiama klaidu.
        bool ok,no_error;
        QVector < double > TempVect;

        csFile=fopen(fileName.toLatin1(),"r");
        fgets(TempLine,MAXLINELENGTH,csFile);
        pch=strtok(TempLine,delimiters);
        PDPDelays=QString::fromUtf8(pch).toDouble(&no_error);

        pch=strtok(NULL,delimiters);
        while(pch!=NULL)
        {
          Point=QString::fromUtf8(pch).toDouble(&ok);
          TempVect.push_back(Point);
          pch=strtok(NULL,delimiters);
          no_error&=ok;
        }

        if ((no_error) && (PDPDelays!=0))
        {
            int i,counter=0;
            Spectrum->ClearSpectra();
            Spectrum->Wavelength=TempVect;
            while(fscanf(csFile,"%lf",&Point)!=EOF)
            {
                switch(counter/PDPDelays)
                {
                    case 0:
                        {
                            Spectrum->ProbeTime.push_back(Point);
                            Spectrum->WL00.push_back(QVector<double>());
                            for(i=0;i<Spectrum->Wavelength.size();i++)
                            {
                                fscanf(csFile,"%lf",&Point);
                                Spectrum->WL00.last().push_back(Point);
                            }
                            break;
                        }
                    case 1:
                        {
                            Spectrum->WL01.push_back(QVector<double>());
                            for(i=0;i<Spectrum->Wavelength.size();i++)
                            {
                                fscanf(csFile,"%lf",&Point);
                                Spectrum->WL01.last().push_back(Point);
                            }
                            break;
                        }
                    case 2:
                        {
                            Spectrum->WL10.push_back(QVector<double>());
                            for(i=0;i<Spectrum->Wavelength.size();i++)
                            {
                                fscanf(csFile,"%lf",&Point);
                                Spectrum->WL10.last().push_back(Point);
                            }
                            break;
                        }
                    case 3:
                        {
                            Spectrum->WL11.push_back(QVector<double>());
                            for(i=0;i<Spectrum->Wavelength.size();i++)
                            {
                                fscanf(csFile,"%lf",&Point);
                                Spectrum->WL11.last().push_back(Point);
                            }
                            break;
                        }
                    case 4:
                        {
                            Spectrum->AUX01.push_back(QVector<double>());
                            for(i=0;i<Spectrum->Wavelength.size();i++)
                            {
                                fscanf(csFile,"%lf",&Point);
                                Spectrum->AUX01.last().push_back(Point);
                            }
                            break;
                        }
                    case 5:
                        {
                            Spectrum->AUX11.push_back(QVector<double>());
                            for(i=0;i<Spectrum->Wavelength.size();i++)
                            {
                                fscanf(csFile,"%lf",&Point);
                                Spectrum->AUX11.last().push_back(Point);
                            }
                            break;
                        }
                    default:
                        break;
                }
                counter++;
            }

            if ((!Spectrum->WL10.isEmpty()) && (Spectrum->WL11.isEmpty()))
            {
                Spectrum->AUX01=Spectrum->WL10;
                Spectrum->WL10.clear();
                Spectrum->Type=PPRawAux;
            }
            else if(Spectrum->WL10.isEmpty())
                Spectrum->Type=PPRaw;
            else if (Spectrum->AUX01.isEmpty())
                Spectrum->Type=PDPRaw;
            else
                Spectrum->Type=PDPRawAux;

            ui->delaySlider->setMinimum(0);
            ui->delaySlider->setMaximum(Spectrum->ProbeTime.size()-1);
            ui->zeroSpinBox->setMinimum(Spectrum->ProbeTime.first());
            ui->zeroSpinBox->setMaximum(Spectrum->ProbeTime.last());
            setWindowTitle(fileName);
            replot_some();
        }
        else
        {
            QMessageBox msgBox;
            msgBox.setText("Invalid file.");
            msgBox.exec();
        }
        fclose(csFile);
    }
}

void MainWindow::clickedGraph(QMouseEvent* event)
{
    if (!Spectrum->Wavelength.isEmpty())
    {
        int index,pos;

        double xx1 = x1->pixelToCoord(event->pos().x()),
               yy1 = y1->pixelToCoord(event->pos().y());

        double xx2 = x2->pixelToCoord(event->pos().x()),
               yy2 = y2->pixelToCoord(event->pos().y());

        double xx3 = x3->pixelToCoord(event->pos().x()),
               yy3 = y3->pixelToCoord(event->pos().y());

        if (xx1 >= x1->range().lower &&  xx1 <= x1->range().upper && yy1 >= y1->range().lower &&  yy1 <= y1->range().upper)
        {
            // hunt?
            locate(&Spectrum->Wavelength,xx1,&index);
            if(event->button()==Qt::LeftButton)
            {
                if(zero_indexes.indexOf(index)==-1)
                {
                    zero_indexes.append(index);
                    qSort(zero_indexes);
                }
            }
            else if (event->button()==Qt::RightButton)
            {
                pos=zero_indexes.indexOf(index);
                if(pos!=-1)
                    zero_indexes.remove(pos);
            }
            replot_some(event->button()==Qt::MiddleButton,true,true);
        }
        else if (xx3 >= x3->range().lower &&  xx3 <= x3->range().upper && yy3 >= y3->range().lower &&  yy3 <= y3->range().upper)
        {
            locate(&Spectrum->Wavelength,xx3,&index);
            if(event->button()==Qt::LeftButton)
            {
                if(gs_zero_indexes.indexOf(index)==-1)
                    gs_zero_indexes.append(index);
                qSort(gs_zero_indexes);
            }
            else if (event->button()==Qt::RightButton)
            {
                pos=gs_zero_indexes.indexOf(index);
                if(pos!=-1)
                    gs_zero_indexes.remove(pos);
            }
            replot_gs(event->button()==Qt::MiddleButton);
        }
        else if (xx2 >= x2->range().lower &&  xx2 <= x2->range().upper && yy2 >= y2->range().lower &&  yy2 <= y2->range().upper)
        {
            replot_some(false,event->button()==Qt::MiddleButton,false);
        }
    }
}

void MainWindow::on_clearButton_clicked()
{
    zero_indexes.clear();
    replot_some();
}

void MainWindow::on_actionSave_Difference_File_triggered()
{
    // Sutvarkyti AUX'ams ir GS Ramanui.
    int i,j;
    if(Spectrum->ProbeTime.size()!=0)
    {
        QString fileName=QFileDialog::getSaveFileName(this, tr("Save Spectrum Data"),DirSettings.value("CURRENT_RAMAN_DIR").toString(),tr("Data files (*.dat);;Text files (*.txt);;Any files (*)"));
        if(!fileName.isEmpty())
        {
            DirSettings.setValue("CURRENT_RAMAN_DIR",GetCurrentDir(fileName));

            FILE *outFile;
            QVector < double > temp_FSRS,temp_raw_GS_FSRS;

            outFile=fopen(fileName.toLatin1(),"w+");
            fprintf(outFile,"%15i\t",0);
            for(i=0;i<Spectrum->Wavelength.size();i++)
                fprintf(outFile,"%15e\t",Spectrum->Wavelength[i]);
            fprintf(outFile,"\n");

            for(i=0;i<Spectrum->ProbeTime.size();i++)
            {
                fprintf(outFile,"%15e\t",Spectrum->ProbeTime[i]);

                /********************************************/

                get_GS_FSRS(false,i,&temp_raw_GS_FSRS);
                get_FSRS(false,i,&temp_FSRS);

                /********************************************/

                for(j=0;j<Spectrum->Wavelength.size();j++)
                    fprintf(outFile,"%15e\t",ExcState[j]);
                fprintf(outFile,"\n");
            }
            fclose(outFile);
        }
    }
}

void MainWindow::on_polyBox_valueChanged(int arg1)
{
    replot_some();
}

void MainWindow::on_zeroBox_toggled(bool checked)
{
    ui->zeroSpinBox->setEnabled(checked);
    replot_some();
}

void MainWindow::on_linButton_toggled(bool checked)
{
    replot_some();
}

void MainWindow::on_smoothBox_toggled(bool checked)
{
    ui->windowSpinBox->setEnabled(checked);
    ui->degSpinBox->setEnabled(checked);
    replot_some();
}

void MainWindow::on_windowSpinBox_valueChanged(int arg1)
{
    if (arg1 < ui->degSpinBox->value())
        ui->windowSpinBox->setValue(ui->degSpinBox->value());
    if(ui->smoothBox->isChecked())
        replot_some();
}

void MainWindow::on_degSpinBox_valueChanged(int arg1)
{
    if (arg1 > ui->windowSpinBox->value())
        ui->degSpinBox->setValue(ui->windowSpinBox->value());
    if(ui->smoothBox->isChecked())
        replot_some();
}

void MainWindow::on_subGSBox_toggled(bool checked)
{
    replot_some();
}

void MainWindow::on_actionInvert_Signals_triggered()
{
    // Not sure what to do with AUX spectra. I should probably check the posibilites of a fuck up.
    if((Spectrum->ProbeTime.size()!=0) &&
       (Spectrum->Type==PP || Spectrum->Type==PPRaw || Spectrum->Type==PDPRaw))
    {
        QVector < QVector < double > > WL00, WL01, WL10, WL11, FSRS;

        WL00=Spectrum->WL00;
        WL01=Spectrum->WL01;
        WL10=Spectrum->WL10;
        WL11=Spectrum->WL11;

        FSRS=Spectrum->FSRS;

        switch (Spectrum->Type)
        {
            case PDPRaw:
            {
                Spectrum->WL10=WL11;
                Spectrum->WL11=WL10;
            }
            case PPRaw:
            {
                Spectrum->WL00=WL01;
                Spectrum->WL01=WL00;
                break;
            }
            case PP:
            {
                Spectrum->FSRS=Spectrum->WL11;
                for(int i=0;i<Spectrum->ProbeTime.size();i++)
                    for(int j=0;j<Spectrum->Wavelength.size();j++)
                        Spectrum->FSRS[i][j]=-FSRS[i][j];
                break;
            }
            default: break;
        }
        replot_some();
    }
}

void MainWindow::on_actionReverse_Wavenumber_triggered()
{
    if(Spectrum->ProbeTime.size()!=0)
    {
        int i;
        reverse_wavenumber();
        for(i=0;i<zero_indexes.size();i++)
            zero_indexes[i]=Spectrum->Wavelength.size()-1-zero_indexes[i];
        for(i=0;i<gs_zero_indexes.size();i++)
            gs_zero_indexes[i]=Spectrum->Wavelength.size()-1-gs_zero_indexes[i];
        replot_some();
    }
}

void MainWindow::on_clearGSButton_clicked()
{
    gs_zero_indexes.clear();
    replot_some();
}

void MainWindow::reverse_wavenumber()
{
    int i,j;
    QVector < QVector < double > > WL00, WL01, WL10, WL11, AUX01, AUX11, FSRS;

    WL00=Spectrum->WL00;
    WL01=Spectrum->WL01;
    WL10=Spectrum->WL10;
    WL11=Spectrum->WL11;

    AUX01=Spectrum->AUX01;
    AUX11=Spectrum->AUX11;

    FSRS=Spectrum->FSRS;

    for(i=0;i<Spectrum->ProbeTime.size();i++)
    {
        for(j=0;j<Spectrum->Wavelength.size();j++)
        {

            switch (Spectrum->Type)
            {
                case PDPRawAux:
                {
                    Spectrum->AUX01[i][j]=AUX01[i][Spectrum->Wavelength.size()-1-j];
                    Spectrum->AUX11[i][j]=AUX11[i][Spectrum->Wavelength.size()-1-j];
                }
                case PDPRaw:
                {
                    Spectrum->WL10[i][j]=WL10[i][Spectrum->Wavelength.size()-1-j];
                    Spectrum->WL11[i][j]=WL11[i][Spectrum->Wavelength.size()-1-j];
                }
                case PPRaw:
                {
                    Spectrum->WL00[i][j]=WL00[i][Spectrum->Wavelength.size()-1-j];
                    Spectrum->WL01[i][j]=WL01[i][Spectrum->Wavelength.size()-1-j];
                    break;
                }
                case PPRawAux:
                {
                    Spectrum->WL00[i][j]=WL00[i][Spectrum->Wavelength.size()-1-j];
                    Spectrum->WL01[i][j]=WL01[i][Spectrum->Wavelength.size()-1-j];
                    Spectrum->AUX01[i][j]=AUX01[i][Spectrum->Wavelength.size()-1-j];
                    break;
                }
                case PP:
                {
                    Spectrum->FSRS[i][j]=FSRS[i][Spectrum->Wavelength.size()-1-j];
                    break;
                }
                default: break;
            }
        }
    }
}

void MainWindow::ReadPPPoint(FILE* pFile, PPTempData *Temp, QVector < QString > *NaNs, int npix, bool Reference)
{
    QString Str;
    QStringList List;
    bool Unfinished=true;
    int i,n00,n01;
    double point;
    char LongLine[MAXLINELENGTH];

    Temp->smp00.fill(0.0);
    Temp->smp01.fill(0.0);
    n00=n01=0;

    while(Unfinished)
    {
        Temp->FileEnd=false;
        if(fgets(LongLine,MAXLINELENGTH,pFile)==NULL)
        {
            Temp->FileEnd=true;
            break;
        }
        if(strncmp(LongLine,"Pump",2)==0)
        {
            fscanf(pFile,"%lf",&point);
            if(NanCheck(point))
            {
                Temp->smp01[0]+=point;
                for(i=1;i<npix;i++)
                {
                    fscanf(pFile,"%lf",&point);
                    Temp->smp01[i]+=point;
                }
                n01++;
            }
            else
            {
                NaNs->append("NaNs in scan " + QString::number(Temp->NextScanNo) + " @ " + QString::number(Temp->NextDelayPump,'f',2) + " ps.\n");
            }
            if (!Reference) fgets(LongLine,MAXLINELENGTH,pFile);
            fgets(LongLine,MAXLINELENGTH,pFile);

            fscanf(pFile,"%lf",&point);
            if(NanCheck(point))
            {
                Temp->smp00[0]+=point;
                for(i=1;i<npix;i++)
                {
                    fscanf(pFile,"%lf",&point);
                    Temp->smp00[i]+=point;
                }
                n00++;
            }
            else
            {
                NaNs->append("NaNs in scan " + QString::number(Temp->NextScanNo) + " @ " + QString::number(Temp->NextDelayPump,'f',2) + " ps.\n");
            }
            if (!Reference) fgets(LongLine,MAXLINELENGTH,pFile);
            fgets(LongLine,MAXLINELENGTH,pFile);
        }
        else
        {
            //Gal palikti streamus kaip anksciau?
            Unfinished=false;
            Str=LongLine;
            Str.remove(",");
            List=Str.split(" ",QString::SkipEmptyParts);
            Temp->NextType=List.at(0);
            Temp->NextScanNo=List.at(2).toInt();
            Temp->NextDelayPump=List.at(4).toDouble();
        }
    }
    for(i=0;i<npix;i++)
    {
        Temp->smp00[i]/=n00;
        Temp->smp01[i]/=n01;
    }
}

void MainWindow::on_actionOpen_PP_Dataset_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Pump Probe File"),DirSettings.value("CURRENT_RAMAN_DIR").toString());
    if(!fileName.isEmpty())
    {
        DirSettings.setValue("CURRENT_RAMAN_DIR",GetCurrentDir(fileName));

        FILE *pFile;
        QString Header;
        char mystring[MAXLINELENGTH];

        pFile=fopen(fileName.toLatin1(),"r");
        fgets(mystring,MAXLINELENGTH,pFile);
        Header=mystring;

        if (Header.contains("Pump-Probe",Qt::CaseInsensitive) || Header.contains("PumpProbe",Qt::CaseInsensitive))
        {
            PPTempData TempData;
            QVector< double > bksmp00,bksmp01;
            QVector< double > DelTimes,TV00,TV01;
            QVector< QVector< double > > WL00,WL01;
            QVector < QString > NansAndZeros;
            bool Ref=false;
            fpos_t position;

            QString CurrType;
            int i,j,k,CurrScanNo,n=0,HowMany=1,npix=256;
            double CurrDelay,TempF;

            Ref=(!Header.contains("Not referenced",Qt::CaseInsensitive));

            Header = "Checking...";
            while(!Header.contains("scan 1",Qt::CaseInsensitive))
            {
                fgetpos(pFile,&position);
                fgets(mystring,MAXLINELENGTH,pFile);
                Header=mystring;
            }
            fgets(mystring,MAXLINELENGTH,pFile);
            fgets(mystring,MAXLINELENGTH,pFile);
            Header=mystring;
            npix=Header.count("\t")+1;
            fsetpos(pFile,&position);

            if (Ref)
            {
                bksmp00.fill(0.0,npix);
                bksmp01.fill(0.0,npix);
            }
            TempData.smp00.resize(npix);
            TempData.smp01.resize(npix);
            Spectrum->ClearSpectra();
            Spectrum->Wavelength.resize(npix);
            TV00.resize(npix);
            TV01.resize(npix);

            ReadPPPoint(pFile,&TempData,&NansAndZeros,npix,Ref);
            CurrType=TempData.NextType;
            CurrScanNo=TempData.NextScanNo;
            CurrDelay=TempData.NextDelayPump;

            while(!TempData.FileEnd)
            {
                ReadPPPoint(pFile,&TempData,&NansAndZeros,npix,Ref);
                if(TempData.FileEnd)
                {
                    WL00.push_back(QVector<double>(npix));
                    WL01.push_back(QVector<double>(npix));
                    for(i=npix-1;i>=0;i--)
                    {
                        WL00[n][i]=TempData.smp00[npix-1-i]-bksmp00[npix-1-i];
                        WL01[n][i]=TempData.smp01[npix-1-i]-bksmp01[npix-1-i];
                        Spectrum->Wavelength[i]=i+1;
                    }
                    DelTimes.push_back(CurrDelay);
                    n++;
                    if(CurrScanNo<2) HowMany=n;
                    break;
                }
                if(CurrType.compare("Background")==0)
                {
                    bksmp00=TempData.smp00;
                    bksmp01=TempData.smp01;
                }
                else if(CurrType.compare("Measurement")==0)
                {
                    WL00.push_back( QVector<double>(npix) );
                    WL01.push_back( QVector<double>(npix) );
                    for(i=npix-1;i>=0;i--)
                    {
                        WL00[n][i]=TempData.smp00[npix-1-i]-bksmp00[npix-1-i];
                        WL01[n][i]=TempData.smp01[npix-1-i]-bksmp01[npix-1-i];
                    }
                    DelTimes.push_back(CurrDelay);
                    n++;
                }
                if(CurrScanNo<2) HowMany=n;
                CurrType=TempData.NextType;
                CurrScanNo=TempData.NextScanNo;
                CurrDelay=TempData.NextDelayPump;
            }
            fclose(pFile);

            for (i=0;i<n;i++)
            {
                for (j=0;j<npix;j++)
                {
                    if (fabs(WL00[i][j])<1e-20)
                    {
                        NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(DelTimes[i],'f',1) + " ps, " +
                                                             QString::number(j+1) + " nm");
                        WL00[i][j]=1e-20;
                    }
                    if (fabs(WL01[i][j])<1e-20)
                    {
                        NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(DelTimes[i],'f',1) + " ps, " +
                                                             QString::number(j+1) + " nm");
                        WL01[i][j]=1e-20;
                    }
                }
            }

            bool Unfinished=(n!=CurrScanNo*HowMany);
            if(Unfinished)
            {
                for(i=0;i<n-(CurrScanNo-1)*HowMany;i++)
                {
                    DelTimes.pop_back();
                    WL00.pop_back();
                    WL01.pop_back();
                }
                CurrScanNo--;
            }
            if (Unfinished||(!NansAndZeros.isEmpty()))
            {
                QString UnfMeas="Unfinished measurement. Displaying only the completed scans.";
                if (NansAndZeros.isEmpty())
                {
                    QMessageBox msgBox;
                    msgBox.setText(UnfMeas);
                    msgBox.exec();
                }
                else
                {
                    if (Unfinished) NansAndZeros.push_front(UnfMeas);
                    NaNReportDialog *nanReport;
                    nanReport=new NaNReportDialog(this,&NansAndZeros);
                    nanReport->setAttribute(Qt::WA_DeleteOnClose,true);
                    nanReport->setModal(true);
                    nanReport->exec();
                }
            }

            for(j=0;j<HowMany;j++)
            {
                TempF=0.0;
                for(i=0;i<CurrScanNo;i++)
                    TempF+=DelTimes[j+i*HowMany];
                Spectrum->ProbeTime.push_back(TempF/CurrScanNo);
            }

            QVector < QVector < double > > dummy=WL00;
            shell_sort_PDP(Spectrum->ProbeTime,WL00,WL01,dummy,CurrScanNo);

            ////////////////////////////
            //// Palyginimo langas: ////
            ////////////////////////////

            QVector < double > Weights;
            double summ=0.0;
            Weights.fill(1.0,CurrScanNo);

            QVector< QVector < double > > mOD;
            mOD.fill(QVector < double >(npix), CurrScanNo*HowMany);
            for(i=0;i<CurrScanNo*HowMany;i++)
                for(j=0;j<npix;j++)
                    mOD[i][j]=1000.0*log10(fabs(WL01[i][j]/WL00[i][j]));

            CompareTraces *compWindow;
            compWindow=new CompareTraces(this,&mOD,&Spectrum->ProbeTime,CurrScanNo,&Weights);
            compWindow->setAttribute(Qt::WA_DeleteOnClose,true);
            compWindow->setModal(true);
            compWindow->exec();

            ////////////////////////////
            ////////////////////////////
            ////////////////////////////

            for(i=0;i<CurrScanNo;i++)
                summ+=Weights[i];
            if (summ==0.0) summ=1.0;

            for(j=0;j<HowMany;j++)
            {
                TV00.fill(0.0);
                TV01.fill(0.0);
                for(i=0;i<CurrScanNo;i++)
                    for(k=0;k<npix;k++)
                    {
                        TV00[k]+=(WL00[j+i*HowMany][k]*Weights[i]/summ);
                        TV01[k]+=(WL01[j+i*HowMany][k]*Weights[i]/summ);
                    }
                Spectrum->WL00.push_back(TV00);
                Spectrum->WL01.push_back(TV01);
            }

            Spectrum->FSRS=Spectrum->WL00;
            for(i=0;i<Spectrum->ProbeTime.size();i++)
                for(j=0;j<Spectrum->Wavelength.size();j++)
                    Spectrum->FSRS[i][j]=fabs(Spectrum->WL01[i][j]/Spectrum->WL00[i][j]);

            Spectrum->Type=PPRaw;

            ui->delaySlider->setMinimum(0);
            ui->delaySlider->setMaximum(Spectrum->ProbeTime.size()-1);
            ui->zeroSpinBox->setMinimum(Spectrum->ProbeTime.first());
            ui->zeroSpinBox->setMaximum(Spectrum->ProbeTime.last());
            setWindowTitle(fileName);
            if (Ref) reverse_wavenumber();
            replot_some();
        }
        else
        {
            fclose(pFile);
            QMessageBox msgBox;
            msgBox.setText("Invalid file.");
            msgBox.exec();
        }
    }
}

void MainWindow::on_actionOpen_Common_PP_File_triggered()
{
    QString fileName=QFileDialog::getOpenFileName(this,tr("Open Common Spectra File"),DirSettings.value("CURRENT_RAMAN_DIR").toString());
    if(!fileName.isEmpty())
    {
        DirSettings.setValue("CURRENT_RAMAN_DIR",GetCurrentDir(fileName));
        FILE *csFile;
        char *pch;
        char delimiters[]=" \t\n\v\f\r";
        char TempLine[MAXLINELENGTH];
        double Point;
        int PDPDelays;

        // Neefektyvu, bet taip isvengiama klaidu.
        bool ok,no_error;
        QVector <double> TempVect;

        csFile=fopen(fileName.toLatin1(),"r");
        fgets(TempLine,MAXLINELENGTH,csFile);
        pch=strtok(TempLine,delimiters);
        PDPDelays=QString::fromUtf8(pch).toDouble(&no_error);

        pch=strtok(NULL,delimiters);
        while(pch!=NULL)
        {
            Point=QString::fromUtf8(pch).toDouble(&ok);
            TempVect.push_back(Point);
            pch=strtok(NULL,delimiters);
            no_error&=ok;
        }

        if(no_error)
        {
            Spectrum->ClearSpectra();
            Spectrum->Wavelength=TempVect;
            if(PDPDelays==0)
            {
                while(fscanf(csFile,"%lf",&Point)!=EOF)
                {
                    Spectrum->ProbeTime.push_back(Point);
                    Spectrum->FSRS.push_back(QVector<double>());
                    for(int i=0;i<Spectrum->Wavelength.size();i++)
                    {
                        fscanf(csFile,"%lf",&Point);
                        Spectrum->FSRS.last().push_back(Point);
                    }
                }
            }
            else
            {
                int i,j,counter=0;
                QVector < double > zs;
                zs.fill(0.0,Spectrum->Wavelength.size());
                Spectrum->FSRS.fill(zs,PDPDelays);
                Spectrum->ProbeTime.resize(PDPDelays);
                while(fscanf(csFile,"%lf",&Point)!=EOF)
                {
                    j=counter%PDPDelays;
                    switch(counter/PDPDelays)
                    {
                        case 0:
                            {
                                Spectrum->ProbeTime[j]=Point;
                                for(i=0;i<Spectrum->Wavelength.size();i++)
                                {
                                    fscanf(csFile,"%lf",&Point);
                                    Spectrum->FSRS[j][i]-=Point;
                                }
                                break;
                            }
                        case 1:
                            {
                                for(i=0;i<Spectrum->Wavelength.size();i++)
                                {
                                    fscanf(csFile,"%lf",&Point);
                                    Spectrum->FSRS[j][i]-=Point;
                                }
                                break;
                            }
                        case 2:
                            {
                                for(i=0;i<Spectrum->Wavelength.size();i++)
                                {
                                    fscanf(csFile,"%lf",&Point);
                                    Spectrum->FSRS[j][i]+=Point;
                                }
                                break;
                            }
                        default:
                            break;
                    }
                    counter++;
                }
            }
            Spectrum->Type=PP;
            ui->delaySlider->setMinimum(0);
            ui->delaySlider->setMaximum(Spectrum->ProbeTime.size()-1);
            ui->zeroSpinBox->setMinimum(Spectrum->ProbeTime.first());
            ui->zeroSpinBox->setMaximum(Spectrum->ProbeTime.last());
            setWindowTitle(fileName);
            replot_some();
        }
        else
        {
            QMessageBox msgBox;
            msgBox.setText("Invalid file.");
            msgBox.exec();
        }
        fclose(csFile);
    }
}

void MainWindow::on_actionCalibrate_wavenumber_triggered()
{
    int ret=QMessageBox::Cancel;
    if (!Spectrum->ProbeTime.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("Average the GS FSRS signal over the entire probetime?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::No);
        ret=msgBox.exec();
    }
    if ((ret!=QMessageBox::Cancel) || (Spectrum->ProbeTime.isEmpty()))
    {
        QVector < double > AverageGS(Spectrum->Wavelength.size()), dummy;
        if (ret==QMessageBox::Yes)
        {
            AverageGS.fill(0.0);
            for (int i=0;i<Spectrum->ProbeTime.size();i++)
            {
                get_GS_FSRS(false,i,&dummy);
                for (int j=0;j<Spectrum->Wavelength.size();j++)
                    AverageGS[j]+=GroundState[j]/double(Spectrum->ProbeTime.size());
            }
        }
        else
        {
            replot_gs();
        }
        HolmiumComparison *holmWindow;
        holmWindow=new HolmiumComparison(this,&DirSettings,&Spectrum->Wavelength,(ret==QMessageBox::Yes ? &AverageGS : &GroundState));
        holmWindow->setAttribute(Qt::WA_DeleteOnClose,true);
        if (!Spectrum->ProbeTime.isEmpty())
            connect(holmWindow,SIGNAL(sendCalibration(double,double,double)),this,SLOT(getScale(double,double,double)));
        holmWindow->setModal(true);
        holmWindow->exec();
    }
}

void MainWindow::getScale(double k, double b, double v0)
{
    for (int i=0;i<Spectrum->Wavelength.size();i++)
        Spectrum->Wavelength[i]=1e7*(1/v0-1/(Spectrum->Wavelength[i]*k+b));
    replot_some();
}

void MainWindow::on_actionScale_Wavenumber_triggered()
{
    if(!Spectrum->ProbeTime.isEmpty())
    {
        wnScaleDialog *ScaleWindow;
        ScaleWindow=new wnScaleDialog(this);
        ScaleWindow->setAttribute(Qt::WA_DeleteOnClose,true);
        connect(ScaleWindow,SIGNAL(sendCalibration(double,double,double)),this,SLOT(getScale(double,double,double)));
        ScaleWindow->setModal(true);
        ScaleWindow->show();
    }
}

void MainWindow::on_actionSave_Averaged_GS_Spectrum_triggered()
{
    if(!GroundState.isEmpty())
    {
        QString fileName=QFileDialog::getSaveFileName(this, tr("Save GS Data"),DirSettings.value("CURRENT_RAMAN_DIR").toString(),tr("Data files (*.dat);;Text files (*.txt);;Any files (*)"));
        if(!fileName.isEmpty())
        {
            FILE *outFile;
            outFile=fopen(fileName.toLatin1(),"w+");

            int i,j;
            QVector < double > AVGVect( GroundState.size() ), Temp;

            AVGVect.fill(0.0);
            for (i=0;i<Spectrum->ProbeTime.size();i++)
            {
                get_GS_FSRS(false,i,&Temp);
                for(j=0;j<GroundState.size();j++)
                    AVGVect[j]+=(GroundState[j]/(Spectrum->ProbeTime.size()));
            }

            for(i=0;i<Spectrum->Wavelength.size();i++)
                fprintf(outFile,"%15e\t%15e\n",Spectrum->Wavelength[i],AVGVect[i]);

            fclose(outFile);
        }
    }
}

void MainWindow::on_isoBox_valueChanged(int arg1)
{
    replot_some();
}

void MainWindow::on_actionCorrect_Dispersion_triggered()
{
    if (!Spectrum->ProbeTime.isEmpty())
    {
        DispersionSuite *disp;
        disp=new DispersionSuite(this,Spectrum,&DirSettings);
        disp->setAttribute(Qt::WA_DeleteOnClose,true);
        disp->setModal(true);
        disp->exec();
        replot_some();
    }
}

void MainWindow::on_actionPreview_FSRS_Spectrum_triggered()
{
    /*
    QVector < QVector < double > > myFSRS, myAUX;
    QVector < double > temp_GS_FSRS, temp_AUX;
    bool auxExists = (Spectrum->Type==PPRawAux || Spectrum->Type==PDPRawAux);
    for(int i=0;i<Spectrum->ProbeTime.size();i++)
    {
        get_GS_FSRS(false,i,&temp_GS_FSRS,(auxExists ? &temp_AUX : NULL));
        myFSRS.push_back(GroundState);
        if (auxExists) myAUX.push_back(AuxGroundState);
    }
    PreviewSpectrum(&myFSRS,(auxExists ? &myAUX : NULL));
    */

    //if (!Spectrum->WL00.isEmpty() && !Spectrum->WL01.isEmpty() && !Spectrum->WL10.isEmpty() && !Spectrum->WL11.isEmpty())
    if (!Spectrum->ProbeTime.isEmpty())
    {
        QVector < QVector < double > > myFSRS, myAUX;
        QVector < double > temp_GS_FSRS, temp_AUX_GS_FSRS,
                           temp_FSRS, temp_AUX;
        bool auxExists = (Spectrum->Type==PPRawAux || Spectrum->Type==PDPRawAux);
        for(int i=0;i<Spectrum->ProbeTime.size();i++)
        {
            if(ui->subGSBox->isChecked()) get_GS_FSRS(false,i,&temp_GS_FSRS,(auxExists ? &temp_AUX_GS_FSRS : NULL));
            get_FSRS(false,i,&temp_FSRS,(auxExists ? &temp_AUX : NULL));
            myFSRS.push_back(ExcState);
            if (auxExists) myAUX.push_back(AuxExcState);
        }
        PreviewSpectrum(&myFSRS,(auxExists ? &myAUX : NULL));
    }
}

void MainWindow::on_isolateFSRSBox_toggled(bool checked)
{
    ui->isoBox->setEnabled(checked || ui->isolateGSBox->isChecked());
    replot_some();
}

void MainWindow::on_isolateGSBox_toggled(bool checked)
{
    ui->isoBox->setEnabled(checked || ui->isolateFSRSBox->isChecked());
    replot_some();
}

void MainWindow::on_splineFSRSBox_toggled(bool checked)
{
    ui->polyBox->setDisabled(checked && ui->splineGSBox->isChecked());
    replot_some();
}

void MainWindow::on_splineGSBox_toggled(bool checked)
{
    ui->polyBox->setDisabled(checked && ui->splineFSRSBox->isChecked());
    replot_some();
}

void MainWindow::on_actionRemove_Region_triggered()
{
    //if (!Spectrum->WL00.isEmpty() && !Spectrum->WL01.isEmpty() && !Spectrum->WL10.isEmpty() && !Spectrum->WL11.isEmpty())
    if(!Spectrum->ProbeTime.isEmpty())
    {
        RemoveDialog *rem;
        rem=new RemoveDialog(this,Spectrum);
        connect(rem,SIGNAL(SendRemovableRegion(bool,double,double,bool,double,double)),this,SLOT(ReceiveRemovableRegion(bool,double,double,bool,double,double)));
        rem->setAttribute(Qt::WA_DeleteOnClose,true);
        rem->setModal(true);
        rem->show();
    }
}

void MainWindow::ReceiveRemovableRegion(bool wl_b,double w0,double w1,bool pt_b,double p0,double p1)
{
    int i,j,k;
    double ws, we, ps, pe;
    QVector < QVector < double > > WL00, WL01, WL10, WL11, AUX01, AUX11, FSRS;
    QVector < double > PT, WL;
    QVector < int > z_ind, g_ind;

    if (wl_b)
    {
        ws=w0;
        we=w1;
    }
    else
    {
        we=Spectrum->Wavelength.first()-1;
        ws=Spectrum->Wavelength.last()+1;
    }

    if (pt_b)
    {
        ps=p0;
        pe=p1;
    }
    else
    {
        pe=Spectrum->ProbeTime.first()-1;
        ps=Spectrum->ProbeTime.last()+1;
    }

    for (i=0;i<Spectrum->ProbeTime.size();i++)
    {
        if ((Spectrum->ProbeTime[i]<ps) || (Spectrum->ProbeTime[i]>pe))
        {            
            switch (Spectrum->Type)
            {
                case PDPRawAux:
                {
                    AUX01.push_back( QVector < double >() );
                    AUX11.push_back( QVector < double >() );
                }
                case PDPRaw:
                {
                    WL10.push_back( QVector < double >() );
                    WL11.push_back( QVector < double >() );
                }
                case PPRaw:
                {
                    WL00.push_back( QVector < double >() );
                    WL01.push_back( QVector < double >() );
                    break;
                }
                case PPRawAux:
                {
                    WL00.push_back( QVector < double >() );
                    WL01.push_back( QVector < double >() );
                    AUX01.push_back( QVector < double >() );
                    break;
                }
                case PP:
                {
                    FSRS.push_back( QVector < double >() );
                    break;
                }
                default: break;
            }

            for (j=0;j<Spectrum->Wavelength.size();j++)
            {
                k = (Spectrum->Type != PP ? WL00.size()-1 : FSRS.size()-1);
                if ((Spectrum->Wavelength[j]<ws) || (Spectrum->Wavelength[j]>we))
                {

                    switch (Spectrum->Type)
                    {
                        case PDPRawAux:
                        {
                            AUX01[k].push_back( Spectrum->AUX01[i][j] );
                            AUX11[k].push_back( Spectrum->AUX11[i][j] );
                        }
                        case PDPRaw:
                        {
                            WL10[k].push_back( Spectrum->WL10[i][j] );
                            WL11[k].push_back( Spectrum->WL11[i][j] );
                        }
                        case PPRaw:
                        {
                            WL00[k].push_back( Spectrum->WL00[i][j] );
                            WL01[k].push_back( Spectrum->WL01[i][j] );
                            break;
                        }
                        case PPRawAux:
                        {
                            WL00[k].push_back( Spectrum->WL00[i][j] );
                            WL01[k].push_back( Spectrum->WL01[i][j] );
                            AUX01[k].push_back( Spectrum->AUX01[i][j] );
                            break;
                        }
                        case PP:
                        {
                            FSRS[k].push_back( Spectrum->FSRS[i][j] );
                            break;
                        }
                        default: break;
                    }
                }
            }
        }
    }

    for (i=0;i<Spectrum->ProbeTime.size();i++)
        if ((Spectrum->ProbeTime[i]<ps) || (Spectrum->ProbeTime[i]>pe))
            PT.push_back(Spectrum->ProbeTime[i]);

    for (j=0;j<Spectrum->Wavelength.size();j++)
        if ((Spectrum->Wavelength[j]<ws) || (Spectrum->Wavelength[j]>we))
        {
            WL.push_back(Spectrum->Wavelength[j]);
            if (zero_indexes.indexOf(j)!=-1)
                z_ind.push_back(WL.size()-1);
            if (gs_zero_indexes.indexOf(j)!=-1)
                g_ind.push_back(WL.size()-1);
        }

    switch (Spectrum->Type)
    {
        case PDPRawAux:
        {
            Spectrum->AUX01=AUX01;
            Spectrum->AUX11=AUX11;
        }
        case PDPRaw:
        {
            Spectrum->WL10=WL10;
            Spectrum->WL11=WL11;
        }
        case PPRaw:
        {
            Spectrum->WL00=WL00;
            Spectrum->WL01=WL01;
            break;
        }
        case PPRawAux:
        {
            Spectrum->WL00=WL00;
            Spectrum->WL01=WL01;
            Spectrum->AUX01=AUX01;
            break;
        }
        case PP:
        {
            Spectrum->FSRS=FSRS;
            break;
        }
        default: break;
    }
    Spectrum->ProbeTime=PT;
    Spectrum->Wavelength=WL;

    if (wl_b)
    {
        zero_indexes=z_ind;
        gs_zero_indexes=g_ind;
    }
    if (pt_b)
    {
        ui->delaySlider->setMaximum(Spectrum->ProbeTime.size()-1);
    }
    replot_some();
}

void MainWindow::on_actionPreview_GS_FSRS_Spectrum_triggered()
{
    if (!Spectrum->ProbeTime.isEmpty())
    {
        QVector < QVector < double > > myFSRS, myAUX;
        QVector < double > temp_GS_FSRS, temp_AUX;
        bool auxExists = (Spectrum->Type==PPRawAux || Spectrum->Type==PDPRawAux);
        for(int i=0;i<Spectrum->ProbeTime.size();i++)
        {
            get_GS_FSRS(false,i,&temp_GS_FSRS,(auxExists ? &temp_AUX : NULL));
            myFSRS.push_back(GroundState);
            if (auxExists) myAUX.push_back(AuxGroundState);
        }
        PreviewSpectrum(&myFSRS,(auxExists ? &myAUX : NULL));
    }
}

void MainWindow::on_actionInvert_Probetime_triggered()
{
    if (!Spectrum->ProbeTime.isEmpty())
    {
        QVector < QVector < double > > WL00, WL01, WL10, WL11, AUX01, AUX11, FSRS;
        QVector < double > PT=Spectrum->ProbeTime;

        WL00=Spectrum->WL00;
        WL01=Spectrum->WL01;
        WL10=Spectrum->WL10;
        WL11=Spectrum->WL11;

        AUX01=Spectrum->AUX01;
        AUX11=Spectrum->AUX11;

        FSRS=Spectrum->FSRS;

        for(int i=0;i<Spectrum->ProbeTime.size();i++)
        {
            switch (Spectrum->Type)
            {
                case PDPRawAux:
                {
                    Spectrum->AUX01[i]=AUX01[Spectrum->ProbeTime.size()-1-i];
                    Spectrum->AUX11[i]=AUX11[Spectrum->ProbeTime.size()-1-i];
                }
                case PDPRaw:
                {
                    Spectrum->WL10[i]=WL10[Spectrum->ProbeTime.size()-1-i];
                    Spectrum->WL11[i]=WL11[Spectrum->ProbeTime.size()-1-i];
                }
                case PPRaw:
                {
                    Spectrum->WL00[i]=WL00[Spectrum->ProbeTime.size()-1-i];
                    Spectrum->WL01[i]=WL01[Spectrum->ProbeTime.size()-1-i];
                    break;
                }
                case PPRawAux:
                {
                    Spectrum->WL00[i]=WL00[Spectrum->ProbeTime.size()-1-i];
                    Spectrum->WL01[i]=WL01[Spectrum->ProbeTime.size()-1-i];
                    Spectrum->AUX01[i]=AUX01[Spectrum->ProbeTime.size()-1-i];
                    break;
                }
                case PP:
                {
                    Spectrum->FSRS[i]=FSRS[Spectrum->ProbeTime.size()-1-i];
                    break;
                }
                default: break;
            }
            Spectrum->ProbeTime[i]=-PT[Spectrum->ProbeTime.size()-1-i];
        }
        replot_some();
    }
}

void MainWindow::on_actionPreview_PP_Spectrum_triggered()
{
    if (!Spectrum->WL00.isEmpty() && !Spectrum->WL01.isEmpty())
    {
        QVector < QVector < double > > myFSRS;
        for(int i=0;i<Spectrum->ProbeTime.size();i++)
        {
            myFSRS.push_back(QVector < double > (Spectrum->Wavelength.size()));
            for (int j=0;j<Spectrum->Wavelength.size();j++)
                myFSRS[i][j]=1000.0*log10(Spectrum->WL00[i][j]/Spectrum->WL10[i][j]);
        }
        PreviewSpectrum(&myFSRS);
    }
}

void MainWindow::on_actionPreview_DP_Spectrum_triggered()
{
    if (!Spectrum->WL00.isEmpty() && !Spectrum->WL01.isEmpty())
    {
        QVector < QVector < double > > myFSRS;
        for(int i=0;i<Spectrum->ProbeTime.size();i++)
        {
            myFSRS.push_back(QVector < double > (Spectrum->Wavelength.size()));
            for (int j=0;j<Spectrum->Wavelength.size();j++)
                myFSRS[i][j]=1000.0*log10(Spectrum->WL00[i][j]/Spectrum->WL01[i][j]);
        }
        PreviewSpectrum(&myFSRS);
    }
}

void MainWindow::on_actionPreview_PDP_Spectrum_triggered()
{
    if (!Spectrum->WL00.isEmpty() && !Spectrum->WL01.isEmpty() && !Spectrum->WL10.isEmpty() && !Spectrum->WL11.isEmpty())
    {
        QVector < QVector < double > > myFSRS;
        for(int i=0;i<Spectrum->ProbeTime.size();i++)
        {
            myFSRS.push_back(QVector < double > (Spectrum->Wavelength.size()));
            for (int j=0;j<Spectrum->Wavelength.size();j++)
                myFSRS[i][j]=1000.0*log10(Spectrum->WL00[i][j]/Spectrum->WL11[i][j]);
        }
        PreviewSpectrum(&myFSRS);
    }
}

void MainWindow::PreviewSpectrum(QVector<QVector<double> > *spec,QVector<QVector<double> > *aux)
{
    FSRSPreviewDialog *prev;
    prev=new FSRSPreviewDialog(this,Spectrum,spec,aux);
    prev->setAttribute(Qt::WA_DeleteOnClose,true);
    prev->setModal(true);
    prev->exec();
}

void MainWindow::on_actionSave_Configs_triggered()
{
    if (!zero_indexes.isEmpty() && !gs_zero_indexes.isEmpty())
    {
        QString fileName=QFileDialog::getSaveFileName(this, tr("Save Config File"),DirSettings.value("CURRENT_RAMAN_DIR").toString(),tr("Data files (*.dat);;Text files (*.txt);;Any files (*)"));
        if(!fileName.isEmpty())
        {
            DirSettings.setValue("CURRENT_RAMAN_DIR",GetCurrentDir(fileName));

            int i;
            FILE *outFile;
            outFile=fopen(fileName.toLatin1(),"w+");

            /*
            fprintf(outFile,"SUBTR_OFF %4d\n",BoolToBinary(ui->zeroBox->isChecked()));
            fprintf(outFile,"SUBTR_GS %4d\n",BoolToBinary(ui->subGSBox->isChecked()));
            fprintf(outFile,"USE_ALL %4d\n",BoolToBinary(ui->polyAllBox->isChecked()));
            fprintf(outFile,"SMOOTH %4d %4d %4d \n",BoolToBinary(ui->smoothBox->isChecked()),ui->windowSpinBox->value(),ui->degSpinBox->value());
            fprintf(outFile,"ISO_FSRS %4d\n",BoolToBinary(ui->isolateFSRSBox->isChecked()));
            fprintf(outFile,"ISO_GS %4d\n",BoolToBinary(ui->isolateGSBox->isChecked()));
            fprintf(outFile,"NTH %4d\n",ui->isoBox->value());
            fprintf(outFile,"SLP_FSRS %4d\n",BoolToBinary(ui->splineFSRSBox->isChecked()));
            fprintf(outFile,"SPL_GS %4d\n",BoolToBinary(ui->splineGSBox->isChecked()));
            fprintf(outFile,"POLY %4d\n",ui->polyBox->value());
            fprintf(outFile,"%s\n",(ui->linButton->isChecked() ? "LIN" : "LOG"));
            */

            //fprintf(outFile,"FSRS_ZEROS ");
            for(i=0;i<zero_indexes.size();i++)
                fprintf(outFile,"%18.10e\t",Spectrum->Wavelength[zero_indexes[i]]);
            fprintf(outFile,"\n");

            //fprintf(outFile,"GS_FSRS_ZEROS ");
            for(i=0;i<gs_zero_indexes.size();i++)
                fprintf(outFile,"%18.10e\t",Spectrum->Wavelength[gs_zero_indexes[i]]);

            fclose(outFile);
        }
    }
}

void MainWindow::on_actionLoad_Configs_triggered()
{
    if (!Spectrum->ProbeTime.isEmpty())
    {
        QString fileName=QFileDialog::getOpenFileName(this,tr("Open Config File"),DirSettings.value("CURRENT_RAMAN_DIR").toString());
        if(!fileName.isEmpty())
        {
            DirSettings.setValue("CURRENT_RAMAN_DIR",GetCurrentDir(fileName));

            FILE *csFile;
            char *pch;
            char delimiters[]=" \t\n\v\f\r";
            char TempLine[MAXLINELENGTH];
            int i;

            zero_indexes.clear();
            gs_zero_indexes.clear();

            csFile=fopen(fileName.toLatin1(),"r");
            fgets(TempLine,MAXLINELENGTH,csFile);
            pch=strtok(TempLine,delimiters);
            while(pch!=NULL)
            {
              hunt(&Spectrum->Wavelength,atof(pch),&i);
              if (zero_indexes.indexOf(i)==-1)
                zero_indexes.push_back(i);
              pch=strtok(NULL,delimiters);
            }

            fgets(TempLine,MAXLINELENGTH,csFile);
            pch=strtok(TempLine,delimiters);
            while(pch!=NULL)
            {
              hunt(&Spectrum->Wavelength,atof(pch),&i);
              if (gs_zero_indexes.indexOf(i)==-1)
                gs_zero_indexes.push_back(i);
              pch=strtok(NULL,delimiters);
            }

            fclose(csFile);
            replot_some();
        }
    }
}

void MainWindow::on_actionFit_FSRS_Spectrum_triggered()
{
    if (!Spectrum->ProbeTime.isEmpty())
    {
        QVector < QVector < double > > myFSRS;
        QVector < double > temp_raw_GS_FSRS,temp_FSRS;
        for(int i=0;i<Spectrum->ProbeTime.size();i++)
        {
            if((ui->subGSBox->isChecked())&&(!GroundState.isEmpty()))
                get_GS_FSRS(false,i,&temp_raw_GS_FSRS);
            get_FSRS(false,i,&temp_FSRS);
            myFSRS.push_back(ExcState);
        }
        /***********************************************************/
        int i0,j0;
        int its;
        QVector < double > zeros(Spectrum->Wavelength.size());
        hunt(&Spectrum->ProbeTime,-0.5,&its);
        for(i0=0;i0<its;i0++)
            for(j0=0;j0<Spectrum->Wavelength.size();j0++)
                zeros[j0]+=(myFSRS[i0][j0]/double(its));
        for(i0=0;i0<Spectrum->ProbeTime.size();i0++)
            for(j0=0;j0<Spectrum->Wavelength.size();j0++)
                myFSRS[i0][j0]-=zeros[j0];
        /***********************************************************/
        FSRSPreviewAndFit *prev;
        prev=new FSRSPreviewAndFit(this,Spectrum,&myFSRS,&DirSettings);
        prev->setAttribute(Qt::WA_DeleteOnClose,true);
        prev->setModal(true);
        prev->exec();
    }
}

void MainWindow::on_actionAdd_Gaussian_Weights_triggered()
{
    if (!Spectrum->ProbeTime.isEmpty())
    {
        GaussianWeightDialog *wght;
        wght=new GaussianWeightDialog(this,&GaussianMults);
        wght->setAttribute(Qt::WA_DeleteOnClose,true);
        wght->setModal(true);
        wght->exec();
        replot_some();
    }
}

void MainWindow::on_actionSave_On_Screen_Data_triggered()
{
    if (!Spectrum->ProbeTime.isEmpty())
    {
        /********************************
         *  0 - FSRS Curve (black)      *
         *  1 - Red Dots                *
         *  2 - Fit Curve (gray)        *
         * ******************************
         *  3 - FSRS corrected (black)  *
         *  4 - GS FSRS Correctd (gray) *
         * ******************************
         *  5 - GS FSRS (black)         *
         *  6 - Red Dots                *
         *  7 - Fit Curve (gray)        *
         *******************************/

        QString fileName=QFileDialog::getSaveFileName(this, tr("Save On-Screen Data To A Bunch Of Files"),DirSettings.value("CURRENT_RAMAN_DIR").toString(),tr("Data files (*.dat);;Text files (*.txt);;Any files (*)"));
        if(!fileName.isEmpty())
        {
            DirSettings.setValue("CURRENT_RAMAN_DIR",GetCurrentDir(fileName));
            FILE *outFile;
            const QCPDataMap *data, *picks, *curve;
            QMap<double, QCPData>::const_iterator di, pi, ci;

            /********** GS FSRS **********/
            outFile=fopen(fileName.toLatin1()+"_raw_fsrs.dat","w+");
            data =ui->specPlot->graph(0)->data();
            picks=ui->specPlot->graph(1)->data();
            curve=ui->specPlot->graph(2)->data();
            di =  data->constBegin(),
            pi = picks->constBegin(),
            ci = curve->constBegin();
            while (di != data->constEnd())
            {
                fprintf(outFile,"%15e\t%15e\t",di.value().key,di.value().value);
                if (!curve->isEmpty()) {
                    fprintf(outFile,"%15e\t", ci.value().value);
                    ++ci;
                }
                if (!picks->isEmpty()) {
                    if (pi != picks->constEnd())
                    {
                        fprintf(outFile,"%15e\t%15e\t",pi.value().key,pi.value().value);
                        ++pi;
                    }
                }
                fprintf(outFile,"\n");
                ++di;
            }
            fclose(outFile);

            /********** GS FSRS **********/
            outFile=fopen(fileName.toLatin1()+"_gs_fsrs.dat","w+");
            data =ui->specPlot->graph(5)->data();
            picks=ui->specPlot->graph(6)->data();
            curve=ui->specPlot->graph(7)->data();
            di =  data->constBegin(),
            pi = picks->constBegin(),
            ci = curve->constBegin();
            while (di != data->constEnd())
            {
                fprintf(outFile,"%15e\t%15e\t",di.value().key,di.value().value);
                if (!curve->isEmpty()) {
                    fprintf(outFile,"%15e\t", ci.value().value);
                    ++ci;
                }
                if (!picks->isEmpty()) {
                    if (pi != picks->constEnd())
                    {
                        fprintf(outFile,"%15e\t%15e\t",pi.value().key,pi.value().value);
                        ++pi;
                    }
                }
                fprintf(outFile,"\n");
                ++di;
            }
            fclose(outFile);

            /********** FSRS Done **********/
            outFile=fopen(fileName.toLatin1()+"_fsrs.dat","w+");
            data =ui->specPlot->graph(3)->data();
            curve=ui->specPlot->graph(4)->data();
            di =  data->constBegin(),
            ci = curve->constBegin();
            while (di != data->constEnd())
            {
                fprintf(outFile,"%15e\t%15e\t",di.value().key,di.value().value);
                if (!curve->isEmpty()) {
                    fprintf(outFile,"%15e\t", ci.value().value);
                    ++ci;
                }
                fprintf(outFile,"\n");
                ++di;
            }
            fclose(outFile);
        }

    }
}

void MainWindow::on_zeroSpinBox_valueChanged(double arg1)
{
    replot_some();
}

void MainWindow::on_refBox_toggled(bool checked)
{
    ui->solvBox->setEnabled(checked);
    ui->convoBox->setEnabled(checked);
    ui->convoSpinBox->setEnabled(checked && ui->convoBox->isChecked());
    replot_some();
}

/* Non-standard FSRS measurements */

void MainWindow::on_actionOpen_Temporally_Shifted_FSRS_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Raman Dataset (PDP Type)"),DirSettings.value("CURRENT_RAMAN_DIR").toString());
    if(!fileName.isEmpty())
    {
         DirSettings.setValue("CURRENT_RAMAN_DIR",GetCurrentDir(fileName));

         FILE *pFile;
         QString Header;
         char mystring[MAXLINELENGTH];

         pFile=fopen(fileName.toLatin1(),"r");
         fgets(mystring,MAXLINELENGTH,pFile);
         Header=mystring;

         if ( Header.contains("Pump-Dump-Probe",Qt::CaseInsensitive) || Header.contains("PumpDump-Probe",Qt::CaseInsensitive) ||
                       Header.contains("PumpDumpProbe",Qt::CaseInsensitive) )
         {
             fpos_t position;
             PDPTempData TempData;
             QVector<double> bksmp00,bksmp01,bksmp10,bksmp11;
             QVector<double> PumpTimes,DumpTimes,
                             TV_00,TV_01,TV_10,TV_11,
                             TA_00,TA_01,TA_10,TA_11;
             QVector< QVector < double > > WL00,WL01,WL10,WL11,
                                           AUX00,AUX01,AUX10,AUX11;
             QVector < QString > NansAndZeros;
             bool Ref=false;

             QString CurrType;
             int i,j,k,CurrScanNo,n=0,HowMany=1,npix=256;
             double CurrDelayPump,CurrDelayDump;

             Ref = (!Header.contains("Not referenced",Qt::CaseInsensitive));

             Header = "Checking...";
             while(!Header.contains("scan 1",Qt::CaseInsensitive))
             {
                 fgetpos(pFile,&position);
                 fgets(mystring,MAXLINELENGTH,pFile);
                 Header=mystring;
             }

             /**************/
             /* Pixel hack */
             /**************/
             fgets(mystring,MAXLINELENGTH,pFile);
             fgets(mystring,MAXLINELENGTH,pFile);
             Header=mystring;
             npix=Header.count("\t")+1;
             fsetpos(pFile,&position);

             if(Ref)
             {
                 bksmp00.fill(0.0,npix);
                 bksmp01.fill(0.0,npix);
                 bksmp10.fill(0.0,npix);
                 bksmp11.fill(0.0,npix);
             }
             TempData.smp00.resize(npix);
             TempData.smp01.resize(npix);
             TempData.smp10.resize(npix);
             TempData.smp11.resize(npix);
             TV_00.resize(npix);
             TV_01.resize(npix);
             TV_10.resize(npix);
             TV_11.resize(npix);
             TA_00.resize(npix);
             TA_01.resize(npix);
             TA_10.resize(npix);
             TA_11.resize(npix);

             ReadPDPPoint(pFile,&TempData,&NansAndZeros,npix,Ref);
             CurrType=TempData.NextType;
             CurrScanNo=TempData.NextScanNo;
             CurrDelayPump=TempData.NextDelayPump;
             CurrDelayDump=TempData.NextDelayDump;

             Spectrum->ClearSpectra();
             Spectrum->Wavelength.resize(npix);

             while(!TempData.FileEnd)
             {
                 ReadPDPPoint(pFile,&TempData,&NansAndZeros,npix,Ref);

                 if(TempData.FileEnd)
                 {
                     if (WL00.size()<AUX00.size())
                     {
                         WL00.push_back( QVector<double>(npix) );
                         WL01.push_back( QVector<double>(npix) );
                         WL10.push_back( QVector<double>(npix) );
                         WL11.push_back( QVector<double>(npix) );
                         for(i=0;i<npix;i++)
                         {
                             WL00[n][i]=TempData.smp00[i]-bksmp00[i];
                             WL01[n][i]=TempData.smp01[i]-bksmp01[i];
                             WL10[n][i]=TempData.smp10[i]-bksmp10[i];
                             WL11[n][i]=TempData.smp11[i]-bksmp11[i];
                             Spectrum->Wavelength[i]=i+1;
                         }
                         PumpTimes.push_back(CurrDelayPump);
                         DumpTimes.push_back(CurrDelayDump);
                         n++;
                         if(CurrScanNo<2) HowMany=n;
                     }
                     /* Priesingu atveju - paskutinis yra AUX'as ir i ji kreipti demesio nereikia. */
                     break;
                 }
                 if(CurrType.compare("Background")==0)
                 {
                     bksmp00=TempData.smp00;
                     bksmp01=TempData.smp01;
                     bksmp10=TempData.smp10;
                     bksmp11=TempData.smp11;
                 }
                 if(CurrType.compare("Off-Resonance")==0)
                 {
                     AUX00.push_back( QVector<double>(npix) );
                     AUX01.push_back( QVector<double>(npix) );
                     AUX10.push_back( QVector<double>(npix) );
                     AUX11.push_back( QVector<double>(npix) );
                     for(i=0;i<npix;i++)
                     {
                         AUX00[n][i]=TempData.smp00[i]-bksmp00[i];
                         AUX01[n][i]=TempData.smp01[i]-bksmp01[i];
                         AUX10[n][i]=TempData.smp10[i]-bksmp10[i];
                         AUX11[n][i]=TempData.smp11[i]-bksmp11[i];
                     }
                 }
                 if(CurrType.compare("Resonance")==0)
                 {
                     WL00.push_back( QVector<double>(npix) );
                     WL01.push_back( QVector<double>(npix) );
                     WL10.push_back( QVector<double>(npix) );
                     WL11.push_back( QVector<double>(npix) );
                     for(i=0;i<npix;i++)
                     {
                         WL00[n][i]=TempData.smp00[i]-bksmp00[i];
                         WL01[n][i]=TempData.smp01[i]-bksmp01[i];
                         WL10[n][i]=TempData.smp10[i]-bksmp10[i];
                         WL11[n][i]=TempData.smp11[i]-bksmp11[i];
                     }
                     PumpTimes.push_back(CurrDelayPump);
                     DumpTimes.push_back(CurrDelayDump);
                     n++;
                 }
                 if(CurrScanNo<2) HowMany=n;
                 CurrType=TempData.NextType;
                 CurrScanNo=TempData.NextScanNo;
                 CurrDelayPump=TempData.NextDelayPump;
                 CurrDelayDump=TempData.NextDelayDump;
             }
             fclose(pFile);

             // Dabar galima ideti ir Pumptimes
             for (i=0;i<n;i++)
             {
                 for (j=0;j<npix;j++)
                 {
                     /* WL check: */
                     if (WL00[i][j]<1e-20)
                     {
                         NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(DumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                         WL00[i][j]=1e-20;
                     }
                     if (WL01[i][j]<1e-20)
                     {
                         NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(DumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                         WL01[i][j]=1e-20;
                     }
                     if (WL10[i][j]<1e-20)
                     {
                         NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(DumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                         WL10[i][j]=1e-20;
                     }
                     if (WL11[i][j]<1e-20)
                     {
                         NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(DumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                         WL11[i][j]=1e-20;
                     }

                     /* AUX check: */
                     if (AUX00[i][j]<1e-20)
                     {
                         NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(DumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                         AUX00[i][j]=1e-20;
                     }
                     if (AUX01[i][j]<1e-20)
                     {
                         NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(DumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                         AUX01[i][j]=1e-20;
                     }
                     if (AUX10[i][j]<1e-20)
                     {
                         NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(DumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                         AUX10[i][j]=1e-20;
                     }
                     if (AUX11[i][j]<1e-20)
                     {
                         NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(DumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                         AUX11[i][j]=1e-20;
                     }
                 }
             }

             bool Unfinished=(n!=CurrScanNo*HowMany);
             if(Unfinished)
             {
                 for(i=0;i<n-(CurrScanNo-1)*HowMany;i++)
                 {
                     PumpTimes.pop_back();
                     DumpTimes.pop_back();
                     WL00.pop_back();
                     WL01.pop_back();
                     WL10.pop_back();
                     WL11.pop_back();
                     AUX00.pop_back();
                     AUX01.pop_back();
                     AUX10.pop_back();
                     AUX11.pop_back();
                 }
                 CurrScanNo--;
             }
             if (Unfinished||(!NansAndZeros.isEmpty()))
             {
                 QString UnfMeas="Unfinished measurement. Displaying only the completed scans.";
                 if (NansAndZeros.isEmpty())
                 {
                     QMessageBox msgBox;
                     msgBox.setText(UnfMeas);
                     msgBox.exec();
                 }
                 else
                 {
                     if (Unfinished) NansAndZeros.push_front(UnfMeas);
                     NaNReportDialog *nanReport;
                     nanReport=new NaNReportDialog(this,&NansAndZeros);
                     nanReport->setAttribute(Qt::WA_DeleteOnClose,true);
                     nanReport->setModal(true);
                     nanReport->exec();
                 }
             }

             /********************************/
             /****** Delay line selection ****/
             /********************************/

             double TempPT, TempDT, Mean_PT, Mean_DT, STD_PT, STD_DT;
             bksmp00.resize(HowMany); // Pumpdelays
             bksmp01.resize(HowMany); // Dumpdelays
             for(j=0;j<HowMany;j++)
             {
                 TempPT=TempDT=0.0;
                 for(i=0;i<CurrScanNo;i++)
                 {
                     TempPT+=PumpTimes[j+i*HowMany];
                     TempDT-=DumpTimes[j+i*HowMany];
                 }
                 bksmp00[j]=TempPT/CurrScanNo;
                 bksmp01[j]=TempDT/CurrScanNo;
             }
             MeanAndSTD(&bksmp00,&Mean_PT,&STD_PT);
             MeanAndSTD(&bksmp01,&Mean_DT,&STD_DT);
             if(STD_PT>=STD_DT)
                 Spectrum->ProbeTime=bksmp00;
             else
                 Spectrum->ProbeTime=bksmp01;

             /********************************/
             /********************************/
             /********************************/

             bksmp00=Spectrum->ProbeTime; /* dummy */
             shell_sort_Raman(Spectrum->ProbeTime,WL00,WL01,WL10,WL11,CurrScanNo);
             shell_sort_Raman(bksmp00,AUX00,AUX01,AUX10,AUX11,CurrScanNo);

             //qDebug() << t.elapsed();

             ////////////////////////////
             //// Palyginimo langas: ////
             ////////////////////////////

             QVector < double > Weights;
             double summ=0.0;
             Weights.fill(1,CurrScanNo);

             QVector < QVector < double > > LogGain=WL11;
             for(i=0;i<LogGain.size();i++)
                 for(j=0;j<npix;j++)
                     LogGain[i][j]=-1000.0*log10(LogGain[i][j]/WL01[i][j]);

             CompareTraces *compWindow;
             compWindow=new CompareTraces(this,&LogGain,&Spectrum->ProbeTime,CurrScanNo,&Weights);
             compWindow->setAttribute(Qt::WA_DeleteOnClose,true);
             compWindow->setModal(true);
             compWindow->exec();

             ////////////////////////////
             ////////////////////////////
             ////////////////////////////

             for(i=0;i<CurrScanNo;i++)
                 summ += Weights[i];
             if (summ==0) summ=1;

             for(j=0;j<HowMany;j++)
             {
                 /* Sig: */
                 TV_00.fill(0.0);
                 TV_01.fill(0.0);
                 TV_10.fill(0.0);
                 TV_11.fill(0.0);
                 /* Aux: */
                 TA_00.fill(0.0);
                 TA_01.fill(0.0);
                 TA_10.fill(0.0);
                 TA_11.fill(0.0);
                 for(i=0;i<CurrScanNo;i++)
                 {
                     for(k=0;k<npix;k++)
                     {
                         /* Sig: */
                         TV_00[k]+=WL00[j+i*HowMany][k]*Weights[i]/summ;
                         TV_01[k]+=WL01[j+i*HowMany][k]*Weights[i]/summ;
                         TV_10[k]+=WL10[j+i*HowMany][k]*Weights[i]/summ;
                         TV_11[k]+=WL11[j+i*HowMany][k]*Weights[i]/summ;
                         /* Aux: */
                         TA_00[k]+=AUX00[j+i*HowMany][k]*Weights[i]/summ;
                         TA_01[k]+=AUX01[j+i*HowMany][k]*Weights[i]/summ;
                         TA_10[k]+=AUX10[j+i*HowMany][k]*Weights[i]/summ;
                         TA_11[k]+=AUX11[j+i*HowMany][k]*Weights[i]/summ;
                     }
                 }
                 /* Sig: */
                 Spectrum->WL00.push_back(TV_00);
                 Spectrum->WL01.push_back(TV_01);
                 Spectrum->WL10.push_back(TV_10);
                 Spectrum->WL11.push_back(TV_11);
                 /* Aux: */
                 Spectrum->WL00.push_back(TA_00);
                 Spectrum->AUX01.push_back(TA_01);
                 Spectrum->WL10.push_back(TA_10);
                 Spectrum->AUX11.push_back(TA_11);
             }

             Spectrum->FSRS=Spectrum->WL11;
             for(i=0;i<Spectrum->ProbeTime.size();i++)
                 for(j=0;j<Spectrum->Wavelength.size();j++)
                     Spectrum->FSRS[i][j]=1000*log10(Spectrum->WL11[i][j]*Spectrum->WL00[i][j]/Spectrum->WL01[i][j]/Spectrum->WL10[i][j]);

             ui->delaySlider->setMinimum(0);
             ui->delaySlider->setMaximum(Spectrum->ProbeTime.size()-1);
             ui->zeroSpinBox->setMinimum(Spectrum->ProbeTime.first());
             ui->zeroSpinBox->setMaximum(Spectrum->ProbeTime.last());
             setWindowTitle(fileName);
             if (!Ref) reverse_wavenumber();
             replot_some();
         }
         else
         {
             fclose(pFile);
             QMessageBox msgBox;
             msgBox.setText("Invalid file.");
             msgBox.exec();
         }
    }
}

void MainWindow::Open_PDP_Type_Spectrally_Shifted_File(QString fileName)
{
    FILE *pFile;
    QString Header;
    char mystring[MAXLINELENGTH];

    pFile=fopen(fileName.toLatin1(),"r");
    fgets(mystring,MAXLINELENGTH,pFile);
    Header=mystring;

    fpos_t position;
    PDPTempData TempData;
    QVector<double> bksmp00,bksmp01,bksmp10,bksmp11,
                    bkaux01,bkaux11;
    QVector<double> PumpTimes,DumpTimes,
                    TV_00,TV_01,TV_10,TV_11,
                    TA_01,TA_11;
    QVector< QVector < double > > WL00,WL01,WL10,WL11,
                                  AUX01,AUX11;
    QVector < QString > NansAndZeros;
    // bool Ref=false;

    QString CurrType;
    int i,j,k,CurrScanNo,n=0,HowMany=1,npix=256;
    double CurrDelayPump,CurrDelayDump;

    for (i=0;i<3;i++) fgets(mystring,MAXLINELENGTH,pFile);
    fgetpos(pFile,&position);
    for (i=0;i<3;i++) fgets(mystring,MAXLINELENGTH,pFile);
    fsetpos(pFile,&position);

    Header=mystring;
    npix=Header.count("\t")+1;

    TempData.smp00.resize(npix);
    TempData.smp01.resize(npix);
    TempData.smp10.resize(npix);
    TempData.smp11.resize(npix);
    TempData.aux01.resize(npix);
    TempData.aux11.resize(npix);
    TV_00.resize(npix);
    TV_01.resize(npix);
    TV_10.resize(npix);
    TV_11.resize(npix);
    TA_01.resize(npix);
    TA_11.resize(npix);

    ReadFSRSPoint(pFile,&TempData,&NansAndZeros,npix);
    CurrType=TempData.NextType;
    CurrScanNo=TempData.NextScanNo;
    CurrDelayPump=TempData.NextDelayPump;
    CurrDelayDump=TempData.NextDelayDump;

    Spectrum->ClearSpectra();
    Spectrum->Wavelength.resize(npix);

    while(!TempData.FileEnd)
    {
        ReadFSRSPoint(pFile,&TempData,&NansAndZeros,npix);

        if(TempData.FileEnd)
        {
            /* Sig: */
            WL00.push_back(QVector<double>(npix));
            WL01.push_back(QVector<double>(npix));
            WL10.push_back(QVector<double>(npix));
            WL11.push_back(QVector<double>(npix));
            /* Aux: */
            AUX01.push_back(QVector<double>(npix));
            AUX11.push_back(QVector<double>(npix));
            for(i=0;i<npix;i++)
            {
                /* Sig: */
                WL00[n][i]=TempData.smp00[npix-1-i]-bksmp00[npix-1-i];
                WL01[n][i]=TempData.smp01[npix-1-i]-bksmp01[npix-1-i];
                WL10[n][i]=TempData.smp10[npix-1-i]-bksmp10[npix-1-i];
                WL11[n][i]=TempData.smp11[npix-1-i]-bksmp11[npix-1-i];
                Spectrum->Wavelength[i]=i+1;
                /* Aux: */
                AUX01[n][i]=TempData.aux01[npix-1-i]-bkaux01[npix-1-i];
                AUX11[n][i]=TempData.aux11[npix-1-i]-bkaux11[npix-1-i];
            }
            PumpTimes.push_back(CurrDelayPump);
            DumpTimes.push_back(CurrDelayDump);
            n++;
            if(CurrScanNo<2) HowMany=n;
            break;
        }
        if(CurrType.compare("Background")==0)
        {
            /* Sig: */
            bksmp00=TempData.smp00;
            bksmp01=TempData.smp01;
            bksmp10=TempData.smp10;
            bksmp11=TempData.smp11;
            /* Aux: */
            bkaux01=TempData.aux01;
            bkaux11=TempData.aux11;
        }
        if(CurrType.compare("Measurement")==0)
        {
            /* Sig: */
            WL00.push_back(QVector<double>(npix));
            WL01.push_back(QVector<double>(npix));
            WL10.push_back(QVector<double>(npix));
            WL11.push_back(QVector<double>(npix));
            /* Aux: */
            AUX01.push_back(QVector<double>(npix));
            AUX11.push_back(QVector<double>(npix));
            for(i=0;i<npix;i++)
            {
                /* Sig: */
                WL00[n][i]=TempData.smp00[npix-1-i]-bksmp00[npix-1-i];
                WL01[n][i]=TempData.smp01[npix-1-i]-bksmp01[npix-1-i];
                WL10[n][i]=TempData.smp10[npix-1-i]-bksmp10[npix-1-i];
                WL11[n][i]=TempData.smp11[npix-1-i]-bksmp11[npix-1-i];
                /* Aux: */
                AUX01[n][i]=TempData.aux01[npix-1-i]-bkaux01[npix-1-i];
                AUX11[n][i]=TempData.aux11[npix-1-i]-bkaux11[npix-1-i];
            }
            PumpTimes.push_back(CurrDelayPump);
            DumpTimes.push_back(CurrDelayDump);
            n++;
        }
        if(CurrScanNo<2) HowMany=n;
        CurrType=TempData.NextType;
        CurrScanNo=TempData.NextScanNo;
        CurrDelayPump=TempData.NextDelayPump;
        CurrDelayDump=TempData.NextDelayDump;
    }
    fclose(pFile);

    // Dabar galima ideti ir Pumptimes
    for (i=0;i<n;i++)
    {
        for (j=0;j<npix;j++)
        {
            /* WL check: */
            if (WL00[i][j]<1e-20)
            {
                NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(DumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                WL00[i][j]=1e-20;
            }
            if (WL01[i][j]<1e-20)
            {
                NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(DumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                WL01[i][j]=1e-20;
            }
            if (WL10[i][j]<1e-20)
            {
                NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(DumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                WL10[i][j]=1e-20;
            }
            if (WL11[i][j]<1e-20)
            {
                NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(DumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                WL11[i][j]=1e-20;
            }
            /* AUX check: */
            if (AUX01[i][j]<1e-20)
            {
                NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(DumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                AUX01[i][j]=1e-20;
            }
            if (AUX11[i][j]<1e-20)
            {
                NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(DumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                AUX11[i][j]=1e-20;
            }
        }
    }

    bool Unfinished=(n!=CurrScanNo*HowMany);
    if(Unfinished)
    {
        for(i=0;i<n-(CurrScanNo-1)*HowMany;i++)
        {
            PumpTimes.pop_back();
            DumpTimes.pop_back();
            /* Sig: */
            WL00.pop_back();
            WL01.pop_back();
            WL10.pop_back();
            WL11.pop_back();
            /* Aux: */
            AUX01.pop_back();
            AUX11.pop_back();
        }
        CurrScanNo--;
    }
    if (Unfinished||(!NansAndZeros.isEmpty()))
    {
        QString UnfMeas="Unfinished measurement. Displaying only the completed scans.";
        if (NansAndZeros.isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText(UnfMeas);
            msgBox.exec();
        }
        else
        {
            if (Unfinished) NansAndZeros.push_front(UnfMeas);
            NaNReportDialog *nanReport;
            nanReport=new NaNReportDialog(this,&NansAndZeros);
            nanReport->setAttribute(Qt::WA_DeleteOnClose,true);
            nanReport->setModal(true);
            nanReport->exec();
        }
    }

    /********************************/
    /****** Delay line selection ****/
    /********************************/

    double TempPT, TempDT, Mean_PT, Mean_DT, STD_PT, STD_DT;
    bksmp00.resize(HowMany); // Pumpdelays
    bksmp01.resize(HowMany); // Dumpdelays
    for(j=0;j<HowMany;j++)
    {
        TempPT=TempDT=0.0;
        for(i=0;i<CurrScanNo;i++)
        {
            TempPT+=PumpTimes[j+i*HowMany];
            TempDT-=DumpTimes[j+i*HowMany];
        }
        bksmp00[j]=TempPT/CurrScanNo;
        bksmp01[j]=TempDT/CurrScanNo;
    }
    MeanAndSTD(&bksmp00,&Mean_PT,&STD_PT);
    MeanAndSTD(&bksmp01,&Mean_DT,&STD_DT);
    if(STD_PT>=STD_DT)
        Spectrum->ProbeTime=bksmp00;
    else
        Spectrum->ProbeTime=bksmp01;

    /********************************/
    /********************************/
    /********************************/

    // Dummies:
    bksmp00=Spectrum->ProbeTime;
    QVector < QVector < double > > dumm1 = WL00, dumm2 =WL00;

    shell_sort_Raman(Spectrum->ProbeTime,WL00,WL01,WL10,WL11,CurrScanNo);
    shell_sort_Raman(bksmp00,dumm1,AUX01,dumm2,AUX11,CurrScanNo);

    //qDebug() << t.elapsed();

    ////////////////////////////
    //// Palyginimo langas: ////
    ////////////////////////////

    QVector < double > Weights;
    double summ=0.0;
    Weights.fill(1,CurrScanNo);

    QVector < QVector < double > > LogGain=WL11;
    for(i=0;i<LogGain.size();i++)
        for(j=0;j<npix;j++)
            LogGain[i][j]=-1000.0*log10(LogGain[i][j]/WL01[i][j]);

    CompareTraces *compWindow;
    compWindow=new CompareTraces(this,&LogGain,&Spectrum->ProbeTime,CurrScanNo,&Weights);
    compWindow->setAttribute(Qt::WA_DeleteOnClose,true);
    compWindow->setModal(true);
    compWindow->exec();

    ////////////////////////////
    ////////////////////////////
    ////////////////////////////

    for(i=0;i<CurrScanNo;i++)
        summ += Weights[i];
    if (summ==0.0) summ=1.0;

    for(j=0;j<HowMany;j++)
    {
        /* Sig: */
        TV_00.fill(0.0);
        TV_01.fill(0.0);
        TV_10.fill(0.0);
        TV_11.fill(0.0);
        /* Aux: */
        TA_01.fill(0.0);
        TA_11.fill(0.0);
        for(i=0;i<CurrScanNo;i++)
        {
            for(k=0;k<npix;k++)
            {
                /* Sig: */
                TV_00[k]+=WL00[j+i*HowMany][k]*Weights[i]/summ;
                TV_01[k]+=WL01[j+i*HowMany][k]*Weights[i]/summ;
                TV_10[k]+=WL10[j+i*HowMany][k]*Weights[i]/summ;
                TV_11[k]+=WL11[j+i*HowMany][k]*Weights[i]/summ;
                /* Aux: */
                TA_01[k]+=AUX01[j+i*HowMany][k]*Weights[i]/summ;
                TA_11[k]+=AUX11[j+i*HowMany][k]*Weights[i]/summ;
            }
        }
        /* Sig: */
        Spectrum->WL00.push_back(TV_00);
        Spectrum->WL01.push_back(TV_01);
        Spectrum->WL10.push_back(TV_10);
        Spectrum->WL11.push_back(TV_11);
        /* Aux: */
        Spectrum->AUX01.push_back(TA_01);
        Spectrum->AUX11.push_back(TA_11);
    }

    Spectrum->FSRS=Spectrum->WL11;
    for(i=0;i<Spectrum->ProbeTime.size();i++)
        for(j=0;j<Spectrum->Wavelength.size();j++)
            Spectrum->FSRS[i][j]=1000.0*log10(Spectrum->WL11[i][j]*Spectrum->WL00[i][j]/Spectrum->WL01[i][j]/Spectrum->WL10[i][j]);

    Spectrum->Type=PDPRawAux;

    ui->delaySlider->setMinimum(0);
    ui->delaySlider->setMaximum(Spectrum->ProbeTime.size()-1);
    ui->zeroSpinBox->setMinimum(Spectrum->ProbeTime.first());
    ui->zeroSpinBox->setMaximum(Spectrum->ProbeTime.last());
    setWindowTitle(fileName);
    // if (!Ref) reverse_wavenumber();
    replot_some();
}

void MainWindow::Open_PP_Type_Spectrally_Shifted_File(QString fileName)
{
    FILE *pFile;
    QString Header;
    fpos_t position;
    PDPTempData TempData;
    char mystring[MAXLINELENGTH];
    QVector<double> bksmp00,bksmp01,bkaux01;
    QVector<double> PumpTimes,TV_00,TV_01,TA_01;
    QVector< QVector < double > > WL00,WL01,AUX01;
    QVector < QString > NansAndZeros;

    QString CurrType;
    int i,j,k,CurrScanNo,n=0,HowMany=1,npix=256;
    double CurrDelayPump;

    pFile=fopen(fileName.toLatin1(),"r");
    Header = "Checking...";
    while(!Header.contains("scan 1",Qt::CaseInsensitive))
    {
        fgetpos(pFile,&position);
        fgets(mystring,MAXLINELENGTH,pFile);
        Header=mystring;
    }
    fgets(mystring,MAXLINELENGTH,pFile);
    fgets(mystring,MAXLINELENGTH,pFile);
    Header=mystring;
    npix=Header.count("\t")+1;
    fsetpos(pFile,&position);

    TempData.smp00.resize(npix);
    TempData.smp01.resize(npix);
    TempData.aux01.resize(npix);
    TV_00.resize(npix);
    TV_01.resize(npix);
    TA_01.resize(npix);

    ReadGSFSRSPoint(pFile,&TempData,&NansAndZeros,npix);
    CurrType=TempData.NextType;
    CurrScanNo=TempData.NextScanNo;
    CurrDelayPump=TempData.NextDelayPump;

    Spectrum->ClearSpectra();
    Spectrum->Wavelength.resize(npix);

    while(!TempData.FileEnd)
    {
        ReadGSFSRSPoint(pFile,&TempData,&NansAndZeros,npix);
        if(TempData.FileEnd)
        {
            /* Sig: */
            WL00.push_back(QVector<double>(npix));
            WL01.push_back(QVector<double>(npix));
            /* Aux: */
            AUX01.push_back(QVector<double>(npix));
            for(i=0;i<npix;i++)
            {
                /* Sig: */
                WL00[n][i]=TempData.smp00[npix-1-i]-bksmp00[npix-1-i];
                WL01[n][i]=TempData.smp01[npix-1-i]-bksmp01[npix-1-i];
                Spectrum->Wavelength[i]=i+1;
                /* Aux: */
                AUX01[n][i]=TempData.aux01[npix-1-i]-bkaux01[npix-1-i];
            }
            PumpTimes.push_back(CurrDelayPump);
            n++;
            if(CurrScanNo<2) HowMany=n;
            break;
        }
        if(CurrType.compare("Background")==0)
        {
            /* Sig: */
            bksmp00=TempData.smp00;
            bksmp01=TempData.smp01;
            /* Aux: */
            bkaux01=TempData.aux01;
        }
        if(CurrType.compare("Measurement")==0)
        {
            /* Sig: */
            WL00.push_back(QVector<double>(npix));
            WL01.push_back(QVector<double>(npix));
            /* Aux: */
            AUX01.push_back(QVector<double>(npix));
            for(i=0;i<npix;i++)
            {
                /* Sig: */
                WL00[n][i]=TempData.smp00[npix-1-i]-bksmp00[npix-1-i];
                WL01[n][i]=TempData.smp01[npix-1-i]-bksmp01[npix-1-i];
                /* Aux: */
                AUX01[n][i]=TempData.aux01[npix-1-i]-bkaux01[npix-1-i];
            }
            PumpTimes.push_back(CurrDelayPump);
            n++;
        }
        if(CurrScanNo<2) HowMany=n;
        CurrType=TempData.NextType;
        CurrScanNo=TempData.NextScanNo;
        CurrDelayPump=TempData.NextDelayPump;
    }
    fclose(pFile);

    for (i=0;i<n;i++)
    {
        for (j=0;j<npix;j++)
        {
            if (fabs(WL00[i][j])<1e-20)
            {
                NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(PumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                WL00[i][j]=1e-20;
            }
            if (fabs(WL01[i][j])<1e-20)
            {
                NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(PumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                WL01[i][j]=1e-20;
            }
            /* AUX check: */
            if (fabs(AUX01[i][j])<1e-20)
            {
                NansAndZeros.append("INF in scan " + QString::number(i/HowMany+1) + " @ " + QString::number(PumpTimes[i],'f',1) + " ps, " + QString::number(j+1) + " nm");
                AUX01[i][j]=1e-20;
            }
        }
    }

    bool Unfinished=(n!=CurrScanNo*HowMany);
    if(Unfinished)
    {
        for(i=0;i<n-(CurrScanNo-1)*HowMany;i++)
        {
            PumpTimes.pop_back();
            /* Sig: */
            WL00.pop_back();
            WL01.pop_back();
            /* Aux: */
            AUX01.pop_back();
        }
        CurrScanNo--;
    }
    if (Unfinished||(!NansAndZeros.isEmpty()))
    {
        QString UnfMeas="Unfinished measurement. Displaying only the completed scans.";
        if (NansAndZeros.isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText(UnfMeas);
            msgBox.exec();
        }
        else
        {
            if (Unfinished) NansAndZeros.push_front(UnfMeas);
            NaNReportDialog *nanReport;
            nanReport=new NaNReportDialog(this,&NansAndZeros);
            nanReport->setAttribute(Qt::WA_DeleteOnClose,true);
            nanReport->setModal(true);
            nanReport->exec();
        }
    }

    double TempF;
    for(j=0;j<HowMany;j++)
    {
        TempF=0.0;
        for(i=0;i<CurrScanNo;i++)
            TempF+=PumpTimes[j+i*HowMany];
        Spectrum->ProbeTime.push_back(TempF/CurrScanNo);
    }

    shell_sort_PDP(Spectrum->ProbeTime,WL00,WL01,AUX01,CurrScanNo);

    //qDebug() << t.elapsed();

    ////////////////////////////
    //// Palyginimo langas: ////
    ////////////////////////////

    QVector < double > Weights;
    double summ=0.0;
    Weights.fill(1,CurrScanNo);

    QVector < QVector < double > > LogGain=WL00;
    for(i=0;i<LogGain.size();i++)
        for(j=0;j<npix;j++)
            LogGain[i][j]=-1000.0*log10(LogGain[i][j]/WL01[i][j]);

    CompareTraces *compWindow;
    compWindow=new CompareTraces(this,&LogGain,&Spectrum->ProbeTime,CurrScanNo,&Weights);
    compWindow->setAttribute(Qt::WA_DeleteOnClose,true);
    compWindow->setModal(true);
    compWindow->exec();

    ////////////////////////////
    ////////////////////////////
    ////////////////////////////

    for(i=0;i<CurrScanNo;i++)
        summ += Weights[i];
    if (summ==0.0) summ=1.0;

    for(j=0;j<HowMany;j++)
    {
        /* Sig: */
        TV_00.fill(0.0);
        TV_01.fill(0.0);
        /* Aux: */
        TA_01.fill(0.0);
        for(i=0;i<CurrScanNo;i++)
        {
            for(k=0;k<npix;k++)
            {
                /* Sig: */
                TV_00[k]+=WL00[j+i*HowMany][k]*Weights[i]/summ;
                TV_01[k]+=WL01[j+i*HowMany][k]*Weights[i]/summ;
                /* Aux: */
                TA_01[k]+=AUX01[j+i*HowMany][k]*Weights[i]/summ;
            }
        }
        /* Sig: */
        Spectrum->WL00.push_back(TV_00);
        Spectrum->WL01.push_back(TV_01);
        /* Aux: */
        Spectrum->AUX01.push_back(TA_01);
    }

    Spectrum->Type=PPRawAux;

    ui->delaySlider->setMinimum(0);
    ui->delaySlider->setMaximum(Spectrum->ProbeTime.size()-1);
    ui->zeroSpinBox->setMinimum(Spectrum->ProbeTime.first());
    ui->zeroSpinBox->setMaximum(Spectrum->ProbeTime.last());
    setWindowTitle(fileName);
    replot_some();
}

void MainWindow::on_actionOpen_Spectrally_Shifted_FSRS_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Raman Dataset (PDP Type)"),DirSettings.value("CURRENT_RAMAN_DIR").toString());
    if(!fileName.isEmpty())
    {
         DirSettings.setValue("CURRENT_RAMAN_DIR",GetCurrentDir(fileName));

         FILE *pFile;
         QString Header;
         char mystring[MAXLINELENGTH];

         pFile=fopen(fileName.toLatin1(),"r");
         fgets(mystring,MAXLINELENGTH,pFile);
         fclose(pFile);
         Header=mystring;

         // Kvailas patikrinimas, ateityje sugalvosime geriau.
         if ( Header.contains("Not referenced",Qt::CaseInsensitive) &&
             (Header.contains("Pump-Dump-Probe",Qt::CaseInsensitive) || Header.contains("PumpDump-Probe",Qt::CaseInsensitive) ||
              Header.contains("PumpDumpProbe",Qt::CaseInsensitive)) )
         {
             Open_PDP_Type_Spectrally_Shifted_File(fileName);
         }
         else if ( Header.contains("Not referenced",Qt::CaseInsensitive) &&
                   (Header.contains("Pump-Probe",Qt::CaseInsensitive) || Header.contains("PumpProbe",Qt::CaseInsensitive) ) )
         {
             Open_PP_Type_Spectrally_Shifted_File(fileName);
         }
         else
         {
             QMessageBox msgBox;
             msgBox.setText("Invalid file.");
             msgBox.exec();
         }
    }
}

void MainWindow::ReadGSFSRSPoint(FILE* pFile, PDPTempData *Temp, QVector < QString > *NaNs, int npix)
{
    /* Currently without reference: */
    QString Str;
    QStringList List;
    bool Unfinished=true;
    int i,n00,n01,a01;
    double point;
    char LongLine[MAXLINELENGTH];

    Temp->smp00.fill(0.0);
    Temp->smp01.fill(0.0);
    Temp->aux01.fill(0.0);
    n00=n01=a01=0;

    while(Unfinished)
    {
        Temp->FileEnd=false;
        if(fgets(LongLine,MAXLINELENGTH,pFile)==NULL)
        {
            Temp->FileEnd=true;
            break;
        }
        if(strncmp(LongLine,"Pump",2)==0)
        {
            fscanf(pFile,"%lf",&point);
            if (!_isnan(point))
            {
                Temp->smp01[0]+=point;
                for(i=1;i<npix;i++)
                {
                    fscanf(pFile,"%lf",&point);
                    Temp->smp01[i]+=point;
                }
                n01++;
            }
            else
            {
                NaNs->append("NaNs in scan " + QString::number(Temp->NextScanNo) + " @ " + QString::number(Temp->NextDelayDump,'f',2) + " ps");
            }
            fgets(LongLine,MAXLINELENGTH,pFile);

            fscanf(pFile,"%lf",&point);
            if (!_isnan(point))
            {
                Temp->aux01[0]+=point;
                for(i=1;i<npix;i++)
                {
                    fscanf(pFile,"%lf",&point);
                    Temp->aux01[i]+=point;
                }
                a01++;
            }
            else
            {
                NaNs->append("NaNs in scan " + QString::number(Temp->NextScanNo) + " @ " + QString::number(Temp->NextDelayDump,'f',2) + " ps");
            }
            fgets(LongLine,MAXLINELENGTH,pFile);

            fscanf(pFile,"%lf",&point);
            if (!_isnan(point))
            {
                Temp->smp00[0]+=point;
                for(i=1;i<npix;i++)
                {
                    fscanf(pFile,"%lf",&point);
                    Temp->smp00[i]+=point;
                }
                n00++;
            }
            else
            {
                NaNs->append("NaNs in scan " + QString::number(Temp->NextScanNo) + " @ " + QString::number(Temp->NextDelayDump,'f',2) + " ps");
            }
            fgets(LongLine,MAXLINELENGTH,pFile);
            fgets(LongLine,MAXLINELENGTH,pFile);
        }
        else
        {
            Unfinished=false;
            Str=LongLine;
            Str.remove(",");
            List=Str.split(" ",QString::SkipEmptyParts);
            Temp->NextType=List.at(0);
            Temp->NextScanNo=List.at(2).toInt();
            Temp->NextDelayPump=List.at(4).toDouble();
        }
    }
    for(i=0;i<npix;i++)
    {
        Temp->smp00[i]/=n00;
        Temp->smp01[i]/=n01;
        Temp->aux01[i]/=a01;
    }
}

void MainWindow::ReadFSRSPoint(FILE* pFile, PDPTempData *Temp, QVector < QString > *NaNs, int npix)
{
    /* Currently without reference: */
    QString Str;
    QStringList List;
    bool Unfinished=true;
    int i, n00, n01, n10, n11, a01, a11;
    double point;
    char LongLine[MAXLINELENGTH];

    Temp->smp00.fill(0.0);
    Temp->smp01.fill(0.0);
    Temp->smp10.fill(0.0);
    Temp->smp11.fill(0.0);
    Temp->aux01.fill(0.0);
    Temp->aux11.fill(0.0);
    n00=n01=n10=n11=a01=a11=0;

    while(Unfinished)
    {
        Temp->FileEnd=false;
        if(fgets(LongLine,MAXLINELENGTH,pFile)==NULL)
        {
            Temp->FileEnd=true;
            break;
        }
        if(strncmp(LongLine,"Pump",2)==0)
        {
            fscanf(pFile,"%lf",&point);
            if (!_isnan(point))
            {
                Temp->smp00[0]+=point;
                for(i=1;i<npix;i++)
                {
                    fscanf(pFile,"%lf",&point);
                    Temp->smp00[i]+=point;
                }
                n00++;
            }
            else
            {
                NaNs->append("NaNs in scan " + QString::number(Temp->NextScanNo) + " @ " + QString::number(Temp->NextDelayDump,'f',2) + " ps");
            }
            fgets(LongLine,MAXLINELENGTH,pFile);
            fgets(LongLine,MAXLINELENGTH,pFile);

            fscanf(pFile,"%lf",&point);
            if (!_isnan(point))
            {
                Temp->smp01[0]+=point;
                for(i=1;i<npix;i++)
                {
                    fscanf(pFile,"%lf",&point);
                    Temp->smp01[i]+=point;
                }
                n01++;
            }
            else
            {
                NaNs->append("NaNs in scan " + QString::number(Temp->NextScanNo) + " @ " + QString::number(Temp->NextDelayDump,'f',2) + " ps");
            }
            fgets(LongLine,MAXLINELENGTH,pFile);
            fscanf(pFile,"%lf",&point);
            if (!_isnan(point))
            {
                Temp->aux01[0]+=point;
                for(i=1;i<npix;i++)
                {
                    fscanf(pFile,"%lf",&point);
                    Temp->aux01[i]+=point;
                }
                a01++;
            }
            else
            {
                NaNs->append("NaNs in scan " + QString::number(Temp->NextScanNo) + " @ " + QString::number(Temp->NextDelayDump,'f',2) + " ps");
            }
            fgets(LongLine,MAXLINELENGTH,pFile);

            fscanf(pFile,"%lf",&point);
            if (!_isnan(point))
            {
                Temp->smp10[0]+=point;
                for(i=1;i<npix;i++)
                {
                    fscanf(pFile,"%lf",&point);
                    Temp->smp10[i]+=point;
                }
                n10++;
            }
            else
            {
                NaNs->append("NaNs in scan " + QString::number(Temp->NextScanNo) + " @ " + QString::number(Temp->NextDelayDump,'f',2) + " ps");
            }
            fgets(LongLine,MAXLINELENGTH,pFile);
            fgets(LongLine,MAXLINELENGTH,pFile);

            fscanf(pFile,"%lf",&point);
            if (!_isnan(point))
            {
                Temp->smp11[0]+=point;
                for(i=1;i<npix;i++)
                {
                    fscanf(pFile,"%lf",&point);
                    Temp->smp11[i]+=point;
                }
                n11++;
            }
            else
            {
                NaNs->append("NaNs in scan " + QString::number(Temp->NextScanNo) + " @ " + QString::number(Temp->NextDelayDump,'f',2) + " ps");
            }
            fgets(LongLine,MAXLINELENGTH,pFile);
            fscanf(pFile,"%lf",&point);
            if (!_isnan(point))
            {
                Temp->aux11[0]+=point;
                for(i=1;i<npix;i++)
                {
                    fscanf(pFile,"%lf",&point);
                    Temp->aux11[i]+=point;
                }
                a11++;
            }
            else
            {
                NaNs->append("NaNs in scan " + QString::number(Temp->NextScanNo) + " @ " + QString::number(Temp->NextDelayDump,'f',2) + " ps");
            }
            fgets(LongLine,MAXLINELENGTH,pFile);
        }
        else
        {
            Unfinished=false;
            Str=LongLine;
            Str.remove(",");
            List=Str.split(" ",QString::SkipEmptyParts);
            Temp->NextType=List.at(0);
            Temp->NextScanNo=List.at(2).toInt();
            Temp->NextDelayPump=List.at(4).toDouble();
            Temp->NextDelayDump=List.at(6).toDouble();
        }
    }
    for(i=0;i<npix;i++)
    {
        Temp->smp00[i]/=n00;
        Temp->smp01[i]/=n01;
        Temp->smp10[i]/=n10;
        Temp->smp11[i]/=n11;
        Temp->aux01[i]/=a01;
        Temp->aux11[i]/=a11;
    }
}

void MainWindow::on_convoBox_toggled(bool checked)
{
    ui->convoSpinBox->setEnabled(checked);
    LoadReferenceSolvent(ui->solvBox->getCurrentSolvent());
}

void MainWindow::on_convoSpinBox_valueChanged(double arg1)
{
    LoadReferenceSolvent(ui->solvBox->getCurrentSolvent());
}

void MainWindow::on_actionPreview_Difference_Spectrum_triggered()
{
    if ((!Spectrum->AUX01.isEmpty()) && (!Spectrum->AUX11.isEmpty()))
    {
        QVector < QVector < double > > myFSRS;
        QVector < double > temp_raw_GS_FSRS, temp_FSRS, aGS, aFSRS, temp;
        for(int i=0;i<Spectrum->ProbeTime.size();i++)
        {
            if((ui->subGSBox->isChecked())&&(!GroundState.isEmpty()))
                get_GS_FSRS(false,i,&temp_raw_GS_FSRS,&aGS);
            get_FSRS(false,i,&temp_FSRS,&aFSRS);

            temp.clear();
            for(int j=0;j<temp_FSRS.size();j++)
                //temp << (temp_FSRS[j]-aFSRS[j]);
                temp << (ExcState[j]-AuxExcState[j]);
            myFSRS.push_back(temp);
        }
        PreviewSpectrum(&myFSRS);

        QString fileName=QFileDialog::getSaveFileName(this, tr("Save Spectrum Data"),DirSettings.value("CURRENT_RAMAN_DIR").toString(),tr("Data files (*.dat);;Text files (*.txt);;Any files (*)"));
        if(!fileName.isEmpty())
        {
            DirSettings.setValue("CURRENT_RAMAN_DIR",GetCurrentDir(fileName));

            FILE *outFile;
            outFile=fopen(fileName.toLatin1(),"w+");
            fprintf(outFile,"%15i\t",0);
            for(int i=0;i<Spectrum->Wavelength.size();i++)
                fprintf(outFile,"%15e\t",Spectrum->Wavelength[i]);
            fprintf(outFile,"\n");

            for(int i=0;i<Spectrum->ProbeTime.size();i++)
            {
                fprintf(outFile,"%15e\t",Spectrum->ProbeTime[i]);
                for(int j=0;j<Spectrum->Wavelength.size();j++)
                    fprintf(outFile,"%15e\t",myFSRS[i][j]);
                fprintf(outFile,"\n");
            }
            fclose(outFile);
        }
    }
}
