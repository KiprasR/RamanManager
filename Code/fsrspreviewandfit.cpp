#include "fsrspreviewandfit.h"
#include "ui_fsrspreviewandfit.h"

FSRSPreviewAndFit::FSRSPreviewAndFit(QWidget *parent, SpectrumData *mainData, QVector< QVector< double > > *mainFSRS, QSettings *mainSettings) :
    QDialog(parent),
    ui(new Ui::FSRSPreviewAndFit),
    mData(mainData), mFSRS(mainFSRS), myThread(new GaussFitThread(this)), mSettings(mainSettings)
{
    ui->setupUi(this);

    QVector < double > zeros;
    zeros.fill(0.0,mData->Wavelength.size());
    GaussFit.fill(zeros,mData->ProbeTime.size());

    myPlot=new CustomPlot2D(new QWidget(this),mData,mFSRS,&GaussFit);
    ui->verticalLayout_2->addWidget(myPlot);

    ui->peakTable->setRowCount(1);
    ui->peakTable->setColumnCount(3);
    ui->peakTable->setHorizontalHeaderLabels(QString::fromUtf8("ν0;Δν;FSRS?").split(";"));
    for (int i=0;i<3;i++)
        ui->peakTable->setColumnWidth(i,55);
    ui->peakTable->setItem(0,0,new QTableWidgetItem(" "));
    ui->peakTable->setItem(0,1,new QTableWidgetItem(" "));
    ui->peakTable->setItem(0,2,new QTableWidgetItem("1"));

    connect(this->myThread,SIGNAL(ProgressChanged(int)),ui->progressBar,SLOT(setValue(int)));
    connect(this->myThread,SIGNAL(SendInfo(QString)),ui->statusLabel,SLOT(setText(QString)));
    connect(this->myThread,SIGNAL(ThreadCancelled()),this,SLOT(ResetAuxToZero()));
    connect(this->myThread,SIGNAL(finished()),this,SLOT(ResetLabels()));
    connect(this->myThread,SIGNAL(SendParamsVector()),this,SLOT(GetParamsVector()));
    connect(this->myPlot,SIGNAL(SendMouseLocation(int,int)),this,SLOT(JumpToRow(int,int)));
}

FSRSPreviewAndFit::~FSRSPreviewAndFit()
{
    if (myThread->isRunning()) {
        myThread->Stop=true;
        while(myThread->isRunning()){};
    }
    // delete myPlot?
    delete ui;
}

void FSRSPreviewAndFit::JumpToRow(int row, int col)
{
    if(ui->paramsTable->rowCount()!=0)
        ui->paramsTable->selectRow(ui->paramsTable->rowCount()-1-row);
}

void FSRSPreviewAndFit::GetParamsVector()
{
    int i,j;
    QString HeadSTR="";
    ui->paramsTable->clear();
    ui->paramsTable->setColumnCount(TableParams.size()/mData->ProbeTime.size());
    ui->paramsTable->setRowCount(mData->ProbeTime.size());

    for(i=0;i<ui->paramsTable->columnCount()/3;i++)
        HeadSTR+=QString::fromUtf8("A%1;ν%2;Δν%3;").arg(QString::number(i+1)).arg(QString::number(i+1)).arg(QString::number(i+1));
    ui->paramsTable->setHorizontalHeaderLabels(HeadSTR.split(";"));
    HeadSTR="";
    for(i=0;i<mData->ProbeTime.size();i++)
        HeadSTR+=(QString::number(mData->ProbeTime[mData->ProbeTime.size()-1-i],'f',2)+";");
    HeadSTR.replace(".00",""); HeadSTR.replace(",00","");
    ui->paramsTable->setVerticalHeaderLabels(HeadSTR.split(";"));

    for (i=0;i<ui->paramsTable->columnCount();i++)
        for (j=0;j<ui->paramsTable->rowCount();j++)
            ui->paramsTable->setItem(j,i,new QTableWidgetItem(QString::number(TableParams[ui->paramsTable->columnCount()*j+i],'f',5)));

    ui->paramsTable->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    connect(ui->paramsTable,SIGNAL(cellChanged(int,int)),this,SLOT(UpdateFit(int,int)));

    FitPreview *ftPrev = new FitPreview(this,&(mData->ProbeTime),ui->paramsTable,mSettings);
    ftPrev->setAttribute(Qt::WA_DeleteOnClose,true);
    ftPrev->setModal(true);
    ftPrev->show();
}

