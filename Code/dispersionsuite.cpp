#include "dispersionsuite.h"
#include "ui_dispersionsuite.h"

DispersionSuite::DispersionSuite(QWidget *parent, SpectrumData *mainData, QSettings *mainSettings) :
    QDialog(parent),
    ui(new Ui::DispersionSuite),
    mData(mainData), mSettings(mainSettings),
    HintMap(QVector < HintCluster > (mainData->Wavelength.size())),
    wlStart(mainData->Wavelength.first()), wlEnd(mainData->Wavelength.last())
{
    ui->setupUi(this);

    int i,j;
    PPSpectra.fill(QVector < double > (mData->Wavelength.size()), mData->ProbeTime.size());
    if (!mainData->WL10.isEmpty())
    {

        for (i=0;i<mainData->ProbeTime.size();i++)
            for (j=0;j<mainData->Wavelength.size();j++)
                PPSpectra[i][j]=-1000.0*log10(mainData->WL10[i][j]/mainData->WL00[i][j]);
    }
    else if (!mainData->WL01.isEmpty())
    {
        for (i=0;i<mainData->ProbeTime.size();i++)
            for (j=0;j<mainData->Wavelength.size();j++)
                PPSpectra[i][j]=-1000.0*log10(mainData->WL01[i][j]/mainData->WL00[i][j]);
    }
    else
    {
        for (i=0;i<mainData->ProbeTime.size();i++)
            for (j=0;j<mainData->Wavelength.size();j++)
                PPSpectra[i][j]=mainData->FSRS[i][j];
    }

    plot = new DispersionPlot(this,mainData,&HintMap,&Map,&PPSpectra);
    plot->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->plotLayout->addWidget(plot);

    ui->pointBox->setMinimum(1);
    ui->pointBox->setMaximum(mData->Wavelength.size()-1);
    ui->timeBox->setMinimum(mData->ProbeTime.first());
    ui->timeBox->setMaximum(mData->ProbeTime.last());

    // Turi buti bent dvi PT vertes:
    ui->startBox->setMinimum(mData->ProbeTime.first());
    ui->startBox->setMaximum(mData->ProbeTime.at(mData->ProbeTime.size()-2));
    ui->endBox->setMinimum(mData->ProbeTime.at(1));
    ui->endBox->setMaximum(mData->ProbeTime.last());
    ui->startBox->setValue(-1.0);
    ui->endBox->setValue(mData->ProbeTime.last());

    UpdateHintMap();
    connect(plot,SIGNAL(AttemptToFit(bool)),this,SLOT(PolynomialFit(bool)));
    connect(this,SIGNAL(axisScaleChanged(bool,double,double)),plot,SLOT(ReceiveAxisScale(bool,double,double)));

    setWindowTitle("Left-click to select points; right-click to remove selected points; hold Ctrl to select at a fixed spectral location.");
}

DispersionSuite::~DispersionSuite()
{
    delete ui;
}

void DispersionSuite::UpdateHintMap()
{
    int i,j,where;
    double k,d_min,d_max;
    int imin,imax;
    locate(&(mData->ProbeTime),ui->timeBox->value(),&where);
    if (where==mData->ProbeTime.size()-1) where--;
    for(j=0;j<mData->Wavelength.size();j++)
    {
        d_min=1.0e10,d_max=-1.0e10;
        for(i=0;i<=where;i++)
        {
            if(PPSpectra.at(i).at(j)>=d_max) {imax=i; d_max=PPSpectra.at(i).at(j);}
            if(PPSpectra.at(i).at(j)<=d_min) {imin=i; d_min=PPSpectra.at(i).at(j);}
        }
        HintMap[j].Max.Time=mData->ProbeTime.at(imax);
        HintMap[j].Max.Amp=d_max;
        HintMap[j].Min.Time=mData->ProbeTime.at(imin);
        HintMap[j].Min.Amp=d_min;
        for(i=0;i<=where;i++) if(PPSpectra.at(i).at(j)>=d_max/2){imax=i; break;}
        for(i=0;i<=where;i++) if(PPSpectra.at(i).at(j)<=d_min/2){imin=i; break;}
        if(imin==0) imin++;
        if(imax==0) imax++;
        k=(PPSpectra.at(imax).at(j)-PPSpectra.at(imax-1).at(j))/(mData->ProbeTime.at(imax)-mData->ProbeTime.at(imax-1));
        HintMap[j].MaxHalf.Time=(d_max/2-PPSpectra.at(imax).at(j))/k+mData->ProbeTime.at(imax);
        HintMap[j].MaxHalf.Amp=d_max/2;
        k=(PPSpectra.at(imin).at(j)-PPSpectra.at(imin-1).at(j))/(mData->ProbeTime.at(imin)-mData->ProbeTime.at(imin-1));
        HintMap[j].MinHalf.Time=(d_min/2-PPSpectra.at(imin).at(j))/k+mData->ProbeTime.at(imin);
        HintMap[j].MinHalf.Amp=d_min/2;
    }
}

