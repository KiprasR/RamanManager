#include "rescaleaxesdialog.h"
#include "ui_rescaleaxesdialog.h"

RescaleAxesDialog::RescaleAxesDialog(QWidget *parent, double *y00, double *y01) :
    QDialog(parent),
    ui(new Ui::RescaleAxesDialog),
    y0_ptr(y00), y1_ptr(y01)
{
    ui->setupUi(this);
    setFixedSize(295,75);

    ui->y0->setText(QString::number(*y00,'f',3).replace(".000","").replace(",000",""));
    ui->y1->setText(QString::number(*y01,'f',3).replace(".000","").replace(",000",""));
}

RescaleAxesDialog::~RescaleAxesDialog()
{
    delete ui;
}

void RescaleAxesDialog::on_okButton_clicked()
{
    bool   ok0,ok1;
    double yy0=ui->y0->text().toDouble(&ok0),
           yy1=ui->y1->text().toDouble(&ok1);
    if (ok0 && ok1)
    {
        if (yy0 < yy1)
        {
            (*y0_ptr)=yy0;
            (*y1_ptr)=yy1;
            close();
        }
    }
}

void RescaleAxesDialog::on_cancelButton_clicked()
{
    close();
}