void FSRSPreviewAndFit::UpdateFit(int row, int col)
{
    int i;
    double y;
    QVector < double > coeffs, dummy, wl;
    for (i=0;i<ui->paramsTable->columnCount();i++)
        coeffs <<  ui->paramsTable->item(row,i)->text().toDouble();
    for (i=0;i<coeffs.size()/3;i++)
    {
        coeffs[i*3+1]=(coeffs[i*3+1]-mData->Wavelength.first())/(mData->Wavelength.last()-mData->Wavelength.first());
        coeffs[i*3+2]=coeffs[i*3+2]/(mData->Wavelength.last()-mData->Wavelength.first());
    }
    for (i=0;i<mData->Wavelength.size();i++)
        wl << (mData->Wavelength[i]-mData->Wavelength.first())/(mData->Wavelength.last()-mData->Wavelength.first());
    dummy.resize(coeffs.size());
    for(i=0;i<mData->Wavelength.size();i++)
    {
        fgauss(wl[i],coeffs,y,dummy);
        GaussFit[mData->ProbeTime.size()-1-row][i]=y;
    }
    myPlot->ReplotCarpet(mData->ProbeTime[mData->ProbeTime.size()-1-row],mData->Wavelength[col]);
}

void FSRSPreviewAndFit::ResetAuxToZero()
{
    QVector < double > zeros(mData->Wavelength.size());
    zeros.fill(0.0);
    GaussFit.fill(zeros,mData->ProbeTime.size());
    ui->statusLabel->clear();
}

void FSRSPreviewAndFit::ResetLabels()
{
    ui->statusLabel->clear();
    ui->progressBar->setValue(100);
    ui->fitButton->setText("Fit");
}

void FSRSPreviewAndFit::on_addButton_clicked()
{
    ui->peakTable->setRowCount( ui->peakTable->rowCount()+1 );
    ui->peakTable->setItem(ui->peakTable->rowCount()-1,2,new QTableWidgetItem("1"));
}

void FSRSPreviewAndFit::on_removeButton_clicked()
{
    if (ui->peakTable->rowCount()>1)
    {
        QModelIndexList list=ui->peakTable->selectionModel()->selection().indexes();
        if(!list.isEmpty())
        {
            bool clearAll=list.size()==ui->peakTable->rowCount();
            int n=(clearAll ? list.size()-1 : list.size());
            for (int i=0,j=0;i<n;i++,j++)
                ui->peakTable->removeRow(list.at(i).row()-j);
            if (clearAll)
            {
                ui->peakTable->clearContents();
                ui->peakTable->setItem(0,2,new QTableWidgetItem("1"));
            }
        }
        else
            ui->peakTable->setRowCount( ui->peakTable->rowCount()-1 );
    }
}

void FSRSPreviewAndFit::on_fitButton_clicked()
{

    if (!myThread->isRunning())
    {
        int i,j,temp;
        bool ok,start=true;

        for (i=0;i<ui->peakTable->rowCount();i++)
        {
            for (j=0;j<ui->peakTable->columnCount()-1;j++)
            {
                ui->peakTable->item(i,j)->text().toDouble(&ok);
                start &= ok;
            }
            temp=ui->peakTable->item(i,j)->text().toInt(&ok);
            start &= (ok & ((temp==0)||(temp==1)));
        }

        if (start)
        {
            if (ui->paramsTable->rowCount()!=0)
                disconnect(ui->paramsTable,SIGNAL(cellChanged(int,int)),this,SLOT(UpdateFit(int,int)));

            TableParams.clear();
            ui->paramsTable->clear();  //?
            ui->paramsTable->setRowCount(0);
            ui->paramsTable->setColumnCount(0);

            QVector < double > params;

            for (i=0;i<ui->peakTable->rowCount();i++)
                params << 1.0
                       << ui->peakTable->item(i,0)->text().toDouble()
                       << ui->peakTable->item(i,1)->text().toDouble()
                       << ui->peakTable->item(i,2)->text().toInt();

            myThread->AssignVariables(mData,mFSRS,&params,&GaussFit,&TableParams,ui->tolEdit->text().toDouble());
            myThread->start();
            ui->fitButton->setText("Stop");
        }
    }
    else
    {
        myThread->Stop=true;
    }
}

