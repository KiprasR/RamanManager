#ifndef DISPERSIONSUITE_H
#define DISPERSIONSUITE_H

#define SPECTRAL_RESOLUTION 256

#include <QDialog>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>

#include "svdfit.h"
#include "dispersionplot.h"
#include "helpful_functions.h"

namespace Ui {
class DispersionSuite;
}

class DispersionSuite : public QDialog
{
    Q_OBJECT

public:
    explicit DispersionSuite(QWidget *parent = 0, SpectrumData *mainData=NULL, QSettings *mainSettings=NULL);
    ~DispersionSuite();

signals:
    void dispersionDone();
    void axisScaleChanged(bool axLog, double axStart, double axEnd);

private slots:
    void on_hintBox_toggled(bool checked);
    void on_pointBox_valueChanged(int arg1);
    void on_timeBox_valueChanged(double arg1);
    void on_polyBox_valueChanged(int arg1);
    void PolynomialFit(bool selPoly);
    void on_cancelButton_clicked();
    void on_clearButton_clicked();
    void on_applyButton_clicked();
    void on_saveButton_clicked();
    void on_logBox_toggled(bool checked);
    void on_startBox_valueChanged(double arg1);
    void on_endBox_valueChanged(double arg1);

private:
    Ui::DispersionSuite *ui;
    DispersionPlot *plot;
    SpectrumData *mData;
    QSettings *mSettings;
    QVector < HintCluster > HintMap;
    double wlStart;
    double wlEnd;
    QVector<double> coeffs;
    QMap<int, DispPoint> Map;
    QVector < QVector < double > > PPSpectra;

    void UpdateHintMap();
    void ReplotHints(bool updMap=true);
    inline double scaleWavelength(double val);
    inline bool HintCheck();
};

#endif // DISPERSIONSUITE_H
