#ifndef GAUSSIANWEIGHTDIALOG_H
#define GAUSSIANWEIGHTDIALOG_H

#include <QDialog>
#include <QDebug>

namespace Ui {
class GaussianWeightDialog;
}

class GaussianWeightDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GaussianWeightDialog(QWidget *parent = 0, QVector< double > *weights=NULL);
    ~GaussianWeightDialog();

private slots:
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void on_okButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::GaussianWeightDialog *ui;
    QVector < double > *mWeights;
};

#endif // GAUSSIANWEIGHTDIALOG_H