void FSRSPreviewAndFit::on_saveButton_clicked()
{
    QString fileName=QFileDialog::getSaveFileName(this, tr("Save Spectrum Data"),mSettings->value("CURRENT_RAMAN_DIR").toString());
    if(!fileName.isEmpty())
    {
        mSettings->setValue("CURRENT_RAMAN_DIR",GetCurrentDir(fileName));

        int i,j;
        FILE *outFile;
        outFile=fopen(fileName.toLatin1(),"w+");
        fprintf(outFile,"%15i\t",0);
        for(i=0;i<mData->Wavelength.size();i++)
            fprintf(outFile,"%15e\t",mData->Wavelength[i]);
        fprintf(outFile,"\n");

        for(i=0;i<mData->ProbeTime.size();i++)
        {
            fprintf(outFile,"%15e\t",mData->ProbeTime[i]);
            for(j=0;j<mData->Wavelength.size();j++)
                fprintf(outFile,"%15e\t",GaussFit[i][j]);
            fprintf(outFile,"\n");
        }
        fclose(outFile);
    }
}

void FSRSPreviewAndFit::on_saveButton_2_clicked()
{
    if ((ui->peakTable->rowCount()!=0) && (ui->peakTable->columnCount()!=0))
    {
        QString fileName=QFileDialog::getSaveFileName(this, tr("Save Spectrum Data"),mSettings->value("CURRENT_RAMAN_DIR").toString());
        if(!fileName.isEmpty())
        {
            mSettings->setValue("CURRENT_RAMAN_DIR",GetCurrentDir(fileName));

            FILE *outFile;
            outFile=fopen(fileName.toLatin1(),"w+");
            for(int i=0;i<ui->peakTable->rowCount();i++)
                fprintf(outFile,"%8.8e\t%8.8e\t%8d\n",ui->peakTable->item(i,0)->text().toDouble(),
                                                      ui->peakTable->item(i,1)->text().toDouble(),
                                                      ui->peakTable->item(i,2)->text().toInt());
            fclose(outFile);
        }
    }
}

void FSRSPreviewAndFit::on_loadButton_clicked()
{
    QString fileName=QFileDialog::getOpenFileName(this,tr("Open Config File"),mSettings->value("CURRENT_RAMAN_DIR").toString());
    if(!fileName.isEmpty())
    {
        ui->peakTable->clearContents();
        ui->peakTable->setRowCount(0);
        ui->peakTable->setColumnCount(3);

        mSettings->setValue("CURRENT_RAMAN_DIR",GetCurrentDir(fileName));

        FILE *csFile;
        char *pch;
        char delimiters[]=" \t\n\v\f\r";
        char TempLine[MAXLINELENGTH];
        double db;
        int it,i=0;

        csFile=fopen(fileName.toLatin1(),"r");
        while(fgets(TempLine,MAXLINELENGTH,csFile)!=NULL)
        {
            ui->peakTable->setRowCount(i+1);
            pch=strtok(TempLine,delimiters);
            db=atof(pch);
            ui->peakTable->setItem(i,0,new QTableWidgetItem(QString::number(db,'f',2).replace(".00","").replace(",00","")));
            pch=strtok(NULL,delimiters);
            db=atof(pch);
            ui->peakTable->setItem(i,1,new QTableWidgetItem(QString::number(db,'f',2).replace(".00","").replace(",00","")));
            pch=strtok(NULL,delimiters);
            it=atoi(pch);
            ui->peakTable->setItem(i,2,new QTableWidgetItem(QString::number(it)));
            i++;
        }
        fclose(csFile);
    }
}
