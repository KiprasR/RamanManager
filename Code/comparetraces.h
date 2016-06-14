#ifndef COMPARETRACES_H
#define COMPARETRACES_H

#include <QDialog>
#include "qcpsemilogaxisrect.h"
#include "helpful_functions.h"

namespace Ui {
class CompareTraces;
}

class CompareTraces : public QDialog
{
    Q_OBJECT
    
public:
    explicit CompareTraces(QWidget *parent=0, QVector< QVector< double > > *Data=NULL, const QVector< double > *Delays=NULL, int NScans=0, QVector< double > *Weights=NULL);
    ~CompareTraces();
    
private:
    Ui::CompareTraces *ui;
    QVector< QVector< double > > *FullData;
    const QVector < double > *LinDelays;
    QVector< double > *WeightsMain;
    QVector< double > LogDelays;
    QVector< double > LogTicks;
    QVector < QString > LogTickLabels;
    int NoOfPlots;

    void plot(int value);
    void rescaleLogAxes();

private slots:
    void on_wlSlider_valueChanged(int value);
    void on_tableWidget_cellChanged(int row, int column);
    void on_linButton_toggled(bool checked);
    void on_tableWidget_itemSelectionChanged();
};

#endif // COMPARETRACES_H
