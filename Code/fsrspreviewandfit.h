#ifndef FSRSPREVIEWANDFIT_H
#define FSRSPREVIEWANDFIT_H

#include <QDialog>
#include <QSettings>
#include <QFileDialog>

#include "customplot2d.h"
#include "gaussfitthread.h"
#include "fitpreview.h"

namespace Ui {
class FSRSPreviewAndFit;
}

class FSRSPreviewAndFit : public QDialog
{
    Q_OBJECT

public:
    explicit FSRSPreviewAndFit(QWidget *parent = 0, SpectrumData *mainData = NULL, QVector < QVector < double > > *mainFSRS = NULL, QSettings *mainSettings=NULL);
    ~FSRSPreviewAndFit();

private slots:
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void on_fitButton_clicked();
    void on_saveButton_clicked();
    void ResetAuxToZero();
    void ResetLabels();
    void GetParamsVector();
    void UpdateFit(int row, int col);
    void JumpToRow(int row, int col);
    void on_saveButton_2_clicked();
     void on_loadButton_clicked();

private:
    Ui::FSRSPreviewAndFit *ui;
    CustomPlot2D *myPlot;
    QVector < QVector < double > > GaussFit;
    QVector < double > TableParams;
    SpectrumData *mData;
    QVector < QVector < double > > *mFSRS;
    GaussFitThread *myThread;
    QSettings *mSettings;

};

#endif // FSRSPREVIEWANDFIT_H
