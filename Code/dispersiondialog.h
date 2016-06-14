#ifndef DISPERSIONDIALOG_H
#define DISPERSIONDIALOG_H

#define DELTA 1.0

#include <QToolTip>
#include <QDialog>
#include <QDebug>

#include "qcustomplot.h"
#include "helpful_functions.h"
#include "svdfit.h"

namespace Ui {
class DispersionDialog;
}

class DispersionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DispersionDialog(QWidget *parent = 0, SpectrumData *mainPointer=NULL);
    ~DispersionDialog();

private slots:
    void on_hintBox_toggled(bool checked);
    void on_applyHintButton_clicked();
    void on_applyButton_clicked();
    void on_cancelButton_clicked();
    void on_everyBox_valueChanged(int arg1);
    void on_stopBox_valueChanged(double arg1);
    void on_polyBox_valueChanged(int arg1);
    void on_fromSpinBox_valueChanged(double arg1);
    void on_toSpinBox_valueChanged(double arg1);
    void on_clearButton_clicked();
    void clickedGraph(QMouseEvent *mouseEvent);
    void mouseHover(QMouseEvent *event);

private:
    Ui::DispersionDialog *ui;
    SpectrumData *mainData;
    QVector < QVector < double > > PP;
    QVector < double > dotX, dotY;
    QCPColorMap *cMap;
    QCPColorScale *cScale;

    void ReplotDots();
    void ReplotHint();
    void InitializeBoxes();
    void InitializeCarpet(bool alloc=false);
    void InitializeCurves();
    void CalculatePP();
};

#endif // DISPERSIONDIALOG_H
