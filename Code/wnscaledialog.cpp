#include "wnscaledialog.h"
#include "ui_wnscaledialog.h"

wnScaleDialog::wnScaleDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::wnScaleDialog)
{
    ui->setupUi(this);
    setFixedSize(180,125);
    ui->slopeEdit->setFocus();
}

wnScaleDialog::~wnScaleDialog()
{
    delete ui;
}

void wnScaleDialog::on_buttonBox_accepted()
{
    bool ok1, ok2, ok3;
    double d1, d2, d3;
    d1=ui->slopeEdit->text().toDouble(&ok1);
    d2=ui->offsetEdit->text().toDouble(&ok2);
    d3=ui->v0Edit->text().toDouble(&ok3);
    if ((ok1)&&(ok2)&&(ok3))
    {
        emit sendCalibration(d1,d2,d3);
        close();
    }
}

void wnScaleDialog::on_buttonBox_rejected()
{
    close();
}