void DispersionSuite::PolynomialFit(bool selPoly)
{
    int c1 = (selPoly ? 7 : 9),
        c2 = (selPoly ? 8 : 10);
    if ((selPoly && !Map.isEmpty()) || (!selPoly))
    {
        int order=ui->polyBox->value();
        if((selPoly && Map.size()>=order) || (!selPoly && HintCheck()))
        {
            double chi=0.0;
            QVector<double> tempx, tempy, xfit, tempxfit, yfit;
            if (selPoly)
            {
                QMap<int, DispPoint>::iterator it;
                for (it = Map.begin(); it != Map.end(); ++it)
                {
                    tempx << scaleWavelength(mData->Wavelength.at(it.key()));
                    tempy << it.value().Time;
                }
                //shell_sort_TwoArrays(&tempx,&tempy);
            }
            else
            {
                for (int i=0;i<mData->Wavelength.size();i+=ui->pointBox->value())
                {
                    tempx << scaleWavelength(mData->Wavelength.at(i));
                    tempy << (fabs(HintMap[i].Max.Amp) >= fabs(HintMap[i].Min.Amp) ? linToLog(HintMap[i].MaxHalf.Time,1.0) : linToLog(HintMap[i].MinHalf.Time,1.0));
                }
            }
            svdfit_wrapper(tempx,tempy,coeffs,order,&chi);
            ui->chiSqEdit->setText(QString::number(chi,'e',5));
            double dx=(wlEnd-wlStart)/(double(SPECTRAL_RESOLUTION-1));
            for(double di=0.0;di<SPECTRAL_RESOLUTION;di+=1.0)
            {
                xfit << mData->Wavelength.first()+di*dx;
                tempxfit << scaleWavelength(xfit.last());
            }
            if (ui->logBox->isChecked()) polynomialRecursionLog(&tempxfit,&coeffs,1.0,&yfit);
            else polynomialRecursionLin(&tempxfit,&coeffs,&yfit);
            plot->graph(c2)->setData(xfit,yfit);
        }
        else
        {
            plot->graph(c2)->clearData();
        }
    }
    else
    {
        plot->graph(c1)->clearData();
        plot->graph(c2)->clearData();
    }
    plot->replot();
}

inline double DispersionSuite::scaleWavelength(double val)
{
    return (val-wlStart)/(wlEnd-wlStart);
}

inline bool DispersionSuite::HintCheck()
{
    return ((ui->hintBox->isChecked()) && (int(floor(mData->Wavelength.size()/ui->pointBox->value())+1)>=ui->polyBox->value()));
}

void DispersionSuite::on_hintBox_toggled(bool checked)
{
    plot->SetHints(checked);
    ui->pointBox->setEnabled(checked);
    ui->timeBox->setEnabled(checked);
    ReplotHints(false);
}

void DispersionSuite::ReplotHints(bool updMap)
{
    if (updMap) UpdateHintMap();
    if (ui->hintBox->isChecked())
    {
        QVector < double > xs, ys;
        for (int i=0;i<mData->Wavelength.size();i+=ui->pointBox->value())
        {
            xs << mData->Wavelength.at(i);
            ys << (fabs(HintMap[i].Max.Amp) >= fabs(HintMap[i].Min.Amp) ? HintMap[i].MaxHalf.Time : HintMap[i].MinHalf.Time);
            if (ui->logBox->isChecked()) ys[ys.size()-1]=linToLog(ys[ys.size()-1],1.0);
        }
        plot->graph(9)->setData(xs,ys);
        PolynomialFit(false);
    }
    else
    {
        plot->graph(9)->clearData();
        plot->graph(10)->clearData();
        plot->replot();
    }
}

void DispersionSuite::on_pointBox_valueChanged(int arg1)
{
    ReplotHints();
}

void DispersionSuite::on_timeBox_valueChanged(double arg1)
{
    ReplotHints();
}

