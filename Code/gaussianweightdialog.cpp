#include "gaussianweightdialog.h"
#include "ui_gaussianweightdialog.h"

GaussianWeightDialog::GaussianWeightDialog(QWidget *parent, QVector<double> *weights) :
    QDialog(parent),
    ui(new Ui::GaussianWeightDialog),
    mWeights(weights)
{
    ui->setupUi(this);
    setFixedSize(200,300);

    int i;
    ui->gaussTable->setColumnCount(2);
    ui->gaussTable->setHorizontalHeaderLabels(QString::fromUtf8("ν0;Δν").split(";"));
    for (i=0;i<2;i++) ui->gaussTable->setColumnWidth(i,70);

    if (!mWeights->isEmpty())
    {
        ui->gaussTable->setRowCount(mWeights->size()/2);
        for (i=0;i<mWeights->size();i+=2)
        {
            ui->gaussTable->setItem(0,i  ,new QTableWidgetItem(QString::number((*mWeights)[i  ],'f',3).replace(".000","").replace(",000","")));
            ui->gaussTable->setItem(0,i+1,new QTableWidgetItem(QString::number((*mWeights)[i+1],'f',3).replace(".000","").replace(",000","")));
        }
    }
}

GaussianWeightDialog::~GaussianWeightDialog()
{
    delete ui;
}

void GaussianWeightDialog::on_addButton_clicked()
{
    ui->gaussTable->setRowCount(ui->gaussTable->rowCount()+1);
}

void GaussianWeightDialog::on_removeButton_clicked()
{
    if(ui->gaussTable->rowCount()!=0) {
        QVector <int> rowList;
        for(int i=0;i<ui->gaussTable->rowCount();i++) {
            if((ui->gaussTable->item(i,0)!=0) && (ui->gaussTable->item(i,1)!=0)) {
                if(ui->gaussTable->item(i,0)->isSelected() || ui->gaussTable->item(i,1)->isSelected()) {
                    rowList << i;
                }
            }
            else {
                rowList << i;
            }
        }
        qDebug() << rowList;
        qSort(rowList);
        for(int i=0;i<rowList.size();i++)
            ui->gaussTable->removeRow(rowList.at(i)-i);
    }
}

void GaussianWeightDialog::on_okButton_clicked()
{
    if (ui->gaussTable->rowCount()==0) {
        mWeights->clear();
        close();
    }
    else {
        int i,j;
        double temp=0.0;
        bool ok, send=true;
        for (i=0;i<ui->gaussTable->rowCount();i++) {
            for (j=0;j<ui->gaussTable->columnCount();j++) {
                if(ui->gaussTable->item(i,j)!=0) {
                    temp=ui->gaussTable->item(i,j)->text().toDouble(&ok);
                    if(!ok) {
                        send=false;
                        break;
                    }
                }
                else {
                    send=false;
                    break;
                }
            }
            if (!send) break;
        }
        if (send) {
            mWeights->clear();
            for (i=0;i<ui->gaussTable->rowCount();i++) {
                mWeights->append(temp=ui->gaussTable->item(i,0)->text().toDouble());
                mWeights->append(temp=ui->gaussTable->item(i,1)->text().toDouble());
            }
            close();
        }
    }
}

void GaussianWeightDialog::on_cancelButton_clicked()
{
    close();
}
