#include "removedialog.h"
#include "ui_removedialog.h"

RemoveDialog::RemoveDialog(QWidget *parent, SpectrumData *mainData) :
    QDialog(parent),
    mData(mainData),
    ui(new Ui::RemoveDialog)
{
    ui->setupUi(this);
    setFixedSize(370,105);
    ui->waveEnd->setValue(mData->Wavelength.last());
    ui->probeEnd->setValue(mData->ProbeTime.last());
    ui->waveStart->setValue(mData->Wavelength.first());
    ui->probeStart->setValue(mData->ProbeTime.first());
}

RemoveDialog::~RemoveDialog()
{
    delete ui;
}

void RemoveDialog::on_waveBox_toggled(bool checked)
{
    ui->waveStart->setEnabled(checked);
    ui->waveEnd->setEnabled(checked);
}

void RemoveDialog::on_probeBox_toggled(bool checked)
{
    ui->probeStart->setEnabled(checked);
    ui->probeEnd->setEnabled(checked);
}

void RemoveDialog::on_applyButton_clicked()
{
    // Reiketu parasyti daugiau salygu arba sukurti protinga checkinga
    if ( (ui->waveBox->isChecked()  && (ui->waveStart->value()<ui->waveEnd->value())) ||
         (ui->probeBox->isChecked() && (ui->probeStart->value()<ui->probeEnd->value())) )
    {
        emit SendRemovableRegion(ui->waveBox->isChecked(), ui->waveStart->value(), ui->waveEnd->value(),
                                 ui->probeBox->isChecked(),ui->probeStart->value(),ui->probeEnd->value());
        close();
    }
}

void RemoveDialog::on_cancelButton_clicked()
{
    close();
}

void RemoveDialog::on_waveStart_editingFinished()
{
    if (ui->waveStart->value()>=mData->Wavelength.last())
        ui->waveStart->setValue(mData->Wavelength.first());
}

void RemoveDialog::on_waveEnd_editingFinished()
{
    if (ui->waveEnd->value()<=mData->Wavelength.first())
        ui->waveEnd->setValue(mData->Wavelength.last());
}

void RemoveDialog::on_probeStart_editingFinished()
{
    if (ui->probeStart->value()>=mData->ProbeTime.last())
        ui->probeStart->setValue(mData->ProbeTime.first());
}

void RemoveDialog::on_probeEnd_editingFinished()
{
    if (ui->probeEnd->value()<=mData->ProbeTime.first())
        ui->probeEnd->setValue(mData->ProbeTime.last());
}