void DispersionSuite::on_polyBox_valueChanged(int arg1)
{
    PolynomialFit(true);
    PolynomialFit(false);
}

void DispersionSuite::on_cancelButton_clicked()
{
    close();
}

void DispersionSuite::on_clearButton_clicked()
{
    Map.clear();
    plot->graph(7)->clearData();
    plot->graph(8)->clearData();
    plot->replot();
}

void DispersionSuite::on_applyButton_clicked()
{
    bool isSel = Map.size()>=ui->polyBox->value(),
         isFit = HintCheck();
    if (isSel || isFit)
    {
        int ret=(isSel ? QMessageBox::Yes : QMessageBox::No);
        if (isSel && isFit)
        {
            QMessageBox msgBox;
            msgBox.setText("Which polynomial curve do you want to use?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            msgBox.button(QMessageBox::Yes)->setText("Selected points");
            msgBox.button(QMessageBox::No)->setText("Hint points");
            msgBox.setDefaultButton(QMessageBox::Yes);
            ret=msgBox.exec();
        }
        if (ret!=QMessageBox::Cancel)
        {
            PolynomialFit(ret==QMessageBox::Yes); // Galima sugudrauti, kad apskritai nereiketu kviesti, bet tai daug resursu nekainuoja.
            QVector<double> temp=mData->Wavelength;
            for(int i=0;i<mData->Wavelength.size();i++)
                mData->Wavelength[i]=scaleWavelength(mData->Wavelength.at(i));
            correctDispersion(mData,&coeffs);
            mData->Wavelength=temp;
            emit dispersionDone();
            close();
        }
    }
}

void DispersionSuite::on_saveButton_clicked()
{
    bool isSel = !Map.isEmpty(),
         isFit = HintCheck();
    if (isSel || isFit)
    {
        int ret=(isSel ? QMessageBox::Yes : QMessageBox::No);
        if (isSel && isFit)
        {
            QMessageBox msgBox;
            msgBox.setText("Which polynomial curve do you want to use?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            msgBox.button(QMessageBox::Yes)->setText("Selected points");
            msgBox.button(QMessageBox::No)->setText("Hint points");
            msgBox.setDefaultButton(QMessageBox::Yes);
            ret=msgBox.exec();
        }
        if (ret!=QMessageBox::Cancel)
        {
            QString fileName = QFileDialog::getSaveFileName(this, tr("Save Dispersion Curve"),mSettings->value("CURRENT_DIR").toString(),tr("Data file (*.dat);;Plain text file (*.txt);;All files (*.*)"));
            if (!fileName.isEmpty())
            {
                mSettings->setValue("CURRENT_DIR",GetCurrentDir(fileName));
                FILE *outFile;
                outFile = fopen(fileName.toLatin1(),"w+");
                if (ret==QMessageBox::Yes)
                {
                    QMap<int, DispPoint>::iterator it;
                    for (it = Map.begin(); it != Map.end(); ++it)
                        fprintf(outFile,"%18.10e\t%18.10e\n",mData->Wavelength.at(it.key()),it.value().Time);
                }
                else if (ret==QMessageBox::No)
                {
                    for(int i=0;i<mData->Wavelength.size();i+=ui->pointBox->value())
                        fprintf(outFile,"%18.10e\t%18.10e\n",mData->Wavelength.at(i),((fabs(HintMap[i].Max.Amp) >= fabs(HintMap[i].Min.Amp)) ? HintMap[i].MaxHalf.Time : HintMap[i].MinHalf.Time));
                }
                fclose(outFile);
            }
        }
    }
}

void DispersionSuite::on_logBox_toggled(bool checked)
{
    emit axisScaleChanged(checked,ui->startBox->value(),ui->endBox->value());
}

void DispersionSuite::on_startBox_valueChanged(double arg1)
{
    if(arg1>=ui->endBox->value())
    {
        ui->startBox->setValue(ui->endBox->value()-0.1);
    }
    else
    {
        emit axisScaleChanged(ui->logBox->isChecked(),arg1,ui->endBox->value());
    }
}

void DispersionSuite::on_endBox_valueChanged(double arg1)
{
    if(arg1<=ui->startBox->value())
    {
        ui->endBox->setValue(ui->startBox->value()+0.1);
    }
    else
    {
        emit axisScaleChanged(ui->logBox->isChecked(),ui->startBox->value(),arg1);
    }
}
