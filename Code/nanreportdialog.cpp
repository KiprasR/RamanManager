#include "nanreportdialog.h"
#include "ui_nanreportdialog.h"

NaNReportDialog::NaNReportDialog(QWidget *parent, QVector<QString> *NaNs) :
    QDialog(parent),
    ui(new Ui::NaNReportDialog)
{
    ui->setupUi(this);
    for(int i=0;i<NaNs->size();i++)
        ui->nanList->addItem(QString::number(i+1) + ". " + NaNs->at(i));
}

NaNReportDialog::~NaNReportDialog()
{
    delete ui;
}

void NaNReportDialog::on_OKButton_clicked()
{
    close();
}
