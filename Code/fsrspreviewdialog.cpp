#include "fsrspreviewdialog.h"
#include "ui_fsrspreviewdialog.h"

FSRSPreviewDialog::FSRSPreviewDialog(QWidget *parent, SpectrumData *mainData, QVector< QVector< double > > *mainFSRS,
                                     QVector<QVector<double> > *auxFSRS) :
    QDialog(parent),
    ui(new Ui::FSRSPreviewDialog)
{
    ui->setupUi(this);
    ui->verticalLayout->addWidget(new CustomPlot2D(new QWidget(this),mainData,mainFSRS,auxFSRS));
    // Tikiuosi, jokiu klaidu nelieka...
}

FSRSPreviewDialog::~FSRSPreviewDialog()
{
    delete ui;
}
